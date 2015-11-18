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

using namespace C2;

LibraryLoader::~LibraryLoader() {
    for (LibrariesConstIter iter = libs.begin(); iter != libs.end(); ++iter) {
        delete iter->second;
    }
}

void LibraryLoader::scan() {
    struct stat statbuf;
    int err = stat(libdir.c_str(), &statbuf);
    if (err) {
        fprintf(stderr, "c2c: error: cannot open libdir %s: %s\n", libdir.c_str(), strerror(errno));
        // just continue to get 'cannot find library X' errors
    }
    for (unsigned i=0; i<components.size(); i++) {
        Component* C = components[i];
        if (!C->isExternal) continue;

        char fullname[512];
        sprintf(fullname, "%s/%s/%s", libdir.c_str(), C->name.c_str(), MANIFEST_FILE);
        err = stat(fullname, &statbuf);
        if (err) {
            fprintf(stderr, "c2c: error: cannot find library '%s'\n", C->name.c_str());
            continue;
        }

        ManifestReader manifest(fullname);
        if (!manifest.parse()) {
            fprintf(stderr, "c2c: error: %s: %s\n", fullname, manifest.getErrorMsg());
            continue;
        }

        for (unsigned e=0; e<manifest.numEntries(); e++) {
            const ManifestEntry& entry = manifest.get(e);
            // TODO check if already exists
            StringBuilder c2file(512);
            c2file << libdir << '/' << C->name << '/' << entry.name << ".c2i";
            libs[entry.name] = new File(entry.headerFile, c2file.c_str(), C, !manifest.isNative());
        }
    }
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
            out << "    " << entry->d_name << '\n';
            {
                // TODO parse manifest, print modules
                ManifestReader manifest(fullname);
                if (!manifest.parse()) {
                    fprintf(stderr, "c2c: error: %s: %s\n", fullname, manifest.getErrorMsg());
                    goto next;
                }

                for (unsigned e=0; e<manifest.numEntries(); e++) {
                    const ManifestEntry& entry = manifest.get(e);
                    out << "      " << entry.name << '\n';
                }
            }
        }
next:
        entry = readdir(dir);
    }
    puts(out);
}

C2::Module* LibraryLoader::loadModule(const std::string& moduleName) {
    LibrariesConstIter iter = libs.find(moduleName);
    if (iter == libs.end()) return 0;

    const File* file = iter->second;
    file->component->addFile(file->c2file);

    Module* M = new Module(moduleName, true, file->isClib);
    return M;
}

const std::string& LibraryLoader::getIncludeName(const std::string& modName) {
    LibrariesConstIter iter = libs.find(modName);
    assert(iter != libs.end());
    const File* file = iter->second;
    return file->headerFile;
}

