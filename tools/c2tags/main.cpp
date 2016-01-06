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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "TagReader.h"
#include "Builder/RootFinder.h"

using namespace C2;

static const char* target;
static bool reverse;

static const char* filename;
static int line;
static int column;

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

int main(int argc, char *argv[])
{
    parse_arguments(argc, argv);

    RootFinder root;
    root.findTopDir();
    TagReader tags(target);

    std::string fullname = root.orig2Root(filename);
    unsigned numResults;
    if (reverse) {
        numResults = tags.findReverse(fullname.c_str(), line, column);
    } else {
        numResults = tags.find(fullname.c_str(), line, column);
    }

    if (numResults == 0) {
        printf("no result found\n");
        return 0;
    }
    if (numResults > 1) printf("multiple matches:\n");
    for (unsigned i=0; i<numResults; i++) {
        TagReader::Result R = tags.getResult(i);
        fullname = root.root2Orig(R.filename);
        printf("found %s %d %d\n", fullname.c_str(), R.line, R.column);
    }

    return 0;
}

