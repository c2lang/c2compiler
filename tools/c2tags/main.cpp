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

static const char* target;
static bool reverse;

static const char* filename;
static int line;
static int column;

typedef std::vector<std::string> Strings;
Strings refFiles;

struct Result {
    std::string filename;
    unsigned line;
    unsigned column;
};
typedef std::vector<Result> Results;

static void usage(const char* me) {
    printf("%s <args> <file> <line> <col>\n", me);
    printf("    -h          show this help\n");
    printf("    -r          reverse search (symbol users)\n");
    printf("    -t <name>   use target <name>\n");
    exit(-1);
}

static void parse_arguments(int argc, char *argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "hrt:")) != -1) {
        switch (opt) {
        case 'h':
            usage(argv[0]);
            break;
        case 'r':
            reverse = true;
            break;
        case 't':
            target = optarg;
            break;
        default:
            usage(argv[0]);
            break;
        }
    }
    if (argc != optind + 3) usage(argv[0]);

    filename = argv[optind];
    line = atoi(argv[optind+1]);
    column = atoi(argv[optind+2]);
}

static void findRefFiles(void) {
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

int main(int argc, char *argv[])
{
    parse_arguments(argc, argv);

    RootFinder root;
    root.findTopDir();

    findRefFiles();
    if (refFiles.empty()) {
        printf("error: cannot find ref files\n");
        return -1;
    }

    std::string fullname = root.orig2Root(filename);
    RefDest origin = { fullname.c_str(), (uint32_t)line, (uint16_t)column };

    Results results;

    for (unsigned i=0; i<refFiles.size(); i++) {
        const char* refFile = refFiles[i].c_str();
        Refs* refs = refs_load(refFile);
        if (!refs) {
            fprintf(stderr, "c2tags: error: invalid refs %s\n", refFile);
            return -1;
        }

        RefDest result = refs_findRef(refs, &origin);
        if (result.filename) {
            fullname = root.root2Orig(result.filename);
            results.push_back({fullname, result.line, result.col});
        }

    }
    if (results.size() ==  0) printf("no result found\n");
    if (results.size() > 1) printf("multiple matches:\n");
    for (unsigned i=0; i<results.size(); i++) {
        const Result& res = results[i];
        printf("found %s %u %u\n", res.filename.c_str(), res.line, res.column);
    }
    return 0;
}

