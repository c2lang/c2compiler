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

#ifndef BUILDER_C2BUILDER_H
#define BUILDER_C2BUILDER_H

#include <vector>
#include <string>

#include "Builder/LibraryLoader.h"
#include "AST/Module.h"
#include "AST/ASTContext.h"
#include "AST/Component.h"
#include "Utils/TargetInfo.h"

namespace clang {
class DiagnosticsEngine;
class SourceManager;
class LangOptions;
}

namespace C2 {

class AST;
class Recipe;
class ParseHelper;

struct BuildOptions {
    BuildOptions()
        : printAST0(false)
        , printAST1(false)
        , printAST2(false)
        , printAST3(false)
        , printASTLib(false)
        , printTiming(false)
        , printSymbols(false)
        , printLibSymbols(false)
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
    bool printLibSymbols;
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
    void generateDeps(bool showFiles, bool showPrivate, bool showExternals, const std::string& path) const;

    // For external tools
    const Components& getComponents() const { return components; }
private:
    Module* findModule(const std::string& name) const;
    bool checkImports(ParseHelper& helper);
    unsigned analyse();
    void printSymbols(bool printLibs) const;
    void printComponents() const;
    void log(const char* color, const char* format, ...) const;

    bool checkMainFunction(clang::DiagnosticsEngine& Diags);
    bool checkExportedPackages() const;
    typedef std::deque<std::string> ImportsQueue;
    bool checkModuleImports(ParseHelper& helper, Component* component, Module* module, ImportsQueue& queue, const LibInfo* lib = 0);
    void createC2Module();

    void rewriterTest(clang::SourceManager& SM, clang::LangOptions& LangOpts);
    void generateOptionalDeps();
    void generateOptionalTags(const clang::SourceManager& SM) const;
    void generateInterface() const;
    void generateOptionalC();
    void generateOptionalIR();

    const Recipe& recipe;
    BuildOptions options;
    TargetInfo targetInfo;

    ASTContext context;
    Modules modules;
    Module* c2Mod;

    Components components;
    Component* mainComponent;
    LibraryLoader libLoader;

    bool useColors;

    C2Builder(const C2Builder&);
    C2Builder& operator= (const C2Builder&);
};

}

#endif

