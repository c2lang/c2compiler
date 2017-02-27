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

#ifndef BUILDER_RECIPE_H
#define BUILDER_RECIPE_H

#include "Utils/StringList.h"
#include "AST/Component.h"

namespace C2 {

class Recipe {
public:
    Recipe(const std::string& name_, Component::Type type_)
        : name(name_)
        , type(type_)
        , generateDeps(false)
        , generateRefs(false)
        , generateIR(false)
        , generateCCode(false)
        , noLibC(false)
    {}
    ~Recipe() {}

    void addFile(const std::string& name_);
    void addConfig(const std::string& config_);
    void addExported(const std::string& mod_);
    void addAnsiCConfig(const std::string& config_);
    void addCodeGenConfig(const std::string& config_);
    void addDepsConfig(const std::string& config_);
    void addLibrary(const std::string& lib_, Component::Type type_);
    bool hasLibrary(const std::string& lib_) const;
    void silenceWarning(const std::string& warn_);

    unsigned size() const { return files.size(); }
    const std::string& get(int i) const;
    bool hasExported(const std::string& mod) const;
    const StringList& getExports() const { return exported; }
    bool needsInterface() const {
        switch (type) {
        case Component::EXECUTABLE:
            return false;
        case Component::SHARED_LIB:
        case Component::STATIC_LIB:
            return true;
        }
        return false;
    }

    std::string name;
    Component::Type type;

    bool generateDeps;
    bool generateRefs;
    bool generateIR;
    bool generateCCode;
    bool noLibC;

    StringList files;
    StringList configs;
    StringList exported;
    StringList cConfigs;
    StringList genConfigs;
    StringList depConfigs;

    struct Dependency {
        Dependency(const std::string& n, Component::Type t)
            : name(n), type(t)
        {}
        std::string name;
        Component::Type type;
    };

    typedef std::vector<Dependency> Dependencies;
    Dependencies libraries;

    StringList silentWarnings;
private:
    Recipe(const Recipe&);
    Recipe& operator= (const Recipe&);
};

}

#endif

