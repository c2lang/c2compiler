/* Copyright 2013-2016 Bas van den Berg
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

#include "TagReader.h"
#include "FileUtils/FileMap.h"

#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <setjmp.h>

#define MAX_TEXT 128
static const char* TAGS_FILE = "refs";
static const char* OUTPUT_DIR = "output";

namespace tok {
enum TokenKind {
    text,          // any text (until delimiter)
    number,
    colon,
    dest,
    open,
    close,
    eof,
    error,
};

static const char* getTokenName(TokenKind k) {
    switch (k) {
    case text : return "text";
    case number : return "number";
    case colon : return ":";
    case dest : return "->";
    case open : return "{";
    case close : return "}";
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

    void setKind(tok::TokenKind k) {
        kind = k;
    }
    bool is(tok::TokenKind k) const {
        return kind == k;
    }
    bool isNot(tok::TokenKind k) const {
        return kind != k;
    }

    Location getLoc() const {
        return loc;
    }
    void setLocation(Location l) {
        loc = l;
    }

    const char* getText() const {
        return text;
    }
    void setData(const char* d) {
        text = d;
    }

    uint32_t getNumber() const {
        return number;
    }
    void setNumber(uint32_t n) {
        number = n;
    }

    void clear() {
        text = 0;
        number = 0;
    }

    //const char* getName() const { return tok::getTokenName(kind); }

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
            case ' ':
            case '\t':
                advanceToken(1);
                break;
            case '\n':
                current++;
                loc.line++;
                loc.column = 1;
                break;
            case ':':
                Result.setLocation(loc);
                Result.setKind(tok::colon);
                advanceToken(1);
                return;
            case '{':
                Result.setLocation(loc);
                Result.setKind(tok::open);
                advanceToken(1);
                return;
            case '}':
                Result.setLocation(loc);
                Result.setKind(tok::close);
                advanceToken(1);
                return;
            case '-': // expect '->'
                Result.setLocation(loc);
                if (*(current+1) != '>') {
                    sprintf(text, "expected '>' after '-' at %s", loc.str());
                    Result.setKind(tok::error);
                    Result.setData(text);
                    return;
                }
                advanceToken(2);
                Result.setKind(tok::dest);
                return;
            default:
                Result.setLocation(loc);
                if (isdigit(*current)) {
                    parseNumber(Result);
                    return;
                }
                if (*current == '_' || *current == '/' || isalpha(*current)) {        // may also be path: /
                    parseText(Result);
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
        return ch == ':' || ch == '_' || ch == '/';
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
    void parseText(Token& Result) {
        const char* start = current;
        while (*current && IsNameChar(*current)) {
            current++;
        }
        size_t len = current -start;
        assert(len < sizeof(text));
        memcpy(text, start, len);
        text[len] = 0;
        Result.setKind(tok::text);
        Result.setData(text);
        // NOTE: dont use advance() since current is already moved
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


struct TagEntry {
    uint32_t src_line;
    uint32_t src_column;
    uint32_t dst_fileid;
    uint32_t dst_line;
    uint32_t dst_column;
};


struct UserEntry {
    uint32_t src_fileid;
    uint32_t src_line;
    uint32_t src_column;
};
typedef std::vector<UserEntry> Users;


struct TagFile {
    TagFile(const char* filename_, int offset_, int id_)
        : filename(filename_)
        , offset(offset_)
        , id(id_)
    {}

    std::string filename;
    int offset;
    int id;
};


class TagParser {
public:
    TagParser(const std::string& filename)
        : file(filename.c_str())
    {
        memset(errorMsg, 0, sizeof(errorMsg));

        file.open();
        tokenizer.init((const char*)file.region);
        int result = setjmp(jump_err);
        if (result == 0) {
            ConsumeToken();

            parseFiles();
        } // else got error, errorMsg should be set
    }
    const char* getErrorMsg() const {
        return errorMsg;
    }

    const TagFile* findFile(const char* filename) const {
        for (FilesConstIter iter = files.begin(); iter != files.end(); ++iter) {
            const TagFile* tf = *iter;
            if (tf->filename == filename) return tf;
        }
        return 0;
    }
    const TagFile* findFile(uint32_t file_id) const {
        for (FilesConstIter iter = files.begin(); iter != files.end(); ++iter) {
            const TagFile* tf = *iter;
            if (tf->id == (int)file_id) return tf;
        }
        return 0;
    }

    bool findRef(TagEntry& tag, const char* filename, uint32_t line, uint32_t col) {
        const TagFile* tf = findFile(filename);
        if (tf == 0) return false;

        int result = setjmp(jump_err);
        if (result == 0) {
            tokenizer.skipTo(tf->offset);
            ConsumeToken();

            return parseRefs(tag, line, col);
        } // else got error, errorMsg should be set
        fprintf(stderr, "error: %s\n", errorMsg);
        exit(-1);
        return false;
    }
    void findSymbol(Users& users, const char* filename, uint32_t line, uint32_t col) {
        const TagFile* destFile = findFile(filename);
        if (destFile == 0) return;

        // for all files
        int result = setjmp(jump_err);
        if (result == 0) {
            for (FilesConstIter iter = files.begin(); iter != files.end(); ++iter) {
                const TagFile* T = *iter;
                tokenizer.skipTo(T->offset);
                ConsumeToken();
                parseRefsReverse(users, T->id, destFile->id, line, col);
            }
            return;
        } // else got error, errorMsg should be set
        fprintf(stderr, "error: %s\n", errorMsg);
        exit(-1);
    }
private:
    void parseFiles() {
        // syntax: files {
        ExpectAndConsumeText("files");
        ExpectAndConsume(tok::open);
        while (!Tok.is(tok::close)) {
            // syntax: 0 file1.c2 1245
            Expect(tok::number);
            int file_id = Tok.getNumber();
            ConsumeToken();

            Expect(tok::text);
            char filename[MAX_TEXT];
            strcpy(filename, Tok.getText());
            ConsumeToken();

            Expect(tok::number);
            int offset = Tok.getNumber();
            ConsumeToken();

            TagFile* TF = new TagFile(filename, offset, file_id);
            files.push_back(TF);
        }
        ExpectAndConsume(tok::close);
    }
    bool parseRefs(TagEntry& tag, uint32_t line, uint32_t col) {
        // syntax: file <file_id> {
        ExpectAndConsumeText("file");

        Expect(tok::number);
        //int number = Tok.getNumber();
        ConsumeToken();

        ExpectAndConsume(tok::open);
        char symbol[MAX_TEXT];
        while (!Tok.is(tok::close)) {
            parseTag(tag, symbol);
            if (tag.src_line == line) {
                // check if we are somewhere in sybol
                if (col >= tag.src_column && col < tag.src_column+strlen(symbol)) {
                    return true;
                }
            }
        }
        ExpectAndConsume(tok::close);
        return false;
    }
    void parseRefsReverse(Users& users, uint32_t src_fileid, uint32_t fileid, uint32_t line, uint32_t col) {
        // syntax: file <file_id> {
        ExpectAndConsumeText("file");

        Expect(tok::number);
        //int number = Tok.getNumber();
        ConsumeToken();

        ExpectAndConsume(tok::open);
        char symbol[MAX_TEXT];
        TagEntry tag;
        while (!Tok.is(tok::close)) {
            parseTag(tag, symbol);
            if (tag.dst_fileid == fileid && tag.dst_line == line) {
                if (col >= tag.dst_column && col < tag.dst_column+strlen(symbol)) {
                    UserEntry U;
                    U.src_fileid = src_fileid;
                    U.src_line = tag.src_line;
                    U.src_column = tag.src_column;
                    users.push_back(U);
                }
            }
        }
        ExpectAndConsume(tok::close);
    }
    void parseTag(TagEntry& tag, char* symbol) {
        // syntax: 9:5 a -> 0:5:7
        // source part
        Expect(tok::number);
        tag.src_line = Tok.getNumber();
        ConsumeToken();

        ExpectAndConsume(tok::colon);

        Expect(tok::number);
        tag.src_column = Tok.getNumber();
        ConsumeToken();

        // symbol
        Expect(tok::text);
        strcpy(symbol, Tok.getText());
        ConsumeToken();

        ExpectAndConsume(tok::dest);

        // dest part
        Expect(tok::number);
        tag.dst_fileid = Tok.getNumber();
        ConsumeToken();

        ExpectAndConsume(tok::colon);

        Expect(tok::number);
        tag.dst_line = Tok.getNumber();
        ConsumeToken();

        ExpectAndConsume(tok::colon);

        Expect(tok::number);
        tag.dst_column = Tok.getNumber();
        ConsumeToken();
    }
    Location ConsumeToken() {
        prev = Tok.getLoc();
        tokenizer.Lex(Tok);
#if 0
        {
            printf("  %8s at %s", Tok.getName(), Tok.getLoc().str());
            if (Tok.is(tok::number)) printf("  %d", Tok.getNumber());
            if (Tok.is(tok::text)) printf("  %s", Tok.getText());
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

    void Expect(tok::TokenKind k) {
        if (Tok.isNot(k)) {
            sprintf(errorMsg, "expected '%s' at %s", getTokenName(k), Tok.getLoc().str());
            longjmp(jump_err, 1);
        }
    }

    typedef std::vector<TagFile*> Files;
    typedef Files::const_iterator FilesConstIter;
    Files files;

    C2::FileMap file;
    char errorMsg[256];

    Tokenizer tokenizer;
    Token Tok;
    Location prev;

    jmp_buf jump_err;
};


TagReader::TagReader(const char* target_)
    : target(target_)
{
    findRefs();
    if (refFiles.empty()) {
        printf("error: cannot find ref files\n");
        exit(-1);
    }
}

void TagReader::findRefs() {
    char fullname[PATH_MAX];
    if (target) {
        sprintf(fullname, "%s/%s/%s", OUTPUT_DIR, target, TAGS_FILE);
        struct stat statbuf;
        if (stat(fullname, &statbuf) == 0) {
            refFiles.push_back(fullname);
        }
        return;
    }
    // check for output dir
    DIR* dir = opendir(OUTPUT_DIR);
    if (dir == NULL) {
        fprintf(stderr, "error: cannot open output dir\n");
        exit(-1);
    }
    struct dirent* entry = readdir(dir);
    while (entry != 0) {
        switch (entry->d_type) {
        case DT_DIR:
        {
            // check for output/<target>/refs
            if (entry->d_name[0] == '.') break;
            sprintf(fullname, "%s/%s/%s", OUTPUT_DIR,  entry->d_name, TAGS_FILE);
            struct stat statbuf;
            if (stat(fullname, &statbuf) == 0) {
                refFiles.push_back(fullname);
            }
            break;
        }
        default:
            // ignore
            break;
        }
        entry = readdir(dir);
    }
}

unsigned TagReader::find(const char* filename, uint32_t line, uint32_t col) {
    results.clear();

    for (unsigned i=0; i<refFiles.size(); ++i) {
        TagParser parser(refFiles[i]);
        const char* errorMsg = parser.getErrorMsg();
        if (errorMsg[0] != 0) {
            fprintf(stderr, "Error: %s\n", errorMsg);
            continue;
        }

        TagEntry tag;
        if (parser.findRef(tag, filename, line, col)) {
            addResult(parser.findFile(tag.dst_fileid), tag.dst_line, tag.dst_column);
        }
    }
    return results.size();
}

unsigned TagReader::findReverse(const char* filename, uint32_t line, uint32_t col) {
    results.clear();

    for (unsigned i=0; i<refFiles.size(); ++i) {
        TagParser parser(refFiles[i]);
        const char* errorMsg = parser.getErrorMsg();
        if (errorMsg[0] != 0) {
            fprintf(stderr, "Error: %s\n", errorMsg);
            continue;
        }

        Users users;
        parser.findSymbol(users, filename, line, col);
        for (unsigned u=0; u<users.size(); u++) {
            const UserEntry& U = users[u];
            addResult(parser.findFile(U.src_fileid), U.src_line, U.src_column);
        }
    }
    return results.size();
}

bool TagReader::isDuplicate(const Result& res) const {
    for (unsigned i=0; i<results.size(); ++i) {
        if (results[i] == res) return true;
    }
    return false;
}

void TagReader::addResult(const TagFile* file, uint32_t line, uint32_t column) {
    assert(file);

    Result res;
    res.filename = file->filename;
    res.line = line;
    res.column = column;

    if (!isDuplicate(res)) results.push_back(res);
}

