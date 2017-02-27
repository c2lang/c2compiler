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

#ifndef AST_COMPONENT_H
#define AST_COMPONENT_H

#include <string>
#include <vector>

#include "AST/Module.h"
#include "Utils/StringList.h"

namespace C2 {

class AST;
class StringBuilder;

class Component {
public:
    enum Type { EXECUTABLE=0, SHARED_LIB, STATIC_LIB };

    Component(const std::string& name_,
            Type type_,
            bool isExternal_,
            bool isCLib_,
            const StringList& exportList_)
        : name(name_)
          , type(type_)
          , is_external(isExternal_)
          , is_clib(isCLib_)
          , exportList(exportList_)
    {}
    ~Component();

    const std::string& getName() const { return name; }
    const std::string& getLinkPath() const { return linkPath; }
    const std::string& getLinkName() const { return linkName; }
    Type getType() const { return type; }
    bool isExternal() const { return is_external; }
    bool isCLib() const { return is_clib; }
    bool isSharedLib() const { return type == SHARED_LIB; }
    bool isStaticLib() const { return type == STATIC_LIB; }

    void setLinkInfo(const std::string& path_, const std::string& name_) {
        linkPath = path_;
        linkName = name_;
    }

    void print(StringBuilder& output) const;
    void printSymbols(StringBuilder& output) const;


    Module* getModule(const std::string& name_);
    const ModuleList& getModules() const { return modules; }
    Module* findModule(const std::string& name_) const;

    typedef std::vector<Component*> Dependencies;
    void addDep(Component* c) { deps.push_back(c); }
    const Dependencies& getDeps() const { return deps; }
    bool hasDep(const Component* other) const;

    static bool compareDeps(const Component* a, const Component* b) {
        if (a->hasDep(b)) return false;
        if (b->hasDep(a)) return true;
        return true;
    }
private:
    bool isExported(const std::string& moduleName) const;

    std::string name;
    std::string linkPath;       // used for external libs (-L..)
    std::string linkName;       // used for external libs (-l..)
    Type type;
    bool is_external;
    bool is_clib;

    ModuleList modules;
    const StringList& exportList;
    Dependencies deps;

    Component(const Component&);
    Component& operator= (const Component&);
};

typedef std::vector<Component*> Components;

const char* Str(Component::Type type);

}

#endif

