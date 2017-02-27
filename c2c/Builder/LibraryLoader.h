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

#ifndef BUILDER_LIBRARY_LOADER_H
#define BUILDER_LIBRARY_LOADER_H

#include <string>
#include <deque>
#include <map>

#include "AST/Component.h"
#include "CGenerator/HeaderNamer.h"
#include "Utils/StringList.h"

struct dirent;

namespace C2 {

class Module;

struct LibInfo {
    LibInfo(const std::string& h, const std::string& f, Component* c, Module* m, bool isClib_)
        : headerLibInfo(h), c2file(f), component(c), module(m), isClib(isClib_)
    {}
    std::string headerLibInfo;
    std::string c2file;
    Component* component;
    Module* module;
    bool isClib;
};

class LibraryLoader : public HeaderNamer {
public:
    LibraryLoader(Components& components_,
                  Modules& modules_,
                  const char* libdir_,
                  const StringList& exportList_)
        : libdir(libdir_)
        , components(components_)
        , modules(modules_)
        , exportList(exportList_)
    {}
    ~LibraryLoader();

    void addDep(Component* src, const std::string& dest, Component::Type type);

    bool scan();
    void showLibs(bool useColors) const;

    virtual const std::string& getIncludeName(const std::string& modName) const;

    const LibInfo* findModuleLib(const std::string& moduleName) const;
private:
    struct Dependency {
        Dependency(Component* s, const std::string& n, Component::Type t)
            : src(s), name(n), type(t)
        {}
        Component* src;
        std::string name;
        Component::Type type;
    };
    Component* findComponent(const std::string& name) const;
    Component* createComponent(const std::string& name, bool isCLib, Component::Type type);
    Component* findModuleComponent(const std::string& moduleName) const;
    bool checkLibrary(const Dependency& dep);
    void addDependencies(const Component* C);

    std::string libdir;

    Components& components;
    Modules& modules;

    typedef std::map<std::string, LibInfo*> Libraries; // moduleName -> LibInfo
    typedef Libraries::const_iterator LibrariesConstIter;
    typedef Libraries::iterator LibrariesIter;
    Libraries libs;

    const StringList& exportList;

    typedef std::deque<Dependency> Queue;
    Queue deps;
};

}

#endif

