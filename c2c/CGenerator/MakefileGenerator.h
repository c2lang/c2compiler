/* Copyright 2013-2021 Bas van den Berg
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

namespace C2 {

class Component;
class TargetInfo;
class StringBuilder;
class BuildFile;

class MakefileGenerator {
public:
    MakefileGenerator(const Component& component_,
                      bool singleFile_,
                      const TargetInfo& targetInfo_,
                      const BuildFile* buildFile_);

    void write(const std::string& path);
private:
    const Component& component;
    std::string target;
    const TargetInfo& targetInfo;
    const BuildFile* buildFile;
    bool singleFile;

    void addLinkFlags(const Component* dep, StringBuilder& out);

    MakefileGenerator(const MakefileGenerator&);
    MakefileGenerator& operator= (const MakefileGenerator&);
};

}

#endif

