/* Copyright 2013-2016 Bas van den Berg
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

#ifndef UTILS_GEN_UTILS_H
#define UTILS_GEN_UTILS_H

#include <string>
#include <vector>

namespace C2 {

class StringBuilder;

class GenUtils {
public:
    enum TargetType { EXECUTABLE, SHARED_LIB, STATIC_LIB };

    struct Dependency {
        Dependency(const std::string& n, TargetType t) : name(n), type(t) {}
        std::string name;
        TargetType type;
    };
    typedef std::vector<GenUtils::Dependency> Dependencies;
    typedef Dependencies::const_iterator DependenciesConstIter;

    static void addName(const std::string& modName, const std::string& name, StringBuilder& buffer);
    static void toCapital(const std::string& input, StringBuilder& output);
    static bool needsInterface(TargetType type);
};

const char* Str(GenUtils::TargetType type);

}

#endif

