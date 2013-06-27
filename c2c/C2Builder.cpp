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
#include <clang/Sema/SemaDiagnostic.h>
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
#include "CodeGenModule.h"
#include "CCodeGenerator.h"
#include "AST.h"
#include "color.h"
#include "Recipe.h"
#include "Utils.h"
#include "Package.h"
#include "Decl.h"
#include "Scope.h"
#include "ScopeAnalyser.h"
#include "GlobalVarAnalyser.h"
#include "FunctionAnalyser.h"
#include "StringBuilder.h"

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
    // TODO note: Diags makes copy constr, pass DiagnosticIDs?
        , Diags(Diags_)
        , FileMgr(FileSystemOpts)
        , SM(Diags, FileMgr)
        , Headers(HSOpts, FileMgr, Diags, LangOpts_, pti)
        , PPOpts(new PreprocessorOptions())
        , PP(PPOpts, Diags, LangOpts_, pti, SM, Headers, loader)
    {
        ApplyHeaderSearchOptions(PP.getHeaderSearchInfo(), *HSOpts, LangOpts_, pti->getTriple());

        PP.setPredefines(configs);

        // File stuff
        const FileEntry *pFile = FileMgr.getFile(filename);
        SM.createMainFileID(pFile);
        PP.EnterMainSourceFile();
        // NOTE: filling zero instead of PP works
        //Diags.getClient()->BeginSourceFile(LangOpts_, &PP);
        Diags.getClient()->BeginSourceFile(LangOpts_, 0);

    }
    ~FileInfo() {
        delete globals;
    }

    bool parse(const BuildOptions& options) {
        u_int64_t t1 = Utils::getCurrentTime();
        C2Sema sema(SM, Diags, typeContext, ast);
        C2Parser parser(PP, sema);
        parser.Initialize();
        // parse the file into AST
        bool ok = parser.Parse();
        u_int64_t t2 = Utils::getCurrentTime();
        if (options.printTiming) printf(COL_TIME"parsing took %lld usec"ANSI_NORMAL"\n", t2 - t1);
        if (options.printAST) ast.print(filename);
        return ok;
    }

    // analyse use statements, Types and build file scope
    int analyse1(const BuildOptions& options, const Pkgs& pkgs) {
        globals = new FileScope(ast.pkgName, pkgs, Diags);
        ScopeAnalyser visitor(*globals, Diags);
        ast.visitAST(visitor);
        return visitor.getErrors();
    }

    // analyse Global var types + initialization
    int analyse2(const BuildOptions& options) {
        GlobalVarAnalyser visitor(*globals, typeContext, Diags);
        ast.visitAST(visitor);
        return visitor.getErrors();
    }

    // analyse Function bodies
    int analyse3(const BuildOptions& options) {
        u_int64_t t1 = Utils::getCurrentTime();

        // analyse function bodies
        FunctionAnalyser visitor(*globals, typeContext, Diags);
        ast.visitAST(visitor);

        u_int64_t t2 = Utils::getCurrentTime();
        if (options.printTiming) printf(COL_TIME"analysis took %lld usec"ANSI_NORMAL"\n", t2 - t1);
        return visitor.getErrors();
    }

    std::string filename;

    TypeContext typeContext;

    // Diagnostics
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
    AST ast;
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
    DiagnosticConsumer* client = Diags.getClient();

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
    if (client->getNumErrors()) goto out;

    // phase 1b: merge file's symbol tables to package symbols tables
    errors = !createPkgs();
    if (options.printSymbols) dumpPkgs();
    if (options.printPackages) {
        printf("List of packages:\n");
        for (PkgsIter iter = pkgs.begin(); iter != pkgs.end(); ++iter) {
            Package* P = iter->second;
            printf("  %s\n", P->getName().c_str());
        }
    }
    if (client->getNumErrors()) goto out;

    addDummyPackages();

    // phase 2: run analysing on all files
    for (unsigned int i=0; i<files.size(); i++) {
        FileInfo* info = files[i];
        errors += info->analyse1(options, pkgs);
    }
    if (client->getNumErrors()) goto out;

    for (unsigned int i=0; i<files.size(); i++) {
        FileInfo* info = files[i];
        errors += info->analyse2(options);
    }
    if (client->getNumErrors()) goto out;

    for (unsigned int i=0; i<files.size(); i++) {
        FileInfo* info = files[i];
        errors += info->analyse3(options);
    }

    if (options.printASTAfter) {
        for (unsigned int i=0; i<files.size(); i++) {
            FileInfo* info = files[i];
            info->ast.print(info->filename);
        }
    }

    if (client->getNumErrors()) goto out;
    // check that main() function is present
    {
        Decl* mainDecl = 0;
        for (PkgsIter iter = pkgs.begin(); iter != pkgs.end(); ++iter) {
            Package* P = iter->second;
            Decl* decl = P->findSymbol("main");
            if (decl) {
                if (mainDecl) {
                    // TODO duplicate main functions
                } else {
                    mainDecl = decl;
                }
            }
        }
        if (!mainDecl) {
            Diags.Report(diag::err_main_missing);
            goto out;
        }
    }

    // (optional) phase 3a: C code generation
    if (options.generateC) {
        if (options.single_module) {
            u_int64_t t1 = Utils::getCurrentTime();
            std::string filename = "test";
            CCodeGenerator gen(filename, CCodeGenerator::SINGLE_FILE, pkgs);
            for (unsigned int i=0; i<files.size(); i++) {
                FileInfo* info = files[i];
                gen.addEntry(info->filename, info->ast);
            }
            gen.generate();
            u_int64_t t2 = Utils::getCurrentTime();
            if (options.printTiming) printf(COL_TIME"C code generation took %lld usec"ANSI_NORMAL"\n", t2 - t1);
            if (options.printC) gen.dump();
        } else {
            for (PkgsIter iter = pkgs.begin(); iter != pkgs.end(); ++iter) {
                Package* P = iter->second;
                u_int64_t t1 = Utils::getCurrentTime();
                if (P->isPlainC()) continue;
                // for now filter out 'c2' as well
                if (P->getName() == "c2") continue;
                CCodeGenerator gen(P->getName(), CCodeGenerator::MULTI_FILE, pkgs);
                for (unsigned int i=0; i<files.size(); i++) {
                    FileInfo* info = files[i];
                    if (info->ast.pkgName == P->getName()) {
                        gen.addEntry(info->filename, info->ast);
                    }
                }
                gen.generate();
                u_int64_t t2 = Utils::getCurrentTime();
                if (options.printTiming) printf(COL_TIME"C code generation took %lld usec"ANSI_NORMAL"\n", t2 - t1);
                if (options.printC) gen.dump();
                //cgm.write(recipe.name, P->getName());
            }
        }
    }

    // (optional) phase 3b: IR code generation
    if (options.generateIR) {
        for (PkgsIter iter = pkgs.begin(); iter != pkgs.end(); ++iter) {
            Package* P = iter->second;
            u_int64_t t1 = Utils::getCurrentTime();
            if (P->isPlainC()) continue;
            // for now filter out 'c2' as well
            if (P->getName() == "c2") continue;
            CodeGenModule cgm(P);
            for (unsigned int i=0; i<files.size(); i++) {
                FileInfo* info = files[i];
                if (info->ast.pkgName == P->getName()) {
                    cgm.addEntry(info->filename, info->ast);
                }
            }
            cgm.generate();
            u_int64_t t2 = Utils::getCurrentTime();
            if (options.printTiming) printf(COL_TIME"IR generation took %lld usec"ANSI_NORMAL"\n", t2 - t1);
            if (options.printIR) cgm.dump();
            bool ok = cgm.verify();
            if (ok) cgm.write(recipe.name, P->getName());
        }
    }

out:
    raw_ostream &OS = llvm::errs();
    unsigned NumWarnings = client->getNumWarnings();
    unsigned NumErrors = client->getNumErrors();
    if (NumWarnings)
      OS << NumWarnings << " warning" << (NumWarnings == 1 ? "" : "s");
    if (NumWarnings && NumErrors)
      OS << " and ";
    if (NumErrors)
      OS << NumErrors << " error" << (NumErrors == 1 ? "" : "s");
    if (NumWarnings || NumErrors)
      OS << " generated.\n";
}

Package* C2Builder::getPackage(const std::string& name, bool isCLib) {
    PkgsIter iter = pkgs.find(name);
    if (iter == pkgs.end()) {
        Package* P = new Package(name, isCLib);
        pkgs[name] = P;
        return P;
    } else {
        // TODO check that isCLib matches returned package?
        return iter->second;
    }
}

// merges symbols of all files of each package
bool C2Builder::createPkgs() {
    for (unsigned int i=0; i<files.size(); i++) {
        FileInfo* info = files[i];
        Package* pkg = getPackage(info->ast.pkgName, false);
        for (unsigned int i=0; i<info->ast.decls.size(); i++) {
            Decl* New = info->ast.decls[i];
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
            // also add enum constant names to symbol list, Bit nasty to do here
            // This should be done in C2Sema!!
            if (New->dtype() == DECL_TYPE) {
                TypeDecl* T = DeclCaster<TypeDecl>::getType(New);
                Type* type = T->getType();
                if (type->isEnumType()) {
                    const MemberList* members = type->getMembers();
                    for (unsigned i=0; i<members->size(); i++) {
                        DeclExpr* de = (*members)[i];
                        // wrap in EnumConstantDecl
                        // TODO MEMLEAK or throw away in ~Scope() ?
                        EnumConstantDecl* ecd = new EnumConstantDecl(de, New->isPublic());
                        ecd->setCanonicalType(type);
                        pkg->addSymbol(ecd);
                    }
                }
            }
        }
    }
    return true;
}

void C2Builder::addDummyPackages() {
    Package* c2Pkg = getPackage("c2", false);
    // TODO add dummy decls;
    // NOTE: MEMLEAK on Types

    Package* stdioPkg = getPackage("stdio", true);
    SourceLocation loc;
    // int puts(const char* s);
    {
        FunctionDecl* func = new FunctionDecl("puts", loc, true, BuiltinType::get(TYPE_INT));
        // TODO correct arg
        Type* ptype = new Type(Type::POINTER, BuiltinType::get(TYPE_CHAR));
        Type* ctype = new Type(Type::QUALIFIER, ptype);
        func->addArg(new DeclExpr("s", loc, ctype, 0));
        stdioPkg->addSymbol(func);
        // canonical type
        Type* proto = new Type(Type::FUNC);
        proto->setReturnType(BuiltinType::get(TYPE_INT));
        proto->addArgument(BuiltinType::get(TYPE_INT));
        func->setCanonicalType(proto);
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
        // canonical type
        Type* proto = new Type(Type::FUNC);
        proto->setReturnType(BuiltinType::get(TYPE_INT));
        proto->addArgument(ctype);
        func->setCanonicalType(proto);
    }
    //int sprintf(char *str, const char *format, ...);
    {
        FunctionDecl* func = new FunctionDecl("sprintf", loc, true, BuiltinType::get(TYPE_INT));
        // NOTE: MEMLEAK ON TYPE, this will go away when we remove these dummy protos
        Type* ptype = new Type(Type::POINTER, BuiltinType::get(TYPE_CHAR));
        Type* ctype = new Type(Type::QUALIFIER, ptype);
        func->addArg(new DeclExpr("str", loc, ptype, 0));
        func->addArg(new DeclExpr("format", loc, ctype, 0));
        func->setVariadic();
        stdioPkg->addSymbol(func);
        // canonical type
        Type* proto = new Type(Type::FUNC);
        proto->setReturnType(BuiltinType::get(TYPE_INT));
        proto->addArgument(ptype);
        proto->addArgument(ctype);
        func->setCanonicalType(proto);
    }

    Package* stdlibPkg = getPackage("stdlib", true);
    //void exit(int status);
    {
        FunctionDecl* func = new FunctionDecl("exit", loc, true, BuiltinType::get(TYPE_VOID));
        // TODO correct arg
        func->addArg(new DeclExpr("status", loc, BuiltinType::get(TYPE_INT), 0));
        stdlibPkg->addSymbol(func);
        // canonical type
        Type* proto = new Type(Type::FUNC);
        proto->setReturnType(BuiltinType::get(TYPE_INT));
        proto->addArgument(BuiltinType::get(TYPE_INT));
        func->setCanonicalType(proto);
    }
}

void C2Builder::dumpPkgs() {
    for (PkgsIter iter = pkgs.begin(); iter != pkgs.end(); ++iter) {
        Package* P = iter->second;
        P->dump();
    }
}

