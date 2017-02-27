/* Copyright 2013-2017 Bas van den Berg
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
#include "AST/Component.h"

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


static Component::Type str2dep(const char* type) {
    if (strcmp("static", type) == 0) return Component::STATIC_LIB;
    if (strcmp("dynamic", type) == 0) return Component::SHARED_LIB;
    return (Component::Type)-1;
}

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
    case rbrace  : return "[";
    case rbrace2 : return "]]";
    case equals  : return "=";
    case comma   : return ",";
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
    Tokenizer() : dataStart(0), current(0) , loc(1, 1) {
        text[0] = 0;
    }
    void init(const char* input) {
        dataStart = input;
        current = input;
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
            case ',':
                Result.setLocation(loc);
                Result.setKind(tok::comma);
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
    ManifestParser(const std::string& name_,
                   const std::string& filename,
                   char* errorMsg_,
                   Entries& entries_,
                   StringList& deps_,
                   std::string& linkName_)
        : _isNative(true)
        , hasStaticLib(false)
        , hasDynamicLib(false)
        , index(-1)
        , file(filename.c_str())
        , componentName(name_)
        , errorMsg(errorMsg_)
        , entries(entries_)
        , deps(deps_)
        , linkName(linkName_)
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
    bool hasStatic() const { return hasStaticLib; }
    bool hasDynamic() const { return hasDynamicLib; }
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

        ExpectAndConsumeWord("type");
        ExpectAndConsume(tok::equals);
        ExpectAndConsume(tok::lbrace);
        while (Tok.isNot(tok::rbrace)) {
            Expect(tok::text);      // for now only support string values
            char value[MAX_TEXT];
            strcpy(value, Tok.getText());
            if (strcmp(value, "static") == 0) {
                hasStaticLib = true;
            } else if (strcmp(value, "dynamic") == 0) {
                hasDynamicLib = true;
            } else {
                sprintf(errorMsg, "invalid type '%s' %s", value, Tok.getLoc().str());
                longjmp(jump_err, 1);
            }
            ConsumeToken();
            if (Tok.is(tok::comma)) {
                ConsumeToken();
            } else if (Tok.isNot(tok::rbrace)) {
                sprintf(errorMsg, "syntax error %s", Tok.getLoc().str());
                longjmp(jump_err, 1);
            }
            // TODO handle other cases
        }
        ConsumeToken();

        while (Tok.isNot(tok::eof)) {
            if (Tok.is(tok::word)) {
                // linkname = "x" (optional)
                if (strcmp("linkname", Tok.getText()) == 0) {
                    parseLinkName();
                    continue;
                }
            }
            ExpectAndConsume(tok::lbrace2);
            Expect(tok::word);
            if (strcmp("modules", Tok.getText()) == 0) {
                ConsumeToken();
                ExpectAndConsume(tok::rbrace2);
                parseModule();
            } else if (strcmp("deps", Tok.getText()) == 0) {
                ConsumeToken();
                ExpectAndConsume(tok::rbrace2);
                parseDep();
            }
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
            handleModuleKeyValue(key, keyLoc, value);
        }
    }
    void parseLinkName() {
        // TODO duplicate checking
        ExpectAndConsumeWord("linkname");
        ExpectAndConsume(tok::equals);
        Expect(tok::text);
        linkName = Tok.getText();
        ConsumeToken();
    }
    void parseDep() {
        index = -1;
        // Syntax: name = <value>
        ExpectAndConsumeWord("name");
        ExpectAndConsume(tok::equals);
        Expect(tok::text);
        char value[MAX_TEXT];
        strcpy(value, Tok.getText());
        Location valueLoc = ConsumeToken();
        if (componentName == value) {
            sprintf(errorMsg, "self dependency %s", valueLoc.str());
            longjmp(jump_err, 1);
        }

        // type = "static" | "dynamic"
        ExpectAndConsumeWord("type");
        ExpectAndConsume(tok::equals);
        Expect(tok::text);
        Component::Type depType = str2dep(Tok.getText());
        if (depType == (Component::Type)-1) {
            sprintf(errorMsg, "invalid dependency type (supported are static|dynamic) %s", Tok.getLoc().str());
            longjmp(jump_err, 1);
        }
        ConsumeToken();
        handleDep(value, valueLoc, depType);
    }
    void handleModuleKeyValue(const char* key, const Location& keyLoc, const char* value) {
        if (index == -1) {
            if (strcmp(key, "name") == 0) {
                index = entries.size();
                entries.push_back(ManifestEntry(value));
            } else {
                sprintf(errorMsg, "module should start with name attribute at %s", keyLoc.str());
                longjmp(jump_err, 1);
            }
        } else {
            sprintf(errorMsg, "unknown key '%s' at %s", key, keyLoc.str());
            longjmp(jump_err, 1);
        }
    }
    void handleDep(const char* dep, const Location& loc, Component::Type depType) {
        for (unsigned i=0; i<deps.size(); i++) {
            if (deps[i] == dep) {
                sprintf(errorMsg, "duplicate dependency '%s' at %s", dep, loc.str());
                longjmp(jump_err, 1);
            }
        }
        deps.push_back(dep);

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
    void Expect(tok::TokenKind k) {
        if (Tok.isNot(k)) {
            sprintf(errorMsg, "expected '%s' at %s", getTokenName(k), Tok.getLoc().str());
            longjmp(jump_err, 1);
        }
    }
    // Entries stuff
    bool _isNative;
    bool hasStaticLib;
    bool hasDynamicLib;
    int index;

    C2::FileMap file;
    const std::string& componentName;
    Tokenizer tokenizer;
    Token Tok;
    Location prev;
    jmp_buf jump_err;
    char* errorMsg;
    Entries& entries;
    StringList& deps;
    std::string& linkName;
};


bool ManifestReader::parse()
{
    ManifestParser parser(componentName, filename, errorMsg, entries, deps, linkName);
    if (errorMsg[0] != 0) {
        return false;
    }
    _isNative = parser.isNative();
    hasStaticLib = parser.hasStatic();
    hasDynamicLib = parser.hasDynamic();
    return true;
}

