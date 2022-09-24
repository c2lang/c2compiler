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

#include "Builder/BuildFileReader.h"
#include "Utils/BuildFile.h"
#include "FileUtils/FileMap.h"

#include <string.h>
#include <stdlib.h>

using namespace C2;

BuildFileReader::BuildFileReader(BuildFile& build_)
    : build(build_)
{
    errorMsg[0] = 0;
}

bool BuildFileReader::findPlugin(const std::string& name) const {
    for (BuildFile::PluginsConstIter iter = build.plugins.begin(); iter != build.plugins.end(); ++iter) {
        if (iter->first == name) return true;
    }
    return false;
}

bool BuildFileReader::getInfo(YamlParser* parser, const std::string& filename) {
    const char* target = yaml_get_scalar_value(parser, "target");
    build.target = expandEnvVar(filename, target);

    const char* outputDir = yaml_get_scalar_value(parser, "output_dir");
    build.outputDir = expandEnvVar(filename, outputDir);

    const char* cc = yaml_get_scalar_value(parser, "toolchain.cc");
    build.cc = expandEnvVar(filename, cc);

    const char* cflags = yaml_get_scalar_value(parser, "toolchain.cflags");
    build.cflags = expandEnvVar(filename, cflags);

    const char* ldflags = yaml_get_scalar_value(parser, "toolchain.ldflags");
    build.ldflags = expandEnvVar(filename, ldflags);

    const char* ldflags2 = yaml_get_scalar_value(parser, "toolchain.ldflags2");
    build.ldflags2 = expandEnvVar(filename, ldflags2);

    const YamlNode* libs = yaml_find_node(parser, "libdir");
    YamlIter iter = yaml_node_get_child_iter(parser, libs);
    while (!yaml_iter_done(&iter)) {
        const char* dir = yaml_iter_get_scalar_value(&iter);
        const char* expanded = expandEnvVar(filename, dir);
        if (expanded) build.libDirs.push_back(expanded);
        yaml_iter_next(&iter);
    }

    const YamlNode* plugin_dirs = yaml_find_node(parser, "plugindir");
    iter = yaml_node_get_child_iter(parser, plugin_dirs);
    while (!yaml_iter_done(&iter)) {
        const char* dir = yaml_iter_get_scalar_value(&iter);
        const char* expanded = expandEnvVar(filename, dir);
        build.pluginDirs.push_back(expanded);
        yaml_iter_next(&iter);
    }

    const YamlNode* plugins = yaml_find_node(parser, "plugin");
    iter = yaml_node_get_child_iter(parser, plugins);
    while (!yaml_iter_done(&iter)) {
        // TODO (see below, also determine exact format)
        yaml_iter_next(&iter);
    }

#if 0
    iter = reader.getNodeIter("plugin");
    while (!iter.done()) {
        const char* name = iter.getValue("name");
        if (!name) {
            sprintf(errorMsg, "%s: error: missing name entry in [[plugin]]", filename.c_str());
            return false;
        }
        const char* expanded = expandEnvVar(filename, name);

        const char* options = iter.getValue("options");
        if (options) options = expandEnvVar(filename, options);

        // ignore duplicate plugins
        if (!findPlugin(expanded)) build.plugins.push_back(BuildFile::Plugin(expanded, options));

        iter.next();
    }
#endif
    return true;
}

bool BuildFileReader::parse(const std::string& filename)
{
    FileMap file(filename.c_str());
    file.open();
    YamlParser* parser = yaml_create((const char*)file.region, file.size);
    bool ok = yaml_parse(parser);
    file.close();
    if (ok) {
        ok = getInfo(parser, filename);
    } else {
        strcpy(errorMsg, yaml_get_error(parser));
    }
    yaml_destroy(parser);
    return ok;
}

const char* BuildFileReader::expandEnvVar(const std::string& filename, const char* raw) {
    if (!raw) return "";
    if (raw[0] != '$') return raw;
    const char* expand = getenv(raw + 1);
    if (!expand) {
        printf("%s: warning: environment variable '%s' not set\n", filename.c_str(), raw + 1);
        return "";
    }
    return expand;
}

