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

#ifndef BUILDER_BUILDFILE_READER_H
#define BUILDER_BUILDFILE_READER_H

#include <string>

namespace C2 {

class BuildFile;

class BuildFileReader {
public:
    BuildFileReader(BuildFile& build_);
    ~BuildFileReader() {}

    bool parse(const std::string& filename);
    const char* getErrorMsg() const { return errorMsg; }
private:
    const char* expandEnvVar(const std::string& filename, const char* raw);

    BuildFile& build;
    char errorMsg[256];
};

}

#endif

