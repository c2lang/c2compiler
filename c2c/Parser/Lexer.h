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

#ifndef PARSER_LEXER_H
#define PARSER_LEXER_H

namespace C2 {

typedef enum {
	// Single-character tokens.
	TOKEN_PAREN_L,
	TOKEN_PAREN_R,
	TOKEN_BRACE_L,
	TOKEN_BRACE_R,
    TOKEN_BRACKET_L,
    TOKEN_BRACKET_R,
	TOKEN_COMMA,
	TOKEN_DOT,
	TOKEN_EOS,

	// One or two character tokens.
    TOKEN_ATTRIBUTE_PAREN,
	TOKEN_PLUS,
	TOKEN_PLUSPLUS,
	TOKEN_PLUS_ASSIGN,
	TOKEN_BIT_NOT,
	TOKEN_NOT,
	TOKEN_MINUS,
	TOKEN_MINUSMINUS,
	TOKEN_MINUS_ASSIGN,
	TOKEN_MULT,
	TOKEN_MULT_ASSIGN,
	TOKEN_POW,
	TOKEN_DIV,
	TOKEN_DIV_ASSIGN,
	TOKEN_NOT_EQUAL,
	TOKEN_ASSIGN,
	TOKEN_EQUAL,
	TOKEN_COLON,
	TOKEN_COLON_ASSIGN,
	// Three or more
		TOKEN_GREATER,
	TOKEN_GREATER_EQUAL,
	TOKEN_RIGHT_SHIFT,
	TOKEN_RIGHT_SHIFT_ASSIGN,
	TOKEN_RIGHT_SHIFT_LOGIC,
	TOKEN_RIGHT_SHIFT_LOGIC_ASSIGN,
	TOKEN_LESS,
	TOKEN_LESS_EQUAL,
	TOKEN_LEFT_SHIFT,
	TOKEN_LEFT_SHIFT_ASSIGN,
	TOKEN_AND,
	TOKEN_AND_ASSIGN,
	TOKEN_BIT_AND,
	TOKEN_BIT_AND_ASSIGN,
	TOKEN_OR,
	TOKEN_OR_ASSIGN,
	TOKEN_BIT_OR,
	TOKEN_BIT_OR_ASSIGN,
	TOKEN_BIT_XOR,
	TOKEN_BIT_XOR_ASSIGN,
	TOKEN_NO_INIT,
    
    
	// Literals.
	TOKEN_IDENTIFIER,
	TOKEN_STRING,
	TOKEN_INTEGER,
	TOKEN_FLOAT,


	// Keywords.
	TOKEN_MODULE,
	TOKEN_IMPORT,
	TOKEN_AS,
	TOKEN_PUBLIC,

	TOKEN_AUTO,
	TOKEN_BOOL,
	TOKEN_CAST,
	TOKEN_CHAR,
	TOKEN_CONST,
	TOKEN_ELEMSOF,
	TOKEN_ENUM_MIN,
	TOKEN_ENUM_MAX,
	TOKEN_ENUM,
	TOKEN_FALSE,
	TOKEN_F32,
	TOKEN_F64,
	TOKEN_I8,
	TOKEN_I16,
	TOKEN_I32,
	TOKEN_I64,
	TOKEN_LOCAL,
	TOKEN_NIL,
	TOKEN_SIZEOF,
	TOKEN_STRUCT,
	TOKEN_TRUE,
	TOKEN_TYPE,
	TOKEN_U8,
	TOKEN_U16,
	TOKEN_U32,
	TOKEN_U64,
	TOKEN_UNION,
	TOKEN_VOID,
	TOKEN_VOLATILE,

	TOKEN_BREAK,
	TOKEN_CASE,
	TOKEN_CONTINUE,
	TOKEN_DEFAULT,
	TOKEN_DO,
	TOKEN_ELSE,
	TOKEN_FALLTHROUGH,
	TOKEN_FOR,
	TOKEN_GOTO,
	TOKEN_IF,
	TOKEN_RETURN,
	TOKEN_SWITCH,
	TOKEN_WHILE,
	TOKEN_FUNC,
	TOKEN_VAR,


	TOKEN_ERROR,
	TOKEN_EOF,

} TokenType;

typedef struct {
	TokenType type;
	const char *start;
	int length;
	int line;
} Token;

typedef struct
{
	const char *start;
	const char *current;
	int line;
} Lexer;

void lexer_init(Lexer *lexer, const char *source);
Token lexer_scan_token(Lexer *lexer);

}

#endif

