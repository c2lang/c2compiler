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
#include "Utils/arg.h"

using namespace C2;

static const char* targetFilter;
static const char* other_dir;
static bool print_targets = false;
static bool use_recipe = true;

static void usage(const char* name) {
    fprintf(stderr, "Usage: %s <options> <target>\n", name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "   --a0          - print AST after parsing\n");
    fprintf(stderr, "   --a1          - print AST after analysis 1\n");
    fprintf(stderr, "   --a2          - print AST after analysis 2\n");
    fprintf(stderr, "   --a3          - print AST after analysis 3 (final)\n");
    fprintf(stderr, "   --aL          - also print library AST\n");
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

static void about() {
    fprintf(stderr, "The C2 Compiler by Bas van den Berg\n");
    fprintf(stderr, "\nC2 is a programming language aiming to keep the good of C and remove/improve its\n");
    fprintf(stderr, "bad parts. It provides stricter syntax, great tooling, better compilation times\n");
    fprintf(stderr, "than C, easy debugging, smart integrated build system, friendly and readable\n");
    fprintf(stderr, "syntax, requires less typing than C and allows higher development speed.\n");
    fprintf(stderr, "Its aim is to be used for problems where currently C would be used. So low-\n");
    fprintf(stderr, "level programs like bootloaders, kernels, drivers and system-level tooling.\n");
    fprintf(stderr, "\nC2 is based on LLVM+Clang.\nSee c2lang.org for more information\n");
    exit(-1);
}

static int option(BuildOptions& opts, char** argv, int* argc, const char* argv0) {
    if(!argv[0]) return 1;
    if(argv[0][0] == '-' && strlen(argv[0]) > 1 && argv[0][1] != '-') return 1;
    START_OPTION("--a0", opts.printAST0 = true)
          OPTION("--a1", opts.printAST1 = true)
          OPTION("--a2", opts.printAST2 = true)
          OPTION("--a3", opts.printAST3 = true)
          OPTION("--aL", opts.printASTLib = true)
          OPTION("--test", opts.testMode = true)
          OPTION("--deps", opts.printDependencies = true)
          OPTION("--refs", opts.generateRefs = true)
          OPTION("--about", about())
          OPTION("--help", usage(argv0))
          OPTION("--check", opts.checkOnly = true)
          OPTION("--showlibs", opts.showLibs = true)
          else if(strncmp(argv[0], "--", 2) == 0) fprintf(stderr, "error: unrecognized option: %s\n", argv[0]);
          else {
          	if(targetFilter) usage(argv0);
          	targetFilter = argv[0];
          }

    if (!use_recipe && !targetFilter) {
        fprintf(stderr, "error: argument -f needs a filename\n");
        exit(-1);
    }
    if (!use_recipe && print_targets) {
        fprintf(stderr, "error: -f cannot be used together with -l\n");
        exit(-1);
    }
	return 1;
}

static void parse_arguments(int argc, char* argv[], BuildOptions& opts) {
    ARGBEGIN {
        FLAG('c', opts.generateC = true)
        FLAG('C', opts.generateC = true; opts.printC = true)
        FLAG('f', use_recipe = false)
        FLAG('h', usage(argv0))
        FLAG('i', opts.generateIR = true)
        FLAG('I', opts.generateIR = true; opts.printIR = true)
        FLAG('l', print_targets = true)
        FLAG('p', opts.printModules = true)
        FLAG('s', opts.printSymbols = true)
        FLAG('S', opts.printSymbols = true; opts.printLibSymbols = true)
        FLAG('t', opts.printTiming = true)
        FLAG('v', opts.verbose = true)
        ARGFLAG('d', other_dir = (++argv)[0])
        default:
            fprintf(stderr, "error: unrecognized option '-%c'\n", ARGC());
            break;
    }
    ARGEND
}

int main(int argc, char *argv[])
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

