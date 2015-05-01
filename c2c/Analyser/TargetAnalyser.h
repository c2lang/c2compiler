/* Copyright 2013,2014,2015 Bas van den Berg
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

#ifndef ANALYSER_TARGET_ANALYSER_H
#define ANALYSER_TARGET_ANALYSER_H

#include <vector>
#include "AST/Module.h"

namespace clang {
class DiagnosticsEngine;
}

namespace C2 {

class FileAnalyser;
class AST;
class TypeContext;

class TargetAnalyser {
public:
    TargetAnalyser(const Modules& modules_, clang::DiagnosticsEngine& Diags_, int numfiles, bool verbose_)
        : modules(modules_)
        , Diags(Diags_)
        , verbose(verbose_)
    {
        analysers.reserve(numfiles);
    }

    ~TargetAnalyser();

    void addFile(AST& ast, TypeContext& typeContext);

    unsigned analyse(bool print1, bool print2, bool print3);

private:
    void printASTs() const;

    const Modules& modules;
    clang::DiagnosticsEngine& Diags;
    bool verbose;

    typedef std::vector<FileAnalyser*> Analysers;
    typedef Analysers::iterator AnalysersIter;
    Analysers analysers;

    TargetAnalyser(const TargetAnalyser&);
    TargetAnalyser& operator= (const TargetAnalyser&);
};

}

#endif

