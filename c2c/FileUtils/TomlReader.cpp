/* Copyright 2013-2020 Bas van den Berg
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

#include "FileUtils/TomlReader.h"
#include "FileUtils/FileMap.h"

#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <setjmp.h>

#define MAX_TEXT 256
#define MAX_DEPTH 8

//#define DEBUG_TOKENS
//#define DEBUG_PARSER
//#define DEBUG_NODES
#define TEST_MODE

#define MAX_NODES  (4096*16)
#define MAX_NAMES  (4096)
#define MAX_VALUES (4096*128)

#define NODE_KIND_OFFSET 29
#define NAMES_CACHE_SIZE 8

#ifdef DEBUG_PARSER
#define LOG_FUNC printf("%s()\n", __func__)
#else
#define LOG_FUNC
#endif

#ifdef TEST_MODE
#include "Utils/StringBuilder.h"
#endif

using namespace C2;

/*
    Quoted keys are not supported
    Multi-line strings (""" ... """) not supported
    DateTime (1979-05-27 07:32:00Z) not supported
    Numeric keys (1234 = ) not supported
    Floating point numbers (1e6, 1.4) not supported
    Recursive array values not supported
    Inline tables not supported
    Nested array values are not supported
    Array values with different values types not checked
    Array values that are empty are skipped

    TODO numbers: +/- prefix
    TODO numbers: support _ , like 1000_0000
    TODO numbers: support hex (0x123) and octal (0o123) and binary (0b101010)
    TODO numbers: all int64_t
    TODO Dotted keys (a.b = bla)

    TODO store numbers, floats (store as string, add getValue functions
        also do same for true/false (store 1 byte type before value?)
        (can also contain whether last in array?)

    TODO search previous TableArray node for cache node nodes (remove duplicates)
*/

// ----------------------------------------------------------------------------

namespace toml {

enum TokenKind {
    word,           // abc
    text,           // ".." or ' ..'
    number,         // 1234
    kw_true,        // true
    kw_false,       // false
    lbrace,         // [
    lbrace2,        // [[
    rbrace,         // ]
    rbrace2,        // ]]
    equals,
    dot,
    comma,
    eof,
    error,
};

static const char* getTokenName(TokenKind k) {
    switch (k) {
    case word    : return "word";
    case text    : return "text";
    case number  : return "number";
    case kw_true : return "true";
    case kw_false: return "false";
    case lbrace  : return "[";
    case lbrace2 : return "[[";
    case rbrace  : return "]";
    case rbrace2 : return "]]";
    case equals  : return "=";
    case dot     : return ".";
    case comma   : return ",";
    case eof     : return "eof";
    case error   : return "error";
    }
    return "?";
}

}

namespace {

class Location {
public:
    Location(unsigned l, unsigned c) : line(l), column(c) {}
    Location() : line(0), column(0) {}

    const char* str() const {
        static char msg[32];
        sprintf(msg, "at line %d:%d", line, column);
        return msg;
    }

    uint32_t line;
    uint32_t column;
};


class Token {
public:
    Token() : kind(toml::eof), text(0), number(0) {}

    toml::TokenKind getKind() const { return kind; }
    void setKind(toml::TokenKind k) { kind = k; }
    bool is(toml::TokenKind k) const { return kind == k; }
    bool isNot(toml::TokenKind k) const { return kind != k; }

    Location getLoc() const { return loc; }
    void setLocation(Location l) { loc = l; }

    const char* getText() const { return text; }
    void setData(const char* d) { text = d; }

    uint32_t getNumber() const { return number; }
    void setNumber(uint32_t n) { number = n; }

    void clear() {
        text = 0;
        number = 0;
    }

    const char* getName() const { return toml::getTokenName(kind); }

private:
    Location loc;
    toml::TokenKind kind;
    const char* text;
    uint32_t number;
};

class Tokenizer {
public:
    Tokenizer()
        : dataStart(0), current(0) , loc(1, 1)
        , haveNext(false)
    {
        text[0] = 0;
    }
    void init(const char* input) {
        dataStart = input;
        current = input;
    }
    void Lex(Token& Result) {
        if (haveNext) {
            Result = nextToken;
            haveNext = false;
            return;
        }
        Result.clear();

        while (1) {
            switch (*current) {
            case 0:
                Result.setLocation(loc);
                Result.setKind(toml::eof);
                return;
            case '#':
                if (loc.column != 1) {
                    sprintf(text, "unexpected '#' after line start at %s", loc.str());
                    Result.setKind(toml::error);
                    Result.setData(text);
                    return;
                }
                parseComments();
                break;
            case ' ':
            case '\t':
                advanceToken(1);
                break;
            case '\n':
                current++;
                loc.line++;
                loc.column = 1;
                break;
            case '=':
                Result.setLocation(loc);
                Result.setKind(toml::equals);
                advanceToken(1);
                return;
            case '.':
                Result.setLocation(loc);
                Result.setKind(toml::dot);
                advanceToken(1);
                return;
            case ',':
                Result.setLocation(loc);
                Result.setKind(toml::comma);
                advanceToken(1);
                return;
            case '[':
                Result.setLocation(loc);
                if (current[1] == '[') {
                    advanceToken(2);
                    Result.setKind(toml::lbrace2);
                } else {
                    advanceToken(1);
                    Result.setKind(toml::lbrace);
                }
                return;
            case ']':
                Result.setLocation(loc);
                if (current[1] == ']') {
                    advanceToken(2);
                    Result.setKind(toml::rbrace2);
                } else {
                    advanceToken(1);
                    Result.setKind(toml::rbrace);
                }
                return;
            case '"':
                parseText(Result);
                return;
            default:
                // key or number
                Result.setLocation(loc);
                if (isdigit(*current)) {
                    parseNumber(Result);
                    return;
                }
                if (*current == 'f' && strncmp("false", current, 5) == 0) {
                    advanceToken(5);
                    Result.setKind(toml::kw_false);
                    return;
                }
                if (*current == 't' && strncmp("true", current, 4) == 0) {
                    advanceToken(4);
                    Result.setKind(toml::kw_true);
                    return;
                }
                if (isalpha(*current)) {
                    parseKey(Result);
                    return;
                }
                sprintf(text, "unexpected char '%c' at %s", *current, loc.str());
                Result.setKind(toml::error);
                Result.setData(text);
                return;
            }
        }
    }
    Token& LookAhead() {
        if (!haveNext) {
            Lex(nextToken);
            haveNext = true;
        }
        return nextToken;
    }
private:
    inline bool IsKeyChar( unsigned char ch ) {
        // This is a heuristic guess in attempt to not implement Unicode-aware isalpha()
        if (ch >= 128) return true;
        if (isalpha(ch)) return true;
        if (isdigit(ch)) return true;
        if (ch == '_' || ch == '-') return true;
        return false;
    }
    void advanceToken(unsigned amount) {
        loc.column += amount;
        current += amount;
    }
    void parseComments() {
        while (1) {
            switch (*current) {
            case 0:
                return;
            case '\n':
                current++;
                loc.line++;
                loc.column = 1;
                return;
            default:
                current++;
                loc.column++;
                break;
            }
        }
    }
    void parseText(Token& Result) {
        // TODO handle literal strings (' .. ') -> no escaping
        // TODO escape chars for normal strings (" .. \" \r \n ")
        advanceToken(1);
        const char* start = current;
        while (*current && *current != '"') {
            current++;
        }
        size_t len = current - start;
        assert(len < sizeof(text));
        memcpy(text, start, len);
        text[len] = 0;
        Result.setKind(toml::text);
        Result.setData(text);
        // NOTE: dont use advance() since current is already moved
        loc.column += len;
        advanceToken(1);
    }
    void parseKey(Token& Result) {
        const char* start = current;
        while (*current && IsKeyChar(*current)) {
            current++;
        }
        size_t len = current -start;
        assert(len < sizeof(text));
        memcpy(text, start, len);
        text[len] = 0;
        Result.setKind(toml::word);
        Result.setData(text);
        loc.column += len;
    }
    void parseNumber(Token& Result) {
        // TODO handle prefix +/- and make int64_t
        // TODO handle hexadecimal/octal/binary numbers
        // TODO handle '_' , like 1_000_000
        uint32_t number = atoi(current);
        Result.setKind(toml::number);
        Result.setNumber(number);
        while (*current && isdigit(*current)) {
            current++;
            loc.column++;
        }
    }

    const char* dataStart;
    const char* current;

    Location loc;
    char text[MAX_TEXT];

    Token nextToken;
    bool haveNext;
};

}

// ----------------------------------------------------------------------------

namespace C2 {

enum NodeKind {
    NODE_TABLE = 0,
    NODE_TABLE_ARRAY,
    NODE_VALUE_ARRAY,
    NODE_VALUE,
    NODE_ARRAY_CHILD,
};

#ifdef DEBUG_NODES
static const char* Str(NodeKind k) {
    switch (k) {
    case NODE_TABLE:        return "TBL";
    case NODE_TABLE_ARRAY:  return "TAR";
    case NODE_VALUE_ARRAY:  return "KAR";
    case NODE_VALUE:        return "KEY";
    case NODE_ARRAY_CHILD:  return "CHD";
    }
    return "";
}
#endif

#define VALUE_IS_ARRAY (1 << 31)

static inline uint32_t addKind(uint32_t value, NodeKind k) {
    return value | (k << NODE_KIND_OFFSET);
}

static NodeKind getKind(uint32_t value) {
    return static_cast<NodeKind>(value >> NODE_KIND_OFFSET);
}

static uint32_t getValue(uint32_t node_value) {
    return node_value & ~(0x7 << NODE_KIND_OFFSET);
}

static bool same(const char* a, const char* b) {
    unsigned i = 0;
    while (a[i] == b[i]) {
        if (a[i] == 0) return true;
        ++i;
    }
    return false;
}

struct Node {
    uint32_t name_off;      // highest 3-bits are NodeKind
    uint32_t next_node;
    union {
        uint32_t child;
        uint32_t value;
    };
} __attribute((packed));

struct Blocks {
    Node* nodes;
    uint32_t node_count;
    uint32_t max_nodes;

    char* names;
    uint32_t names_off;
    uint32_t names_size;
    // TODO remove, only used during parsing for matching
    uint32_t names_cache[NAMES_CACHE_SIZE];
    uint32_t last_cache;

    char* values;
    uint32_t values_off;
    uint32_t values_size;

    Blocks() {
        memset(this, 0, sizeof(Blocks));
        nodes = (Node*)calloc(MAX_NODES, sizeof(Node));
        max_nodes = MAX_NODES;

        names_size = MAX_NAMES;
        names = (char*)calloc(1, names_size);
        names[0] = 0;
        names_off = 1;      // 0 indicates no name

        values_size = MAX_VALUES;
        values = (char*)calloc(1, values_size);
        values[0] = 0;
        values_off = 1;     // 0 indicates no value

        last_cache = 0;
        memset(names_cache, 0, sizeof(names_cache));
    }
    ~Blocks() {
        free(values);
        free(names);
        free(nodes);
    }
    uint32_t searchNameCache(const char* name) const {
        for (unsigned i=0; i<NAMES_CACHE_SIZE; ++i) {
            uint32_t off = names_cache[i];
            if (off && same(&names[off], name)) return off;
        }
        return 0;
    }
    uint32_t addNode(const char* name, NodeKind kind) {
        //printf(">>> ADD_NODE %s  %s\n", name, Str(kind));
        if (node_count == MAX_NODES) {
            printf("node limit reached (%u)\n", MAX_NODES);
            exit(EXIT_FAILURE);
        }
        uint32_t off = node_count;
        Node* node = &nodes[off];
        node_count++;

        if (name[0] == 0) {
            node->name_off = 0;
        } else {
            uint32_t name_off = searchNameCache(name);
            if (name_off != 0) {
                node->name_off = name_off;
            } else {
                uint32_t len = strlen(name) + 1;
                name_off = names_off;
                node->name_off = name_off;
                char* newname = &names[name_off];
                memcpy(newname, name, len);
                names_cache[last_cache] = name_off;
                last_cache = (last_cache + 1) % NAMES_CACHE_SIZE;
                names_off += len;
            }
        }
        node->name_off = addKind(node->name_off, kind);
        return off;
    }
    uint32_t addValue(const char* value) {
        if (value[0] == 0) return 0;
        uint32_t off = values_off;
        uint32_t len = strlen(value) + 1;
        memcpy(&values[off], value, len);
        values_off += len;
        return off;
    }
    void addNull() {
        values[values_off] = 0;
        values_off++;
    }
    const char* getName(const Node* node) const {
        return &names[getValue(node->name_off)];
    }
    Node* findNode(const char* name, const Node* parent) const {
        if (node_count == 0) return 0;
        Node* node = &nodes[0];
        if (parent) {
            if (!parent->child) return 0;
            node = &nodes[parent->child];
        }
        while (1) {
            const char* node_name = &names[getValue(node->name_off)];
            if (same(name, node_name)) {
                return node;
            }
            if (!node->next_node) return 0;
            node = &nodes[node->next_node];
        }
        return 0;
    }
#ifdef DEBUG_NODES
    void dump() const {
        printf("Nodes (%u/%u)  (%u bytes)\n", node_count, max_nodes, node_count * (int)sizeof(Node));
        for (unsigned i=0; i<node_count; i++) {
            const Node* n = &nodes[i];
            uint32_t name_off = getValue(n->name_off);
            NodeKind kind = getKind(n->name_off);
            switch (kind) {
            case NODE_TABLE:
            case NODE_TABLE_ARRAY:
                printf("  [%3u]  %s name %3u   next %3u   child %3u  (%s)\n",
                    i, Str(kind), name_off, n->next_node, n->child, &names[name_off]);
                break;
            case NODE_VALUE_ARRAY:
            case NODE_VALUE:
                printf("  [%3u]  %s name %3u   next %3u   value %3u  (%s)\n",
                    i, Str(kind), name_off, n->next_node, n->value, &names[name_off]);
                break;
            case NODE_ARRAY_CHILD:
                printf("  [%3u]  %s name ---   next %3u   child %3u\n",
                    i, Str(kind), n->next_node, n->child);
                break;
            }
        }
        printf("Names (%u/%u)\n", names_off, names_size);
        uint32_t i = 1;
        uint32_t start = i;
        while (i < names_off) {
            if (names[i] == 0) {
                printf("  [%3u] %s\n", start, &names[start]);
                i++;
                start = i;
            } else {
                i++;
            }
        }
        printf("Values (%u/%u)\n", values_off, values_size);
        i = 1;
        start = i;
        while (i < values_off) {
            if (values[i] == 0) {
                printf("  [%3u] %s\n", start, &values[start]);
                i++;
                start = i;
            } else {
                i++;
            }
        }
        if (node_count != 0) {
            printf("\n");
            dumpNode(0, 0);
        }
    }
    void dumpNode(uint32_t off, uint32_t indent) const {
        const Node* node = &nodes[off];
        NodeKind kind = getKind(node->name_off);
        switch (kind) {
        case NODE_TABLE:
            printf("%*s[%s]\n", 3*indent, "", &names[getValue(node->name_off)]);
            if (node->child) dumpNode(node->child, indent+1);
            break;
        case NODE_TABLE_ARRAY:
            printf("%*s[[%s]]\n", 3*indent, "", &names[getValue(node->name_off)]);
            if (node->child) dumpNode(node->child, indent+1);
            break;
        case NODE_VALUE_ARRAY:
        {
            printf("%*s%s = ", 3*indent, "", &names[getValue(node->name_off)]);
            const char* value = &values[node->value];
            printf("[ ");
            while (value[0]) {
                printf("'%s', ", value);
                value += strlen(value) + 1;
            }
            printf("]\n");
            break;
        }
        case NODE_VALUE:
            printf("%*s%s = ", 3*indent, "", &names[getValue(node->name_off)]);
            printf("'%s'\n", &values[node->value]);
            break;
        case NODE_ARRAY_CHILD:
            printf("%*s<>\n", 3*indent, "");
            if (node->child) dumpNode(node->child, indent+1);
            break;
        }
        if (node->next_node) dumpNode(node->next_node, indent);
    }
#endif
#ifdef TEST_MODE
    void dumpTest(StringBuilder& out, uint32_t off, uint32_t indent) const {
        const Node* node = &nodes[off];
        NodeKind kind = getKind(node->name_off);
        switch (kind) {
        case NODE_TABLE:
            out.print("%*s[%s]\n", 2*indent, "", &names[getValue(node->name_off)]);
            if (node->child) dumpTest(out, node->child, indent+1);
            break;
        case NODE_TABLE_ARRAY:
            out.print("%*s[[%s]]\n", 2*indent, "", &names[getValue(node->name_off)]);
            if (node->child) dumpTest(out, node->child, indent+1);
            break;
        case NODE_VALUE_ARRAY:
        {
            out.print("%*s%s = ", 2*indent, "", &names[getValue(node->name_off)]);
            const char* value = &values[node->value];
            out << "[ ";
            while (value[0]) {
                out.print("'%s', ", value);
                value += strlen(value) + 1;
            }
            out << "]\n";
            break;
        }
        case NODE_VALUE:
            out.print("%*s%s = ", 2*indent, "", &names[getValue(node->name_off)]);
            out.print("'%s'\n", &values[node->value]);
            break;
        case NODE_ARRAY_CHILD:
            out.print("%*s<>\n", 2*indent, "");
            if (node->child) dumpTest(out, node->child, indent+1);
            break;
        }
        if (node->next_node) dumpTest(out, node->next_node, indent);
    }
#endif
};

}

// ----------------------------------------------------------------------------

class TomlParser {
public:
    TomlParser(const char* input,
               char* errorMsg_,
               Blocks& blocks_)
        : errorMsg(errorMsg_)
        , blocks(blocks_)
        , numParents(0)
        , topParent(0)
    {
        memset(parents, 0, sizeof(parents));
        memset(lastChild, 0, sizeof(lastChild));
        errorMsg[0] = 0;

        tokenizer.init(input);
        int result = setjmp(jump_err);
        if (result == 0) {
            ConsumeToken();

            //while (!Tok.is(toml::eof)) ConsumeToken();

            parseTopLevel();
        } // else got error, errorMsg should be set
    }
#ifdef DEBUG_NODES
    void dump() const {
        blocks.dump();
    }
#endif
private:
    void parseTopLevel() {
        //    key = value
        //  | [[array]]
        //  | [table]
        while (Tok.isNot(toml::eof)) {
            switch (Tok.getKind()) {
            case toml::word:
                parseKeyValue();
                break;
            case toml::lbrace:
                parseTable();
                break;
            case toml::lbrace2:
                parseTableArray();
                break;
            default:
                sprintf(errorMsg, "syntax error %s", Tok.getLoc().str());
                longjmp(jump_err, 1);
                break;
            }
        }
    }
    void parseKeyValue() {
        LOG_FUNC;
        char key[MAX_TEXT];
        strcpy(key, Tok.getText());
        ConsumeToken();
        ExpectAndConsume(toml::equals);
        uint32_t value = parseValue();
        bool isArray = ((value & VALUE_IS_ARRAY) != 0);
        uint32_t off = blocks.addNode(key, isArray ? NODE_VALUE_ARRAY : NODE_VALUE);
        Node* node = &blocks.nodes[off];
        node->value = value & ~VALUE_IS_ARRAY;
        if (lastChild[numParents]) {
            lastChild[numParents]->next_node = off;
        } else {
            if (topParent) topParent->child = off;
        }
        lastChild[numParents] = node;
        //dumpAdmin("added key-value");
    }
    void parseTable() {
        LOG_FUNC;
        ConsumeToken();
        Expect(toml::word);
        const char* name = Tok.getText();
        uint32_t depth = 0;
        bool isTop = NextToken().isNot(toml::dot);
        depth += addTable(name, depth, isTop, NODE_TABLE);
        ConsumeToken();

        while (Tok.is(toml::dot)) {
            depth++;
            ConsumeToken();
            Expect(toml::word);
            name = Tok.getText();
            isTop = NextToken().isNot(toml::dot);
            depth += addTable(name, depth, isTop, NODE_TABLE);
            ConsumeToken();
        }
        ExpectAndConsume(toml::rbrace);
    }
    void parseTableArray() {
        LOG_FUNC;
        ConsumeToken();
        Expect(toml::word);
        const char* name = Tok.getText();
        uint32_t depth = 0;
        bool isTop = NextToken().isNot(toml::dot);
        depth += addTable(name, depth, isTop, NODE_TABLE_ARRAY);
        ConsumeToken();

        while (Tok.is(toml::dot)) {
            depth++;
            ConsumeToken();
            Expect(toml::word);
            name = Tok.getText();
            isTop = NextToken().isNot(toml::dot);
            depth += addTable(name, depth, isTop, NODE_TABLE_ARRAY);
            ConsumeToken();
        }
        ExpectAndConsume(toml::rbrace2);
    }
    uint32_t parseValue() {
        LOG_FUNC;
        uint32_t value = 0;
        switch (Tok.getKind()) {
        case toml::word:
            sprintf(errorMsg, "unexpected word %s", Tok.getLoc().str());
            longjmp(jump_err, 1);
            break;
        case toml::text:
            value = blocks.addValue(Tok.getText());
            ConsumeToken();
            break;
        case toml::number:
        case toml::kw_true:
        case toml::kw_false:
            // TODO keep number, true/false as text
            ConsumeToken();
            break;
        case toml::lbrace:
            value = parseArrayValues();
            //sprintf(errorMsg, "array values not supported %s", Tok.getLoc().str());
            //longjmp(jump_err, 1);
            break;
        default:
            break;
        }
        return value;
    }
    uint32_t parseArrayValues() {
        LOG_FUNC;
        // syntax: [ value , {value} ]
        ConsumeToken();
        uint32_t value = parseValue() | VALUE_IS_ARRAY;
        while (Tok.is(toml::comma)) {
            ConsumeToken();
            if (Tok.is(toml::rbrace)) break;     // trailing comma is allowed
            parseValue();
        }
        ExpectAndConsume(toml::rbrace);
        blocks.addNull();
        return value;
    }
#ifdef DEBUG_NODES
    void dumpAdmin(const char* msg) const {
        printf("----[ %s ] ----------\n", msg);
        int off = -1;
        if (topParent) off = topParent - blocks.nodes;
        printf("numParents %u   topParent -> node[%d]\n", numParents, off);
        for (unsigned i=0; i<numParents; i++) {
            const Node* node = parents[i];
            off = -1;
            if (node) off = node - blocks.nodes;
            printf("  parent[%u] = node[%d]\n", i, off);
        }
        if (numParents) printf("lastChild\n");
        for (unsigned i=0; i<numParents+1; i++) {
            off = -1;
            const Node* node = lastChild[i];
            if (node) off = node - blocks.nodes;
            printf("  child [%u] = node[%d]\n", i, off);
        }
        blocks.dump();
    }
#endif
    unsigned addTable(const char* name, uint32_t depth, bool isTop, NodeKind kind)
    {
        //printf(">>> ADD_TABLE %s  %s  %u/%u\n", name, Str(kind), depth, numParents);
        if (!isTop && numParents > depth && same(blocks.getName(parents[depth]), name)) {
            if (getKind(parents[depth]->name_off) == NODE_TABLE_ARRAY) return 1;
            // Do nothing
        } else {
            if (kind == NODE_TABLE_ARRAY) {
                // TODO also check if previous is also TableArray
                if (numParents > depth && same(blocks.getName(parents[depth]), name)) {
                    //printf("SAME\n");
                    numParents = depth + 1;
                } else {
                    uint32_t off = blocks.addNode(name, kind);
                    if (numParents > depth) parents[depth]->next_node = off;
                    Node* node = &blocks.nodes[off];
                    parents[depth] = node;

                    if (lastChild[depth]) {
                        lastChild[depth]->next_node = off;
                    } else {
                        if (depth > 0) parents[depth-1]->child = off;
                    }
                    numParents = depth + 1;
                    topParent = node;
                    lastChild[depth] = node;
                    lastChild[depth + 1] = 0;
                }
                if (isTop) {
                    // add iterator node as child or next
                    uint32_t off = blocks.addNode("", NODE_ARRAY_CHILD);
                    Node* iter = &blocks.nodes[off];
                    if (lastChild[depth]->child) { //already has children
                        lastChild[depth+1]->next_node = off;
                    } else {
                        lastChild[depth]->child = off;
                    }
                    lastChild[depth+1] = iter;
                    parents[depth+1] = iter;
                    lastChild[depth+2] = 0;
                    topParent = iter;
                    numParents++;
                }
                //dumpAdmin("added table_array");
                return 1;
            }
            uint32_t off = blocks.addNode(name, kind);
            if (numParents > depth) parents[depth]->next_node = off;
            Node* node = &blocks.nodes[off];
            parents[depth] = node;

            if (lastChild[depth]) {
                lastChild[depth]->next_node = off;
            } else {
                if (depth > 0) parents[depth-1]->child = off;
            }
            numParents = depth + 1;
            topParent = node;
            lastChild[depth] = node;
            lastChild[depth + 1] = 0;
        }
        return 0;
    }
    Location ConsumeToken() {
        Location prev = Tok.getLoc();
        tokenizer.Lex(Tok);
#ifdef DEBUG_TOKENS
        {
            printf("  %8s at %s", Tok.getName(), Tok.getLoc().str());
            if (Tok.is(toml::number)) printf("  %d", Tok.getNumber());
            if (Tok.is(toml::text)) printf("  \"%s\"", Tok.getText());
            if (Tok.is(toml::word)) printf("  %s", Tok.getText());
            printf("\n");
        }
#endif
        if (Tok.is(toml::error)) {
            strcpy(errorMsg, Tok.getText());
            longjmp(jump_err, 1);
        }
        return prev;
    }
    Token& NextToken() {
        return tokenizer.LookAhead();
    }
    void ExpectAndConsume(toml::TokenKind k) {
        if (Tok.isNot(k)) {
            sprintf(errorMsg, "expected '%s' at %s", getTokenName(k), Tok.getLoc().str());
            longjmp(jump_err, 1);
        }
        ConsumeToken();
    }
    void Expect(toml::TokenKind k) {
        if (Tok.isNot(k)) {
            sprintf(errorMsg, "expected '%s' at %s", getTokenName(k), Tok.getLoc().str());
            longjmp(jump_err, 1);
        }
    }
    // Entries stuff
    Tokenizer tokenizer;
    Token Tok;
    jmp_buf jump_err;
    char* errorMsg;

    Blocks& blocks;
    Node* parents[MAX_DEPTH];
    Node* lastChild[MAX_DEPTH];
    unsigned numParents;
    Node* topParent;
};

TomlReader::TomlReader()
    : blocks(new Blocks())
{
}

TomlReader::~TomlReader()
{
    delete blocks;
}

bool TomlReader::parse(const char* filename)
{
    FileMap file(filename);
    file.open();
    TomlParser parser((const char*)file.region, errorMsg, *blocks);
    if (errorMsg[0] != 0) return false;
#ifdef DEBUG_NODES
    parser.dump();
#endif
#if 0
    printf("Nodes (%u/%u)  (%u bytes)\n", blocks->node_count, blocks->max_nodes, blocks->node_count * (int)sizeof(Node));
    printf("Names (%u/%u)\n", blocks->names_off, blocks->names_size);
    printf("Values (%u/%u)\n", blocks->values_off, blocks->values_size);
#endif
    return true;
}

bool TomlReader::parse(const char* input, int)
{
    TomlParser parser(input, errorMsg, *blocks);
    if (errorMsg[0] != 0) return false;
#ifdef DEBUG_NODES
    parser.dump();
#endif
    return true;
}

void TomlReader::test(StringBuilder& out) const {
#ifdef TEST_MODE
    blocks->dumpTest(out, 0, 0);
#endif
}

const Node* TomlReader::findNode(const char* key) const {
    char name[MAX_TEXT];
    const char* cp = key;
    const char* start = cp;
    unsigned len = 0;
    Node* node = 0;
    while (1) {
        switch (*cp) {
        case 0:
            len = cp - start;
            memcpy(name, start, len);
            name[len] = 0;
            node = blocks->findNode(name, node);
            return node;
        case '.':
            len = cp - start;
            memcpy(name, start, len);
            name[len] = 0;
            start = cp + 1;
            node = blocks->findNode(name, node);
            if (!node) return 0;
            if (getKind(node->name_off) == NODE_VALUE) return 0;
            break;
        default:
            break;
        }
        cp++;
    }
    return 0;

}

const char* TomlReader::getValue(const char* key) const {
    const Node* node = findNode(key);
    if (!node) return 0;
    if (getKind(node->name_off) != NODE_VALUE) return 0;
    return &blocks->values[node->value];
}

TomlReader::NodeIter TomlReader::getNodeIter(const char* key) const
{
    const Node* node = findNode(key);
    if (node && getKind(node->name_off) == NODE_TABLE_ARRAY) {
        node = &blocks->nodes[node->child];
    }
    NodeIter iter(blocks, node);
    return iter;
}


bool TomlReader::NodeIter::done() const {
    return node == 0;
}

void TomlReader::NodeIter::next() {
    if (node == 0) return;
    uint32_t next = node->next_node;
    if (next == 0) node = 0;
    else node = &blocks->nodes[next];
}

const char* TomlReader::NodeIter::getValue(const char* key) const {
    const Node* child = blocks->findNode(key, node);
    if (!child) return 0;
    if (getKind(child->name_off) != NODE_VALUE) return 0;
    return &blocks->values[child->value];
}


bool TomlReader::ValueIter::done() const {
    return values[0] == 0;
}

void TomlReader::ValueIter::next() {
    if (values[0] == 0) return;
    while (values[0] != 0) values++;
    if (isArray) values++;   // skip 0-terminator
}

const char* TomlReader::ValueIter::getValue() const {
    return values;
}

TomlReader::ValueIter TomlReader::getValueIter(const char* key) const
{
    const Node* node = findNode(key);
    if (node) {
        switch (getKind(node->name_off)) {
        case NODE_TABLE:
        case NODE_TABLE_ARRAY:
            break;
        case NODE_VALUE_ARRAY:
            return ValueIter(&blocks->values[node->value], true);
        case NODE_VALUE:
            return ValueIter(&blocks->values[node->value], false);
        case NODE_ARRAY_CHILD:
            // TODO
            break;
        }
    }
    return ValueIter(&blocks->values[0], false);
}

