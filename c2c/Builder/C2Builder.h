/* Copyright 2013,2014 Bas van den Berg
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

#ifndef BUILDER_C2BUILDER_H
#define BUILDER_C2BUILDER_H

#include <vector>
#include <string>

#include "AST/Module.h"
#include "FileUtils/FileDb.h"

namespace clang {
class DiagnosticsEngine;
}

namespace C2 {

class Recipe;
class FileInfo;

struct BuildOptions {
    BuildOptions()
        : printAST0(false)
        , printAST1(false)
        , printAST2(false)
        , printAST3(false)
        , printTiming(false)
        , printSymbols(false)
        , generateIR(false)
        , printIR(false)
        , generateC(false)
        , printC(false)
        , printModules(false)
        , printDependencies(false)
        , verbose(false)
        , testMode(false)
    {}
    bool printAST0;
    bool printAST1;
    bool printAST2;
    bool printAST3;
    bool printTiming;
    bool printSymbols;
    bool generateIR;
    bool printIR;
    bool generateC;
    bool printC;
    bool printModules;
    bool printDependencies;
    bool verbose;
    bool testMode;
};


class C2Builder {
public:
    C2Builder(const Recipe& recipe_, const BuildOptions& opts);
    ~C2Builder();

    int checkFiles();
    int build();
private:
    bool haveModule(const std::string& name) const;
    Module* getModule(const std::string& name, bool isExternal, bool isCLib);
    bool createModules();
    bool loadExternalModules();
    bool loadModule(const std::string& name);
    unsigned analyse();
    void dumpModules() const;
    void printASTs() const;

    bool checkMainFunction(clang::DiagnosticsEngine& Diags);
    void generateOptionalC();
    void generateOptionalIR();
    void generateOptionsDeps() const;

    const Recipe& recipe;
    BuildOptions options;

    typedef std::vector<FileInfo*> Files;
    Files files;
    FileDb filenames;

    Modules modules;

    C2Builder(const C2Builder&);
    C2Builder& operator= (const C2Builder&);
};

}

#endif

