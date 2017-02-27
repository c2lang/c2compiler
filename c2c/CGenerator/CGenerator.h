/* Copyright 2013-2017 Bas van den Berg
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

#ifndef CGENERATOR_CGENERATOR_H
#define CGENERATOR_CGENERATOR_H

#include <string>

#include "AST/Module.h"
#include "Utils/StringList.h"

namespace C2 {

class HeaderNamer;
class Component;
class TargetInfo;

class CGenerator {
public:
    struct Options {
        Options(const std::string& outputDir_, const std::string& buildDir_, const std::string& libDir_)
            : single_module(false), printC(false)
            , outputDir(outputDir_)
            , buildDir(buildDir_)
            , libDir(libDir_)
        {}
        bool single_module;
        bool printC;
        std::string outputDir;
        std::string buildDir;
        std::string libDir;
    };

    CGenerator(const Component& component_,
               const Modules& moduleMap_,
               const HeaderNamer& namer_,
               const Options& options_,
               const TargetInfo& targetInfo_);

    void generate();
    void build();
    void generateExternalHeaders();
    void generateInterfaceFiles();
private:
    const Component& component;
    const Modules& moduleMap;
    const HeaderNamer& includeNamer;
    const Options& options;
    const TargetInfo& targetInfo;
};

}

#endif

