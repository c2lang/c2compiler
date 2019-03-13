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

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "Builder/C2Builder.h"
#include "Builder/BuildOptions.h"
#include "Builder/Recipe.h"
#include "Builder/RootFinder.h"
#include "Builder/RecipeReader.h"
#include "Builder/BuildFileReader.h"
#include "AST/Component.h"
#include "Utils/BuildFile.h"
#include "Utils/Utils.h"
#include "Utils/color.h"

using namespace C2;

static const char* targetFilter;
static const char* other_dir;
static const char* build_file;
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
    fprintf(stderr, "   -b <file>     - use specified build file\n");
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
    exit(EXIT_FAILURE);
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
            case 'b':
                if (i==argc-1) {
                    fprintf(stderr, "error: -b needs an argument\n");
                    exit(EXIT_FAILURE);
                }
                i++;
                build_file = argv[i];
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
                    exit(EXIT_FAILURE);
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
                    fprintf(stderr, "The C2 Compiler by Bas van den Berg\n\n");

                    fprintf(stderr, "C2 is a programming language that aims to keep the good parts \n");
                    fprintf(stderr, "of C and remove/improve the bad parts. C2 provides stricter \n");
                    fprintf(stderr, "and cleaner syntax, better compilation times, great tooling \n");
                    fprintf(stderr, "and a smart build system among other things. \n\n");

                    fprintf(stderr, "C2 aims to be the direct successor to C, replacing it in \n");
                    fprintf(stderr, "domains currently dominated by C, like bootloaders, kernels, \n");
                    fprintf(stderr, "drivers and system-level tooling. \n\n");

                    fprintf(stderr, "C2 is based on LLVM and Clang. \n");
                    fprintf(stderr, "See c2lang.org for more information. \n");
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
    if (!use_recipe) {
        if (!targetFilter) {
            fprintf(stderr, "error: argument -f needs a filename\n");
            exit(EXIT_FAILURE);
        }
        if (print_targets) {
            fprintf(stderr, "error: -f cannot be used together with -l\n");
            exit(EXIT_FAILURE);
        }
        if (build_file) {
            fprintf(stderr, "error: -f cannot be used together with -b\n");
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, const char *argv[])
{
    uint64_t t1 = Utils::getCurrentTime();
    BuildOptions opts;
    parse_arguments(argc, argv, opts);

    opts.libdir = getenv("C2_LIBDIR");
    if (!opts.libdir) printf("Warning: environment variable C2_LIBDIR not set!\n");

    if (other_dir) {
        if (chdir(other_dir)) {
            fprintf(stderr, "cannot chdir to %s: %s\n", other_dir, strerror(errno));
            return EXIT_FAILURE;
        }
    }

    if (!use_recipe) {
        // NOTE: don't support build file in this mode
        Recipe dummy("dummy", Component::EXECUTABLE);
        dummy.addFile(targetFilter);
        C2Builder builder(dummy, 0, opts);
        int errors = builder.checkFiles();
        if (!errors) errors = builder.build();
        return errors ? EXIT_FAILURE : EXIT_SUCCESS;
    }

    RootFinder finder;
    finder.findTopDir();

    RecipeReader reader;
    if (print_targets) {
        reader.print();
        return EXIT_SUCCESS;
    }

    BuildFile buildFile;
    BuildFile* buildFilePtr = NULL;
    BuildFileReader buildReader(buildFile);
    if (!build_file) build_file = finder.getBuildFile();
    if (build_file) {
        if (!buildReader.parse(build_file)) {
            fprintf(stderr, "Error reading %s: %s\n", build_file, buildReader.getErrorMsg());
            return EXIT_FAILURE;
        }
        buildFilePtr = &buildFile;
    }

    int count = 0;
    bool hasErrors = false;
    for (int i=0; i<reader.count(); i++) {
        const Recipe& recipe = reader.get(i);
        if (targetFilter && recipe.name != targetFilter) continue;
        C2Builder builder(recipe, buildFilePtr, opts);
        int errors = builder.checkFiles();
        if (!errors) errors = builder.build();
        if (errors) hasErrors = true;
        count++;
    }
    if (targetFilter && count == 0) {
        fprintf(stderr, "error: unknown target '%s'\n", targetFilter);
        return EXIT_FAILURE;
    }
    if (opts.printTiming) {
        uint64_t t2 = Utils::getCurrentTime();
        printf(COL_TIME"total building time: %" PRIu64" usec" ANSI_NORMAL"\n", t2 - t1);
    }

    return hasErrors ? EXIT_FAILURE : EXIT_SUCCESS;
}

