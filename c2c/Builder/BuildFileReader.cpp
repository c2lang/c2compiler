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

using namespace C2;

bool BuildFileReader::parse(const std::string& filename)
{
    TomlReader reader(filename.c_str());
    if (!reader.parse()) {
        strcpy(errorMsg, reader.getErrorMsg());
        return false;
    }

    target = reader.getValue("target");
    cc = reader.getValue("toolchain.cc");
    cflags = reader.getValue("toolchain.cflags");
    ldflags = reader.getValue("toolchain.ldflags");
    // TODO convert env vars "$HOME" to values, warn if not set
    printf("MAN: target %s\n", target.c_str());
    printf("MAN: cc %s\n", cc.c_str());
    printf("MAN: cflags %s\n", cflags.c_str());
    printf("MAN: ldflags %s\n", ldflags.c_str());

    TomlReader::NodeIter iter = reader.getNodeIter("libdir");
    while (!iter.done()) {
        // dir is required
        const char* dir = iter.getValue("dir");
        if (!dir) {
            sprintf(errorMsg, "missing dir entry in [[libdir]]");
            return false;
        }
        printf("MAN: dir %s\n", dir);
        // TODO convert env vars "$HOME" to values, warn if not set
        libDirs.push_back(dir);
        iter.next();
    }
    return true;
}

