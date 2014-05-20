/* Copyright 2013,2014 Bas van den Berg
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
#include <sys/types.h>
#include <sys/wait.h>
#include <map>
#include <list>
#include <string>

#include "FileMap.h"
#include "StringBuilder.h"
#include "color.h"

#define COL_ERROR ANSI_BRED
#define COL_SKIP  ANSI_BCYAN
#define COL_OK    ANSI_GREEN
#define COL_DEBUG ANSI_BMAGENTA

//#define DEBUG

using namespace C2;

static unsigned numtests;
static unsigned numerrors;
static unsigned numskipped;

static int color_output = 1;
static const char* c2c_cmd = "build/c2c";
static const char* test_root = "/tmp/tester";
static char* cwd;
static bool runSkipped;

#ifdef DEBUG
static void debug(const char* format, ...) {
    char buffer[1024];
    va_list(Args);
    va_start(Args, format);
    int len = vsprintf(buffer, format, Args);
    va_end(Args);
    if (color_output) fprintf(stderr, COL_DEBUG"%s"ANSI_NORMAL"\n", buffer);
    else printf("%s\n", buffer);
}
#else
static void debug(const char* format, ...) {}
#endif

static void color_print(const char* color, const char* format, ...) {
    char buffer[1024];
    va_list(Args);
    va_start(Args, format);
    int len = vsprintf(buffer, format, Args);
    va_end(Args);

    if (color_output) printf("%s%s"ANSI_NORMAL"\n", color, buffer);
    else printf("%s\n", buffer);
}


static u_int64_t getCurrentTime() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    u_int64_t now64 = now.tv_sec;
    now64 *= 1000000;
    now64 += (now.tv_nsec/1000);
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

static void writeFile(const char* name, const char* content, unsigned size) {
    char fullname[128];
    sprintf(fullname, "%s/%s", test_root, name);
    int fd = open(fullname, O_WRONLY | O_CREAT, 0660);
    if (fd == -1) {
        perror("open");
        exit(-1);
    }
    ssize_t written = write(fd, content, size);
    if (written != (ssize_t)size) {
        perror("write");
        exit(-1);
    }
    close(fd);
}


class IssueDb {
public:
    IssueDb(File& file_, bool single_)
        : file(file_)
        , single(single_)
        , line_nr(0)
        , current_file("")
        , file_start(0)
        , line_offset(0)
        , hasErrors(false)
    {
        if (single) {
            current_file = cwd;
            current_file  += '/' + file.filename;
        }
    }

    bool parseFile();

    void testFile();

    bool haveErrors() const { return hasErrors; }

    void showWarnings() const {
        for (IssuesConstIter iter = warnings.begin(); iter != warnings.end(); ++iter) {
            color_print(COL_ERROR, "  expected warning '%s' at %s:%d",
                    iter->msg.c_str(), iter->filename.c_str(), iter->line_nr);
        }
    }

    void showErrors() const {
        for (IssuesConstIter iter = errors.begin(); iter != errors.end(); ++iter) {
            color_print(COL_ERROR, "  expected error '%s' at %s:%d",
                    iter->msg.c_str(), iter->filename.c_str(), iter->line_nr);
        }
    }
private:
    void parseLine(const char* start, const char* end);
    void error(const char* msg) {
        color_print(ANSI_BRED, "%s:%d: %s", file.filename.c_str(), line_nr, msg);
        hasErrors = true;
    }

    void checkErrors(const char* buffer, unsigned size);

    void matchWarning(const char* filename, unsigned linenr, const char* msg) {
        for (IssuesIter iter = warnings.begin(); iter != warnings.end(); ++iter) {
            if (iter->line_nr != linenr) continue;
            if (iter->filename == filename) {
                if (iter->msg != msg) {
                    color_print(COL_ERROR, "  wrong warning at %s:%d:", filename, linenr);
                    color_print(COL_ERROR, "     expected: %s", iter->msg.c_str());
                    color_print(COL_ERROR, "     got: %s", msg);
                    hasErrors = true;
                }
                warnings.erase(iter);
                return;
            }
        }
        // not expected
        color_print(COL_ERROR, "  unexpected warning on line %d: %s", linenr, msg);
        hasErrors = true;
    }

    void matchError(const char* filename, unsigned linenr, const char* msg) {
        for (IssuesIter iter = errors.begin(); iter != errors.end(); ++iter) {
            if (iter->line_nr != linenr) continue;
            if (iter->filename == filename) {
                if (iter->msg != msg) {
                    color_print(COL_ERROR, "  wrong error at %s:%d:", filename, linenr);
                    color_print(COL_ERROR, "     expected: %s", iter->msg.c_str());
                    color_print(COL_ERROR, "     got: %s", msg);
                    hasErrors = true;
                }
                errors.erase(iter);
                return;
            }
        }
        // not expected
        color_print(COL_ERROR, "  unexpected error on line %d: %s", linenr, msg);
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

    bool single;
    unsigned line_nr;
    std::string current_file;
    const char* file_start;
    unsigned line_offset;
    StringBuilder recipe;
    bool hasErrors;
};

void IssueDb::parseLine(const char* start, const char* end) {
    unsigned int len = end - start;
    const char* cp = start;
    if (len < 11) return;
    // check for '// @file{' at start
    if (strncmp(cp, "// @file{", 9) == 0) {
        if (single) {
            error("invalid @file tag in single test");
            return;
        }
        if (file_start) {
            const char* file_end = start;
            writeFile(current_file.c_str(), file_start, file_end - file_start);
            recipe << "  " << current_file << '\n';
        }
        cp += 9;
        const char* name_start = cp;
        while (*cp != '}') {
            if (cp == end) {
                error("missing '}'");
                exit(-1);
            }
            cp++;
        }
        std::string name(name_start, cp-name_start);
        if (!endsWith(name.c_str(), ".c2")) name += ".c2";
        current_file = name;
        file_start = end + 1;
        line_offset = line_nr;
        return;
    }
    // TEMP only support single argument for now
    if (strncmp(cp, "// @warnings{", 13) == 0) {
        cp += 13;
        const char* name_start = cp;
        while (*cp != '}') {
            if (cp == end) {
                error("missing '}'");
                exit(-1);
            }
            cp++;
        }
        std::string name(name_start, cp-name_start);
        recipe << "  $warnings " << name << '\n';
        return;
    }
    //if (strncmp(cp, "//", 2) == 0) return;   // skip other comments
    // find // @..

    cp = find(start, end, "// ");
    if (!cp) return;
    cp += 3;    // skip "// ";

    // search for @
    if (*cp != '@') return;

    cp++;   // skip @
    bool isError = true;
    if (strncmp(cp, "error{", 6) == 0) {
        cp += 6;
        isError = true;
        goto parse_msg;
    }
    if (strncmp(cp, "warning{", 8) == 0) {
        cp += 8;
        isError = false;
        goto parse_msg;
    }
    error("unknown tag");
    return;
parse_msg:
    const char* msg_start = cp;
    while (*cp != '}') {
        if (cp == end) {
            error("missing '}'");
            exit(-1);
        }
        cp++;
    }
    char msg[128];
    memcpy(msg, msg_start, cp-msg_start);
    msg[cp-msg_start] = 0;
    if (isError) {
#ifdef DEBUG
        printf(ANSI_BLUE"  expecting error '%s'"ANSI_NORMAL"\n", msg);
#endif
        errors.push_back(Issue(current_file, line_nr - line_offset, msg));
    } else {
#ifdef DEBUG
        printf(ANSI_BLUE"  expecting warning '%s'"ANSI_NORMAL"\n", msg);
#endif
        warnings.push_back(Issue(current_file, line_nr - line_offset, msg));
    }
}

bool IssueDb::parseFile() {
    const char* cp = (const char*) file.region;
    const char* end = cp + file.size;
    line_nr = 1;
    const char* line_start = cp;
    recipe << "target test\n";
    bool hasSkip = strncmp(cp, "// @skip", 8) == 0;
    if (runSkipped != hasSkip) return true;
    while (cp != end) {
        while (*cp != '\n' && cp != end) cp++;
        if (cp != line_start) parseLine(line_start, cp);
        line_nr++;
        if (*cp == '\n') cp++;
        line_start = cp;
    }
    if (!single) {
        const char* file_end = cp;
        writeFile(current_file.c_str(), file_start, file_end- file_start);
    }
    recipe << "  " << current_file << '\n';
    recipe << "end\n";
    writeFile("recipe.txt", recipe, recipe.size());
    return false;
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
        execl(c2c_cmd, "c2c", "-d", test_root, "--test", NULL);
        perror("execv");
        exit(127); /* only if execv fails */
    }
    else { // parent
        close(pipe_stdout[1]);
        close(pipe_stderr[1]);
        int status = 0;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status)) { // child exited abnormally
            color_print(COL_ERROR, "c2c crashed!");
            numerrors++;
            return;
        }
        // check return code
        int retcode = WEXITSTATUS(status);
        if (retcode == 127) {
            color_print(COL_ERROR, "Error spawning compiler '%s'", c2c_cmd);
            exit(-1);
        }
        // check output
        char buffer[4096];
        while (1) {
            ssize_t count = read(pipe_stderr[0], buffer, sizeof(buffer)-1);
            if (count == -1) {
                if (errno == EINTR) continue;
                perror("read");
                exit(1);
            }
            if (count == 0) break;
            buffer[count] = 0;
            checkErrors(buffer, count);
        }
        close(pipe_stdout[0]);
        close(pipe_stderr[0]);
        wait(0);
        if (errors.size() || warnings.size()) hasErrors = true;
    }
}

void IssueDb::checkErrors(const char* buffer, unsigned size) {
#ifdef DEBUG
    printf(ANSI_MAGENTA"stderr:\n%s"ANSI_NORMAL"\n", buffer);
#endif
    const char* cp = buffer;
    const char* end = cp + size;
    while (cp != end) {
        // line syntax: '<filename>.c2:<linenr>:<offset>: error: <msg>\n'
        //char* filename = 0;
        char filename[128];
        char msg[128];
        int error_line = 0;
        int col = 0;
        memset(filename, 0, sizeof(filename));
        memset(msg, 0, sizeof(msg));
        int res = sscanf(cp, "%[^: ]:%d:%d: error: %[^\n]\n", filename, &error_line, &col, msg);
        if (res == 4) {
            // found error
#ifdef DEBUG
            printf(ANSI_CYAN"%s", filename);
            printf("  %d:%d", error_line, col);
            printf("  '%s'"ANSI_NORMAL"\n", msg);
#endif
            matchError(filename, error_line, msg);
        } else {
            res = sscanf(cp, "%[^: ]:%d:%d: warning: %[^\n]\n", filename, &error_line, &col, msg);
            if (res == 4) {
                // found warning
#ifdef DEBUG
                printf(ANSI_CYAN"%s", filename);
                printf("  %d:%d", error_line, col);
                printf("  '%s'"ANSI_NORMAL"\n", msg);
#endif
                matchWarning(filename, error_line, msg);
            }
        }

        if (res == 4) {
            // match msg string and set cp to that to avoid duplicates on empty lines
            const char* found = strstr(cp, msg);
            assert(found);
            cp = found;
        }

        while (*cp != '\n') {
             cp++;
             if (cp == end) return;
        }
        cp++;
    }
}

static void handle_file(const char* filename) {
    debug("%s() %s", __func__, filename);
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
    int err = system("rm -rf /tmp/tester/");
    if (err != 0) {
        perror("system");
        exit(-1);
    }
    // create test dir
    err = mkdir(test_root, 0777);
    if (err) {
        perror("mkdir");
        exit(-1);
    }

    numtests++;
    FileMap file(filename);
    file.open();
    IssueDb db(file, single);

    bool skip = db.parseFile();
    if (skip) {
        numskipped++;
        printf(COL_SKIP"%s SKIPPED"ANSI_NORMAL"\n", filename);
        return;
    } else {
        printf("%s\n", filename);
    }
    if (db.haveErrors()) goto out;

    db.testFile();
    db.showErrors();
    db.showWarnings();
out:
    if (db.haveErrors()) {
        numerrors++;
    }
}

static void handle_dir(const char* path) {
    debug("%s() %s", __func__, path);
    DIR* dir = opendir(path);
    if (dir == NULL) {
        color_print(COL_ERROR, "Cannot open dir '%s': %s", path, strerror(errno));
        return;
    }
    struct dirent* dir2 = readdir(dir);
    char temp[256];
    while (dir2 != 0) {
        sprintf(temp, "%s/%s", path, dir2->d_name);
        switch (dir2->d_type) {
        case DT_REG:
            handle_file(temp);
            break;
        case DT_DIR:
            if (strcmp(dir2->d_name, ".") != 0 && strcmp(dir2->d_name, "..") != 0) {
                handle_dir(temp);
            }
            break;
        default:
            break;
        }
        dir2 = readdir(dir);
    }
    closedir(dir);
}

static void usage(const char* name) {
    printf("Usage: %s [file/dir] <options>\n", name);
    printf("    -s    only run skipped tests\n");
    exit(-1);
}

int main(int argc, const char *argv[])
{
    if (argc == 1 || argc > 3) usage(argv[0]);
    const char* target = argv[1];

    if (argc == 3) {
        if (strcmp(argv[2], "-s") == 0) {
            runSkipped = true;
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

    cwd = get_current_dir_name();
    if (cwd == 0) {
        perror("get_current_dir_name");
        exit(-1);
    }

    u_int64_t t1 = getCurrentTime();
    if (S_ISREG(statbuf.st_mode)) {
        handle_file(target);
    } else if (S_ISDIR(statbuf.st_mode)) {
        // TODO strip off optional trailing '/'
        handle_dir(target);
    } else {
        usage(argv[0]);
    }
    u_int64_t t2 = getCurrentTime();
    const char* color = (numerrors ? COL_ERROR : COL_OK);
    color_print(color, "RESULTS: %u test%s (%u ok, %u failed, %u skipped) ran in %llu ms", numtests, numtests == 1 ? "" : "s", numtests - (numerrors+numskipped), numerrors, numskipped, (t2-t1)/1000);

    return 0;
}

