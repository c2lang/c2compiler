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

#ifndef RECIPE_H
#define RECIPE_H

#include <string>
#include <vector>

namespace C2 {

class Recipe {
public:
    Recipe(const std::string& name_) : name(name_) {}
    ~Recipe() {}

    void addFile(const std::string& name_);
    void addConfig(const std::string& config_);
    int size() const { return files.size(); }
    const std::string& get(int i) const;

    std::string name;

    typedef std::vector<std::string> Files;
    Files files;

    typedef std::vector<std::string> Configs;
    Configs configs;
private:
    Recipe(const Recipe&);
    Recipe& operator= (const Recipe&);
};

}

#endif

