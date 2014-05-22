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

#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "Builder/Recipe.h"
#include "Builder/RecipeReader.h"
#include "FileUtils/FileMap.h"

using namespace C2;

const char* RECIPE_FILE = "recipe.txt";

RecipeReader::RecipeReader()
    : current(0)
    , line_nr(1)
    , state(START)
    , token_ptr(0)
{
    findTopDir();
    readRecipeFile();
}

RecipeReader::~RecipeReader() {
    for (unsigned i=0; i<recipes.size(); i++) {
        delete recipes[i];
    }
}

void RecipeReader::findTopDir() {
    char rpath[PATH_MAX];
    while (1) {
        char* path = getcwd(rpath, PATH_MAX);
        if (path == NULL) {
            perror("getcwd");
            exit(-1);
        }
        struct stat buf;
        int err = stat(RECIPE_FILE, &buf);
        if (err) {
            if (rpath[0] == '/' && rpath[1] == 0) {
                fprintf(stderr, "cannot find recipe file\n");
                exit(-1);
            }
            if (errno != ENOENT) {
                perror("stat");
                exit(-1);
            }
        } else {
            return;
        }
        err = chdir("..");
        if (err) {
            perror("chdir");
            exit(-1);
        }
    }
}


void RecipeReader::readRecipeFile() {
    FileMap file(RECIPE_FILE);
    file.open();
    const char* cp = (char*)file.region;
    state = START;
    line_nr = 1;
    const char* end = (const char*)file.region + file.size;
    while (cp < end) {
        char* op = buffer;
        while (*cp != '\n' && cp < end) {
            *op++ = *cp++;
        }
        cp++;
        *op = 0;
        token_ptr = buffer;
        int len = strlen(buffer);
        if (len > 0) handleLine(buffer);
        line_nr++;
    }
    file.close();
    if (state == INSIDE_TARGET) {
        error("missing 'end' keyword");
    }
}


void RecipeReader::handleLine(char* line) {
    if (line[0] == '#') return; // skip comments

    switch (state) {
    case START:
        {
            // line should be 'TARGET <name>'
            const char* kw = get_token();
            if (strcmp(kw, "target") == 0) {
                const char* target_name = get_token();
                if (target_name == 0) error("expected target name");
                current = new Recipe(target_name, true);
                recipes.push_back(current);
                state = INSIDE_TARGET;
            } else if (strcmp(kw, "lib") == 0) {
                const char* target_name = get_token();
                if (target_name == 0) error("expected lib name");
                current = new Recipe(target_name, false);
                recipes.push_back(current);
                state = INSIDE_TARGET;
            } else {
                error("expected keyword target|lib");
            }
        }
        break;
    case INSIDE_TARGET:
        {
            // line should be '<filename>' or 'end'
            const char* tok = get_token();
            if (tok[0] == '$') {
                tok++;
                if (strcmp(tok, "config") == 0) {
                    while (1) {
                        const char* tok2 = get_token();
                        if (!tok2) break;
                        // TODO check duplicate configs
                        current->addConfig(tok2);
                    }
                } else if (strcmp(tok, "ansi-c") == 0) {
                    current->generateCCode = true;
                    while (1) {
                        const char* tok2 = get_token();
                        if (!tok2) break;
                        // TODO check duplicate configs
                        current->addAnsiCConfig(tok2);
                    }
                } else if (strcmp(tok, "codegen") == 0) {
                    while (1) {
                        const char* tok2 = get_token();
                        if (!tok2) break;
                        // TODO check duplicate configs
                        current->addCodeGenConfig(tok2);
                    }
                } else if (strcmp(tok, "warnings") == 0) {
                    while (1) {
                        const char* tok2 = get_token();
                        if (!tok2) break;
                        // TODO check duplicate silence?
                        current->silenceWarning(tok2);
                    }
                } else if (strcmp(tok, "deps") == 0) {
                    current->generateDeps = true;
                    while (1) {
                        const char* tok2 = get_token();
                        if (!tok2) break;
                        // TODO check duplicate configs
                        current->addDepsConfig(tok2);
                    }
                } else {
                    error("unknown option '%s'", tok);
                }
            } else if (strcmp(tok, "end") == 0) {
                state = START;
                current = 0;
            } else {
                struct stat buf;
                int err = stat(tok, &buf);
                if (err) {
                    // TODO also check read rights (access() )
                    error("file '%s' does not exist", tok);
                }
                current->addFile(tok);
            }
        }
        break;
    }
}

char* RecipeReader::get_token() {
    // skip white space
    while (*token_ptr == ' ' || *token_ptr == '\t' ) token_ptr++;
    if (*token_ptr == 0) return 0;

    char* result = token_ptr;

    // find end of token
    char* cp = token_ptr;
    while (*cp != 0 && *cp != ' ' && *cp != '\t') cp++;
    if (*cp == 0) { // token ends at end of line
        token_ptr = cp;
    } else {
        *cp = 0;
        token_ptr = cp + 1;
    }

    return result;
}

void RecipeReader::error(const char* fmt, ...) {
    char tmp[256];

    va_list argp;
    char* cp = tmp;

    cp += sprintf(cp, "recipe: line %d ", line_nr);
    va_start(argp, fmt);
    int size = vsprintf(cp, fmt, argp);
    cp += size;
    va_end(argp);
    fprintf(stderr, "Error: %s\n", tmp);
    exit(-1);
}

const Recipe& RecipeReader::get(int i) const {
    return *recipes[i];
}

void RecipeReader::print() const {
    printf("Targets:\n");
    for (unsigned i=0; i<recipes.size(); i++) {
        Recipe* R = recipes[i];
        printf("  %s\n", R->name.c_str());
    }
}

