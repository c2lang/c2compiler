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

#ifndef BUILDER_C2BUILDER_H
#define BUILDER_C2BUILDER_H

#include <vector>
#include <string>

#include "Builder/LibraryLoader.h"
#include "Builder/BuildOptions.h"
#include "AST/Module.h"
#include "AST/ASTContext.h"
#include "AST/Component.h"
#include "Utils/TargetInfo.h"

namespace c2lang {
class DiagnosticsEngine;
class SourceManager;
class DiagnosticConsumer;
}

namespace C2 {

class Recipe;
class ParseHelper;
class BuildFile;
class C2Builder;

class PluginHandler {
public:
    virtual ~PluginHandler() {}

    virtual void beginTarget(C2Builder& builder) = 0;
    // TODO dont pass C2Builder, in-elegant
    virtual bool build(C2Builder& builder) = 0;
    virtual bool generate(C2Builder& builder, const c2lang::SourceManager& src_mgr__) = 0;
};

class C2Builder {
public:
    C2Builder(Recipe& recipe_, const BuildFile* buildFile_, const BuildOptions& opts, PluginHandler* pluginHandler_);
    ~C2Builder();

    int build();

    // For external tools
    const Components& getComponents() const { return components; }

    // For plugins
    Module* addPluginModule(const std::string& name);
    const std::string& getOutputDir() const { return outputDir; }
    const std::string& getRecipeName() const;
    Recipe& getRecipe() const { return recipe; }
private:
    Module* findModule(const std::string& name) const;
    void configDiagnostics(c2lang::DiagnosticsEngine &Diags);
    bool checkImports(ParseHelper& helper);
    unsigned analyse();
    void printSymbols(bool printLibs, bool printNonPublic) const;
    void printComponents(bool printLibs) const;
    int report(c2lang::DiagnosticConsumer* client, uint64_t t1_build);

    bool checkExportedPackages() const;
    typedef std::deque<const LibInfo*> ImportsQueue;
    bool checkModuleImports(ParseHelper& helper, ImportsQueue& queue, const LibInfo* lib);
    void createC2Module();

    void generateInterface() const;
    void generateOptionalC();
    void generateOptionalIR();
    void writeAST() const;

    Recipe& recipe;
    const BuildFile* buildFile;
    BuildOptions options;
    TargetInfo targetInfo;
    std::string outputDir;
    PluginHandler* pluginHandler;

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

