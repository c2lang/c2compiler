/* Copyright 2013-2015 Bas van den Berg
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

// TODO remove BBB (only for files)
#include "AST/AST.h"

namespace C2 {

class AST;
class StringBuilder;

class Component {
public:
    Component(const std::string& name_, bool isExternal_, bool isCLib_)
        : name(name_)
        , isExternal(isExternal_)
        , isCLib(isCLib_)
    {}
    ~Component();

    Module* addAST(AST* ast, const std::string& moduleName);

    void print(StringBuilder& output) const;

    std::string name;
    bool isExternal;
    bool isCLib;

    // TODO BBB REMOVE (kept to keep complication ok)
    Files files;

    ModuleList& getModules() { return modules; }
private:
    ModuleList modules;

    Module* getModule(const std::string& name);

    Component(const Component&);
    Component& operator= (const Component&);
};

typedef std::vector<Component*> Components;

}

#endif

