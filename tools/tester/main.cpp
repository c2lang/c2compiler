/* Copyright 2013-2019 Bas van den Berg
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

/*
    Syntax:
    .c2:
        (optional) // @warnings{..}
        (optional) // @skip
        (optional) // @target{target-triplet}
    .c2t:
        (required) // @recipe bin/lib shared/static
        (optional) // @skip
        (required) // @file{filename}
        (optional) // @expect{filename}
        (optional) // @expect_lines(filename}
*/

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <map>
#include <list>
#include <vector>
#include <string>

#include "FileUtils/FileMap.h"
#include "Utils/StringBuilder.h"
#include "Utils/color.h"

#define COL_ERROR ANSI_BRED
#define COL_SKIP  ANSI_BCYAN
#define COL_OK    ANSI_GREEN
#define COL_DEBUG ANSI_BMAGENTA

#define MAX_LINE 512
#define MAX_THREADS 32

//#define DEBUG

using namespace C2;

static int color_output = 1;
static const char* c2c_cmd = "build/c2c/c2c";
static char* cwd;
static bool runSkipped;

class Test {
public:
    Test(const char* filename_)
        : filename(strdup(filename_))
        , next(0)
    {}
    ~Test() { free(filename); }

    char* filename;
    Test* next;
};

class TestQueue {
public:
    TestQueue()
        : head(0)
        , tail(0)
        , cur(0)
    {
        pthread_mutex_init(&lock, 0);
    }
    ~TestQueue() {
        while (head) {
            Test* next = head->next;
            delete head;
            head = next;
        }
    }

    void add(const char* filename) {
        Test* t = new Test(filename);
        pthread_mutex_lock(&lock);
        if (tail) {
            tail->next = t;
        } else {
            head = t;
            cur = t;
        }
        tail = t;
        count++;
        pthread_mutex_unlock(&lock);
    }
    const char* get() {
        const char* filename = 0;
        pthread_mutex_lock(&lock);
        if (cur) {
            filename = cur->filename;
            cur = cur->next;
        }
        pthread_mutex_unlock(&lock);
        return filename;
    }
    void dump() const {
        printf("%u tests\n", count);
    }
private:
    pthread_mutex_t lock;
    Test* head;
    Test* tail;
    Test* cur;
    unsigned count;
};

#ifdef DEBUG
static void debug(const char* format, ...) {
    char buffer[4096];
    va_list(Args);
    va_start(Args, format);
    //int len = vsprintf(buffer, format, Args);
    vsprintf(buffer, format, Args);
    //(void*)len; // silence warning
    va_end(Args);
    if (color_output) fprintf(stderr, COL_DEBUG"%s" ANSI_NORMAL"\n", buffer);
    else printf("%s\n", buffer);
}
#else
static void debug(const char* format, ...) {}
#endif

static void skipWhitespace(const char** start, const char* end) {
    const char* cp = *start;
    while (cp != end && isblank(*cp)) cp++;
    *start = cp;
}

static void color_print(const char* color, const char* format, ...) {
    char buffer[1024];
    va_list(Args);
    va_start(Args, format);
    vsnprintf(buffer, sizeof(buffer), format, Args);
    va_end(Args);

    if (color_output) printf("%s%s" ANSI_NORMAL"\n", color, buffer);
    else printf("%s\n", buffer);
}

static void color_print2(StringBuilder& output ,const char* color, const char* format, ...) {
    char buffer[1024];
    va_list(Args);
    va_start(Args, format);
    vsnprintf(buffer, sizeof(buffer), format, Args);
    va_end(Args);

    if (color_output) output.print("%s%s" ANSI_NORMAL"\n", color, buffer);
    else output.print("%s\n", buffer);
}

static uint64_t getCurrentTime() {
    struct timeval now;
    gettimeofday(&now, 0);
    uint64_t now64 = now.tv_sec;
    now64 *= 1000000;
    now64 += now.tv_usec;
    return now64;
}

static int endsWith(const char* name, const char* tail) {
    int len = strlen(name);
    int tlen = strlen(tail);
    if (tlen > len + 1) return 0;
    return strcmp(name + len - tlen, tail) == 0;
}

static const char* find(const char* start, const char* end, const char* text) {
    const char* cp = start;
    while (cp != end) {
        if (strncmp(cp, text, strlen(text)) == 0) return cp;
        cp++;
    }
    return 0;
}

class ExpectFile {
public:
    enum Mode {
        ATLEAST,
        COMPLETE,
    };
    ExpectFile(const std::string& name, Mode m)
        : filename(name), mode(m)
        , lineStart(0), lineEnd(0) {}

    void addLine(const char* start, const char* end) {
        if (strncmp(start, "//", 2) == 0) return;   // ignore comments

        skipWhitespace(&start, end);
        // TODO strip trailing whitespace
        std::string line_str(start, end);
        lines.push_back(line_str);
    }
    bool check(StringBuilder& output, const std::string& basedir) {
        debug("checking %s %s\n", basedir.c_str(), filename.c_str());
        std::string fullname = basedir + filename;
        // check if file exists
        struct stat statbuf;
        int err = stat(fullname.c_str(), &statbuf);
        if (err) {
            color_print2(output, COL_ERROR, "  missing expected file '%s' (%s)",
                        filename.c_str(), fullname.c_str());
            return false;
        }
        FileMap file(fullname.c_str());
        file.open();

        if (lines.size() == 0) return true;

        lineStart = (const char*)file.region;
        lineEnd = 0;
        unsigned expIndex = 0;
        const char* expectedLine = lines[expIndex].c_str();
#ifdef DEBUG
        color_print2(output, ANSI_GREEN, "exp '%s'", expectedLine);
#endif
        while (1) {
            // find next real line
            const char* line = nextRealLine();
            if (line == 0) break;
            if (line[0] == 0) continue;     // ignore empty lines
            if (line[0] == '/' && line[1] == '/') continue; // ignore comments
#ifdef DEBUG
            color_print2(output, ANSI_YELLOW, "got '%s'", line);
#endif

            if (!expectedLine) {
                color_print2(output, COL_ERROR, "  in file %s: unexpected line '%s'", filename.c_str(), line);
                return false;
            }

            // TODO dont copy in nextRealLine, but do memcmp on substring
            if (strcmp(expectedLine, line) == 0) {
#ifdef DEBUG
                color_print2(output, ANSI_CYAN, "match");
#endif
                expIndex++;
                if (expIndex == lines.size()) expectedLine = 0;
                else expectedLine = lines[expIndex].c_str();
#ifdef DEBUG
                color_print2(output, ANSI_GREEN, "exp '%s'", expectedLine ? expectedLine : "<none>");
#endif
                if (!expectedLine && mode != COMPLETE) return true;
            } else {
                if (mode == COMPLETE) {
                    color_print2(output, COL_ERROR, "  in file %s:\n  expected '%s'\n       got '%s'", filename.c_str(), expectedLine, line);
                    return false;
                }
            }
        }
        if (expectedLine != 0) {
            color_print2(output, COL_ERROR, "  in file %s: expected '%s'", filename.c_str(), expectedLine);
            return false;
        }
        return true;
    }
private:
    const char* nextRealLine() {
        assert(lineStart);
        if (lineEnd == 0) {
            lineEnd = lineStart;
        } else {
            if (*lineEnd == 0) return 0;
            lineStart = lineEnd + 1;
            lineEnd = lineStart;
        }

        while (*lineEnd != 0) {
            if (*lineEnd == '\n') break;
            lineEnd++;
        }
        skipWhitespace(&lineStart, lineEnd);
        int size = lineEnd - lineStart;
        assert(size < MAX_LINE);
        memcpy(line_buf, lineStart, size);
        while (isblank(line_buf[size-1])) size--;
        line_buf[size] = 0;
        return line_buf;
    }

    std::string filename;
    Mode mode;
    const char* lineStart;
    const char* lineEnd;
    char line_buf[MAX_LINE];

    typedef std::vector<std::string> Lines;
    Lines lines;
};


class IssueDb {
public:
    IssueDb(StringBuilder& output_, File& file_, bool single_, const char* tmp_dir_)
        : file(file_)
        , currentExpect(0)
        , output(output_)
        , single(single_)
        , line_nr(0)
        , mode(OUTSIDE)
        , current_file("")
        , file_start(0)
        , line_offset(0)
        , hasErrors(false)
        , skip(false)
        , cur(0)
        , tmp_dir(tmp_dir_)
    {
        if (single) {
            current_file = cwd;
            current_file  += '/' + file.filename;
        }
    }

    bool parse();

    void testFile();

    bool haveErrors() const {
        return hasErrors;
    }

    void printIssues() const {
        for (IssuesConstIter iter = errors.begin(); iter != errors.end(); ++iter) {
            output.setColor(COL_ERROR);
            output.print("  expected error '%s' at %s:%d",
                        iter->msg.c_str(), iter->filename.c_str(), iter->line_nr);
            output.setColor(COL_NORM);
        }
        for (IssuesConstIter iter = warnings.begin(); iter != warnings.end(); ++iter) {
            output.setColor(COL_ERROR);
            output.print("  expected warning '%s' at %s:%d",
                        iter->msg.c_str(), iter->filename.c_str(), iter->line_nr);
            output.setColor(COL_NORM);
        }
        for (IssuesConstIter iter = notes.begin(); iter != notes.end(); ++iter) {
            output.setColor(COL_ERROR);
            output.print("  expected note '%s' at %s:%d",
                        iter->msg.c_str(), iter->filename.c_str(), iter->line_nr);
            output.setColor(COL_NORM);
        }
    }
private:
    void parseLine(const char* start, const char* end);

    // NEW API
    bool parseRecipe();
    bool parseFile();
    bool parseExpect();
    bool parseKeyword();
    bool parseOuter();
    void skipLine();
    const char* findEndOfLine();
    const char* readWord();
    const char* readLine();
    const char* readUntil(char delim);


    // OLD API
    void parseLineOutside(const char* start, const char* end);
    void parseLineFile(const char* start, const char* end);
    void parseLineExpect(const char* start, const char* end);
    void parseTags(const char* start, const char* end);

    void writeFile(const char* name, const char* content, unsigned size) {
        char fullname[128];
        sprintf(fullname, "%s/%s", tmp_dir, name);
        int fd = open(fullname, O_WRONLY | O_CREAT, 0660);
        if (fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        ssize_t written = write(fd, content, size);
        if (written != (ssize_t)size) {
            perror("write");
            exit(EXIT_FAILURE);
        }
        close(fd);
    }

    void error(const char* msg) {
        color_print2(output, ANSI_BRED, "%s:%d: %s", file.filename.c_str(), line_nr, msg);
        hasErrors = true;
    }

    void checkDiagnosticLine(const char* line);
    void checkErrors(const char* buffer, unsigned size);
    void checkExpectedFiles();

    void matchNote(const char* filename, unsigned linenr, const char* msg) {
        for (IssuesIter iter = notes.begin(); iter != notes.end(); ++iter) {
            if (iter->line_nr != linenr) continue;
            if (iter->filename == filename) {
                if (iter->msg != msg) {
                    color_print2(output, COL_ERROR, "  wrong note at %s:%d:", filename, linenr);
                    color_print2(output, COL_ERROR, "     expected: %s", iter->msg.c_str());
                    color_print2(output, COL_ERROR, "     got: %s", msg);
                    hasErrors = true;
                }
                notes.erase(iter);
                return;
            }
        }
        // not expected
        color_print2(output, COL_ERROR, "  unexpected note on line %d: %s", linenr, msg);
        hasErrors = true;
    }

    void matchWarning(const char* filename, unsigned linenr, const char* msg) {
        for (IssuesIter iter = warnings.begin(); iter != warnings.end(); ++iter) {
            if (iter->line_nr != linenr) continue;
            if (iter->filename == filename) {
                if (iter->msg != msg) {
                    color_print2(output, COL_ERROR, "  wrong warning at %s:%d:", filename, linenr);
                    color_print2(output, COL_ERROR, "     expected: %s", iter->msg.c_str());
                    color_print2(output, COL_ERROR, "     got: %s", msg);
                    hasErrors = true;
                }
                warnings.erase(iter);
                return;
            }
        }
        // not expected
        color_print2(output, COL_ERROR, "  unexpected warning on line %d: %s", linenr, msg);
        hasErrors = true;
    }

    void matchError(const char* filename, unsigned linenr, const char* msg) {
        for (IssuesIter iter = errors.begin(); iter != errors.end(); ++iter) {
            if (iter->line_nr != linenr) continue;
            if (iter->filename == filename) {
                if (iter->msg != msg) {
                    color_print2(output, COL_ERROR, "  wrong error at %s:%d:", filename, linenr);
                    color_print2(output, COL_ERROR, "     expected: %s", iter->msg.c_str());
                    color_print2(output, COL_ERROR, "     got: %s", msg);
                    hasErrors = true;
                }
                errors.erase(iter);
                return;
            }
        }
        // not expected
        color_print2(output, COL_ERROR, "  unexpected error on line %d: %s", linenr, msg);
        fflush(stdout);
        hasErrors = true;
    }

    struct Issue {
        Issue(const std::string& name, unsigned line, const char* msg_)
            : filename(name)
            , line_nr(line)
            , msg(msg_)
        {}
        std::string filename;
        unsigned line_nr;
        std::string msg;
    };

    File& file;
    typedef std::list<Issue> Issues;
    typedef Issues::const_iterator IssuesConstIter;
    typedef Issues::iterator IssuesIter;
    Issues errors;
    Issues warnings;
    Issues notes;

    typedef std::vector<ExpectFile*> ExpectFiles;
    ExpectFiles expectedFiles;
    ExpectFile* currentExpect;

    StringBuilder& output;
    bool single;
    unsigned line_nr;
    enum Mode { OUTSIDE, INFILE, INEXPECTFILE };
    Mode mode;
    std::string current_file;
    const char* file_start;
    unsigned line_offset;
    StringBuilder recipe;
    std::string target;
    bool hasErrors;
    bool skip;
    const char* cur;
    StringBuilder errorMsg;
    const char* tmp_dir;

    char word_buffer[32];
    char line_buffer[MAX_LINE];
    char until_buffer[128];
};

void IssueDb::parseLineExpect(const char* start, const char* end) {
    // if line starts with '// @' stop filemode
    if (strncmp(start, "// @", 4) == 0) {
        currentExpect = 0;
        mode = OUTSIDE;
        parseLine(start, end);
        return;
    }
    assert(currentExpect);
    currentExpect->addLine(start, end);
    // add non-empty lines (stripped of heading+trailing whitespace) to list
    // TODO
}

void IssueDb::parseLineFile(const char* start, const char* end) {

    // if line starts with '// @' stop filemode
    if (strncmp(start, "// @", 4) == 0) {
        assert(file_start);
        const char* file_end = start;
        writeFile(current_file.c_str(), file_start, file_end - file_start);
        file_start = 0;
        mode = OUTSIDE;
        parseLine(start, end);
        return;
    }

    parseTags(start, end);
}

void IssueDb::parseTags(const char* start, const char* end) {
    // if finding '// @' somewhere else, it's a note/warning/error
    const char* cp = find(start, end, "// ");
    if (!cp) return;
    cp += 3;    // skip "// ";

    // search for @
    if (*cp != '@') return;
    cp++;   // skip @

    enum Type { ERROR, WARNING, NOTE };
    Type type = ERROR;
    if (strncmp(cp, "error{", 6) == 0) {
        cp += 6;
        type = ERROR;
        goto parse_msg;
    }
    if (strncmp(cp, "warning{", 8) == 0) {
        cp += 8;
        type = WARNING;
        goto parse_msg;
    }
    if (strncmp(cp, "note{", 5) == 0) {
        cp += 5;
        type = NOTE;
        goto parse_msg;
    }
    error("unknown note/warning/error tag");
    return;
parse_msg:
    // todo extract to function
    const char* msg_start = cp;
    while (*cp != '}') {
        if (cp == end) {
            error("missing '}'");
            exit(EXIT_FAILURE);
        }
        cp++;
    }
    char msg[128];
    memcpy(msg, msg_start, cp-msg_start);
    msg[cp-msg_start] = 0;
    switch (type) {
    case ERROR:
#ifdef DEBUG
        printf(ANSI_BLUE"  expecting error '%s' at %d" ANSI_NORMAL"\n", msg, line_nr - line_offset);
#endif
        errors.push_back(Issue(current_file, line_nr - line_offset, msg));
        break;
    case WARNING:
#ifdef DEBUG
        printf(ANSI_BLUE"  expecting warning '%s' at %d " ANSI_NORMAL"\n", msg, line_nr - line_offset);
#endif
        warnings.push_back(Issue(current_file, line_nr - line_offset, msg));
        break;
    case NOTE:
#ifdef DEBUG
        printf(ANSI_BLUE"  expecting note '%s' at %d" ANSI_NORMAL"\n", msg, line_nr - line_offset);
#endif
        notes.push_back(Issue(current_file, line_nr - line_offset, msg));
        break;
    }
}

void IssueDb::parseLineOutside(const char* start, const char* end) {
    const char* cp = start;
    skipWhitespace(&cp, end);
    if (cp == end) return;

    // TODO if !single mode, only accept tags or comments
    if (!single && strncmp(cp, "// ", 3) != 0) {
        error("unexpected line");
        return;
    }

    if (strncmp(cp, "// @", 4) == 0) {
        cp += 4;
        if (strncmp(cp, "warnings{", 9) == 0) {
            cp += 9;

            // TODO extract parsing on name
            const char* name_start = cp;
            while (*cp != '}') {
                if (cp == end) {
                    error("missing '}'");
                    exit(EXIT_FAILURE);
                }
                cp++;
            }
            std::string name(name_start, cp-name_start);
            recipe << "    $warnings " << name << '\n';
        } else if (strncmp(cp, "target{", 7) == 0) {
            if (!single) {
                error("keyword 'target' only allowed in .c2 files");
                return;
            }
            cp += 7;

            const char* target_start = cp;
            while (*cp != '}') {
                if (cp == end) {
                    error("missing '}'");
                    exit(EXIT_FAILURE);
                }
                cp++;
            }
            std::string target_(target_start, cp-target_start);
            target = target_;
        } else if (strncmp(cp, "file{", 5) == 0) {
            if (single) {
                error("invalid @file tag in single test");
                return;
            }
            cp += 5;
            // parse name
            const char* name_start = cp;
            while (*cp != '}') {
                if (cp == end) {
                    error("missing '}'");
                    exit(EXIT_FAILURE);
                }
                cp++;
            }
            std::string name(name_start, cp-name_start);
            if (!endsWith(name.c_str(), ".c2")) name += ".c2";
            current_file = name;
            recipe << "  " << current_file << '\n';
            file_start = end + 1;
            line_offset = line_nr;
            mode = INFILE;
        } else if (strncmp(cp, "expect{", 7) == 0) {
            if (single) {
                error("invalid @expect tag in single test");
                return;
            }
            cp += 7;
            // parse name
            const char* name_start = cp;
            while (*cp != '}') {
                if (cp == end) {
                    error("missing '}'");
                    exit(EXIT_FAILURE);
                }
                cp++;
            }
            std::string name(name_start, cp-name_start);
            currentExpect = new ExpectFile(name, ExpectFile::ATLEAST);
            // TODO check for name duplicates
            expectedFiles.push_back(currentExpect);
            mode = INEXPECTFILE;
        } else if (strncmp(cp, "generate-c", 10) == 0) {
            if (single) {
                error("invalid @generate-c tag in single test");
                return;
            }
            cp += 10;
            // parse args
            char args[128];
            char* out = args;
            while (cp < end) {
                *out++ = *cp++;
            }
            *out = 0;
            recipe << "  $ansi-c " << args << '\n';
        } else {
            error("unknown tag");
        }
        return;
    }
    parseTags(start, end);
}


void IssueDb::parseLine(const char* start, const char* end) {
    switch (mode) {
    case OUTSIDE:
        parseLineOutside(start, end);
        break;
    case INFILE:
        parseLineFile(start, end);
        break;
    case INEXPECTFILE:
        parseLineExpect(start, end);
        break;
    }
}

bool IssueDb::parseRecipe() {
    if (strncmp(cur, "bin", 3) == 0) {
        recipe << "executable test\n";
    } else if (strncmp(cur, "lib", 3) == 0) {
        cur += 4;
        // Syntax lib shared/static
        const char* libtype = readWord();
        if (strcmp(libtype, "shared") == 0 || strcmp(libtype, "static") == 0) {
        } else {
            errorMsg << "unknown library type '" << libtype << "'";
            return false;
        }
        recipe << "lib test " << libtype << "\n";
    } else {
        errorMsg << "unknown target type '" << readWord() << "'";
        return false;
    }
    skipLine();
    // TODO check that each line starts with $? (after optional whitespace)
    while (1) {
        if (*cur == 0 || strncmp(cur, "// @", 4) == 0) return true;

        if (*cur != '\n') {
            recipe << readLine() << '\n';
        }
        skipLine();
    }

    return true;
}

bool IssueDb::parseFile() {
    // Syntax file{name}
    if (*cur != '{') {
        errorMsg << "expected { after file";
        return false;
    }
    cur++;

    std::string filename = readUntil('}');
    if (filename.empty()) {
        errorMsg << "expected filename";
        return false;
    }
    if (!endsWith(filename.c_str(), ".c2")) filename += ".c2";
    recipe << "    " << filename << '\n';
    line_offset = line_nr;
    current_file = filename;
    skipLine();
    const char* start = cur;
    while (1) {
        if (*cur == 0 || strncmp(cur, "// @", 4) == 0) {
            const char* end = cur;
            writeFile(filename.c_str(), start, end - start);
            break;
        }
        parseTags(cur, findEndOfLine());
        skipLine();
    }
    return true;
}

bool IssueDb::parseExpect() {
    // Syntax expect{mode, name}
    if (*cur != '{') {
        errorMsg << "expected { after expect";
        return false;
    }
    cur++;
    const char* modeStr = readWord();
    ExpectFile::Mode em = ExpectFile::ATLEAST;
    if (strcmp(modeStr, "atleast") == 0) {
        em = ExpectFile::ATLEAST;
    } else if (strcmp(modeStr, "complete") == 0) {
        em = ExpectFile::COMPLETE;
    } else {
        errorMsg << "unknown mode: " << modeStr;
        return false;
    }

    cur += strlen(modeStr);
    if (*cur != ',') {
        errorMsg << "expected comma";
        return false;
    }
    cur++;

    while (*cur == ' ') cur++;

    std::string filename = readUntil('}');
    if (filename.empty()) {
        errorMsg << "expected filename";
        return false;
    }
    skipLine();

    currentExpect = new ExpectFile(filename, em);
    // TODO check for name duplicates
    expectedFiles.push_back(currentExpect);
    while (*cur != 0) {
        if (strncmp(cur, "// @", 4) == 0) {
            break;
        }

        if (*cur == '\n') {
            skipLine();
            continue;
        }

        const char* end = cur;
        while (*end != 0 && *end != '\n') end++;
        currentExpect->addLine(cur, end);
        skipLine();
    }
    currentExpect = 0;
    return true;
}

bool IssueDb::parseKeyword() {
    // NOTE: cur points to start of keyword after // @
    const char* keyword = readWord();

    if (strcmp(keyword, "skip") == 0) {
        if (!runSkipped) skip = true;
        return true;
    } else if (strcmp(keyword, "recipe") == 0) {
        if (single) {
            errorMsg << "keyword 'recipe' only allowed in .c2t files";
            return false;
        }
        cur += 7;
        return parseRecipe();
    } else if (strcmp(keyword, "warnings") == 0) {
        if (!single) {
            errorMsg << "keyword 'warnings' only allowed in .c2 files";
            return false;
        }
        recipe << "\t$warnings " << readLine() << '\n';
        skipLine();
    } else if (strcmp(keyword, "file") == 0) {
        if (single) {
            errorMsg << "keyword 'file' only allowed in .c2t files";
            return false;
        }
        cur += 4;
        return parseFile();
    } else if (strcmp(keyword, "expect") == 0) {
        if (single) {
            errorMsg << "keyword 'expect' only allowed in .c2t files";
            return false;
        }
        cur += 6;
        return parseExpect();
    } else {
        errorMsg << "unknown keyword '" << keyword << "'";
        return false;
    }
    return true;
}

const char* IssueDb::findEndOfLine() {
    const char* cp = cur;
    while (*cp != 0 && *cp != '\n') {
        cp++;
    }
    return cp;
}

const char* IssueDb::readWord() {
    const char* cp = cur;
    while (*cp != 0 && cp - cur < 31) {
        if ((*cp < 'a' || *cp > 'z') && *cp != '-' && *cp != '_') break;
        cp++;
    }
    int len = cp - cur;
    memcpy(word_buffer, cur, len);
    word_buffer[len] = 0;
    return word_buffer;
}

const char* IssueDb::readLine() {
    const char* cp = cur;
    while (*cp != 0 && cp - cur < MAX_LINE) {
        if (*cp == 0 || *cp == '\n') break;
        cp++;
    }
    int len = cp - cur;
    memcpy(line_buffer, cur, len);
    line_buffer[len] = 0;
    return line_buffer;
}

const char* IssueDb::readUntil(char delim) {
    const char* cp = cur;
    while (1) {
        if (*cp == 0) return 0;
        if (*cp == delim) break;
        if (cp - cur > 127) return 0;
        cp++;
    }
    int len = cp - cur;
    memcpy(until_buffer, cur, len);
    until_buffer[len] = 0;
    return until_buffer;
}

// returns if OK
bool IssueDb::parseOuter() {
    // NOTE: cur always points to beginning of line

    while (*cur != 0) {
        if (*cur == '\n') {
            skipLine();
            continue;
        }
        // search for lines starting with // @..
        if (strncmp(cur, "// @", 4) == 0) {
            cur += 4;
            if (!parseKeyword()) return false;
            continue;
        }

        // TODO give error on non-empty lines
        skipLine();
    }
    return true;
}

void IssueDb::skipLine() {
    while (*cur != 0) {
        if (*cur == '\n') {
            line_nr++;
            cur++;
            return;
        }
        cur++;
    }
}

bool IssueDb::parse() {
    const char* cp = (const char*) file.region;
    cur = cp;
    line_nr = 1;
    if (single) {
        const char* end = cp + file.size;
        const char* line_start = cp;
        recipe << "executable test\n";
        recipe << "  $warnings no-unused-module\n";
        recipe << "  $generate-c skip\n";
        recipe << "  " << current_file << '\n';

        bool hasSkip = (strncmp(cp, "// @skip", 8) == 0);
        if (runSkipped != hasSkip) return true;
        while (cp != end) {
            while (*cp != '\n' && cp != end) cp++;
            if (cp != line_start) parseLine(line_start, cp);
            line_nr++;
            if (*cp == '\n') cp++;
            line_start = cp;
        }
        if (!single) {
            if (file_start) {
                const char* file_end = cp;
                writeFile(current_file.c_str(), file_start, file_end- file_start);
            }
        }
        if (target[0] != 0) {
            StringBuilder build;
            build << "target = \"" << target << "\"\n";
            build << "[[libdir]]\n";
            build << "dir = \"$C2_LIBDIR\"\n";
            writeFile("build.toml", build, build.size());
        }
        recipe << "end\n";
        writeFile("recipe.txt", recipe, recipe.size());
        return false;
    } else {
        if (!parseOuter()) {
            fprintf(stderr, ANSI_BYELLOW"Error in recipe: %s on line %d" ANSI_NORMAL"\n", (const char*)errorMsg, line_nr);
            return false;
        }
        recipe << "end\n";
        writeFile("recipe.txt", recipe, recipe.size());
        return skip;
    }

}

void IssueDb::testFile() {
    int pipe_stdout[2];
    if (pipe(pipe_stdout) == -1) {
        perror("pipe");
        exit(1);
    }
    int pipe_stderr[2];
    if (pipe(pipe_stderr) == -1) {
        perror("pipe");
        exit(1);
    }

    // spawn a child to run c2c
    pid_t pid=fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    if (pid==0) { // child
        while ((dup2(pipe_stdout[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
        close(pipe_stdout[1]);
        close(pipe_stdout[0]);
        while ((dup2(pipe_stderr[1], STDERR_FILENO) == -1) && (errno == EINTR)) {}
        close(pipe_stderr[1]);
        close(pipe_stderr[0]);
        execl(c2c_cmd, "c2c", "-d", tmp_dir, "--test", 0);
        perror("execl");
        exit(127); /* only if execv fails */
    }
    else { // parent
        close(pipe_stdout[1]);
        close(pipe_stderr[1]);
        int status = 0;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status)) { // child exited abnormally
            // TODO print pipe_stderr
            output.setColor(COL_ERROR);
            output << "c2c crashed!";
            output.setColor(COL_NORM);
            output << '\n';
            hasErrors = true;
            return;
        }
        // check return code
        int retcode = WEXITSTATUS(status);
        if (retcode == 127) {
            color_print2(output, COL_ERROR, "Error spawning compiler '%s'", c2c_cmd);
            exit(EXIT_FAILURE);
        }
        if (retcode == 254) { // from TODO/FATAL_ERROR macros
            output.setColor(COL_ERROR);
            output << "c2c returned error";
            output.setColor(COL_NORM);
            output << '\n';
            hasErrors = true;
            return;
        }
        // check output
        char buffer[1024*1024];
        while (1) {
            ssize_t count = read(pipe_stderr[0], buffer, sizeof(buffer)-1);
            if (count == -1) {
                if (errno == EINTR) continue;
                perror("read");
                exit(1);
            }
            if (count == 0) break;
            if (count == sizeof(buffer)-1) color_print2(output, COL_ERROR, "Too many error messages for single read!");
            buffer[count] = 0;
            checkErrors(buffer, count);
        }
        close(pipe_stdout[0]);
        close(pipe_stderr[0]);
        wait(0);
        checkExpectedFiles();
        if (!errors.empty() || !warnings.empty()) hasErrors = true;
    }
}

void IssueDb::checkDiagnosticLine(const char* line) {
#ifdef DEBUG
    printf(ANSI_WHITE"line: '%s'" ANSI_NORMAL"\n", line);
#endif
    // line syntax: '<filename>.c2:<linenr>:<offset>: error/warning/note: <msg>\n'
    char filename[128];
    char msg[128];
    int error_line = 0;
    int col = 0;
    memset(filename, 0, sizeof(filename));
    memset(msg, 0, sizeof(msg));

    int res = sscanf(line, "%[^: ]:%d:%d: error: %[^\n]\n", filename, &error_line, &col, msg);
    if (res == 4) {
        // found error
#ifdef DEBUG
        printf(ANSI_CYAN"%s", filename);
        printf("  %d:%d", error_line, col);
        printf("  '%s'" ANSI_NORMAL"\n", msg);
#endif
        matchError(filename, error_line, msg);
    } else {
        res = sscanf(line, "%[^: ]:%d:%d: warning: %[^\n]\n", filename, &error_line, &col, msg);
        if (res == 4) {
            // found warning
#ifdef DEBUG
            printf(ANSI_CYAN"%s", filename);
            printf("  %d:%d", error_line, col);
            printf("  '%s'" ANSI_NORMAL"\n", msg);
#endif
            matchWarning(filename, error_line, msg);
        } else {
            res = sscanf(line, "%[^: ]:%d:%d: note: %[^\n]\n", filename, &error_line, &col, msg);
            if (res == 4) {
                // found note
#ifdef DEBUG
                printf(ANSI_CYAN"%s", filename);
                printf("  %d:%d", error_line, col);
                printf("  '%s'" ANSI_NORMAL"\n", msg);
#endif
                matchNote(filename, error_line, msg);
            }
        }
    }

    if (res == 4) {
        // match msg string and set cp to that to avoid duplicates on empty lines
        const char* found = strstr(line, msg);
        assert(found);
        line = found;
    }
}

void IssueDb::checkErrors(const char* buffer, unsigned size) {
#ifdef DEBUG
    printf(ANSI_MAGENTA"stderr:\n%s" ANSI_NORMAL"\n", buffer);
#endif
    const char* cp = buffer;
    const char* end = cp + size;
    const char* line = cp;
    bool haveColon = false;
    while (cp != end) {
        // cut up into lines
        if (*cp == ':') haveColon = true;
        if (*cp == '\n') {
            char data[512];
            unsigned len = cp - line;
            assert(len < sizeof(data));
            memcpy(data, line, len);
            data[len] = 0;
            if (haveColon) checkDiagnosticLine(data);
            cp++;
            line = cp;
            haveColon = false;
        } else {
            cp++;
        }
    }
}

void IssueDb::checkExpectedFiles() {
    StringBuilder basedir;
    basedir << tmp_dir << "/output/test/";
    for (unsigned i=0; i<expectedFiles.size(); ++i) {
        ExpectFile* E = expectedFiles[i];
        if (!E->check(output, (const char*)basedir)) hasErrors = true;
    }
}

static void handle_file(TestQueue& queue, const char* filename) {
    // TODO filter on .c2 and .c2t
    queue.add(filename);
}

static void handle_dir(TestQueue& queue, const char* path) {
    debug("%s() %s", __func__, path);
    DIR* dir = opendir(path);
    if (dir == 0) {
        color_print(COL_ERROR, "Cannot open dir '%s': %s", path, strerror(errno));
        return;
    }
    struct dirent* dir2 = readdir(dir);
    char temp[MAX_LINE];
    while (dir2 != 0) {
        sprintf(temp, "%s/%s", path, dir2->d_name);
        switch (dir2->d_type) {
        case DT_REG:
            handle_file(queue, temp);
            break;
        case DT_DIR:
            if (strcmp(dir2->d_name, ".") != 0 && strcmp(dir2->d_name, "..") != 0) {
                handle_dir(queue, temp);
            }
            break;
        default:
            break;
        }
        dir2 = readdir(dir);
    }
    closedir(dir);
}


class Tester {
public:
    unsigned numtests;
    unsigned numerrors;
    unsigned numskipped;

    Tester(unsigned idx, TestQueue& queue_)
        : numtests(0)
        , numerrors(0)
        , numskipped(0)
        , index(idx)
        , queue(queue_)
    {
        sprintf(tmp_dir, "/tmp/tester%u", index);
        pthread_create(&thread, 0, thread_main, this);
    }
    ~Tester() {}

    void join() {
        pthread_join(thread, 0);
    }
private:
    void run_test(const char* filename) {
        debug("[%u] %s() %s", index, __func__, filename);
        bool single = true;
        if (endsWith(filename, ".c2")) {
            single = true;
        } else if (endsWith(filename, ".c2t")) {
            single = false;
        } else {
            return;
        }

        // setup dir
        // temp, just delete this way
        char cmd[64];
        sprintf(cmd, "rm -rf %s", tmp_dir);
        int err = system(cmd);
        if (err != 0 && errno != 10) {
            int saved = errno;
            fprintf(stderr, "Error running '%s': %s, %d\n", cmd, strerror(errno), saved);
            exit(EXIT_FAILURE);
        }
        // create test dir
        err = mkdir(tmp_dir, 0777);
        if (err) {
            perror("mkdir");
            exit(EXIT_FAILURE);
        }

        numtests++;

        StringBuilder buf(4096);
        buf.enableColor(true);
        buf.print("%s ", filename);
        FileMap file(filename);
        file.open();
        IssueDb db(buf, file, single, tmp_dir);

        bool skip = db.parse();
        if (skip) {
            numskipped++;
            buf.clear();
            color_print2(buf, COL_SKIP, "[%02u] %s SKIPPED", index, filename);
            printf("%s", buf.c_str());
            return;
        }

        buf << '\n';

        if (!db.haveErrors()) {
            db.testFile();
            db.printIssues();
        }
        printf("%s", buf.c_str());

        if (db.haveErrors()) {
            numerrors++;
        }
    }
    void run() {
        while (1) {
            const char* filename = queue.get();
            if (!filename) break;
            run_test(filename);
        }
    }
    static void* thread_main(void* arg) {
        Tester* tester = reinterpret_cast<Tester*>(arg);
        tester->run();
        return 0;
    }

    unsigned index;
    TestQueue& queue;
    pthread_t thread;
    char tmp_dir[32];
};

static int online_cpus()
{
    long ncpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (ncpus > 0) return (int)ncpus;
    return 1;
}

static void usage(const char* name) {
    printf("Usage: %s [file/dir] <options>\n", name);
    printf("    -s    only run skipped tests\n");
    printf("    -n    no multi-threading\n");
    exit(EXIT_FAILURE);
}

int main(int argc, const char *argv[])
{
    unsigned num_threads = MAX_THREADS;
    num_threads = online_cpus();
    if (num_threads > MAX_THREADS) num_threads = MAX_THREADS;

    if (argc == 1 || argc > 3) usage(argv[0]);
    const char* target = argv[1];

    if (argc == 3) {
        if (strcmp(argv[2], "-s") == 0) {
            runSkipped = true;
        } else if (strcmp(argv[2], "-n") == 0) {
            num_threads = 1;
        } else {
            usage(argv[0]);
        }
    }

    color_output = isatty(1);

    struct stat statbuf;
    if (stat(target, &statbuf)) {
        perror("stat");
        return -1;
    }

    // strip off trailing '/'
    if (target[strlen(target) -1] == '/') {
        char* end = (char*) &target[strlen(target) -1];
        *end = 0;
    }

    cwd = getcwd(0, 0);
    if (cwd == 0) {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }

    TestQueue queue;

    uint64_t t1 = getCurrentTime();
    if (S_ISREG(statbuf.st_mode)) {
        num_threads = 1;
        handle_file(queue, target);
    } else if (S_ISDIR(statbuf.st_mode)) {
        // TODO strip off optional trailing '/'
        handle_dir(queue, target);
    } else {
        usage(argv[0]);
    }

    Tester* testers[MAX_THREADS] = { 0 };
    for (unsigned i=0; i<num_threads; i++) {
        testers[i] = new Tester(i+1, queue);
    }
    // TODO handle ctrl-c

    unsigned numtests = 0;
    unsigned numerrors = 0;
    unsigned numskipped = 0;

    for (unsigned i=0; i<num_threads; i++) {
        Tester* t = testers[i];
        t->join();
        numtests += t->numtests;
        numerrors += t->numerrors;
        numskipped += t->numskipped;
        delete testers[i];
    }

    uint64_t t2 = getCurrentTime();
    const char* color = (numerrors ? COL_ERROR : COL_OK);
    color_print(color, "RESULTS: %u test%s, %u threads (%u ok, %u failed, %u skipped) ran in %llu ms",
        numtests, numtests == 1 ? "" : "s", num_threads, numtests - (numerrors+numskipped), numerrors, numskipped, (t2-t1)/1000);

    return 0;
}

