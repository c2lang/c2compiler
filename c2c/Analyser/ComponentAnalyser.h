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

#ifndef ANALYSER_COMPONENT_ANALYSER_H
#define ANALYSER_COMPONENT_ANALYSER_H

#include "AST/Module.h"

namespace c2lang {
class DiagnosticsEngine;
}

namespace C2 {

class ASTContext;
class Component;
class TargetInfo;
class ModuleAnalyser;

class ComponentAnalyser {
public:
    ComponentAnalyser(Component& C,
                      const Modules& allModules,
                      c2lang::DiagnosticsEngine& Diags,
                      const TargetInfo& target_,
                      ASTContext& context_,
                      bool verbose,
                      bool testMode);
    ~ComponentAnalyser();

    unsigned analyse(bool print1, bool print2, bool print3, bool printLib);
private:
    bool checkMainFunction(bool testMode, c2lang::DiagnosticsEngine& Diags);

    Component& component;
    typedef std::vector<ModuleAnalyser*> Analysers;
    Analysers analysers;

    ComponentAnalyser(const ComponentAnalyser&);
    ComponentAnalyser& operator= (const ComponentAnalyser&);
};

}

#endif

