/* Copyright 2013-2016 Bas van den Berg
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

#ifndef AST_COMPONENT_H
#define AST_COMPONENT_H

#include <string>
#include <vector>

#include "AST/Module.h"
#include "Utils/StringList.h"
#include "Utils/GenUtils.h"

namespace C2 {

class AST;
class StringBuilder;

class Component {
public:
    Component(const std::string& name_, bool isExternal_, bool isCLib_, const StringList& exportList_)
        : name(name_)
        , is_external(isExternal_)
        , is_clib(isCLib_)
        , exportList(exportList_)
    {}
    ~Component();

    const std::string& getName() const { return name; }
    bool isExternal() const { return is_external; }
    bool isCLib() const { return is_clib; }

    void print(StringBuilder& output) const;
    void printSymbols(StringBuilder& output) const;


    Module* getModule(const std::string& name);
    const ModuleList& getModules() const { return modules; }

    void addDep(const GenUtils::Dependency& dep) {
        deps.push_back(dep);
    }
    const GenUtils::Dependencies& getDeps() const { return deps; }
    bool hasDep(const Component* other) const;

    static bool compareDeps(const Component* a, const Component* b) {
        if (a->hasDep(b)) return false;
        if (b->hasDep(a)) return true;
        return true;
    }
private:
    bool isExported(const std::string& moduleName) const;

    std::string name;
    bool is_external;
    bool is_clib;

    ModuleList modules;
    const StringList& exportList;
    GenUtils::Dependencies deps;

    Component(const Component&);
    Component& operator= (const Component&);
};

typedef std::vector<Component*> Components;

}

#endif

