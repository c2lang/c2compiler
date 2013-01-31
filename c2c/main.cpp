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

#include "C2Builder.h"
#include "RecipeReader.h"
#include "Recipe.h"

using namespace C2;

static const char* targetFilter;
static bool print_targets = false;
static bool use_recipe = true;

static void usage(const char* name) {
    fprintf(stderr, "Usage: %s <options> <target>\n", name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "   -a            - print AST\n");
    fprintf(stderr, "   -c            - generate C code\n");
    fprintf(stderr, "   -f <file>     - compile single file without recipe\n");
    fprintf(stderr, "   -h            - show this help\n");
    fprintf(stderr, "   -i            - generate LLVM IR code\n");
    fprintf(stderr, "   -l            - list targets\n");
    fprintf(stderr, "   -s            - print symbols\n");
    fprintf(stderr, "   -t            - print timing\n");
    exit(-1);
}

static void parse_arguments(int argc, const char* argv[], BuildOptions& opts) {
    for (int i=1; i<argc; i++) {
        const char* arg = argv[i];
        if (strcmp("-a", arg) == 0) {
            opts.printAST = true;
            continue;
        }
        if (strcmp("-c", arg) == 0) {
            opts.generateC = true;
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
        if (strcmp("-l", arg) == 0) {
            print_targets = true;
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
    assert(CLANG_C2_VERSION >= 3 && "Please update your clang c2 version");

    BuildOptions opts;
    parse_arguments(argc, argv, opts);

    if (!use_recipe) {
        Recipe dummy("dummy");
        dummy.addFile(targetFilter);
        C2Builder builder(dummy, opts);
        builder.build();
        return 0;
    }

    RecipeReader reader;
    if (print_targets) {
        reader.print();
        return 0;
    }
    int count = 0;
    for (int i=0; i<reader.count(); i++) {
        const Recipe& recipe = reader.get(i);
        if (targetFilter && recipe.name != targetFilter) continue;
        C2Builder builder(recipe, opts);
        builder.build();
        count++;
    }
    if (targetFilter && count == 0) {
        fprintf(stderr, "error: unknown target '%s'\n", targetFilter);
        return -1;
    }

    return 0;
}

