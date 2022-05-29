/* Copyright 2013-2022 Bas van den Berg
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

#include "Builder/LibraryLoader.h"
#include "Builder/Manifest.h"
#include "Builder/ManifestReader.h"
#include "Builder/BuilderConstants.h"
#include "Builder/Recipe.h"
#include "AST/Module.h"
#include "Utils/Utils.h"
#include "Utils/StringBuilder.h"
#include "Utils/color.h"
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <algorithm>

using namespace C2;

LibraryLoader::~LibraryLoader() {
    for (LibrariesConstIter iter = libs.begin(); iter != libs.end(); ++iter) {
        delete iter->second;
    }
}

void LibraryLoader::addDir(const std::string& libdir) {
    // TODO check duplicates
    libraryDirs.push_back(libdir);
}

void LibraryLoader::addDep(Component* src, const std::string& dest, Component::Type type) {
    Component* C = findComponent(dest);
    if (C) {
        if (C->getType() != type) {
            // main shared lib, ext shared lib
            fprintf(stderr, "c2c: error: different library types needed for %s\n", dest.c_str());
            TODO;
        }
    } else {
        C = createComponent(dest, "", true, type);
    }
    src->addDep(C);
}

void LibraryLoader::checkLibDirs() {
    for (StringListConstIter iter = libraryDirs.begin(); iter != libraryDirs.end(); ++iter) {
        const std::string& libdir = *iter;
        struct stat statbuf;
        int err = stat(libdir.c_str(), &statbuf);
        if (err) {
            fprintf(stderr, "c2c: error: cannot open library dir %s: %s\n", libdir.c_str(), strerror(errno));
            // just continue to get 'cannot find library X' errors
        }
    }
}

bool LibraryLoader::createComponents() {
    createMainComponent();
    checkLibDirs();

    return loadExternals();
}

C2::Component* LibraryLoader::getPluginComponent() {
    if (!pluginComponent) {
        pluginComponent = new Component("plugins", "", Component::PLUGIN, true, true, false, recipe.getExports());
        components.push_back(pluginComponent);
    }
    return pluginComponent;
}

void LibraryLoader::createMainComponent() {
    mainComponent = new Component(recipe.name, "TODO", recipe.type, false, false, false, recipe.getExports());
    components.push_back(mainComponent);

    if (!recipe.noLibC) {
        // NOTE: libc always EXT_SHARED_LIB for now
        addDep(mainComponent, "libc", Component::EXT_SHARED_LIB);
    }
    for (unsigned i=0; i<recipe.libraries.size(); i++) {
        addDep(mainComponent, recipe.libraries[i].name, recipe.libraries[i].type);
    }
}

bool LibraryLoader::loadExternals() {
    bool hasErrors = false;
    for (unsigned i=0; i<components.size(); i++) {
        hasErrors |= checkComponent(components[i]);
    }

#if 0
    printf("before\n");
    for (unsigned i=0; i<components.size(); i++) { printf("  %s\n", components[i]->getName().c_str()); }
#endif
    // sort Components by dep, bottom->top
    std::sort(components.begin(), components.end(), Component::compareDeps);
#if 0
    printf("after\n");
    for (unsigned i=0; i<components.size(); i++) { printf("  %s\n", components[i]->getName().c_str()); }
#endif
    return !hasErrors;
}

bool LibraryLoader::checkMainModule(const Module* m) const {
    ModulesConstIter iter = modules.find(m->getName());
    if (iter != modules.end()) {
        const Component* c = findModuleComponent(m->getName());
        assert(c);
        fprintf(stderr, "c2c: error: module %s already exists in external component %s\n", m->getName().c_str(), c->getName().c_str());
        return false;
    }
    return true;
}

const LibInfo* LibraryLoader::addModule(Component* C, Module* M, const std::string& header, const std::string& file) {
    modules[M->getName()] = M;
    LibInfo* L = new LibInfo(header, file, C, M);
    libs[M->getName()] = L;
    return L;
}

bool LibraryLoader::checkComponent(Component* C) {
    if (C == mainComponent) return false;   // skip main component
    if (C->isInternalOrPlugin()) return false;

    const std::string& name = C->getName();

    // search all libDirs until file is found
    char componentDir[512];
    if (!findComponentDir(name, componentDir)) {
        fprintf(stderr, "c2c: error: cannot find library '%s'\n", name.c_str());
        return true;
    }
    //printf("LIBS: found %s in %s\n", name.c_str(), componentDir);
    char fullname[512];
    sprintf(fullname, "%s/%s", componentDir, MANIFEST_FILE);

    Manifest manifest(fullname);
    ManifestReader reader(manifest);
    if (!reader.parse()) {
        fprintf(stderr, "c2c: error: %s: %s\n", fullname, reader.getErrorMsg());
        return true;
    }
    bool hasErrors = false;
    if (C->getType() == Component::MAIN_SHARED_LIB && !manifest.hasDynamicLib) {
        fprintf(stderr, "c2c: error: '%s' is not available as shared library in %s\n",
            name.c_str(), fullname);
        hasErrors = true;
    }
    if (C->getType() == Component::MAIN_STATIC_LIB && !manifest.hasStaticLib) {
        fprintf(stderr, "c2c: error: '%s' is not available as static library in %s\n",
            name.c_str(), fullname);
        hasErrors = true;
    }

    C->updateFields(!manifest.isNative, componentDir, manifest.linkName);

    const StringList& libDeps = manifest.deps;
    for (unsigned i=0; i<libDeps.size(); i++) {
        addDep(C, libDeps[i], Component::EXT_SHARED_LIB);
    }

    for (unsigned i=0; i<manifest.modules.size(); i++) {
        const std::string& moduleName = manifest.modules[i];

        ModulesConstIter iter = modules.find(moduleName);
        if (iter != modules.end()) {
            const Component* other = findModuleComponent(moduleName);
            assert(other);
            fprintf(stderr, "c2c: error: module-name '%s' is used in %s and %s\n",
                moduleName.c_str(), C->getName().c_str(), other->getName().c_str());
            hasErrors = true;
        } else {
            Module* M = C->getModule(moduleName);

            StringBuilder c2file(512);
            c2file << componentDir << '/' << moduleName;
            if (manifest.hasSourceLib) {
                c2file << ".c2";
            } else {
                c2file << ".c2i";
            }
            addModule(C, M, moduleName + ".h", c2file.c_str());
        }
    }
    return hasErrors;
}

void LibraryLoader::showLib(StringBuilder& out, const std::string& libdir, bool useColors, bool showModules) const {
    out << "  ";
    out.setColor(ANSI_BLUE);
    out << libdir;
    out.setColor(ANSI_NORMAL);
    out << '\n';
    DIR* dir = opendir(libdir.c_str());
    if (dir == 0) {
        fprintf(stderr, "c2c: error: cannot open library dir '%s'\n", libdir.c_str());
        return;
    }
    struct dirent* entry = readdir(dir);
    while (entry != 0) {
        if (entry->d_name[0] == '.') goto next;
        if (entry->d_type == DT_DIR) {

            // check for manifest file
            char fullname[512];
            sprintf(fullname, "%s/%s/%s", libdir.c_str(), entry->d_name, MANIFEST_FILE);
            struct stat statbuf;
            int err = stat(fullname, &statbuf);
            if (err) goto next;
            out << "    " << entry->d_name;
            {
                Manifest manifest(fullname);
                ManifestReader reader(manifest);
                if (!reader.parse()) {
                    fprintf(stderr, "c2c: error: %s: %s\n", fullname, reader.getErrorMsg());
                    goto next;
                }
                out << "  ";
                out.setColor(ANSI_YELLOW);
                bool first = true;
                if (manifest.hasStaticLib) {
                    out << "static";
                    first = false;
                }
                if (manifest.hasDynamicLib) {
                    if (!first) out << '|';
                    out << "dynamic";
                    first = false;
                }
                if (manifest.hasSourceLib) {
                    if (!first) out << '|';
                    out << "source";
                    first = false;
                }
                out << "  ";
                const StringList& deps = manifest.deps;
                if (!deps.empty()) {
                    out.setColor(ANSI_MAGENTA);
                    out << "requires: ";
                    for (unsigned i=0; i<deps.size(); i++) {
                        if (i != 0) out << ", ";
                        out << deps[i];
                    }
                }
                out.setColor(ANSI_NORMAL);
                out << '\n';

                if (showModules) {
                    for (unsigned i=0; i<manifest.modules.size(); i++) {
                        out << "      " << manifest.modules[i] << '\n';
                    }
                }
            }
        }
next:
        entry = readdir(dir);
    }
}

void LibraryLoader::showLibs(bool useColors, bool showModules) const {
    StringBuilder out(8192);
    out.enableColor(useColors);
    out << "libraries:\n";

    for (StringListConstIter iter = libraryDirs.begin(); iter != libraryDirs.end(); ++iter) {
        std::string libdir = *iter;
        showLib(out, *iter, useColors, showModules);
    }
    puts(out);
}

const LibInfo* LibraryLoader::findModuleLib(const std::string& moduleName) const {
    // TODO search plugins as well
    LibrariesConstIter iter = libs.find(moduleName);
    if (iter == libs.end()) return 0;
    return iter->second;
}

const std::string& LibraryLoader::getIncludeName(const std::string& modName) const {
    LibrariesConstIter iter = libs.find(modName);
    assert(iter != libs.end());
    const LibInfo* file = iter->second;
    return file->headerLibInfo;
}

Component* LibraryLoader::findComponent(const std::string& name) const {
    for (unsigned i=0; i<components.size(); i++) {
        Component* C = components[i];
        if (C->getName() == name) return C;
    }
    return 0;
}

Component* LibraryLoader::createComponent(const std::string& name, const std::string& path, bool isCLib, Component::Type type) {
    bool isInterface = (type != Component::MAIN_SOURCE_LIB);
    Component* C = new Component(name, path, type, true, isInterface, isCLib, recipe.getExports());
    components.push_back(C);
    return C;
}

Component* LibraryLoader::findModuleComponent(const std::string& moduleName) const {
    for (unsigned i=0; i<components.size(); i++) {
        Component* C = components[i];
        if (C->findModule(moduleName)) return C;
    }
    return 0;
}

bool LibraryLoader::findComponentDir(const std::string& name, char* dirname) const {
    for (StringListConstIter iter = libraryDirs.begin(); iter != libraryDirs.end(); ++iter) {
        const std::string& libdir = *iter;
        char fullname[512];
        sprintf(fullname, "%s/%s/%s", libdir.c_str(), name.c_str(), MANIFEST_FILE);
        struct stat statbuf;
        if (stat(fullname, &statbuf) == 0) {
            sprintf(dirname, "%s/%s", libdir.c_str(), name.c_str());
            return true;
        }
    }
    dirname[0] = 0;
    return false;
}

