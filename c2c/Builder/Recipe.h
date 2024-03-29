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

#ifndef BUILDER_RECIPE_H
#define BUILDER_RECIPE_H

#include "Utils/StringList.h"
#include "AST/Component.h"

#include <vector>

namespace C2 {

typedef std::pair<std::string, std::string> Config;
typedef std::vector<Config> ConfigList;
typedef ConfigList::const_iterator ConfigsIter;

class Recipe {
public:
    Recipe(const std::string& name_, Component::Type type_)
        : name(name_)
        , type(type_)
        , generateIR(false)
        , generateCCode(false)
        , disableAsserts(false)
        , writeAST(false)
        , noLibC(false)
    {
        IrGenFlags.single_module = false;
        IrGenFlags.single_threaded = false;

        CGenFlags.single_module = false;
        CGenFlags.no_build = false;
        CGenFlags.gen_checks = false;
        CGenFlags.fast = false;

        WarningFlags.no_unused = false;
        WarningFlags.no_unused_variable = false;
        WarningFlags.no_unused_function = false;
        WarningFlags.no_unused_parameter = false;
        WarningFlags.no_unused_type = false;
        WarningFlags.no_unused_module = false;
        WarningFlags.no_unused_import = false;
        WarningFlags.no_unused_public = false;
        WarningFlags.no_unused_label = false;
    }
    ~Recipe() {}

    void addFile(const std::string& name_) {
        files.push_back(name_);
    }
    void addConfig(const Config& config_) {
        configs.push_back(config_);
    }
    void addExported(const std::string& mod_) {
        exported.push_back(mod_);
    }
    void addLibrary(const std::string& lib_, Component::Type type_);
    bool hasLibrary(const std::string& lib_) const;

    unsigned size() const { return files.size(); }

    const std::string& get(int i) const;
    bool hasExported(const std::string& mod) const;
    const StringList& getExports() const { return exported; }

    typedef std::pair<std::string, std::string> Plugin;
    typedef std::vector<Plugin> Plugins;
    const Plugins& getPlugins() const { return plugins; }
    Plugins plugins;

    bool needsInterface() const {
        switch (type) {
        case Component::MAIN_EXECUTABLE:
            return false;
        case Component::MAIN_SHARED_LIB:
        case Component::MAIN_STATIC_LIB:
            return true;
        case Component::MAIN_SOURCE_LIB:
            return false;
        case Component::EXT_SHARED_LIB:
        case Component::EXT_STATIC_LIB:
        case Component::EXT_SOURCE_LIB:
            return false;
        case Component::INTERNAL:
            return false;
        case Component::PLUGIN:
            return false;
        }
        return false;
    }

    std::string name;
    Component::Type type;

    bool generateIR;
    bool generateCCode;
    bool disableAsserts;
    bool writeAST;
    bool noLibC;

    struct {
        bool single_module;
        bool single_threaded;
        bool keep_intermediates;
    } IrGenFlags;

    struct {
        bool single_module;
        bool no_build;
        bool gen_checks;
        bool fast;
    } CGenFlags;

    struct {
        bool no_unused;
        bool no_unused_variable;
        bool no_unused_function;
        bool no_unused_parameter;
        bool no_unused_type;
        bool no_unused_module;
        bool no_unused_import;
        bool no_unused_public;
        bool no_unused_label;
    } WarningFlags;

    StringList files;
    ConfigList configs;
    StringList exported;

    struct Dependency {
        Dependency(const std::string& n, Component::Type t)
            : name(n), type(t)
        {}
        std::string name;
        Component::Type type;
    };

    typedef std::vector<Dependency> Dependencies;
    Dependencies libraries;

private:
    Recipe(const Recipe&);
    Recipe& operator= (const Recipe&);
};

}

#endif

