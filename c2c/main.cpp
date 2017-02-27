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

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include "Builder/C2Builder.h"
#include "Builder/Recipe.h"
#include "Builder/RootFinder.h"
#include "Builder/RecipeReader.h"
#include "AST/Component.h"
#include "Utils/Utils.h"
#include "Utils/color.h"

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
    fprintf(stderr, "   -aL           - also print library AST\n");
    fprintf(stderr, "   -c            - generate C code\n");
    fprintf(stderr, "   -C            - generate + print C code\n");
    fprintf(stderr, "   -d <dir>      - change directory first\n");
    fprintf(stderr, "   -f <file>     - compile single file without recipe\n");
    fprintf(stderr, "   -h            - show this help\n");
    fprintf(stderr, "   -i            - generate LLVM IR code\n");
    fprintf(stderr, "   -I            - generate + print LLVM IR code\n");
    fprintf(stderr, "   -l            - list targets\n");
    fprintf(stderr, "   -p            - print all modules\n");
    fprintf(stderr, "   -s            - print symbols (excluding library symbols)\n");
    fprintf(stderr, "   -S            - print symbols (including library symbols)\n");
    fprintf(stderr, "   -t            - print timing\n");
    fprintf(stderr, "   -v            - verbose logging\n");
    fprintf(stderr, "   --about       - print information about C2 and c2c\n");
    fprintf(stderr, "   --test        - test mode (don't check for main())\n");
    fprintf(stderr, "   --deps        - print module dependencies\n");
    fprintf(stderr, "   --refs        - generate c2tags file\n");
    fprintf(stderr, "   --check       - only parse + analyse\n");
    fprintf(stderr, "   --showlibs    - print available libraries\n");
    exit(-1);
}

static void parse_arguments(int argc, const char* argv[], BuildOptions& opts) {
    for (int i=1; i<argc; i++) {
        const char* arg = argv[i];
        if (arg[0] == '-') {
            switch (arg[1]) {
            case 'a':
                switch (arg[2]) {
                case '0':
                    opts.printAST0 = true;
                    break;
                case '1':
                    opts.printAST1 = true;
                    break;
                case '2':
                    opts.printAST2 = true;
                    break;
                case '3':
                    opts.printAST3 = true;
                    break;
                case 'L':
                    opts.printASTLib = true;
                    break;
                default:
                    usage(argv[0]);
                    break;
                }
                break;
            case 'c':
                opts.generateC = true;
                break;
            case 'C':
                opts.generateC = true;
                opts.printC = true;
                break;
            case 'd':
                if (i==argc-1) {
                    fprintf(stderr, "error: -d needs an argument\n");
                    exit(-1);
                }
                i++;
                other_dir = argv[i];
                break;
            case 'f':
                use_recipe = false;
                break;
            case 'h':
                usage(argv[0]);
                break;
            case 'i':
                opts.generateIR = true;
                break;
            case 'I':
                opts.generateIR = true;
                opts.printIR = true;
                break;
            case 'l':
                print_targets = true;
                break;
            case 'p':
                opts.printModules = true;
                break;
            case 's':
                opts.printSymbols = true;
                break;
            case 'S':
                opts.printSymbols = true;
                opts.printLibSymbols = true;
                break;
            case 't':
                opts.printTiming = true;
                break;
            case 'v':
                opts.verbose = true;
                break;
            case '-':
                if (strcmp(&arg[2], "about") == 0) {
                    fprintf(stderr, "The C2 Compiler by Bas van den Berg\n");
                    fprintf(stderr, "\nC2 is a programming language aiming to keep the good of C and remove/improve its\n");
                    fprintf(stderr, "bad parts. It provides stricter syntax, great tooling, better compilation times\n");
                    fprintf(stderr, "than C, easy debugging, smart integrated build system, friendly and readable\n");
                    fprintf(stderr, "syntax, requires less typing than C and allows higher development speed.\n");
                    fprintf(stderr, " Its aim is to be used for problems where currently C would be used. So low-\n");
                    fprintf(stderr, "level programs, like bootloaders, kernels, drivers and system-level tooling.\n");
                    fprintf(stderr, "\nC2 is based on LLVM+Clang.\nSee c2lang.org for more information\n");
                    exit(0);
                }
                if (strcmp(&arg[2], "test") == 0) {
                    opts.testMode = true;
                    continue;
                }
                if (strcmp(&arg[2], "deps") == 0) {
                    opts.printDependencies = true;
                    continue;
                }
                if (strcmp(&arg[2], "refs") == 0) {
                    opts.generateRefs = true;
                    continue;
                }
                if (strcmp(&arg[2], "check") == 0) {
                    opts.checkOnly = true;
                    continue;
                }
                if (strcmp(&arg[2], "showlibs") == 0) {
                    opts.showLibs = true;
                    continue;
                }
                usage(argv[0]);
                break;
            default:
                usage(argv[0]);
                break;
            }
        } else {
            if (targetFilter) usage(argv[0]);
            targetFilter = arg;
        }
    }
    if (!use_recipe && !targetFilter) {
        fprintf(stderr, "error: argument -f needs a filename\n");
        exit(-1);
    }
    if (!use_recipe && print_targets) {
        fprintf(stderr, "error: -f cannot be used together with -l\n");
        exit(-1);
    }
}

int main(int argc, const char *argv[])
{
    uint64_t t1 = Utils::getCurrentTime();
    BuildOptions opts;
    parse_arguments(argc, argv, opts);

    // TODO get ENV C2LIBDIR -> should be set
    {
        const char* envname = "C2_LIBDIR";
        opts.libdir = getenv(envname);
        if (!opts.libdir) {
            fprintf(stderr, "please set %s in the ENV to point at the directory containing c2libs/\n", envname);
            return -1;
        }
    }
    if (other_dir) {
        if (chdir(other_dir)) {
            fprintf(stderr, "cannot chdir to %s: %s\n", other_dir, strerror(errno));
            return -1;
        }
    }


    if (!use_recipe) {
        Recipe dummy("dummy", Component::EXECUTABLE);
        dummy.addFile(targetFilter);
        C2Builder builder(dummy, opts);
        int errors = builder.checkFiles();
        if (!errors) errors = builder.build();
        return errors ? 1 : 0;
    }

    RootFinder finder;
    finder.findTopDir();

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
        uint64_t t2 = Utils::getCurrentTime();
        printf(COL_TIME"total building time: %" PRIu64" usec" ANSI_NORMAL"\n", t2 - t1);
    }

    return hasErrors ? 1 : 0;
}

