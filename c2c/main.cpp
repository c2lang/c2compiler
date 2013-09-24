/* Copyright 2013 Bas van den Berg
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

#include <clang/Basic/Version.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "C2Builder.h"
#include "RecipeReader.h"
#include "Recipe.h"
#include "Utils.h"
#include "color.h"

using namespace C2;

static const char* targetFilter;
static const char* other_dir;
static bool print_targets = false;
static bool use_recipe = true;

static void usage(const char* name) {
    fprintf(stderr, "Usage: %s <options> <target>\n", name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "   -a0           - print AST after parsing\n");
    fprintf(stderr, "   -a1           - print AST after analysis 1\n");
    fprintf(stderr, "   -a2           - print AST after analysis 2\n");
    fprintf(stderr, "   -a3           - print AST after analysis 3 (final)\n");
    fprintf(stderr, "   -c            - generate C code\n");
    fprintf(stderr, "   -C            - generate + print C-code\n");
    fprintf(stderr, "   -d <dir>      - change directory first\n");
    fprintf(stderr, "   -f <file>     - compile single file without recipe\n");
    fprintf(stderr, "   -h            - show this help\n");
    fprintf(stderr, "   -i            - generate LLVM IR code\n");
    fprintf(stderr, "   -I            - generate + print LLVM IR code\n");
    fprintf(stderr, "   -l            - list targets\n");
    fprintf(stderr, "   -p            - print all packages\n");
    fprintf(stderr, "   -s            - print symbols\n");
    fprintf(stderr, "   -t            - print timing\n");
    fprintf(stderr, "   --test        - test mode (don't check for main())\n");
    fprintf(stderr, "   -v            - verbose logging\n");
    exit(-1);
}

static void parse_arguments(int argc, const char* argv[], BuildOptions& opts) {
    for (int i=1; i<argc; i++) {
        const char* arg = argv[i];
        if (strcmp("-a0", arg) == 0) {
            opts.printAST0 = true;
            continue;
        }
        if (strcmp("-a1", arg) == 0) {
            opts.printAST1 = true;
            continue;
        }
        if (strcmp("-a2", arg) == 0) {
            opts.printAST2 = true;
            continue;
        }
        if (strcmp("-a3", arg) == 0) {
            opts.printAST3 = true;
            continue;
        }
        if (strcmp("-c", arg) == 0) {
            opts.generateC = true;
            continue;
        }
        if (strcmp("-C", arg) == 0) {
            opts.generateC = true;
            opts.printC = true;
            continue;
        }
        if (strcmp("-d", arg) == 0) {
            if (i==argc-1) {
                fprintf(stderr, "error: -d needs an argument\n");
                exit(-1);
            }
            i++;
            other_dir = argv[i];
            continue;
        }
        if (strcmp("-f", arg) == 0) {
            use_recipe = false;
            continue;
        }
        if (strcmp("-h", arg) == 0) {
            usage(argv[0]);
        }
        if (strcmp("-i", arg) == 0) {
            opts.generateIR = true;
            continue;
        }
        if (strcmp("-I", arg) == 0) {
            opts.generateIR = true;
            opts.printIR = true;
            continue;
        }
        if (strcmp("-l", arg) == 0) {
            print_targets = true;
            continue;
        }
        if (strcmp("-p", arg) == 0) {
            opts.printPackages = true;
            continue;
        }
        if (strcmp("-s", arg) == 0) {
            opts.printSymbols = true;
            continue;
        }
        if (strcmp("-t", arg) == 0) {
            opts.printTiming = true;
            continue;
        }
        if (strcmp("--test", arg) == 0) {
            opts.testMode = true;
            continue;
        }
        if (strcmp("-v", arg) == 0) {
            opts.verbose = true;
            continue;
        }
        if (arg[0] == '-') {
            usage(argv[0]);
        }
        if (targetFilter) usage(argv[0]);
        targetFilter = arg;
    }
    if (!use_recipe && !targetFilter) {
        fprintf(stderr, "error: argument -f needs filename\n");
        exit(-1);
    }
    if (!use_recipe && print_targets) {
        fprintf(stderr, "error: -f cannot be used together with -l\n");
        exit(-1);
    }
}

int main(int argc, const char *argv[])
{
    assert(CLANG_C2_VERSION >= 5 && "Please update your clang c2 version");

    u_int64_t t1 = Utils::getCurrentTime();
    BuildOptions opts;
    parse_arguments(argc, argv, opts);

    if (other_dir) {
        if (chdir(other_dir)) {
            fprintf(stderr, "cannot chdir to %s: %s\n", other_dir, strerror(errno));
            return -1;
        }
    }
    if (!use_recipe) {
        Recipe dummy("dummy");
        dummy.addFile(targetFilter);
        C2Builder builder(dummy, opts);
        int errors = builder.checkFiles();
        if (!errors) errors = builder.build();
        return errors ? 1 : 0;
    }

    RecipeReader reader;
    if (print_targets) {
        reader.print();
        return 0;
    }
    int count = 0;
    bool hasErrors = false;
    for (int i=0; i<reader.count(); i++) {
        const Recipe& recipe = reader.get(i);
        if (targetFilter && recipe.name != targetFilter) continue;
        C2Builder builder(recipe, opts);
        int errors = builder.checkFiles();
        if (!errors) errors = builder.build();
        if (errors) hasErrors = true;
        count++;
    }
    if (targetFilter && count == 0) {
        fprintf(stderr, "error: unknown target '%s'\n", targetFilter);
        return -1;
    }
    if (opts.printTiming) {
        u_int64_t t2 = Utils::getCurrentTime();
        printf(COL_TIME"total building time: %lld usec"ANSI_NORMAL"\n", t2 - t1);
    }

    return hasErrors ? 1 : 0;
}

