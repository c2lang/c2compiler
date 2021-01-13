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

#ifndef ANALYSER_MODULE_ANALYSER_H
#define ANALYSER_MODULE_ANALYSER_H

#include <vector>
#include "AST/Module.h"

namespace c2lang {
class DiagnosticsEngine;
}

namespace C2 {

class ASTContext;
class TargetInfo;
class FileAnalyser;

class ModuleAnalyser {
public:
    ModuleAnalyser(Module& m_,
                   const Modules& allModules,
                   c2lang::DiagnosticsEngine& Diags_,
                   const TargetInfo& target_,
                   ASTContext& context_,
                   bool verbose_);
    ~ModuleAnalyser();

    unsigned analyse(bool print1, bool print2, bool print3, bool printLib);
    void checkUnused();
private:
    void printASTs(bool printLib) const;

    typedef std::vector<FileAnalyser*> Analysers;
    Analysers analysers;

    Module& module;
    c2lang::DiagnosticsEngine& Diags;
    ASTContext& context;
    bool verbose;

    ModuleAnalyser(const ModuleAnalyser&);
    ModuleAnalyser& operator= (const ModuleAnalyser&);
};

}

#endif

