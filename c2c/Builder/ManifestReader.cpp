/* Copyright 2013-2015 Bas van den Berg
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

#include "Builder/ManifestReader.h"
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

#define MAX_TEXT 128

using namespace C2;

namespace tok {
enum TokenKind {
    word,
    text,          // any text (until delimiter)
    number,
    kw_true,
    kw_false,
    lbrace,
    lbrace2,
    rbrace,
    rbrace2,
    equals,
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
    case rbrace  : return "[";
    case rbrace2 : return "]]";
    case equals  : return "=";
    case eof : return "eof";
    case error : return "error";
    }
    return "?";
}

}

class Location {
public:
    Location(unsigned l, unsigned c) : line(l), column(c) {}
    Location() : line(0), column(0) {}

    const char* str() const {
        static char msg[32];
        sprintf(msg, " at line %d:%d", line, column);
        return msg;
    }

    uint32_t line;
    uint32_t column;
};


class Token {
public:
    Token() : kind(tok::eof), text(0), number(0) {}

    tok::TokenKind getKind() const { return kind; }
    void setKind(tok::TokenKind k) { kind = k; }
    bool is(tok::TokenKind k) const { return kind == k; }
    bool isNot(tok::TokenKind k) const { return kind != k; }

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

    const char* getName() const { return tok::getTokenName(kind); }

private:
    Location loc;
    tok::TokenKind kind;
    const char* text;
    uint32_t number;
};

class Tokenizer {
public:
    Tokenizer() : dataStart(0), current(0) , loc(1, 1) {}
    void init(const char* input) {
        dataStart = input;
        current = input;
    }
    void skipTo(unsigned offset) {
        current = dataStart + offset;
        loc.line = 1;   // from offset
        loc.column = 1;
    }
    void Lex(Token& Result) {
        Result.clear();

        while (1) {
            switch (*current) {
            case 0:
                Result.setLocation(loc);
                Result.setKind(tok::eof);
                return;
            case '#':
                if (loc.column != 1) {
                    sprintf(text, "unexpected '#' after line start at %s", loc.str());
                    Result.setKind(tok::error);
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
                Result.setKind(tok::equals);
                advanceToken(1);
                return;
            case '[':
                Result.setLocation(loc);
                if (current[1] == '[') {
                    advanceToken(2);
                    Result.setKind(tok::lbrace2);
                } else {
                    advanceToken(1);
                    Result.setKind(tok::lbrace);
                }
                return;
            case ']':
                Result.setLocation(loc);
                if (current[1] == ']') {
                    advanceToken(2);
                    Result.setKind(tok::rbrace2);
                } else {
                    advanceToken(1);
                    Result.setKind(tok::rbrace);
                }
                return;
            case '"':
                parseText(Result);
                return;
            default:
                // word, text or number
                Result.setLocation(loc);
                if (isdigit(*current)) {
                    parseNumber(Result);
                    return;
                }
                if (*current == '_' || *current == '/' || isalpha(*current)) {        // may also be path: /
                    parseName(Result);
                    return;
                }
                sprintf(text, "unexpected char '%c' at %s", *current, loc.str());
                Result.setKind(tok::error);
                Result.setData(text);
                return;
            }
        }
    }
private:
    inline bool IsNameStartChar( unsigned char ch ) {
        if ( ch >= 128 ) {
            // This is a heuristic guess in attempt to not implement Unicode-aware isalpha()
            return true;
        }
        if ( isalpha( ch ) ) {
            return true;
        }
        return ch == '_' || ch == '/';
    }
    inline bool IsNameChar( unsigned char ch ) {
        return IsNameStartChar( ch )
               || isdigit( ch )
               || ch == '.'
               || ch == '-'
               || ch == '_';
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
        // TODO escape chars
        // TODO handle newlines
        advanceToken(1);
        const char* start = current;
        while (*current && *current != '"') {
            current++;
        }
        size_t len = current - start;
        assert(len < sizeof(text));
        memcpy(text, start, len);
        text[len] = 0;
        Result.setKind(tok::text);
        Result.setData(text);
        // NOTE: dont use advance() since current is already moved
        loc.column += len;
        advanceToken(1);
    }
    void parseName(Token& Result) {
        const char* start = current;
        while (*current && IsNameStartChar(*current)) {
            current++;
        }
        size_t len = current -start;
        assert(len < sizeof(text));
        memcpy(text, start, len);
        text[len] = 0;
        Result.setKind(tok::word);
        Result.setData(text);
        loc.column += len;
    }
    void parseNumber(Token& Result) {
        uint32_t number = atoi(current);
        Result.setKind(tok::number);
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
};


class ManifestParser {
public:
    ManifestParser(const std::string& filename, char* errorMsg_, Entries& entries_)
        : _isNative(true)
        , index(-1)
        , file(filename.c_str())
        , errorMsg(errorMsg_)
        , entries(entries_)
    {
        errorMsg[0] = 0;

        file.open();
        tokenizer.init((const char*)file.region);
        int result = setjmp(jump_err);
        if (result == 0) {
            ConsumeToken();

            //while (!Tok.is(tok::eof)) ConsumeToken();

            parseTopLevel();
        } // else got error, errorMsg should be set
    }
    bool isNative() const { return _isNative; }
private:
    void parseTopLevel() {
        // syntax: [library]
        ExpectAndConsume(tok::lbrace);
        ExpectAndConsumeWord("library");
        ExpectAndConsume(tok::rbrace);

        // syntax: language = "C" / "C2"
        ExpectAndConsumeWord("language");
        ExpectAndConsume(tok::equals);
        Expect(tok::text);
        const char* lang = Tok.getText();
        if (strcmp(lang, "C") == 0) {
            _isNative = false;
        } else if (strcmp(lang, "C2") == 0) {
            _isNative = true;
        } else {
            sprintf(errorMsg, "unknown language '%s' at %s", lang, Tok.getLoc().str());
            longjmp(jump_err, 1);
        }
        ConsumeToken();

        while (Tok.isNot(tok::eof)) {
            ExpectAndConsume(tok::lbrace2);
            ExpectAndConsumeWord("modules");
            ExpectAndConsume(tok::rbrace2);
            parseModule();
        }
    }
    void parseModule() {
        index = -1;

        while (Tok.is(tok::word)) {
            // Syntax: key = value
            // key
            Expect(tok::word);
            char key[MAX_TEXT];
            strcpy(key, Tok.getText());
            Location keyLoc = Tok.getLoc();
            ConsumeToken();

            ExpectAndConsume(tok::equals);

            // value
            Expect(tok::text);      // for now only support string values
            char value[MAX_TEXT];
            strcpy(value, Tok.getText());
            ConsumeToken();
            handleKeyValue(key, keyLoc, value);
        }
    }
    void handleKeyValue(const char* key, const Location& keyLoc, const char* value) {
        if (index == -1) {
            if (strcmp(key, "name") == 0) {
                index = entries.size();
                entries.push_back(ManifestEntry(value));
            } else {
                sprintf(errorMsg, "module should start with name attribute at %s", keyLoc.str());
                longjmp(jump_err, 1);
            }
        } else {
            if (strcmp(key, "header") == 0) {
                if (entries[index].headerFile == "") {
                    entries[index].headerFile = value;
                } else {
                    sprintf(errorMsg, "duplicate header entry at %s", keyLoc.str());
                    longjmp(jump_err, 1);
                }
            } else {
                sprintf(errorMsg, "unknown key '%s' at %s", key, keyLoc.str());
                longjmp(jump_err, 1);
            }
        }
    }
    Location ConsumeToken() {
        prev = Tok.getLoc();
        tokenizer.Lex(Tok);
#if 0
        {
            printf("  %8s at %s", Tok.getName(), Tok.getLoc().str());
            if (Tok.is(tok::number)) printf("  %d", Tok.getNumber());
            if (Tok.is(tok::text)) printf("  \"%s\"", Tok.getText());
            if (Tok.is(tok::word)) printf("  %s", Tok.getText());
            printf("\n");
        }
#endif
        if (Tok.is(tok::error)) {
            strcpy(errorMsg, Tok.getText());
            longjmp(jump_err, 1);
        }
        return prev;
    }
    void ExpectAndConsume(tok::TokenKind k) {
        if (Tok.isNot(k)) {
            sprintf(errorMsg, "expected '%s' at %s", getTokenName(k), Tok.getLoc().str());
            longjmp(jump_err, 1);
        }
        ConsumeToken();
    }
    void ExpectAndConsumeWord(const char* text) {
        if (Tok.isNot(tok::word)) {
            sprintf(errorMsg, "expected word token at %s", Tok.getLoc().str());
            longjmp(jump_err, 1);
        }
        const char* got = Tok.getText();
        if (strcmp(text, got) != 0) {
            sprintf(errorMsg, "expected '%s' at %s", text, Tok.getLoc().str());
            longjmp(jump_err, 1);
        }
        ConsumeToken();
    }
    void ExpectAndConsumeText(const char* text) {
        if (Tok.isNot(tok::text)) {
            sprintf(errorMsg, "expected 'text' token at %s", Tok.getLoc().str());
            longjmp(jump_err, 1);
        }
        const char* got = Tok.getText();
        if (strcmp(text, got) != 0) {
            sprintf(errorMsg, "expected '%s' at %s", text, Tok.getLoc().str());
            longjmp(jump_err, 1);
        }
        ConsumeToken();
    }
    bool ExpectAndConsumeBoolean() {
        Expect(tok::text);
        const char* text = Tok.getText();
        bool value = false;
        if (strcmp(text, "true") == 0) {
            value = true;
        } else if (strcmp(text, "false") == 0) {
            value = false;
        } else {
            sprintf(errorMsg, "expected boolean at %s", Tok.getLoc().str());
            longjmp(jump_err, 1);
        }
        ConsumeToken();
        return value;
    }
    void Expect(tok::TokenKind k) {
        if (Tok.isNot(k)) {
            sprintf(errorMsg, "expected '%s' at %s", getTokenName(k), Tok.getLoc().str());
            longjmp(jump_err, 1);
        }
    }
    // Entries stuff
    bool _isNative;
    int index;

    C2::FileMap file;
    Tokenizer tokenizer;
    Token Tok;
    Location prev;
    jmp_buf jump_err;
    char* errorMsg;
    Entries& entries;
};


bool ManifestReader::parse()
{
    ManifestParser parser(filename, errorMsg, entries);
    if (errorMsg[0] != 0) {
        return false;
    }
    _isNative = parser.isNative();
    return true;
}

