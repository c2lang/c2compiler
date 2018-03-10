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

#include "Builder/BuildFileReader.h"
#include "FileUtils/TomlReader.h"

#include <string.h>
#include <stdlib.h>

using namespace C2;

BuildFileReader::BuildFileReader()
    : target("")
    , cc("")
    , cflags("")
    , ldflags("")
{
    errorMsg[0] = 0;
}

bool BuildFileReader::parse(const std::string& filename)
{
    TomlReader reader(filename.c_str());
    if (!reader.parse()) {
        strcpy(errorMsg, reader.getErrorMsg());
        return false;
    }

    const char* target_ = reader.getValue("target");
    target = expandEnvVar(filename, target_);

    const char* cc_ = reader.getValue("toolchain.cc");
    cc = expandEnvVar(filename, cc_);

    const char* cflags_ = reader.getValue("toolchain.cflags");
    cflags = expandEnvVar(filename, cflags_);

    const char* ldflags_ = reader.getValue("toolchain.ldflags");
    ldflags = expandEnvVar(filename, ldflags_);

    TomlReader::NodeIter iter = reader.getNodeIter("libdir");
    while (!iter.done()) {
        // dir is required
        const char* dir = iter.getValue("dir");
        if (!dir) {
            sprintf(errorMsg, "%s: error: missing dir entry in [[libdir]]", filename.c_str());
            return false;
        }
        const char* expanded = expandEnvVar(filename, dir);
        if (expanded) libDirs.push_back(expanded);

        iter.next();
    }
    return true;
}

const char* BuildFileReader::expandEnvVar(const std::string& filename, const char* raw) {
    if (!raw) return "";
    if (raw[0] != '$') return "";
    const char* expand = getenv(raw + 1);
    if (!expand) {
        printf("%s: warning: environment variable '%s' not set\n", filename.c_str(), raw + 1);
        return "";
    }
    return expand;
}

