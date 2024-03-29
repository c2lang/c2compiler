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

#ifndef BUILDER_RECIPE_READER_H
#define BUILDER_RECIPE_READER_H

#include <stdarg.h>
#include <string>
#include <vector>
#include "Builder/Recipe.h"

namespace C2 {

class RecipeReader {
public:
    RecipeReader();
    ~RecipeReader();

    int count() const { return recipes.size(); }
    Recipe& get(int i) const;

    void print() const;
    const Recipe::Plugins& getPlugins() const { return plugins; }

private:
    void startNewRecipe();
    void findTopDir();
    void readRecipeFile();
    void handleLine(char* line);
    char* get_options();
    char* get_token();
    void error(const char *fmt, ...);
    void warn(const char *fmt, ...);
    void checkCurrent();
    void handleCConfigs();
    void handleWarnings();
    void handleDepflags();
    void handleIrGenFlags();

    typedef std::vector<Recipe*> Recipes;
    Recipes recipes;
    Recipe* current;
    ConfigList globalConfigs;
    Recipe::Plugins plugins;

    int line_nr;
    enum State { START=0, INSIDE_TARGET };
    State state;
    bool has_targets;

    char buffer[256];
    char* token_ptr;

    RecipeReader(const RecipeReader&);
    RecipeReader& operator= (const RecipeReader&);
};

}

#endif

