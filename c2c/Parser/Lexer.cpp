/* Copyright 2018 Christoffer Lernö
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

#include "Parser/Lexer.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

namespace C2 {
    
static inline bool is_alphabet(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}


static inline bool is_digit(char c) {
    return c >= '0' && c <= '9';
}


static inline bool is_digit_or_underscore(char c) {
    return c == '_' || (c >= '0' && c <= '9');
}

static inline bool is_oct(char c) {
    return c >= '0' && c <= '7';
}

static inline bool is_alpha(char c) {
    return is_alphabet(c) || is_digit_or_underscore(c);
}


static inline bool is_oct_or_underscore(char c) {
    return c == '_' || is_oct(c);
}


static inline bool is_binary(char c) {
    return c == '0' || c == '1';
}


static inline bool is_binary_or_underscore(char c) {
    return is_binary(c) || c == '_';
}


static inline bool is_hex(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}


static inline bool is_hex_or_underscore(char c) {
    return c == '_' || is_hex(c);
}


static inline bool reached_end(Lexer *lexer) {
    return lexer->current[0] == '\0';
}

static inline char peek_next(Lexer *lexer) {
    return lexer->current[1];
}

static inline char prev(Lexer *lexer) {
    return lexer->current[-1];
}

static inline char peek(Lexer *lexer) {
    return lexer->current[0];
}

static inline char advance(Lexer *lexer) {
    return *(lexer->current++);
}

static Token make_token(Lexer *lexer, TokenType type) {
    Token token;
    token.type = type;
    token.start = lexer->start;
    token.length = (int)(lexer->current - lexer->start);
    token.line = lexer->line;
    return token;
}

static Token error_token(Lexer *lexer, const char *message) {
    Token token;
    token.line = lexer->line;
    token.start = lexer->start;
    token.length = (int)strlen(message);
    token.type = TOKEN_ERROR;
    return token;
}

static inline void skip_whitespace(Lexer *lexer) {
    while (1) {
        char c = peek(lexer);
        switch (c) {
        case '\n':
            lexer->line++;
        case ' ':
        case '\t':
        case '\r':
        case '\f':
            advance(lexer);
            break;
        case '/':
            if (peek_next(lexer) == '/') {
                while (peek(lexer) != '\n') { advance(lexer); }
                break;
            }
            if (peek_next(lexer) == '*') {
                advance(lexer);
                while (!reached_end(lexer)) {
                    advance(lexer);
                    if (peek(lexer) == '*' && peek_next(lexer) == '/') {
                        lexer->current += 2;
                        break;
                    }
                }
                break;
            }
            return;
        default:
            return;
        }
    }
}

static inline Token scan_string(Lexer *lexer) {
    char c;
    while ((c = advance(lexer)) != '"') {
        if (c == '\\' && peek(lexer) == '"') {
            advance(lexer);
            continue;
        }
        if (c == '\n') lexer->line++;
        if (reached_end(lexer)) {
            return error_token(lexer, "Unterminated string.");
        }
    }
    return make_token(lexer, TOKEN_STRING);
}

static inline TokenType check_keyword(Lexer *lexer, const char *keyword, int length, TokenType type) {
    if (lexer->current - lexer->start == length
        && memcmp(lexer->start + 1, keyword + 1, (size_t)length - 1) == 0) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

static inline TokenType check_two_keywords(Lexer *lexer,
                                    const char *keyword1,
                                    const char *keyword2,
                                    int length,
                                    int diffPos,
                                    TokenType type1,
                                    TokenType type2) {
    if (lexer->current - lexer->start != length) return TOKEN_IDENTIFIER;
    if (lexer->start[diffPos] == keyword1[diffPos])
    {
        if (memcmp(lexer->start + 1, keyword1 + 1, (size_t)length - 1) == 0) return type1;
    }
    else
    {
        if (memcmp(lexer->start + 1, keyword2 + 1, (size_t)length - 1) == 0) return type2;
    }
    return TOKEN_IDENTIFIER;
}
    
#define CHECK_KW(X, Y) check_keyword(lexer, X, sizeof X, Y)
#define CHECK_2KW(X1, X2, SPOT, Y1, Y2) check_two_keywords(lexer, X1, X2, sizeof X1, SPOT, Y1, Y2)

// Yes this is an ugly hand written keyword identifier. It should be benchmarked against
// an table based state machine.
static inline TokenType indentifier_type(Lexer *lexer) {
    int len = (int)(lexer->current - lexer->start);
    switch (*lexer->start) {
    case 'a':
        switch (len)
        {
        case 2: return CHECK_KW("as", TOKEN_AS);
        case 4: return CHECK_KW("auto", TOKEN_AUTO);
        default: return TOKEN_IDENTIFIER;
        }
    case 'b':
        switch (len)
        {
        case 4: return CHECK_KW("bool", TOKEN_BOOL);
        case 5: return CHECK_KW("break", TOKEN_BREAK);
        default: return TOKEN_IDENTIFIER;
        }
    case 'c':
        switch (len)
        {
        case 4:
            switch (lexer->start[3])
            {
            case 'r': CHECK_KW("char", TOKEN_CHAR);
            case 't': CHECK_KW("cast", TOKEN_CAST);
            case 'e': CHECK_KW("case", TOKEN_CAST);
            default: return TOKEN_IDENTIFIER;
            }
        case 5: return CHECK_KW("const", TOKEN_CONST);
        case 8: return CHECK_KW("continue", TOKEN_CONTINUE);
        default: return TOKEN_IDENTIFIER;
        }
    case 'd':
        switch (len)
        {
        case 2: CHECK_KW("do", TOKEN_DO);
        case 7: CHECK_KW("default", TOKEN_DEFAULT);
        default: return TOKEN_IDENTIFIER;
        }
    case 'e':
        switch (len)
        {
        case 4: return CHECK_2KW("else", "enum", 1, TOKEN_ELSE, TOKEN_ENUM);
        case 7: return CHECK_KW("elemsof", TOKEN_ELEMSOF);
        case 8: return CHECK_2KW("enum_min", "enum_max", 6, TOKEN_ENUM_MIN, TOKEN_ENUM_MAX);
        default: return TOKEN_IDENTIFIER;
        }
    case 'f':
        switch (len) {
        case 3:
            if (lexer->start[1] == 'o') return CHECK_KW("for", TOKEN_FOR);
            return CHECK_2KW("f32", "f64", 1, TOKEN_F32, TOKEN_F64);
        case 4: return CHECK_KW("func", TOKEN_FUNC);
        case 5: return CHECK_KW("false", TOKEN_FALSE);
        case 11: return CHECK_KW("fallthrough", TOKEN_FALLTHROUGH);
        default: return TOKEN_IDENTIFIER;
        }
    case 'g': return CHECK_KW("goto", TOKEN_GOTO);
    case 'i':
        switch (len) {
        case 2: return CHECK_2KW("if", "i8", 1, TOKEN_IF, TOKEN_I8);
        case 3:
            switch (lexer->start[1]) {
            case '1': return CHECK_KW("i16", TOKEN_I16);
            case '3': return CHECK_KW("i32", TOKEN_I32);
            case '6': return CHECK_KW("i64", TOKEN_I64);
            default: return TOKEN_IDENTIFIER;
            }
        case 6: return CHECK_KW("import", TOKEN_IMPORT);
        default: return TOKEN_IDENTIFIER;
        }
    case 'm': return CHECK_KW("module", TOKEN_MODULE);
    case 'n': return CHECK_KW("nil", TOKEN_NIL);
    case 'l': return CHECK_KW("local", TOKEN_LOCAL);
    case 'p': return CHECK_KW("public", TOKEN_PUBLIC);
    case 'r': return CHECK_KW("return", TOKEN_RETURN);
    case 's':
        if (len != 6) return TOKEN_IDENTIFIER;
        switch (lexer->start[1])
        {
        case 't': return CHECK_KW("struct", TOKEN_STRUCT);
        case 'i': return CHECK_KW("sizeof", TOKEN_SIZEOF);
        case 'w': return CHECK_KW("switch", TOKEN_SWITCH);
        default: return TOKEN_IDENTIFIER;
        }
    case 't': return CHECK_2KW("true", "type", 1, TOKEN_TRUE, TOKEN_TYPE);
    case 'u':
        switch (len) {
        case 2: return CHECK_KW("u8", TOKEN_U8);
        case 3:
            switch (lexer->start[1]) {
            case '1': return CHECK_KW("u16", TOKEN_U16);
            case '3': return CHECK_KW("u32", TOKEN_U32);
            case '6': return CHECK_KW("u64", TOKEN_U64);
            default: return TOKEN_IDENTIFIER;
            }
        case 5: return CHECK_KW("union", TOKEN_UNION);
        default: return TOKEN_IDENTIFIER;
        }
    case 'v':
        switch (len) {
        case 4: return CHECK_KW("void", TOKEN_VOID);
        case 8: return CHECK_KW("volatile", TOKEN_VOLATILE);
        default: return TOKEN_IDENTIFIER;
        }
    case 'w': return CHECK_KW("while", TOKEN_WHILE);
    default: return TOKEN_IDENTIFIER;
    }
}

#undef CHECK_KW
#undef CHECK_2KW

static inline Token scan_ident(Lexer *lexer) {
    while (is_alpha(peek(lexer))) {
        advance(lexer);
    }
    return make_token(lexer, indentifier_type(lexer));
}

static inline bool match(Lexer *lexer, char expected) {
    if (reached_end(lexer)) return false;
    if (*lexer->current != expected) return false;
    lexer->current++;
    return true;
}


#define PARSE_SPECIAL_NUMBER(is_num, is_num_with_underscore, exp, EXP) \
    while (is_num_with_underscore(peek(lexer))) advance(lexer); \
    bool is_float = false; \
    if (peek(lexer) == '.') \
    { \
      is_float = true; \
      advance(lexer); \
      char c = peek(lexer); \
      if (c == '_') return error_token(lexer, "Underscore may only appear between digits."); \
      if (is_num(c)) advance(lexer); \
      while (is_num_with_underscore(peek(lexer))) advance(lexer); \
    } \
    char c = peek(lexer); \
    if (c == exp || c == EXP) \
    { \
       is_float = true; \
       advance(lexer); \
       char c2 = advance(lexer); \
       if (c2 == '+' || c2 == '-') c2 = advance(lexer); \
       if (!is_num(c2)) return error_token(lexer, "Invalid exponential expression"); \
       while (is_digit(peek(lexer))) advance(lexer); \
    } \
    if (prev(lexer) == '_') return error_token(lexer, "Underscore may only appear between digits."); \
    return make_token(lexer, is_float ? TOKEN_FLOAT : TOKEN_INTEGER);


static inline Token scan_hex(Lexer *lexer) {
    advance(lexer); // skip the x
    if (!is_hex(advance(lexer))) return error_token(lexer, "Invalid hex sequence");
    PARSE_SPECIAL_NUMBER(is_hex, is_hex_or_underscore, 'p', 'P');
}

static inline Token scan_oct(Lexer *lexer) {
    advance(lexer); // Skip the o
    if (!is_oct(advance(lexer))) return error_token(lexer, "Invalid octal sequence");
    while (is_oct_or_underscore(peek(lexer))) { advance(lexer); }
    return make_token(lexer, TOKEN_INTEGER);;
}

static inline Token scan_binary(Lexer *lexer) {
    advance(lexer); // Skip the b
    if (!is_binary(advance(lexer))) return error_token(lexer, "Invalid binary sequence");
    while (is_binary_or_underscore(peek(lexer))) { advance(lexer); }
    return make_token(lexer, TOKEN_INTEGER);;
}

static inline Token scan_digit(Lexer *lexer) {
    if (prev(lexer) == '0') {
        switch (peek(lexer)) {
            // case 'X': Let's not support this? REVISIT
        case 'x':
            return scan_hex(lexer);
        case 'o':
            return scan_oct(lexer);
        case 'b':
            return scan_binary(lexer);
        default:
            break;
        }
    }
    PARSE_SPECIAL_NUMBER(is_digit, is_digit_or_underscore, 'e', 'E');
}

#undef PARSE_SPECIAL_NUMBER

void lexer_init(Lexer *lexer, const char *source) {
    lexer->start = source;
    lexer->current = source;
    lexer->line = 1;
}

Token lexer_scan_token(Lexer *lexer) {
    
    skip_whitespace(lexer);

    lexer->start = lexer->current;
    if (reached_end(lexer)) return make_token(lexer, TOKEN_EOF);

    char c = advance(lexer);
    switch (c) {
    case '"':
        return scan_string(lexer);
    case ',':
        return make_token(lexer, TOKEN_COMMA);
    case ';':
        return make_token(lexer, TOKEN_EOS);
    case '{':
        return make_token(lexer, TOKEN_BRACE_L);
    case '}':
        return make_token(lexer, TOKEN_BRACE_R);
    case '(':
        return make_token(lexer, TOKEN_PAREN_L);
    case ')':
        return make_token(lexer, TOKEN_PAREN_R);
    case '[':
        return make_token(lexer, TOKEN_BRACKET_L);
    case ']':
        return make_token(lexer, TOKEN_BRACKET_R);
    case '.':
        return make_token(lexer, TOKEN_DOT);
    case '~':
        return make_token(lexer, TOKEN_BIT_NOT);
    case '@':
        if (match(lexer, '(')) return make_token(lexer, TOKEN_ATTRIBUTE_PAREN);
        return error_token(lexer, "Unexpected character.");
    case ':':
        return make_token(lexer, match(lexer, '=') ? TOKEN_COLON_ASSIGN : TOKEN_COLON);
    case '!':
        return make_token(lexer, match(lexer, '=') ? TOKEN_NOT_EQUAL : TOKEN_NOT);
    case '/':
        return make_token(lexer, match(lexer, '=') ? TOKEN_DIV_ASSIGN : TOKEN_DIV);
    case '*':
        return make_token(lexer, match(lexer, '=') ? TOKEN_MULT_ASSIGN : TOKEN_MULT);
    case '=':
        return make_token(lexer, match(lexer, '=') ? TOKEN_EQUAL : TOKEN_ASSIGN);
    case '^':
        if (match(lexer, '^')) return make_token(lexer, TOKEN_POW);
        return make_token(lexer, match(lexer, '=') ? TOKEN_BIT_XOR_ASSIGN : TOKEN_BIT_XOR);
    case '<':
        if (match(lexer, '<')) return make_token(lexer, match(lexer, '=') ? TOKEN_LEFT_SHIFT_ASSIGN : TOKEN_LEFT_SHIFT);
        return make_token(lexer, match(lexer, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
        if (match(lexer, '>')) {
            if (match(lexer, '>')) {
                return make_token(lexer, match(lexer, '=') ? TOKEN_RIGHT_SHIFT_LOGIC_ASSIGN
                                            : TOKEN_RIGHT_SHIFT_LOGIC);
            }
            return make_token(lexer, match(lexer, '=') ? TOKEN_RIGHT_SHIFT_ASSIGN : TOKEN_RIGHT_SHIFT);
        }
        return make_token(lexer, match(lexer, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    case '&':
        if (match(lexer, '&')) {
            return make_token(lexer, match(lexer, '=') ? TOKEN_AND_ASSIGN : TOKEN_AND);
        }
        return make_token(lexer, match(lexer, '=') ? TOKEN_BIT_AND_ASSIGN : TOKEN_BIT_AND);
    case '|':
        if (match(lexer, '|')) {
            return make_token(lexer, match(lexer, '=') ? TOKEN_OR_ASSIGN : TOKEN_OR);
        }
        return make_token(lexer, match(lexer, '=') ? TOKEN_BIT_OR_ASSIGN : TOKEN_BIT_OR);
    case '+':
        if (match(lexer, '+')) return make_token(lexer, TOKEN_PLUSPLUS);
        if (match(lexer, '=')) return make_token(lexer, TOKEN_PLUS_ASSIGN);
        return make_token(lexer, TOKEN_PLUS);
    case '-':
        if (match(lexer, '-')) return make_token(lexer, match(lexer, '-') ? TOKEN_NO_INIT : TOKEN_MINUSMINUS);
        if (match(lexer, '=')) return make_token(lexer, TOKEN_MINUS_ASSIGN);
        return make_token(lexer, TOKEN_MINUS);

    default:
        if (is_digit(c)) return scan_digit(lexer);
        if (is_alphabet(c)) return scan_ident(lexer);
        return error_token(lexer, "Unexpected character.");
    }

}
    
}

