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

#ifndef BUILDER_C2BUILDER_H
#define BUILDER_C2BUILDER_H

#include <vector>
#include <string>

#include "Builder/LibraryLoader.h"
#include "AST/Module.h"
#include "AST/Component.h"

namespace clang {
class DiagnosticsEngine;
class SourceManager;
}

namespace C2 {

class AST;
class Recipe;

struct BuildOptions {
    BuildOptions()
        : printAST0(false)
        , printAST1(false)
        , printAST2(false)
        , printAST3(false)
        , printASTLib(false)
        , printTiming(false)
        , printSymbols(false)
        , generateIR(false)
        , printIR(false)
        , generateC(false)
        , printC(false)
        , checkOnly(false)
        , showLibs(false)
        , printModules(false)
        , printDependencies(false)
        , generateRefs(false)
        , verbose(false)
        , testMode(false)
        , libdir(0)
    {}
    bool printAST0;
    bool printAST1;
    bool printAST2;
    bool printAST3;
    bool printASTLib;
    bool printTiming;
    bool printSymbols;
    bool generateIR;
    bool printIR;
    bool generateC;
    bool printC;
    bool checkOnly;
    bool showLibs;
    bool printModules;
    bool printDependencies;
    bool generateRefs;
    bool verbose;
    bool testMode;

    const char* libdir;
};


class C2Builder {
public:
    C2Builder(const Recipe& recipe_, const BuildOptions& opts);
    ~C2Builder();

    int checkFiles();
    int build();

#if 0
    // AST extraction (for tools)
    unsigned numASTs() const { return files.size(); }
    const AST& getAST(unsigned i) const {
        assert(i < files.size());
        return *files[i];
    }
#endif
private:
    bool haveModule(const std::string& name) const;
    Module* getModule(const std::string& name, bool isExternal, bool isCLib);
    Module* findModule(const std::string& name) const;
    bool createModules(Component* C, clang::DiagnosticsEngine& Diags);
    bool addFileToModule(clang::DiagnosticsEngine& Diags, Module* mod, AST* ast);
    unsigned analyse();
    void dumpModules() const;
    void printComponents() const;
    void log(const char* color, const char* format, ...) const;

    bool checkMainFunction(clang::DiagnosticsEngine& Diags);
    bool checkExportedPackages() const;
    void generateInterface() const;
    void generateOptionalC();
    void generateOptionalIR();
    void generateOptionalDeps();
    void generateOptionalTags(const clang::SourceManager& SM) const;

    const Recipe& recipe;
    BuildOptions options;

    Modules modules;

    Components components;
    LibraryLoader libLoader;

    bool useColors;

    C2Builder(const C2Builder&);
    C2Builder& operator= (const C2Builder&);
};

}

#endif

