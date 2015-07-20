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

#ifndef CGENERATOR_MAKEFILE_GENERATOR_H
#define CGENERATOR_MAKEFILE_GENERATOR_H

#include <string>
#include <vector>

#include "Utils/GenUtils.h"

namespace C2 {

class MakefileGenerator {
public:
    MakefileGenerator(const std::string& path_, const std::string& target_, GenUtils::TargetType type_)
        : path(path_)
        , target(target_)
        , type(type_)
    {}

    void add(const std::string& filename);
    void write();
private:
    std::string path;
    std::string target;
    GenUtils::TargetType type;

    typedef std::vector<std::string> Files;
    typedef Files::const_iterator FilesConstIter;
    Files files;

    MakefileGenerator(const MakefileGenerator&);
    MakefileGenerator& operator= (const MakefileGenerator&);
};

}

#endif

