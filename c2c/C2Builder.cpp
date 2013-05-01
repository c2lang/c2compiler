/* Copyright 2013 Bas van den Berg
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

#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/FileSystemOptions.h>
#include <clang/Basic/MacroBuilder.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/TargetOptions.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Frontend/Utils.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/HeaderSearchOptions.h>
#include <clang/Lex/ModuleLoader.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/PreprocessorOptions.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <llvm/ADT/OwningPtr.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Host.h>
#include <stdio.h>
#include <unistd.h>

#include "C2Builder.h"
#include "C2Parser.h"
#include "C2Sema.h"
#include "CodeGenerator.h"
#include "color.h"
#include "Recipe.h"
#include "Utils.h"
#include "Package.h"
#include "Decl.h"
#include "Scope.h"
#include "GlobalAnalyser.h"
#include "FunctionBodyAnalyser.h"
#include "StringBuilder.h"

#define COL_TIME ANSI_CYAN

using clang::DiagnosticOptions;
using clang::DiagnosticsEngine;
using clang::FileEntry;
using clang::FileManager;
using clang::FileSystemOptions;
using clang::HeaderSearch;
using clang::HeaderSearchOptions;
using clang::LangOptions;
using clang::ModuleLoader;
using clang::Preprocessor;
using clang::PreprocessorOptions;
using clang::SourceManager;
using clang::TargetInfo;
using clang::TargetOptions;
using clang::TextDiagnosticPrinter;

using namespace C2;

namespace C2 {
class C2ModuleLoader : public ModuleLoader {
public:
    C2ModuleLoader() {}
    virtual ~C2ModuleLoader () {}

    virtual Module *loadModule(SourceLocation ImportLoc, ModuleIdPath Path,
                               Module::NameVisibilityKind Visibility,
                               bool IsInclusionDirective)
    {
        fprintf(stderr, "MODULE LOADER\n");
        return 0;
    }

private:
    C2ModuleLoader(const C2ModuleLoader& rhs);
    C2ModuleLoader& operator= (const C2ModuleLoader& rhs);
};

class FileInfo {
public:
    FileInfo(DiagnosticsEngine& Diags_,
             LangOptions& LangOpts_,
             TargetInfo* pti,
             HeaderSearchOptions* HSOpts,
             const std::string& filename_,
             const std::string& configs)
        : filename(filename_)
        , Diags(Diags_)
        , FileMgr(FileSystemOpts)
        , SM(Diags, FileMgr)
        , Headers(HSOpts, FileMgr, Diags, LangOpts_, pti)
        , PPOpts(new PreprocessorOptions())
        , PP(PPOpts, Diags, LangOpts_, pti, SM, Headers, loader)
        , sema(SM, Diags, typeContext)
        , parser(PP, sema)
    {
        ApplyHeaderSearchOptions(PP.getHeaderSearchInfo(), *HSOpts, LangOpts_, pti->getTriple());

        PP.setPredefines(configs);

        // File stuff
        const FileEntry *pFile = FileMgr.getFile(filename);
        SM.createMainFileID(pFile);
        PP.EnterMainSourceFile();
        Diags.getClient()->BeginSourceFile(LangOpts_, &PP);

        parser.Initialize();
    }
    ~FileInfo() {
        delete globals;
    }

    bool parse(const BuildOptions& options) {
        u_int64_t t1 = Utils::getCurrentTime();
        // parse the file into AST
        bool ok = parser.Parse();
        u_int64_t t2 = Utils::getCurrentTime();
        if (options.printTiming) printf(COL_TIME"parsing took %lld usec"ANSI_NORMAL"\n", t2 - t1);
        if (options.printAST) sema.printAST(filename);
        return ok;
    }

    // analyse FileScope and Types
    int analyse1(const BuildOptions& options, const Pkgs& pkgs) {
        // TODO measure this analysis time as well
        // step 1: do use and type analysis and build global scope
        globals = new FileScope(sema.getPkgName(), pkgs, Diags);
        GlobalAnalyser visitor(*globals, Diags);
        sema.visitAST(visitor);
        return !visitor.getErrors();
    }

    int analyse2(const BuildOptions& options) {
        u_int64_t t1 = Utils::getCurrentTime();
        // step 2: analyse function bodies
        FunctionBodyAnalyser visitor(*globals, typeContext, Diags);
        sema.visitAST(visitor);

        u_int64_t t2 = Utils::getCurrentTime();
        if (options.printTiming) printf(COL_TIME"analysis took %lld usec"ANSI_NORMAL"\n", t2 - t1);
        //return visitor.getErrors();
        return 0;
    }

    void generate_c(const BuildOptions& options) {
        printf("---- C-code %s.c ----\n", sema.getPkgName().c_str());
        u_int64_t t1 = Utils::getCurrentTime();
        StringBuilder buffer;
        sema.generateC(buffer);
        u_int64_t t2 = Utils::getCurrentTime();
        if (options.printTiming) printf(COL_TIME"C generation took %lld usec"ANSI_NORMAL"\n", t2 - t1);
        printf("%s", (const char*)buffer);
    }

    void codegen(const BuildOptions& options) {
        printf("------ generating code for %s ------\n", filename.c_str());
        u_int64_t t1 = Utils::getCurrentTime();
        CodeGenerator codegen(sema);
        codegen.generate();
        u_int64_t t2 = Utils::getCurrentTime();
        if (options.printTiming) printf(COL_TIME"IR generation took %lld usec"ANSI_NORMAL"\n", t2 - t1);
        codegen.dump();
    }

    std::string filename;

    TypeContext typeContext;

    // Diagnostics
    DiagnosticOptions DiagOpts;
    IntrusiveRefCntPtr<DiagnosticIDs> DiagID;
    DiagnosticsEngine Diags;

    // FileManager
    FileSystemOptions FileSystemOpts;
    FileManager FileMgr;

    // SourceManager
    SourceManager SM;

    // HeaderSearch
    IntrusiveRefCntPtr<HeaderSearchOptions> HSOpts;
    HeaderSearch Headers;

    C2ModuleLoader loader;

    // Preprocessor
    IntrusiveRefCntPtr<PreprocessorOptions> PPOpts;
    Preprocessor PP;

    // C2 Parser + Sema
    C2Sema sema;
    C2Parser parser;
    FileScope* globals;
};

}


C2Builder::C2Builder(const Recipe& recipe_, const BuildOptions& opts)
    : recipe(recipe_)
    , options(opts)
{}

C2Builder::~C2Builder()
{
    for (PkgsIter iter = pkgs.begin(); iter != pkgs.end(); ++iter) {
        //delete iter->second;
        // TODO delete CAUSES crash (caused by delete of FileInfo below)
    }
    for (unsigned int i=0; i<files.size(); i++) {
        delete files[i];
    }
}

void C2Builder::build() {
    printf(ANSI_GREEN"building target %s"ANSI_NORMAL"\n", recipe.name.c_str());

    // TODO save these common objects in Builder class?
    // LangOptions
    LangOptions LangOpts;
    LangOpts.C2 = 1;
    LangOpts.Bool = 1;

    // Diagnostics
    DiagnosticOptions DiagOpts;
    IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
    DiagnosticsEngine Diags(DiagID, &DiagOpts,
            // NOTE: setting ShouldOwnClient to true causes crash??
            new TextDiagnosticPrinter(llvm::errs(), &DiagOpts), false);

    // TargetInfo
    TargetOptions* to = new TargetOptions();
    to->Triple = llvm::sys::getDefaultTargetTriple();
    TargetInfo *pti = TargetInfo::CreateTargetInfo(Diags, *to);
    IntrusiveRefCntPtr<TargetInfo> Target(pti);

    HeaderSearchOptions* HSOpts = new HeaderSearchOptions();
    char pwd[512];
    if (getcwd(pwd, 512) == NULL) {
        assert(0);
    }
    HSOpts->AddPath(pwd, clang::frontend::Quoted, false, false, false);

    std::string PredefineBuffer;
    if (!recipe.configs.empty()) {
        PredefineBuffer.reserve(4080);
        llvm::raw_string_ostream Predefines(PredefineBuffer);
        MacroBuilder mbuilder(Predefines);
        for (unsigned int i=0; i<recipe.configs.size(); i++) {
            mbuilder.defineMacro(recipe.configs[i]);
        }
    }

    // phase 1: parse and local analyse
    int errors = 0;
    for (int i=0; i<recipe.size(); i++) {
        FileInfo* info = new FileInfo(Diags, LangOpts, pti, HSOpts, recipe.get(i), PredefineBuffer);
        files.push_back(info);
        bool ok = info->parse(options);
        errors += !ok;
    }
    if (errors) return;

    // phase 1b: merge file's symbol tables to package symbols tables
    errors = !createPkgs();
    if (options.printSymbols) dumpPkgs();
    if (errors) return;

    addDummyPackages();

    // phase 2: run analysing on all files
    for (unsigned int i=0; i<files.size(); i++) {
        FileInfo* info = files[i];
        errors += info->analyse1(options, pkgs);
    }
    for (unsigned int i=0; i<files.size(); i++) {
        FileInfo* info = files[i];
        errors += info->analyse2(options);
    }
    if (options.printASTAfter) {
        for (unsigned int i=0; i<files.size(); i++) {
            FileInfo* info = files[i];
            info->sema.printAST(info->filename);
        }
    }
    if (errors) return;

    // (optional) phase 3a: C code generation
    if (options.generateC) {
        for (unsigned int i=0; i<files.size(); i++) {
            FileInfo* info = files[i];
            info->generate_c(options);
        }
    }

    // (optional) phase 3b: IR code generation
    if (options.generateIR) {
        for (unsigned int i=0; i<files.size(); i++) {
            FileInfo* info = files[i];
            info->codegen(options);
        }
    }
}

Package* C2Builder::getPackage(const std::string& name) {
    PkgsIter iter = pkgs.find(name);
    if (iter == pkgs.end()) {
        Package* P = new Package(name);
        pkgs[name] = P;
        return P;
    } else {
        return iter->second;
    }
}

// merges symbols of all files of each package
bool C2Builder::createPkgs() {
    for (unsigned int i=0; i<files.size(); i++) {
        FileInfo* info = files[i];
        Package* pkg = getPackage(info->sema.getPkgName());
        for (unsigned int i=0; i<info->sema.decls.size(); i++) {
            Decl* New = info->sema.decls[i];
            if (!Decl::isSymbol(New->dtype())) continue;
            Decl* Old = pkg->findSymbol(New->getName());
            if (Old) {
                fprintf(stderr, "MULTI_FILE: duplicate symbol %s\n", New->getName().c_str());
                fprintf(stderr, "TODO NASTY: locs have to be resolved in different SourceManagers..\n");
                // TODO: continue/return?
                return false;
            } else {
                pkg->addSymbol(New);
            }
        }
    }
    return true;
}

void C2Builder::addDummyPackages() {
    Package* c2Pkg = getPackage("c2");
    // TODO add dummy decls;

    Package* stdioPkg = getPackage("stdio");
    SourceLocation loc;
    // int puts(const char* s);
    {
        FunctionDecl* func = new FunctionDecl("puts", loc, true, BuiltinType::get(TYPE_INT));
        // TODO correct arg
        func->addArg(new DeclExpr("s", loc, BuiltinType::get(TYPE_INT), 0));
        stdioPkg->addSymbol(func);
    }
    //int printf(const char *format, ...);
    {
        FunctionDecl* func = new FunctionDecl("printf", loc, true, BuiltinType::get(TYPE_INT));
        // NOTE: MEMLEAK ON TYPE, this will go away when we remove these dummy protos
        Type* ptype = new Type(Type::POINTER, BuiltinType::get(TYPE_CHAR));
        Type* ctype = new Type(Type::QUALIFIER, ptype);
        func->addArg(new DeclExpr("format", loc, ctype, 0));
        func->setVariadic();
        stdioPkg->addSymbol(func);
    }
}

void C2Builder::dumpPkgs() {
    for (PkgsIter iter = pkgs.begin(); iter != pkgs.end(); ++iter) {
        Package* P = iter->second;
        P->dump();
    }
}

