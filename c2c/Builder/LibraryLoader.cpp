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

#include "Builder/LibraryLoader.h"
#include "Builder/ManifestReader.h"
#include "Builder/BuilderConstants.h"
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

void LibraryLoader::addDep(Component* src, const std::string& dest, Component::Type type) {
    deps.push_back(Dependency(src, dest, type));
}

bool LibraryLoader::scan() {
    struct stat statbuf;
    int err = stat(libdir.c_str(), &statbuf);
    if (err) {
        fprintf(stderr, "c2c: error: cannot open libdir %s: %s\n", libdir.c_str(), strerror(errno));
        // just continue to get 'cannot find library X' errors
    }

    assert(components.size() == 1);     // only main component
    Component* C = components[0];
    // create libs entry for MainComponent
    const ModuleList& mainMods = C->getModules();
    for (unsigned i=0; i<mainMods.size(); i++) {
        Module* M = mainMods[i];
        modules[M->getName()] = M;
        libs[M->getName()] = new LibInfo("", "", C, M, true);
    }
    // Dependencies for main component have already been added

    bool hasErrors = false;
    while (!deps.empty()) {
        Dependency dep = deps.front();
        deps.pop_front();
        hasErrors |= checkLibrary(dep);
    }

    if (!hasErrors) {
#if 0
        printf("before\n");
        for (unsigned i=0; i<components.size(); i++) { printf("  %s\n", components[i]->name.c_str()); }
#endif
        // sort Components by dep, bottom->top
        std::sort(components.begin(), components.end(), Component::compareDeps);
#if 0
        printf("after\n");
        for (unsigned i=0; i<components.size(); i++) { printf("  %s\n", components[i]->name.c_str()); }
#endif
    }
    return !hasErrors;
}

void LibraryLoader::showLibs(bool useColors) const {
    StringBuilder out(8192);
    out.enableColor(useColors);
    out << "libraries:\n";

    out << "  ";
    out.setColor(ANSI_BLUE);
    out << libdir;
    out.setColor(ANSI_NORMAL);
    out << '\n';
    DIR* dir = opendir(libdir.c_str());
    if (dir == NULL) {
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
                ManifestReader manifest(entry->d_name, fullname);
                if (!manifest.parse()) {
                    fprintf(stderr, "c2c: error: %s: %s\n", fullname, manifest.getErrorMsg());
                    goto next;
                }
                out << "  ";
                out.setColor(ANSI_YELLOW);
                if (manifest.hasStatic()) out << "static";
                if (manifest.hasDynamic()) {
                    if (manifest.hasStatic()) out << '|';
                    out << "dynamic";
                }
                out << "  ";
                const StringList& deps = manifest.getDeps();
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

                for (unsigned e=0; e<manifest.numEntries(); e++) {
                    const ManifestEntry& entry2 = manifest.get(e);
                    out << "      " << entry2.name << '\n';
                }
            }
        }
next:
        entry = readdir(dir);
    }
    puts(out);
}

const LibInfo* LibraryLoader::findModuleLib(const std::string& moduleName) const {
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

Component* LibraryLoader::createComponent(const std::string& name, bool isCLib, Component::Type type) {
    Component* C = new Component(name, type, true, isCLib, exportList);
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

bool LibraryLoader::checkLibrary(const Dependency& dep) {
    bool hasErrors = false;
    const std::string& componentName = dep.name;

    Component* C = findComponent(componentName);
    if (C) {
        dep.src->addDep(C);
        return false;
    }

    char fullname[512];
    sprintf(fullname, "%s/%s/%s", libdir.c_str(), componentName.c_str(), MANIFEST_FILE);
    struct stat statbuf;
    int err = stat(fullname, &statbuf);
    if (err) {
        fprintf(stderr, "c2c: error: cannot find library '%s'\n", componentName.c_str());
        return true;
    }

    ManifestReader manifest(componentName, fullname);
    if (!manifest.parse()) {
        fprintf(stderr, "c2c: error: %s: %s\n", fullname, manifest.getErrorMsg());
        return true;
    }

    if (dep.type == Component::SHARED_LIB && !manifest.hasDynamic()) {
        fprintf(stderr, "c2c: error: '%s' is not available as shared library in %s\n",
            componentName.c_str(), fullname);
        hasErrors = true;
    }
    if (dep.type == Component::STATIC_LIB && !manifest.hasStatic()) {
        fprintf(stderr, "c2c: error: '%s' is not available as static library in %s\n",
            componentName.c_str(), fullname);
        hasErrors = true;
    }

    C = createComponent(componentName, !manifest.isNative(), dep.type);
    dep.src->addDep(C);
    C->setLinkInfo("", manifest.getLinkName());
    const StringList& libDeps = manifest.getDeps();
    for (unsigned i=0; i<libDeps.size(); i++) {
        addDep(C, libDeps[i], dep.type);
    }

    for (unsigned e=0; e<manifest.numEntries(); e++) {
        const ManifestEntry& entry = manifest.get(e);
        const std::string& moduleName = entry.name;
        Module* M = C->getModule(moduleName);

        ModulesConstIter iter = modules.find(moduleName);
        if (iter != modules.end()) {
            const Component* other = findModuleComponent(moduleName);
            assert(other);
            fprintf(stderr, "c2c: error: module-name '%s' is used in %s and %s\n",
                moduleName.c_str(), C->getName().c_str(), other->getName().c_str());
            hasErrors = true;
        } else {
            modules[moduleName] = M;
        }

        StringBuilder c2file(512);
        c2file << libdir << '/' << componentName << '/' << moduleName << ".c2i";

        libs[moduleName] = new LibInfo(moduleName + ".h", c2file.c_str(), C, M, !manifest.isNative());
    }
    return hasErrors;
}

