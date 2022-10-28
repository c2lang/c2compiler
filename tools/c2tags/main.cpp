/* Copyright 2013-2022 Bas van den Berg
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string>
#include <vector>

#include "common/Refs.h"
#include "Builder/RootFinder.h"

using namespace C2;

static const char* TAGS_FILE = "refs";
static const char* OUTPUT_DIR = "output";

typedef struct {
    const char* refsfile;
    const char* symfile;        // symbol name or filename
    const char* target;
    uint32_t line;
    uint16_t column;
    bool sym_isfile;
    bool reverse;
    bool dump;
    bool verbose;
} Options;

static Options opts;
typedef std::vector<std::string> Strings;
static Strings refFiles;

struct Result {
    std::string filename;
    unsigned line;
    unsigned column;
};
typedef std::vector<Result> Results;

static  void usage(const char* me) {
    printf("Usage: %s [mode] <opts>\n", me);
    printf("  Modes:\n");
    printf("    [name]                  find def. of symbol specified by name\n");
    printf("    [file] [line] [col]     find def. of symbol specify by position\n");
    printf("    -d                      dump refs\n");
    printf("    -D                      dump refs (verbose)\n");
    printf("  Options:\n");
    printf("    -r                      find uses of symbol (specified by name/position)\n");
    printf("    -f [reffile]            use alternative refs file\n");
    printf("    -t [target]             use [target]\n");
    exit(-1);
}

static void parse_options(int argc, char* argv[]) {
    unsigned pos_count = 0;
    int i= 1;
    while (i<argc) {
        const char* arg = argv[i];
        if (arg[0] == '-') {
            switch (arg[1]) {
            case 'd':
                opts.dump = 1;
                break;
            case 'D':
                opts.dump = 1;
                opts.verbose = 1;
                break;
            case 'f':
                i++;
                if (i == argc) usage(argv[0]);
                opts.refsfile = argv[i];
                break;
            case 'r':
                opts.reverse = 1;
                break;
            case 't':
                i++;
                if (i == argc) usage(argv[0]);
                opts.target = argv[i];
            default:
                usage(argv[0]);
                break;
            }
        } else {
            switch (pos_count) {
            case 0:     // symbol/filename
                opts.symfile = arg;
                pos_count++;
                break;
            case 1:     // line
                opts.line = atoi(arg);
                pos_count++;
                break;
            case 2:     // column
                opts.column = atoi(arg);
                pos_count++;
                break;
            default:
                usage(argv[0]);
                break;
            }
        }
        i++;
    }
    if (opts.dump) return;
    if (pos_count != 1 && pos_count != 3) usage(argv[0]);
    if (pos_count > 1) opts.sym_isfile = 1;
}

static void findRefFiles(void) {
    char fullname[PATH_MAX];
    if (opts.target) {
        sprintf(fullname, "%s/%s/%s", OUTPUT_DIR, opts.target, TAGS_FILE);
        struct stat statbuf;
        if (stat(fullname, &statbuf) == 0) {
            refFiles.push_back(fullname);
        }
        return;
    }
    // check for output dir
    DIR* dir = opendir(OUTPUT_DIR);
    if (dir == 0) {
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

static Results results;

static void use_fn(void* arg, const RefDest* res) {
    for (uint32_t i=0; i<results.size(); i++) {
        const Result& cur = results[i];
        if (cur.filename == res->filename &&
            cur.line == res->line &&
            cur.column == res->col) {
            return;
        }
    }
    results.push_back({ res->filename, res->line, res->col });
}

int main(int argc, char *argv[])
{
    parse_options(argc, argv);

    RootFinder root;
    root.findTopDir();

    findRefFiles();
    if (refFiles.empty()) {
        printf("error: cannot find ref files\n");
        return -1;
    }

    std::string fullname = root.orig2Root(opts.symfile);
    RefDest origin = { fullname.c_str(), (uint32_t)opts.line, (uint16_t)opts.column };

    for (unsigned i=0; i<refFiles.size(); i++) {
        const char* refFile = refFiles[i].c_str();
        Refs* refs = refs_load(refFile);
        if (!refs) {
            fprintf(stderr, "c2tags: error: invalid refs %s\n", refFile);
            return -1;
        }

        if (opts.sym_isfile) {
            if (opts.reverse) {
                refs_findRefUses(refs, &origin, use_fn, NULL);
            } else {
                RefDest result = refs_findRef(refs, &origin);
                if (result.filename) {
                    fullname = root.root2Orig(result.filename);
                    results.push_back({ fullname, result.line, result.col });
                }
            }
        } else {
            if (opts.reverse) {
                refs_findSymbolUses(refs, opts.symfile, use_fn, NULL);
            } else {
                RefDest result = refs_findSymbol(refs, opts.symfile);
                if (result.filename) {
                    fullname = root.root2Orig(result.filename);
                    results.push_back({ fullname, result.line, result.col });
                }
            }
        }
        refs_free(refs);
    }

    if (results.size() ==  0) printf("no result found\n");
    if (opts.reverse) {
        printf("found %lu matches\n", results.size());
    } else {
        if (results.size() > 1) printf("multiple matches:\n");
    }
    for (unsigned i=0; i<results.size(); i++) {
        const Result& res = results[i];
        //fullname = root.root2Orig(result.filename);
        printf("found %s %u %u\n", res.filename.c_str(), res.line, res.column);
    }

    return 0;
}

