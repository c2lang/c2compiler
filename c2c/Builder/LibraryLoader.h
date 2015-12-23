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

#ifndef BUILDER_LIBRARY_LOADER_H
#define BUILDER_LIBRARY_LOADER_H

#include <string>
#include <map>

#include "AST/Component.h"
#include "CGenerator/HeaderNamer.h"
#include "Utils/StringList.h"

struct dirent;

namespace C2 {

class Module;

class LibraryLoader : public HeaderNamer {
public:
    LibraryLoader(Components& components_, const char* libdir_)
        : libdir(libdir_)
        , components(components_)
    {}
    ~LibraryLoader();

    void addLib(const std::string& name) {
        compNames.push_back(name);
    }
    void scan();
    void showLibs(bool useColors) const;

    Module* loadModule(const std::string& moduleName);

    virtual const std::string& getIncludeName(const std::string& modName) const;
private:
    Component* getComponent(const std::string& name, bool isCLib);

    std::string libdir;

    StringList compNames;
    Components& components;

    struct File {
        File(const std::string& h, const std::string& f, const std::string& c, bool isClib_)
            : headerFile(h), c2file(f), component(c), isClib(isClib_)
        {}
        std::string headerFile;
        std::string c2file;
        const std::string& component;
        bool isClib;
    };

    typedef std::map<std::string, File*> Libraries;
    typedef Libraries::const_iterator LibrariesConstIter;
    typedef Libraries::iterator LibrariesIter;
    Libraries libs;
};

}

#endif

