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

#ifndef BUILDER_RECIPE_H
#define BUILDER_RECIPE_H

#include <string>
#include <vector>

namespace C2 {

class Recipe {
public:
    Recipe(const std::string& name_, bool isExec_)
        : name(name_) , isExec(isExec_), generateDeps(false), generateCCode(false) {}
    ~Recipe() {}

    void addFile(const std::string& name_);
    void addConfig(const std::string& config_);
    void addAnsiCConfig(const std::string& config_);
    void addCodeGenConfig(const std::string& config_);
    void addDepsConfig(const std::string& config_);
    void silenceWarning(const std::string& warn_);
    int size() const { return files.size(); }
    const std::string& get(int i) const;

    std::string name;
    bool isExec;
    bool generateDeps;
    bool generateCCode;

    typedef std::vector<std::string> Files;
    Files files;

    typedef std::vector<std::string> Configs;

    Configs configs;
    Configs cConfigs;
    Configs genConfigs;
    Configs depConfigs;
    Configs silentWarnings;
private:
    Recipe(const Recipe&);
    Recipe& operator= (const Recipe&);
};

}

#endif

