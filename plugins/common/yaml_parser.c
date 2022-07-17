/* Copyright 2022 Bas van den Berg
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "common/yaml_parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <stdarg.h>
#include <assert.h>

#define YAML_MAX_TEXT 256
//#define YAML_PRINT_TOKENS
//#define YAML_PRINT_TRACE

#define ANSI_YELLOW   "\033[0;33m"
#define ANSI_MAGENTA  "\033[0;35m"
#define ANSI_CYAN     "\033[0;36m"
#define ANSI_NORMAL   "\033[0m"

#ifdef YAML_PRINT_TRACE
#define TRACE printf(ANSI_CYAN "%s: %s(%d) " ANSI_YELLOW "%s" ANSI_NORMAL "\n", loc2str(&p->token.loc), __func__, p->cur_indent, token2str(&p->token));
#else
#define TRACE
#endif

/*
    - remove DIRTY HACK
    - check duplicate keys in map (need locations)
    - scalars: merge plain, quoted, and double-quoted scalars (just produce TOKEN_SCALAR?)
    - strings: add all supported chars: : +
    - escaped characters in scalars
    - support scalar > + | (2.3)
        literal block scalar |
        folded block scalar >
    - flow collection, eg: [ one, two ] and { one: 1, two: 2 }
    - better error messages: add more info
*/

// ----------------------------------------------------------------------------

typedef struct {
    uint32_t line;
    uint32_t column;
} Location;

static void loc_init(Location* loc) {
    loc->line = 1;
    loc->column = 1;
}

static const char* loc2str(const Location* loc) {
    static char msg[32];
    sprintf(msg, "at line %u:%u", loc->line, loc->column);
    return msg;
}

// ----------------------------------------------------------------------------

typedef enum {
   NODE_UNKNOWN,
   NODE_SCALAR,         // can also be for SEQUENCE entries
   NODE_MAP,            // has no value itself, but children
   NODE_SEQUENCE,       // top node, children can be other type
} NodeKind;

// NOTE: keep in sync with NodeKind
static const char* node_names[] = {
    "UNK",
    "SCA",
    "MAP",
    "SEQ",
};

struct YamlNode_ {
    // TODO merge nodeKind (3 bits) with next
    NodeKind kind;
    uint32_t next_idx;  // node index
    uint32_t name_idx;  // text index
    union {
        uint32_t text_idx;  // text index, for SCALAR
        uint32_t child_idx; // node index, for MAP + SEQUENCE
    };
};

// ----------------------------------------------------------------------------

typedef enum {
    TOKEN_NONE,
    TOKEN_PLAIN_SCALAR,
    TOKEN_SINGLE_QUOTED_SCALAR,
    TOKEN_DOUBLE_QUOTED_SCALAR,
    TOKEN_COLON,
    TOKEN_DASH,
    TOKEN_INDENT,   // only at start of line
    TOKEN_DEDENT,   // only at start of line
    TOKEN_DOC_START,
    TOKEN_DOC_END,
    TOKEN_DIRECTIVE,
    TOKEN_EOF,
    TOKEN_ERROR,
} TokenKind;

// NOTE: must be in sync with TokenKind
static const char* token_names[] = {
    "none",
    "scalar",
    "'scalar'",
    "\"scalar\"",
    ":",
    "-",
    "INDENT",
    "DEDENT",
    "---",
    "...",
    "%",
    "EOF",
    "ERROR",
};

typedef struct {
    Location loc;
    TokenKind kind;
    bool same_line;
    union {
        const char* error_msg;   // ERROR
        uint32_t text_idx;   // SCALAR, DIRECTIVE
        int32_t indent;     // INDENT / DEDENT
    };
} Token;

static const char* token2str(const Token* t) {
    return token_names[t->kind];
}

// ----------------------------------------------------------------------------

#define MAX_DEPTH 8

typedef struct {
    int32_t indent; // -1 for root node
    YamlNode* node;
    YamlNode* last_child;
} StackLevel;

typedef struct {
    char* text;
    uint32_t text_size;
    char* text_cur;

    YamlNode* nodes;
    uint32_t nodes_count;
    YamlNode* nodes_cur;

    // needed for node resize
    StackLevel* stack;
} YamlData;

static void yaml_data_init(YamlData* d, uint32_t text_size, uint32_t nodes_count, StackLevel* stack) {
    // TEMP just create a single large buffer
    d->text = (char*)malloc(text_size);
    d->text_size = text_size;
    d->text_cur = d->text + 1; // reserve first byte for empty text
    d->text[0] = 0;

    d->nodes = (YamlNode*)malloc(nodes_count * sizeof(YamlNode));
    d->nodes_count = nodes_count;
    d->nodes_cur = &d->nodes[1]; // reserve first node
    memset(&d->nodes[0], 0, sizeof(YamlNode));

    d->stack = stack;
}

static void yaml_data_destroy(YamlData* d) {
    free(d->text);
    free(d->nodes);
}

static inline uint32_t node2idx(const YamlData* d, const YamlNode* n) {
    return (n - d->nodes);
}

static inline YamlNode* idx2node(const YamlData* d, uint32_t idx) {
    return &d->nodes[idx];
}

static void node_dump(const YamlData* d, const YamlNode* n, int32_t indent) {
    // TEMP crude way
    for (int32_t i=0; i<indent; i++) printf("   ");

    printf("[%2u] %s", node2idx(d, n), node_names[n->kind]);
    printf("  name: ");
    if (n->name_idx) printf("%s", &d->text[n->name_idx]);
    else printf("-");
    printf("  value: ");
    switch (n->kind) {
    case NODE_UNKNOWN:
        printf("-\n");
        break;
    case NODE_SCALAR:
        if (n->text_idx) printf("%s", &d->text[n->text_idx]);
        printf("\n");
        break;
    case NODE_MAP:  // fallthrough
    case NODE_SEQUENCE:
        printf("-\n");
        if (n->child_idx) node_dump(d, idx2node(d, n->child_idx), indent+1);
        break;
    }

    if (n->next_idx) {
        node_dump(d, idx2node(d, n->next_idx), indent);
    }
}

static void yaml_data_dump(const YamlData* d, bool verbose) {
    uint32_t node_count = (uint32_t)(d->nodes_cur - d->nodes);
    if (verbose) {
        printf("Text %ld/%u\n", (d->text_cur - d->text), d->text_size);
        const char* cp = d->text + 1;
        while (cp < d->text_cur) {
            uint32_t len = strlen(cp);
            uint32_t offset = (uint32_t)(cp - d->text);
            printf("  [%3u] %s\n", offset, cp);
            cp += len + 1;
        }

        printf("Nodes %u/%u\n", node_count, d->nodes_count);
        for (uint32_t i=1; i<node_count; i++) {
            const YamlNode* n = &d->nodes[i];
            printf("  [%2u] %s  next %3u  name %3u  value/child %3u\n", i, node_names[n->kind], n->next_idx, n->name_idx, n->text_idx);
        }
    }

    if (node_count > 1) node_dump(d, &d->nodes[1], 0);
}

static void yaml_data_resize_text(YamlData* d) {
    uint32_t idx = (uint32_t)(d->text_cur - d->text);

    d->text_size *= 2;
    char* text2 = (char*)malloc(d->text_size);
    memcpy(text2, d->text, idx + 1); // also copy NULL termination
    free(d->text);
    d->text = text2;
    d->text_cur = &d->text[idx];
}

static void yaml_data_resize_nodes(YamlData* d) {
    uint32_t idx = (uint32_t)(d->nodes_cur - d->nodes);

    d->nodes_count *= 2;
    YamlNode* nodes2 = (YamlNode*)malloc(d->nodes_count * sizeof(YamlNode));
    memcpy(nodes2, d->nodes, idx * sizeof(YamlNode));

    // fix-up stack pointers
    for (unsigned i=0; i<MAX_DEPTH; i++) {
        StackLevel* sl = &d->stack[i];
        if (sl->node) {
            uint32_t node_idx = (uint32_t)(sl->node - d->nodes);
            sl->node = &nodes2[node_idx];
        }
        if (sl->last_child) {
            uint32_t last_child_idx = (uint32_t)(sl->last_child - d->nodes);
            sl->last_child = &nodes2[last_child_idx];
        }
    }

    free(d->nodes);
    d->nodes = nodes2;
    d->nodes_cur = &d->nodes[idx];
}

static uint32_t yaml_data_add_text(YamlData* d, const char* text, uint32_t len) {
    uint32_t idx = (uint32_t)(d->text_cur - d->text);
    while (idx + len + 1 >= d->text_size) yaml_data_resize_text(d);

    memcpy(d->text_cur, text, len);
    d->text_cur[len] = 0;
    d->text_cur += len+1; // add 0-terminator
    return idx;
}

static YamlNode* yaml_data_add_node(YamlData* d, NodeKind kind, uint32_t name_idx) {
    uint32_t idx = (uint32_t)(d->nodes_cur - d->nodes);
    if (idx >= d->nodes_count -1) yaml_data_resize_nodes(d);

    YamlNode* result = d->nodes_cur;
    d->nodes_cur++;
    result->kind = kind;
    result->next_idx = 0;
    result->name_idx = name_idx;
    result->child_idx = 0;
    return result;
}

// ----------------------------------------------------------------------------

typedef struct {
    const char* cur;
    Location loc;
    const char* input_start;
    char* error_msg;
    int cur_indent;
    bool same_line;
    YamlData* data;

    Token next;
} YamlTokenizer;

static void yaml_tok_init(YamlTokenizer* t, const char* input, char* error_msg, YamlData* data) {
    memset(t, 0, sizeof(YamlTokenizer));
    t->input_start = input;
    t->cur = input;
    loc_init(&t->loc);
    t->error_msg = error_msg;
    t->data = data;
    t->next.kind = TOKEN_NONE;
}

static void lex_comment(YamlTokenizer* t) {
    const char* start = t->cur;
    t->cur++;
    while (1) {
        switch (*t->cur) {
        case 0:     // fallthrough
        case '\r':  // fallthrough
        case '\n':
            t->loc.column += (t->cur - start);
            return;
        default:
            t->cur++;
            break;
        }
    }
}

static void lex_directive(YamlTokenizer* t, Token* result) {
    t->cur++;
    const char* start = t->cur;
    uint32_t count;
    while (1) {
        switch (*t->cur) {
        case 0:     // fallthrough
        case '\r':  // fallthrough
        case '\n':
            goto out;
        default:
            t->cur++;
            break;
        }
    }
out:
    count = t->cur - start;
    result->loc = t->loc;
    result->kind = TOKEN_DIRECTIVE;
    result->text_idx = yaml_data_add_text(t->data, start, count);
    t->loc.column += count + 1;
}

static bool is_string(char c) {
    if (isalpha(c) || isdigit(c)
        || c == '_' || c == '-'
        || c == '.' || c == '/'
        || c == '~') {
        return true;
    }
    return false;
}

static void lex_string(YamlTokenizer* t, Token* result) {
    // NOTE: multiple words with only space in between are merged
    const char* start = t->cur;
    t->cur++;
    while (1) {
        char c = *t->cur;
        if (is_string(c)) {
            t->cur++;
            continue;
        }
        if (c == ' ' && is_string(t->cur[1])) {
            t->cur++;
            t->cur++;
            continue;
        }
        break;
    }
    uint32_t count = t->cur - start;
    result->loc = t->loc;
    result->kind = TOKEN_PLAIN_SCALAR;
    result->text_idx = yaml_data_add_text(t->data, start, count);
    t->loc.column += count;
}

static bool lex_indent(YamlTokenizer* t, Token* result) {
    const char* start = t->cur;
    while (*t->cur == ' ') t->cur++;

    int32_t indent = t->cur - start;
    result->loc = t->loc;
    t->loc.column += indent;
    if (t->cur_indent == indent) {
        return false;
    }
    if (t->cur_indent > indent) {
        result->kind = TOKEN_DEDENT;
    } else {
        result->kind = TOKEN_INDENT;
    }
    result->indent = indent;
    t->cur_indent = indent;
    //t->same_line = false;
    return true;
}

static void token_error(YamlTokenizer* t, Token* result) {
    result->loc = t->loc;
    result->kind = TOKEN_ERROR;
    result->error_msg = t->error_msg;
}

static void lex_quoted_string(YamlTokenizer* t, Token* result, char delim) {
    t->cur++;
    const char* start = t->cur;
    uint32_t count;
    while (1) {
        switch (*t->cur) {
        case 0:
        case '\r':
        case '\n':
            t->loc.column += (t->cur - start);
            sprintf(t->error_msg, "unterminated string %s", loc2str(&t->loc));
            token_error(t, result);
            return;
        default:
            if (*t->cur == delim) goto out;
            t->cur++;
            break;
        }
    }
out:
    count = t->cur - start;
    t->cur++;   // skip terminating delimiter
    t->error_msg[count] = 0;
    result->loc = t->loc;
    result->kind = (delim == '"') ? TOKEN_DOUBLE_QUOTED_SCALAR : TOKEN_SINGLE_QUOTED_SCALAR;
    result->text_idx = yaml_data_add_text(t->data, start, count);
    t->loc.column += count + 2;  // add quotes
}

static void yaml_tok_lex(YamlTokenizer* t, Token* result) {
    if (t->next.kind != TOKEN_NONE) {
        memcpy(result, &t->next, sizeof(Token));
        t->next.kind = TOKEN_NONE;
        return;
    }

    //printf("lex %s: %u\n", loc2str(&t->loc), t->same_line);
    result->same_line = t->same_line;
    t->same_line = true;

    result->text_idx = 0;
    while (1) {
        // dont emit DEDENT for empty lines
        if (t->loc.column == 1 && t->cur_indent && *t->cur != ' ' && *t->cur != '\r' && *t->cur != '\n') {
            result->loc = t->loc;
            result->kind = TOKEN_DEDENT;
            result->indent = 0;
            t->cur_indent = 0;
            t->same_line = false;
            return;
        }

        switch (*t->cur) {
        case 0:
            result->loc = t->loc;
            result->kind = TOKEN_EOF;
            return;
        case '\t':
            sprintf(t->error_msg, "file contains tab character %s", loc2str(&t->loc));
            token_error(t, result);
            return;
        case '\r':
            t->cur++;
            if (*t->cur != '\n') {
                sprintf(t->error_msg, "unexpected char 0x%02x %s", *t->cur, loc2str(&t->loc));
                token_error(t, result);
                return;
            } // else fallthrough
        case '\n':
            t->cur++;
            t->loc.line++;
            t->loc.column = 1;
            t->same_line = true;
            result->same_line = false;
            break;
        case ' ':
            if (t->loc.column == 1) {
                if (lex_indent(t, result)) return;
                break;
            }
            t->cur++;
            t->loc.column++;
            break;
        case '"':
            lex_quoted_string(t, result, '"');
            return;
        case '#':
            lex_comment(t);
            break;
        case '%':
            lex_directive(t, result);
            return;
        case '\'':
            lex_quoted_string(t, result, '\'');
            return;
        case '-':
            // if followed by SPACE or newline, it is a dash
            if (t->cur[1] == ' ' || t->cur[1] == '\r' || t->cur[1] == '\n') {
                t->cur++;
                result->loc = t->loc;
                result->kind = TOKEN_DASH;
                t->loc.column++;
                return;
            }

            if (t->loc.column == 1 && t->cur[1] == '-' && t->cur[2] == '-') {
                t->cur += 3;
                result->loc = t->loc;
                result->kind = TOKEN_DOC_START;
                t->loc.column += 3;
                return;
            }
            lex_string(t, result);
            return;
        case '.':
            // can be DOCUMENT_END at start, otherwise scalar
            if (t->loc.column == 1 && t->cur[1] == '.' && t->cur[2] == '.') {
                result->loc = t->loc;
                result->kind = TOKEN_DOC_END;
                t->cur += 3;
                t->loc.column += 3;
                return;
            }
            lex_string(t, result);
            return;
        case ':':
            t->cur++;
            result->loc = t->loc;
            result->kind = TOKEN_COLON;
            t->loc.column++;
            return;
        default:
            if (is_string(*t->cur)) {
                lex_string(t, result);
                return;
            }
            sprintf(t->error_msg, "unhandled char 0x%02x (%c) %s",
                *t->cur, isprint(*t->cur) ? *t->cur : ' ',loc2str(&t->loc));
            token_error(t, result);
            return;
        }
    }
}

static Token* yaml_tok_next(YamlTokenizer* t) {
    if (t->next.kind == TOKEN_NONE) {
        yaml_tok_lex(t, &t->next);
    }
    return &t->next;
}

// ----------------------------------------------------------------------------

struct YamlParser_ {
    const char* input;
    uint32_t size;
    char error_msg[YAML_MAX_TEXT];

    YamlTokenizer tokenizer;
    Token token;
    jmp_buf jump_err;

    int cur_indent;
    bool doc_started;   // only used for first document (implict start)
    bool in_document;

    StackLevel stack[MAX_DEPTH];
    uint32_t stack_size;     // number of items on stack

    YamlData data;
};

YamlParser* yaml_create(const char* input, uint32_t size) {
    YamlParser* p = (YamlParser*)calloc(1, sizeof(YamlParser));
    p->input = input;
    p->size = size;
    p->in_document = true;
    yaml_data_init(&p->data, 1024, 32, p->stack);
    return p;
}

void yaml_destroy(YamlParser* p) {
    yaml_data_destroy(&p->data);
    free(p);
}

#ifdef YAML_PRINT_TRACE
static void stack_dump(const YamlParser* p) {
    printf(ANSI_YELLOW "STACK %u  cur_indent %d" ANSI_NORMAL "\n", p->stack_size, p->cur_indent);
    for (unsigned i=0; i<p->stack_size; i++) {
        const StackLevel* lvl = &p->stack[i];
        uint32_t n_idx = lvl->node ? node2idx(&p->data, lvl->node) : 0;
        uint32_t c_idx = lvl->last_child ? node2idx(&p->data, lvl->last_child) : 0;
        printf(ANSI_YELLOW "  [%u] indent %2d  node %u  last_child %u" ANSI_NORMAL "\n", i, lvl->indent, n_idx, c_idx);
    }
}
#endif

#define PRINTF_FORMAT_CHECK(format_idx, args_idx) __attribute__ ((__format__(printf, format_idx, args_idx)))
static void yaml_error(YamlParser* p, const char* format, ...) PRINTF_FORMAT_CHECK(2, 3);
static void yaml_error(YamlParser* p, const char* format, ...) {
    va_list args;
    va_start(args, format);
    char* cp = p->error_msg;
    cp += vsnprintf(cp, YAML_MAX_TEXT-1, format, args);
    va_end(args);
    sprintf(cp, " %s", loc2str(&p->token.loc));
    longjmp(p->jump_err, 1);
}

#ifdef YAML_PRINT_TOKENS
static const char* value2str(const YamlData* d, const Token* t) {
    static char value[16];
    if (t->kind == TOKEN_INDENT || t->kind == TOKEN_DEDENT) {
        sprintf(value, "(%d)", t->indent);
        return value;
    }
    if (t->indent == 0) return "";
    return &d->text[t->text_idx];
}

static void print_token(const YamlData* d, const Token* t) {
    printf(ANSI_MAGENTA "%s  %s %d " ANSI_YELLOW "%s" ANSI_NORMAL "\n", token2str(t), loc2str(&t->loc), t->same_line, value2str(d, t));
}
#endif

static Location consumeToken(YamlParser* p) {
    Location prev = p->token.loc;
    yaml_tok_lex(&p->tokenizer, &p->token);
#ifdef YAML_PRINT_TOKENS
    print_token(&p->data, &p->token);
#endif
    if (p->token.kind == TOKEN_ERROR) longjmp(p->jump_err, 1);
    return prev;
}

static void expectAndConsume(YamlParser* p, TokenKind kind) {
    if (p->token.kind != kind) {
        yaml_error(p, "expected '%s', got '%s'", token_names[kind], token2str(&p->token));
    }
    consumeToken(p);
}

static void yaml_add_scalar_value(YamlParser* p, uint32_t value_idx) {
    StackLevel* top = &p->stack[p->stack_size-1];
    YamlNode* n = top->node;
    assert(n);
    if (n->kind != NODE_UNKNOWN) {
        yaml_error(p, "%s() cannot add scalar to node", __func__);
    }
    n->kind = NODE_SCALAR;
    n->text_idx = value_idx;
}

static void yaml_pop(YamlParser* p) {
#ifdef YAML_PRINT_TRACE
    printf(ANSI_CYAN "POP idx %u indent %d" ANSI_NORMAL "\n", p->stack_size, p->cur_indent);
#endif
    int32_t indent = p->cur_indent;
    while (1) {
        StackLevel* top = &p->stack[p->stack_size-1];
        if (top->indent <= indent) break;

        if (p->stack_size >= 1) {
            StackLevel* prev = &p->stack[p->stack_size-2];
            prev->last_child = top->node;
        }
        if (top->node->kind == NODE_UNKNOWN) top->node->kind = NODE_SCALAR;

        top->indent = 0;
        top->node = NULL;
        top->last_child = NULL;
        p->stack_size--;
    }
#ifdef YAML_PRINT_TRACE
    yaml_dump(p, true);
    stack_dump(p);
#endif
}

static void yaml_push_root(YamlParser* p) {
    YamlNode* root = yaml_data_add_node(&p->data, NODE_UNKNOWN, 0);
    StackLevel* top = &p->stack[0];
    if (p->stack_size) {
        top->node->next_idx = node2idx(&p->data, root);
    }
    top->node = root;
    top->indent = -1;
    top->last_child = NULL;
    p->stack_size = 1;
#ifdef YAML_PRINT_TRACE
    stack_dump(p);
#endif
}

static void yaml_push_node(YamlParser* p, YamlNode* n, NodeKind parent_kind, int indent) {
    assert(p->stack_size);
    uint32_t n_idx = node2idx(&p->data, n);
    StackLevel* top = &p->stack[p->stack_size-1];

#ifdef YAML_PRINT_TRACE
    printf(ANSI_CYAN "PUSH node %u, idx %u indent %d(cur %d)  kind %s" ANSI_NORMAL "\n", n_idx, p->stack_size, indent, p->cur_indent, node_names[parent_kind]);
#endif

    if (indent < top->indent) { // can happen at end of sequence (1 lower)
        assert(indent + 1 == top->indent);
        yaml_pop(p);
        top = &p->stack[p->stack_size-1];
    }

    if (top->indent == indent) { // same level
        if (top->node) {
            // close old node as SCALAR with empty data
            if (top->node->kind == NODE_UNKNOWN) top->node->kind = NODE_SCALAR;
            top->node->next_idx = n_idx;
        }
        top->last_child = NULL;
    } else {
        assert(p->stack_size + 1 < MAX_DEPTH);
        assert(indent > top->indent);
        YamlNode* parent = top->node;

        if (parent->kind == NODE_UNKNOWN) {
            // just assign it
            if (parent_kind == NODE_UNKNOWN) parent_kind = NODE_MAP;
            parent->kind = parent_kind;
        }
        if (top->last_child) {
            top->last_child->next_idx = n_idx;
        } else {
            assert(parent->child_idx == 0);
            parent->child_idx = n_idx;
        }
        top->last_child = n;    // set last_child on current level before adding level
        p->stack_size++;
        top = &p->stack[p->stack_size-1];
    }
    top->indent = indent;
    top->node = n;

    // add child info to parent
    StackLevel* prev = &p->stack[p->stack_size-2];
    YamlNode* parent = prev->node;

    if (parent->kind != parent_kind && !(parent->kind == NODE_MAP && parent_kind == NODE_UNKNOWN)) {
        if (parent->kind == NODE_SEQUENCE) {
            yaml_error(p, "invalid scalar after sequence");
        } else {
            yaml_error(p, "invalid scalar after %s", node_names[parent->kind]);
        }
    }
#ifdef YAML_PRINT_TRACE
    yaml_dump(p, true);
    stack_dump(p);
#endif
}

static void parse_node(YamlParser* p);
static void parse_node_or_value(YamlParser* p);

static void parse_value(YamlParser* p) {
    TRACE
    switch (p->token.kind) {
    case TOKEN_PLAIN_SCALAR:      // fallthrough
    case TOKEN_SINGLE_QUOTED_SCALAR: // fallthrough
    case TOKEN_DOUBLE_QUOTED_SCALAR:
        if (p->token.same_line) {
            yaml_add_scalar_value(p, p->token.text_idx);
            consumeToken(p);
        } else {
            assert(0); // valid??
        }
        return;
    case TOKEN_DASH: {
        consumeToken(p);
        YamlNode* n = yaml_data_add_node(&p->data, NODE_UNKNOWN, 0);
        yaml_push_node(p, n, NODE_SEQUENCE, p->cur_indent + 1);
        parse_node_or_value(p);
        return;
    }
    case TOKEN_INDENT:
        p->cur_indent = p->token.indent;
        consumeToken(p);
        parse_node(p);
        return;
    case TOKEN_DEDENT:
        p->cur_indent = p->token.indent;
        consumeToken(p);
        yaml_pop(p);
        return;
    case TOKEN_DOC_START:   // fallthrough
    case TOKEN_DOC_END:
        return;
    case TOKEN_EOF:
        yaml_add_scalar_value(p, 0);
        return;
    default:
        yaml_error(p, "%s() unhandled token '%s'", __func__, token2str(&p->token));
        break;
    }
}

static void parse_node(YamlParser* p) {
    TRACE

    switch (p->token.kind) {
    case TOKEN_PLAIN_SCALAR:  // fallthrough
    case TOKEN_SINGLE_QUOTED_SCALAR: // fallthrough
    case TOKEN_DOUBLE_QUOTED_SCALAR: {
        YamlNode* n = yaml_data_add_node(&p->data, NODE_UNKNOWN, p->token.text_idx);
        yaml_push_node(p, n, NODE_UNKNOWN, p->cur_indent);
        consumeToken(p);
        expectAndConsume(p, TOKEN_COLON);
        parse_value(p);
        break;
    }
    case TOKEN_DASH: {
        consumeToken(p);
        YamlNode* n = yaml_data_add_node(&p->data, NODE_UNKNOWN, 0);
        yaml_push_node(p, n, NODE_SEQUENCE, p->cur_indent + 1);
        parse_node_or_value(p);
        break;
    }
    case TOKEN_INDENT:
        p->cur_indent = p->token.indent;
        consumeToken(p);
        break;
    case TOKEN_DEDENT:
        p->cur_indent = p->token.indent;
        consumeToken(p);
        yaml_pop(p);
        break;
    case TOKEN_DOC_START: // fallthrough
    case TOKEN_DOC_END:
        break;
    default:
        yaml_error(p, "%s() unhandled token '%s'", __func__, token2str(&p->token));
        break;
    }
}

static void parse_node_or_value(YamlParser* p) {
    TRACE

    switch (p->token.kind) {
    case TOKEN_PLAIN_SCALAR:  // fallthrough
    case TOKEN_SINGLE_QUOTED_SCALAR: // fallthrough
    case TOKEN_DOUBLE_QUOTED_SCALAR: {
        Token* next = yaml_tok_next(&p->tokenizer);
        if (next->kind == TOKEN_COLON) {
            // NOTE: this doesn't work, because tokenizer doesn't know (and doesn't give DEDENT)
            p->cur_indent += 2; // one for dash, one for node
            // TEMP DIRTY HACK, how to do properly?
            p->tokenizer.cur_indent += 2;
            parse_node(p);
            return;
        }
        break;
    }
    default:
        break;
    }
    parse_value(p);
}

static void yaml_doc_start(YamlParser* p) {
    TRACE
    yaml_push_root(p);
    p->doc_started = true;
    p->in_document = true;
}

static void yaml_doc_end(YamlParser* p) {
    TRACE
    p->cur_indent = -1;
    if (p->stack_size == 1 && p->stack[0].node->kind == NODE_UNKNOWN) {
        p->stack[0].node->kind = NODE_MAP;
    }
    yaml_pop(p);
    p->cur_indent = 0;
    p->in_document = false;
}

static void parse_document(YamlParser* p) {
    TRACE

    while (1) {
        switch (p->token.kind) {
        case TOKEN_DOC_START:
            consumeToken(p);
            if (p->doc_started) {
                yaml_doc_end(p);
            }
            yaml_doc_start(p);
            break;
        case TOKEN_DOC_END:
            if (!p->doc_started || !p->in_document) {
                yaml_error(p, "END document without start");
            }
            consumeToken(p);
            yaml_doc_end(p);
            return;
        case TOKEN_DIRECTIVE:
            // ignore
            consumeToken(p);
            break;
        case TOKEN_EOF:
            return;
        default:
            if (!p->doc_started) {
                yaml_doc_start(p);
            }
            parse_node(p);
            break;
        }
    }
}

bool yaml_parse(YamlParser* p) {
    if (p->size == 0) return true; // is this valid?

    yaml_tok_init(&p->tokenizer, p->input, p->error_msg, &p->data);

    p->token.kind = TOKEN_NONE;
#if 1
    int res = setjmp(p->jump_err);
    if (res == 0) {
        consumeToken(p);

        while (p->token.kind != TOKEN_EOF) parse_document(p);
    } else {
        // got error, errorMsg should be set
        return false;
    }
#else
    while (1) {
        yaml_tok_lex(&p->tokenizer, &p->token);
        print_token(&p->data, &p->token);
        switch (p->token.kind) {
        case TOKEN_EOF:
            return true;
        case TOKEN_ERROR:
            return false;
        default:
            // TEMP
            break;
        }
    }
#endif
    return true;
}

const char* yaml_get_error(const YamlParser* p) {
    return p->error_msg;
}

void yaml_dump(const YamlParser* p, bool verbose) {
    yaml_data_dump(&p->data, verbose);
}

// ----------------------------------------------------------------------------

static const char* starts_with(const char* full, const char* start) {
    uint32_t len = strlen(start);
    if (strncmp(full, start, len) == 0) {
        full += len;
        if (full[0] == '.') return full+1;
        if (full[0] == 0) return full;
    }
    return NULL;
}

static const YamlNode* data_find_child_node(const YamlData* d, const char* path, uint32_t next) {
    // NOTE: path can be 'a.b.c' while node names are 'a.b' and 'c' so look for starts-with
    while (next) {
        const YamlNode* node = idx2node(d, next);
        if (node->name_idx) {
            const char* name = &d->text[node->name_idx];
            const char* rest = starts_with(path, name);
            if (rest) { // match
                path = rest;
                if (path[0] == 0) return node; // found node
                if (node->kind == NODE_SEQUENCE) return NULL;   // dont search in sequence
                next = node->child_idx;
                continue;
            }
        }

        next = node->next_idx;
    }
    return NULL;
}

static const YamlNode* data_find_node(const YamlData* d, const char* path) {
    uint32_t node_count = (uint32_t)(d->nodes_cur - d->nodes) - 1;
    if (node_count == 0) return NULL;
    const YamlNode* root = &d->nodes[1];
    if (root->kind == NODE_SEQUENCE) return NULL;

    return data_find_child_node(d, path, root->child_idx);
}

const YamlNode* yaml_get_root(const YamlParser* p) {
    uint32_t node_count = (uint32_t)(p->data.nodes_cur - p->data.nodes) - 1;
    if (node_count == 0) return NULL;
    return &p->data.nodes[1];
}

const YamlNode* yaml_get_next_root(const YamlParser* p, const YamlNode* root) {
    if (root->next_idx == 0) return NULL;
    return idx2node(&p->data, root->next_idx);
}

const char* yaml_get_scalar_value(const YamlParser* p, const char* path) {
    const YamlNode* n = data_find_node(&p->data, path);
    if (n && n->kind == NODE_SCALAR) return &p->data.text[n->text_idx];
    return NULL;
}

const YamlNode* yaml_find_node(const YamlParser* p, const char* path) {
    return data_find_node(&p->data, path);
}

bool yaml_node_is_scalar(const YamlNode* node) {
    return node->kind == NODE_SCALAR;
}

bool yaml_node_is_sequence(const YamlNode* node) {
    return node->kind == NODE_SEQUENCE;
}

bool yaml_node_is_mapping(const YamlNode* node) {
    return node->kind == NODE_MAP;
}

const char* yaml_node_get_name(const YamlParser* parser, const YamlNode* node) {
    return &parser->data.text[node->name_idx];
}

const char* yaml_node_get_scalar_value(const YamlParser* p, const YamlNode* node) {
    if (node->kind == NODE_SCALAR) return &p->data.text[node->text_idx];
    return NULL;
}

YamlIter yaml_node_get_child_iter(const YamlParser* p, const YamlNode* n) {
    YamlIter iter = { .data = &p->data, .node = NULL };
    if (n->kind != NODE_SCALAR && n->child_idx) {
        iter.node = idx2node(&p->data, n->child_idx);
    }
    return iter;
}

void yaml_iter_next(YamlIter* iter) {
    if (iter->node) {
        if (iter->node->next_idx) iter->node = idx2node((YamlData*)iter->data, iter->node->next_idx);
        else iter->node = NULL;
    }
}

bool yaml_iter_done(const YamlIter* iter) {
    return iter->node == NULL;
}

const char* yaml_iter_get_name(const YamlIter* iter) {
    const YamlData* data = (YamlData*)iter->data;
    if (iter->node) return &data->text[iter->node->name_idx];
    return NULL;
}

const char* yaml_iter_get_scalar_value(const YamlIter* iter) {
    const YamlData* data = (YamlData*)iter->data;
    if (iter->node && iter->node->kind == NODE_SCALAR) {
        return &data->text[iter->node->text_idx];
    }
    return NULL;
}

const char* yaml_iter_get_child_scalar_value(const YamlIter* iter, const char* name) {
    const YamlData* data = (YamlData*)iter->data;
    if (iter->node->kind != NODE_MAP) return NULL;
    const YamlNode* n = data_find_child_node(data, name, iter->node->child_idx);
    if (n && n->kind == NODE_SCALAR) return &data->text[n->text_idx];
    return NULL;
}

