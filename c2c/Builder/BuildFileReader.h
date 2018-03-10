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

#ifndef BUILDER_BUILDFILE_READER_H
#define BUILDER_BUILDFILE_READER_H

#include <string>
#include "Utils/StringList.h"

namespace C2 {

class BuildFileReader {
public:
    BuildFileReader();
    ~BuildFileReader() {}

    bool parse(const std::string& filename);
    const char* getErrorMsg() const { return errorMsg; }

    const std::string& getTarget() const { return target; }
    const std::string& getCC() const { return cc; }
    const std::string& getCflags() const { return cflags; }
    const std::string& getLdflags() const { return ldflags; }
    //const char* getPath() const { return path; }
    const StringList& getLibDirs() const { return libDirs; }
private:
    const char* expandEnvVar(const std::string& filename, const char* raw);

    std::string target;
    std::string cc;
    std::string cflags;
    std::string ldflags;
    //std::string path;
    StringList libDirs;
    char errorMsg[256];
};

}

#endif

