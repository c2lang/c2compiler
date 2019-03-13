/* Copyright 2013-2019 Bas van den Berg
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

#include <vector>
#include "AST/Module.h"

namespace c2lang {
class DiagnosticsEngine;
}

namespace C2 {

class FileAnalyser;
class AST;
class ASTContext;
class Component;
class TargetInfo;

class ComponentAnalyser {
public:
    ComponentAnalyser(Component& C,
                      const Modules& modules_,
                      c2lang::DiagnosticsEngine& Diags_,
                      const TargetInfo& target_,
                      ASTContext& context_,
                      bool verbose_);
    ~ComponentAnalyser();

    unsigned analyse(bool print1, bool print2, bool print3, bool printLib);
private:
    void printASTs(bool printLib) const;

    c2lang::DiagnosticsEngine& Diags;
    ASTContext& context;
    bool verbose;

    typedef std::vector<FileAnalyser*> Analysers;
    Analysers analysers;

    ComponentAnalyser(const ComponentAnalyser&);
    ComponentAnalyser& operator= (const ComponentAnalyser&);
};

}

#endif

