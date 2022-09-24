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

#include "Builder/ManifestReader.h"
#include "Builder/Manifest.h"
#include "FileUtils/YamlParser.h"
#include "FileUtils/FileMap.h"
#include "AST/Component.h"

using namespace C2;

// TODO move to Component
static Component::Type str2dep(const char* type) {
    if (strcmp("static", type) == 0)  return Component::MAIN_STATIC_LIB;
    if (strcmp("dynamic", type) == 0) return Component::MAIN_SHARED_LIB;
    if (strcmp("source", type) == 0)  return Component::MAIN_SOURCE_LIB;
    return (Component::Type)-1;
}

bool ManifestReader::parse()
{
    errorMsg[0] = 0;

    // TODO: fix error handling (not exit -1)
    FileMap file(manifest.filename.c_str());
    file.open();
    YamlParser* parser = yaml_create((const char*)file.region, file.size);
    bool ok = yaml_parse(parser);
    file.close();
    // TODO cleanup yaml_destroy() in destructor
    if (!ok) {
        printf("ERROR parsing %s: %s\n", manifest.filename.c_str(), yaml_get_error(parser));
        return false;
    }
    //yaml_dump(parser, true);

    // info.language (required)
    const char* lang = yaml_get_scalar_value(parser, "info.language");
    if (!lang) {
        sprintf(errorMsg, "missing library.language");
        return false;
    }
    manifest.isNative = (strcmp(lang, "C2") == 0);

    // info.linkname (optional)
    const char* linkname = yaml_get_scalar_value(parser, "info.linkname");
    if (linkname) manifest.linkName = linkname;

    // info.kinds (required)
    const YamlNode* kinds = yaml_find_node(parser, "info.kinds");
    if (!kinds) {
        sprintf(errorMsg, "missing info.kinds");
        return false;
    }
    YamlIter kinds_iter = yaml_node_get_child_iter(parser, kinds);
    while (!yaml_iter_done(&kinds_iter)) {
        const char* kind = yaml_iter_get_scalar_value(&kinds_iter);
        Component::Type type = str2dep(kind);
        if ((int)type == -1) {
            sprintf(errorMsg, "invalid dep type '%s'", kind);
            return false;
        }
        switch (type) {
        case Component::MAIN_EXECUTABLE:
            break;
        case Component::MAIN_SHARED_LIB:
            manifest.hasDynamicLib = true;
            break;
        case Component::MAIN_STATIC_LIB:
            manifest.hasStaticLib = true;
            break;
        case Component::MAIN_SOURCE_LIB:
            if (!manifest.isNative) {
                sprintf(errorMsg, "language must be C2 for source libraries");
                return false;
            }
            if (manifest.hasDynamicLib || manifest.hasStaticLib) {
                sprintf(errorMsg, "source library type cannot be combined with shared or static one");
                return false;
            }
            manifest.hasSourceLib = true;
            break;
        case Component::EXT_SHARED_LIB:
        case Component::EXT_STATIC_LIB:
        case Component::EXT_SOURCE_LIB:
        case Component::INTERNAL:
        case Component::PLUGIN:
            break;
        }

        yaml_iter_next(&kinds_iter);
    }


    // dependencies (optional)
    const YamlNode* deps = yaml_find_node(parser, "dependencies");
    if (deps) {
        YamlIter deps_iter = yaml_node_get_child_iter(parser, deps);
        while (!yaml_iter_done(&deps_iter)) {
            const char* name = yaml_iter_get_name(&deps_iter);
            const char* kind = yaml_iter_get_scalar_value(&deps_iter);
            if (!name || !name) {
                sprintf(errorMsg, "a dependency needs name and kind");
                return false;
            }
            Component::Type type = str2dep(kind);
            if ((int)type == -1) {
                sprintf(errorMsg, "invalid dep kind '%s'", kind);
                return false;
            }
            for (unsigned i=0; i<manifest.deps.size(); i++) {
                if (manifest.deps[i] == name) {
                    sprintf(errorMsg, "duplicate dependency on '%s'", name);
                    return false;
                }
            }
            // TODO check for self-dep

            // TODO also store/use deps type
            manifest.deps.push_back(name);
            yaml_iter_next(&deps_iter);
        }
    }

    // modules (required)
    const YamlNode* mods = yaml_find_node(parser, "modules");
    if (!mods) {
        sprintf(errorMsg, "missing modules");
        return false;
    }
    YamlIter mods_iter = yaml_node_get_child_iter(parser, mods);
    bool got_modules = false;
    while (!yaml_iter_done(&mods_iter)) {
        const char* name = yaml_iter_get_scalar_value(&mods_iter);
        manifest.modules.push_back(name);
        yaml_iter_next(&mods_iter);
        got_modules = true;
    }
    if (!got_modules) {
        sprintf(errorMsg, "there must be at least one module");
        return false;
    }

    yaml_destroy(parser);
    return true;
}

