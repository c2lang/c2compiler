/* Copyright 2013-2023 Bas van den Berg
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
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "Builder/RecipeReader.h"
#include "Builder/BuilderConstants.h"
#include "Builder/Plugin.h"

#include "FileUtils/FileMap.h"
#include "AST/Component.h"

using namespace C2;

RecipeReader::RecipeReader()
    : current(0)
    , line_nr(1)
    , state(START)
    , has_targets(false)
    , token_ptr(0)
{
    readRecipeFile();
}

RecipeReader::~RecipeReader() {
    for (unsigned i=0; i<recipes.size(); i++) {
        delete recipes[i];
    }
}

void RecipeReader::readRecipeFile() {
    FileMap file(RECIPE_FILE);
    file.open();
    const char* cp = (char*)file.region;
    state = START;
    has_targets = false;
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

void RecipeReader::startNewRecipe() {
    recipes.push_back(current);
    state = INSIDE_TARGET;
    has_targets = true;
    for (unsigned i=0; i<globalConfigs.size(); i++) {
        current->addConfig(globalConfigs[i]);
    }
    current->generateCCode = true;
    current->enableAsserts = false;
    current->writeAST = false;
    current->CGenFlags.single_module = true;
    current->IrGenFlags.single_module = false;
    current->IrGenFlags.single_threaded = false;
    current->IrGenFlags.keep_intermediates = false;
}

void RecipeReader::handleLine(char* line) {
if (line[0] == '#') return; // skip comments

    switch (state) {
    case START:
        {
            // line should be 'executable/lib <name>'
            const char* kw = get_token();

            if (strcmp(kw, "plugin") == 0) {
                const char* plugin_name = get_token();
                if  (!plugin_name) error("expected plugin name");
                // TODO check for trailing  stuff? (plugin settings?)

                // search for duplicates
                for (unsigned i=0; i<plugins.size(); i++) {
                    if (plugins[i].first == plugin_name) {
                        warn("duplicate plugin %s", plugin_name);
                        return;
                    }
                }

                const char* plugin_config = get_options();
                plugins.push_back(Recipe::Plugin(plugin_name, plugin_config ? plugin_config : ""));
            } else if (strcmp(kw, "executable") == 0) {
                const char* target_name = get_token();
                if (target_name == 0) error("expected executable name");
                current = new Recipe(target_name, Component::MAIN_EXECUTABLE);
                startNewRecipe();
            } else if (strcmp(kw, "lib") == 0) {
                const char* target_name = get_token();
                if (target_name == 0) error("expected library name");
                const char* type_name = get_token();
                if (type_name == 0) error("expected library type");

                Component::Type type = Component::MAIN_SHARED_LIB;
                if (strcmp(type_name, "shared") == 0) {
                    type = Component::MAIN_SHARED_LIB;
                } else if (strcmp(type_name, "static") == 0) {
                    type = Component::MAIN_STATIC_LIB;
                } else if (strcmp(type_name, "source") == 0) {
                    type = Component::MAIN_SOURCE_LIB;
                } else {
                    error("unknown library type '%s'", type_name);
                }
                current = new Recipe(target_name, type);
                startNewRecipe();
            } else if (strcmp(kw, "config") == 0) {
                // Only allow before targets
                if (has_targets) {
                    error("global configs only allowed before executable/lib targets");
                }
                const char* tok2 = get_token();
                if (!tok2) error("missing config name");
                std::string name = tok2;
                std::string value = "1";
                tok2 = get_token();
                if (tok2) value = tok2;
                // TODO check duplicate configs
                globalConfigs.push_back(Config(name, value));
                if (get_token()) error("expected end of line");
            } else {
                error("expected keyword executable|lib");
            }
        }
        break;
    case INSIDE_TARGET:
        {
            // line should be '<filename>' or 'end' or '$<option>
            const char* tok = get_token();
            if (tok[0] == '$') {
                tok++;
                if (strcmp(tok, "config") == 0) {
                    const char* tok2 = get_token();
                    if (!tok2) error("missing config name");
                    std::string name = tok2;
                    std::string value = "1";
                    tok2 = get_token();
                    if (tok2) value = tok2;
                    // TODO check duplicate configs
                    current->addConfig(Config(name, value));
                    if (get_token()) error("expected end of line");
                } else if (strcmp(tok, "export") == 0) {
                    while (1) {
                        const char* tok2 = get_token();
                        if (!tok2) break;
                        if (current->hasExported(tok2)) {
                            error("duplicate module '%s'", tok2);
                        }
                        current->addExported(tok2);
                    }
                } else if (strcmp(tok, "generate-c") == 0) {
                    current->generateCCode = true;
                    current->CGenFlags.single_module = false;
                    handleCConfigs();
                } else if (strcmp(tok, "generate-ir") == 0) {
                    handleIrGenFlags();
                } else if (strcmp(tok, "enable-assert") == 0) {
                    current->enableAsserts = true;
                } else if (strcmp(tok, "write-AST") == 0) {
                    current->writeAST = true;
                } else if (strcmp(tok, "warnings") == 0) {
                    handleWarnings();
                } else if (strcmp(tok, "nolibc") == 0) {
                    current->noLibC = true;
                } else if (strcmp(tok, "use") == 0) {
                    // syntax: $use <libname> <type>,  type = static/dynamic
                    const char* tok2 = get_token();
                    if (!tok2) error("missing library name");
                    std::string libname = tok2;
                    const char* tok3 = get_token();
                    if (!tok3) error("missing library type");
                    Component::Type libtype = Component::MAIN_SHARED_LIB;
                    if (strcmp(tok3, "static") == 0) {
                        libtype = Component::MAIN_STATIC_LIB;
                    } else if (strcmp(tok3, "dynamic") == 0) {
                        libtype = Component::MAIN_SHARED_LIB;
                    } else if (strcmp(tok3, "source") == 0) {
                        libtype = Component::MAIN_SOURCE_LIB;
                    } else {
                        error("unknown library type '%s'", tok3);
                    }
                    if (current->name == libname) {
                        error("cannot have dependency on self for target %s", libname.c_str());
                    }
                    if (current->hasLibrary(libname)) {
                        error("duplicate dependency for %s", libname.c_str());
                    }
                    current->addLibrary(libname, libtype);
                } else if (strcmp(tok, "plugin") == 0) {
                        const char* plugin_name = get_token();
                        if  (!plugin_name) error("expected plugin name");
                        // TODO check for trailing  stuff? (plugin settings?)

                        // search for duplicates
                        for (unsigned i=0; i<current->plugins.size(); i++) {
                            if (current->plugins[i].first == plugin_name) {
                                warn("duplicate plugin %s", plugin_name);
                                return;
                            }
                        }

                        const char* plugin_config = get_options();
                        current->plugins.push_back(Recipe::Plugin(plugin_name, plugin_config ? plugin_config : ""));
                } else {
                    error("unknown option '%s'", tok);
                }
            } else if (strcmp(tok, "end") == 0) {
                checkCurrent();
                state = START;
                current = 0;
            } else {
                struct stat buf;
                int err = stat(tok, &buf);
                if (err) {
                    error("file '%s' does not exist", tok);
                }
                err = access(tok, R_OK);
                if (err) {
                	error("missing read permissions for file: %s", tok);
                }
                current->addFile(tok);
            }
        }
        break;
    }
}

void RecipeReader::handleCConfigs() {
    while (1) {
        const char* flag = get_token();
        if (!flag) break;

        if (strcmp(flag, "single-module") == 0) {
            current->CGenFlags.single_module = true;
        } else if (strcmp(flag, "multi-module") == 0) {
            current->CGenFlags.single_module = false;
        } else if (strcmp(flag, "no-build") == 0) {
            current->CGenFlags.no_build = true;
        } else if (strcmp(flag, "skip") == 0) {
            current->generateCCode = false;
        } else if (strcmp(flag, "check") == 0) {
            current->CGenFlags.gen_checks = true;
        } else {
            error("unknown c generation flag '%s'", flag);
        }
    }
}

void RecipeReader::handleWarnings() {
    while (1) {
        const char* flag = get_token();
        if (!flag) break;

        if (strcmp(flag, "no-unused") == 0) {
            current->WarningFlags.no_unused = true;
        } else if (strcmp(flag, "no-unused-variable") == 0) {
            current->WarningFlags.no_unused_variable = true;
        } else if (strcmp(flag, "no-unused-function") == 0) {
            current->WarningFlags.no_unused_function = true;
        } else if (strcmp(flag, "no-unused-parameter") == 0) {
            current->WarningFlags.no_unused_parameter = true;
        } else if (strcmp(flag, "no-unused-type") == 0) {
            current->WarningFlags.no_unused_type = true;
        } else if (strcmp(flag, "no-unused-module") == 0) {
            current->WarningFlags.no_unused_module = true;
        } else if (strcmp(flag, "no-unused-import") == 0) {
            current->WarningFlags.no_unused_import = true;
        } else if (strcmp(flag, "no-unused-public") == 0) {
            current->WarningFlags.no_unused_public = true;
        } else if (strcmp(flag, "no-unused-label") == 0) {
            current->WarningFlags.no_unused_label = true;
        } else if (strcmp(flag, "no-unused-enum-constant") == 0) {
            // TODO
        } else if (strcmp(flag, "promote-to-error") == 0) {
            // TODO
        } else {
            error("unknown warning '%s'", flag);
        }
    }
}

void RecipeReader::handleIrGenFlags() {
    current->generateIR = true;

    while (1) {
        const char* flag = get_token();
        if (!flag) break;

        if (strcmp(flag, "single-module") == 0) {
            current->IrGenFlags.single_module = true;
        } else if (strcmp(flag, "single-threaded") == 0) {
            current->IrGenFlags.single_threaded = true;
        } else if (strcmp(flag, "keep-intermediates") == 0) {
            current->IrGenFlags.keep_intermediates = true;
        } else {
            error("unknown IR generation flag '%s'", flag);
        }
    }
}

char* RecipeReader::get_options() {
    // skip white space
    while (*token_ptr == ' ' || *token_ptr == '\t' ) token_ptr++;
    if (*token_ptr == 0) return 0;


    // search for [ <options> ]

    // find start [
    char* cp = token_ptr;
    if (*cp != '[') return 0;
    cp++;

    // find end [
    char* result = cp;
    while (*cp != 0 && *cp != ']') cp++;
    if (*cp == 0) { // token ends at end of line
        error("missing terminating ']'");
    }

    *cp = 0;
    token_ptr = cp + 1;
    return result;
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
    exit(EXIT_FAILURE);
}

void RecipeReader::warn(const char* fmt, ...) {
    char tmp[256];

    va_list argp;
    char* cp = tmp;

    cp += sprintf(cp, "recipe: line %d ", line_nr);
    va_start(argp, fmt);
    int size = vsprintf(cp, fmt, argp);
    cp += size;
    va_end(argp);
    fprintf(stderr, "Warning: %s\n", tmp);
}

Recipe& RecipeReader::get(int i) const {
    return *recipes[i];
}

void RecipeReader::print() const {
    printf("Targets:\n");
    for (unsigned i=0; i<recipes.size(); i++) {
        Recipe* R = recipes[i];
        printf("  %s\n", R->name.c_str());
    }
}

void RecipeReader::checkCurrent() {
    // lib targets must have export entry
    bool needExport = false;
    switch (current->type) {
    case Component::MAIN_EXECUTABLE:
        needExport = false;
        break;
    case Component::MAIN_SHARED_LIB:
    case Component::MAIN_STATIC_LIB:
        needExport = true;
        break;
    case Component::MAIN_SOURCE_LIB:
        needExport = false;
        break;
    case Component::EXT_SHARED_LIB:
    case Component::EXT_STATIC_LIB:
    case Component::EXT_SOURCE_LIB:
    case Component::INTERNAL:
    case Component::PLUGIN:
        needExport = false;
        break;
    }
    if (needExport && current->exported.size() == 0) {
        fprintf(stderr, "recipe: target %s is type lib but has no 'export' entry\n", current->name.c_str());
        exit(EXIT_FAILURE);
    }
}

