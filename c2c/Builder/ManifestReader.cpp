/* Copyright 2013-2018 Bas van den Berg
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

#include "Builder/ManifestReader.h"
#include "FileUtils/TomlReader.h"
#include "AST/Component.h"

using namespace C2;

// TODO move to Component
static Component::Type str2dep(const char* type) {
    if (strcmp("static", type) == 0) return Component::STATIC_LIB;
    if (strcmp("dynamic", type) == 0) return Component::SHARED_LIB;
    return (Component::Type)-1;
}

bool ManifestReader::parse()
{
    TomlReader reader;
    if (!reader.parse(filename.c_str())) return false;

    const char* lang = reader.getValue("library.language");
    if (!lang) {
        printf("%s: missing library.language\n", filename.c_str());
        return false;
    }
    isNative_ = (strcmp(lang, "C2") == 0);

    // optional
    const char* linkname = reader.getValue("library.linkname");
    if (linkname) linkName = linkname;

    // required
    TomlReader::ValueIter typeIter = reader.getValueIter("library.type");
    while (!typeIter.done()) {
        const char* typeStr = typeIter.getValue();
        Component::Type type = str2dep(typeStr);
        if ((int)type == -1) {
            printf("%s: invalid dep type '%s'\n", filename.c_str(), typeStr);
            return false;
        }
        switch (type) {
        case Component::EXECUTABLE:
            break;
        case Component::SHARED_LIB:
            hasDynamicLib = true;
            break;
        case Component::STATIC_LIB:
            hasStaticLib = true;
            break;
        }
        typeIter.next();
    }

    TomlReader::NodeIter depsIter = reader.getNodeIter("deps");
    while (!depsIter.done()) {
        const char* depName = depsIter.getValue("name");
        const char* depType = depsIter.getValue("type");
        if (!depName || !depType) {
            printf("%s: dependency needs name and type\n", filename.c_str());
            return false;
        }
        Component::Type type = str2dep(depType);
        if ((int)type == -1) {
            printf("%s: invalid dep type '%s'\n", filename.c_str(), depType);
            return false;
        }
        for (unsigned i=0; i<deps.size(); i++) {
            if (deps[i] == depName) {
                sprintf(errorMsg, "duplicate dependency on '%s'", depName);
                return false;
            }
        }
        // TODO also store/use deps type
        deps.push_back(depName);
        depsIter.next();
    }

    TomlReader::NodeIter moduleIter = reader.getNodeIter("modules");
    while (!moduleIter.done()) {
        const char* name = moduleIter.getValue("name");
        if (!name) {
            printf("%s: missing modules need a need\n", filename.c_str());
            return false;
        }
        entries.push_back(ManifestEntry(name));
        moduleIter.next();
    }
    return true;
}

