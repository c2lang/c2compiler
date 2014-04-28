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

#ifndef BUILDER_RECIPE_READER_H
#define BUILDER_RECIPE_READER_H

#include <stdarg.h>
#include <string>
#include <vector>

namespace C2 {

class Recipe;

class RecipeReader {
public:
    RecipeReader();
    ~RecipeReader();

    int count() const { return recipes.size(); }
    const Recipe& get(int i) const;
    void print() const;
private:
    void findTopDir();
    void readRecipeFile();
    void handleLine(char* line);
    char* get_token();
    void error(const char *fmt, ...);

    typedef std::vector<Recipe*> Recipes;
    Recipes recipes;
    Recipe* current;

    int line_nr;
    enum State { START=0, INSIDE_TARGET };
    State state;

    char buffer[256];
    char* token_ptr;

    RecipeReader(const RecipeReader&);
    RecipeReader& operator= (const RecipeReader&);
};

}

#endif

