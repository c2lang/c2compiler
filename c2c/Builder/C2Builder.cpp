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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <llvm/ADT/OwningPtr.h>
#include <llvm/Support/Host.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Parse/ParseDiagnostic.h>
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

#include "Builder/C2Builder.h"
#include "Builder/Recipe.h"
#include "AST/AST.h"
#include "AST/Package.h"
#include "AST/Decl.h"
#include "Parser/C2Parser.h"
#include "Parser/C2Sema.h"
#include "Analyser/FileAnalyser.h"
#include "Analyser/DepGenerator.h"
#include "Generators/CodeGenModule.h"
#include "Generators/CCodeGenerator.h"
#include "FileUtils/FileUtils.h"
#include "Utils/StringBuilder.h"
#include "Utils/color.h"
#include "Utils/Utils.h"

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

    virtual ModuleLoadResult loadModule(SourceLocation ImportLoc, ModuleIdPath Path,
                               Module::NameVisibilityKind Visibility,
                               bool IsInclusionDirective)
    {
        fprintf(stderr, "MODULE LOADER: loadModule\n");
        return ModuleLoadResult();
    }

    virtual void makeModuleVisible(Module *Mod,
                                   Module::NameVisibilityKind Visibility,
                                   SourceLocation ImportLoc,
                                   bool Complain)
    {
        fprintf(stderr, "MODULE LOADER: make visible\n");
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
             unsigned file_id,
             const std::string& configs)
        : filename(filename_)
    // TODO note: Diags makes copy constr, pass DiagnosticIDs?
        , Diags(Diags_)
        , FileMgr(FileSystemOpts)
        , SM(Diags, FileMgr)
        , Headers(HSOpts, FileMgr, Diags, LangOpts_, pti)
        , PPOpts(new PreprocessorOptions())
        , PP(PPOpts, Diags, LangOpts_, pti, SM, Headers, loader)
        , ast(filename_, file_id)
        , analyser(0)
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
        delete analyser;
    }

    bool parse(const BuildOptions& options) {
        if (options.verbose) printf(COL_VERBOSE"parsing %s"ANSI_NORMAL"\n", filename.c_str());
        C2Sema sema(SM, Diags, typeContext, ast, PP);
        C2Parser parser(PP, sema);
        parser.Initialize();
        return parser.Parse();
    }

    void createAnalyser(const Pkgs& pkgs, bool verbose) {
        analyser = new FileAnalyser(pkgs, Diags, ast, typeContext, verbose);
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
    HeaderSearch Headers;

    C2ModuleLoader loader;

    // Preprocessor
    IntrusiveRefCntPtr<PreprocessorOptions> PPOpts;
    Preprocessor PP;

    // C2 Parser + Sema
    AST ast;
    FileAnalyser* analyser;
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
    for (unsigned i=0; i<files.size(); i++) {
        delete files[i];
    }
}

int C2Builder::checkFiles() {
    int errors = 0;
    for (int i=0; i<recipe.size(); i++) {
        const std::string& filename = recipe.get(i);
        struct stat buf;
        if (stat(filename.c_str(), &buf)) {
            fprintf(stderr, "c2c: error: %s: '%s'\n", strerror(errno), filename.c_str());
            errors++;
        }
    }
    return errors;
}

int C2Builder::build() {
    printf(ANSI_GREEN"building target %s"ANSI_NORMAL"\n", recipe.name.c_str());

    u_int64_t t1_build = Utils::getCurrentTime();
    // TODO save these common objects in Builder class?
    // LangOptions
    LangOptions LangOpts;
    LangOpts.C2 = 1;
    LangOpts.Bool = 1;
    LangOpts.LineComment = 1;

    // Diagnostics
    // NOTE: DiagOpts is somehow deleted by Diags/TextDiagnosticPrinter below?
    DiagnosticOptions* DiagOpts = new DiagnosticOptions();
    IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
    DiagnosticsEngine Diags(DiagID, DiagOpts,
            // NOTE: setting ShouldOwnClient to true causes crash??
            new TextDiagnosticPrinter(llvm::errs(), DiagOpts), false);
    DiagnosticConsumer* client = Diags.getClient();

    // add these diagnostic groups by default
    Diags.setDiagnosticGroupMapping("conversion", diag::MAP_WARNING);
    Diags.setDiagnosticGroupMapping("all", diag::MAP_WARNING);
    Diags.setDiagnosticGroupMapping("extra", diag::MAP_WARNING);
    Diags.setDiagnosticWarningAsError(diag::warn_integer_too_large, true);
    Diags.setDiagnosticWarningAsError(diag::warn_falloff_nonvoid_function, true);
    // Workaround
    Diags.setDiagnosticErrorAsFatal(diag::warn_integer_too_large, true);
    Diags.setDiagnosticErrorAsFatal(diag::warn_integer_too_large, false);
    Diags.setDiagnosticErrorAsFatal(diag::warn_falloff_nonvoid_function, true);
    Diags.setDiagnosticErrorAsFatal(diag::warn_falloff_nonvoid_function, false);

    // set recipe warning options
    for (unsigned i=0; i<recipe.silentWarnings.size(); i++) {
        const std::string& conf = recipe.silentWarnings[i];

        if (conf == "no-unused") {
            Diags.setDiagnosticGroupMapping("unused", diag::MAP_IGNORE);
            Diags.setDiagnosticGroupMapping("unused-parameter", diag::MAP_IGNORE);
            continue;
        }
        if (conf == "no-unused-variable") {
            Diags.setDiagnosticGroupMapping("unused-variable", diag::MAP_IGNORE);
            continue;
        }
        if (conf == "no-unused-function") {
            Diags.setDiagnosticGroupMapping("unused-function", diag::MAP_IGNORE);
            continue;
        }
        if (conf == "no-unused-parameter") {
            Diags.setDiagnosticGroupMapping("unused-parameter", diag::MAP_IGNORE);
            continue;
        }
        if (conf == "no-unused-type") {
            Diags.setDiagnosticGroupMapping("unused-type", diag::MAP_IGNORE);
            continue;
        }
        if (conf == "no-unused-package") {
            Diags.setDiagnosticGroupMapping("unused-package", diag::MAP_IGNORE);
            continue;
        }
        if (conf == "no-unused-public") {
            Diags.setDiagnosticGroupMapping("unused-public", diag::MAP_IGNORE);
            continue;
        }
        fprintf(stderr, "recipe: unknown warning: '%s'\n", conf.c_str());
        exit(-1);
    }

    // TargetInfo
    TargetOptions* to = new TargetOptions();
    to->Triple = llvm::sys::getDefaultTargetTriple();
    TargetInfo *pti = TargetInfo::CreateTargetInfo(Diags, to);
    IntrusiveRefCntPtr<TargetInfo> Target(pti);

    HeaderSearchOptions* HSOpts = new HeaderSearchOptions();
    char pwd[512];
    if (getcwd(pwd, 512) == NULL) {
        assert(0);
    }
    HSOpts->AddPath(pwd, clang::frontend::Quoted, false, false);

    // set definitions from recipe
    std::string PredefineBuffer;
    if (!recipe.configs.empty()) {
        PredefineBuffer.reserve(4080);
        llvm::raw_string_ostream Predefines(PredefineBuffer);
        MacroBuilder mbuilder(Predefines);
        for (unsigned i=0; i<recipe.configs.size(); i++) {
            mbuilder.defineMacro(recipe.configs[i]);
        }
    }

    // phase 1: parse and local analyse
    unsigned errors = 0;
    u_int64_t t1_parse = Utils::getCurrentTime();
    for (int i=0; i<recipe.size(); i++) {
        const std::string& filename = recipe.get(i);
        unsigned file_id = filenames.add(filename);
        FileInfo* info = new FileInfo(Diags, LangOpts, pti, HSOpts, filename, file_id, PredefineBuffer);
        files.push_back(info);
        bool ok = info->parse(options);
        if (options.printAST0) info->ast.print();
        errors += !ok;
    }
    u_int64_t t2_parse = Utils::getCurrentTime();
    u_int64_t t1_analyse, t2_analyse;
    if (options.printTiming) printf(COL_TIME"parsing took %lld usec"ANSI_NORMAL"\n", t2_parse - t1_parse);
    if (client->getNumErrors()) goto out;
    t1_analyse = Utils::getCurrentTime();

    // phase 1b: merge file's symbol tables to package symbols tables
    errors = !createPkgs();
    if (options.printSymbols) dumpPkgs();

    if (client->getNumErrors()) goto out;

    // phase 1c: load required external packages
    if (!loadExternalPackages()) goto out;

    if (options.printPackages) {
        printf("List of packages:\n");
        for (PkgsConstIter iter = pkgs.begin(); iter != pkgs.end(); ++iter) {
            const Package* P = iter->second;
            printf("  %s %s\n", P->getName().c_str(), P->isExternal() ? "(external)" : "");
        }
    }

    // create analysers/scopes
    for (unsigned i=0; i<files.size(); i++) {
        FileInfo* info = files[i];
        info->createAnalyser(pkgs, options.verbose);
    }

    // phase 2: run analysing on all files
    errors += analyse();

    t2_analyse = Utils::getCurrentTime();
    if (options.printTiming) printf(COL_TIME"analysis took %lld usec"ANSI_NORMAL"\n", t2_analyse - t1_analyse);
    if (client->getNumErrors()) goto out;

    if (options.printDependencies) {
        printDependencies();
    }

    if (!options.testMode && !checkMainFunction(Diags)) goto out;

    generateOptionalC();

    generateOptionalIR();

    if (options.verbose) printf(COL_VERBOSE"done"ANSI_NORMAL"\n");
out:
    u_int64_t t2_build = Utils::getCurrentTime();
    if (options.printTiming) printf(COL_TIME"total build took %lld usec"ANSI_NORMAL"\n", t2_build - t1_build);
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
    return NumErrors;
}

bool C2Builder::havePackage(const std::string& name) const {
    PkgsConstIter iter = pkgs.find(name);
    return iter != pkgs.end();
}

Package* C2Builder::getPackage(const std::string& name, bool isExternal, bool isCLib) {
    PkgsIter iter = pkgs.find(name);
    if (iter == pkgs.end()) {
        Package* P = new Package(name, isExternal, isCLib);
        pkgs[name] = P;
        return P;
    } else {
        // TODO check that isCLib/isExternal matches returned package? otherwise give error
        return iter->second;
    }
}

// merges symbols of all files of each package
bool C2Builder::createPkgs() {
    for (unsigned i=0; i<files.size(); i++) {
        FileInfo* info = files[i];
        Package* pkg = getPackage(info->ast.getPkgName(), false, false);
        const AST::Symbols& symbols = info->ast.getSymbols();
        for (AST::SymbolsConstIter iter = symbols.begin(); iter != symbols.end(); ++iter) {
            Decl* New = iter->second;
            Decl* Old = pkg->findSymbol(iter->first);
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

bool C2Builder::loadExternalPackages() {
    // collect all external packages
    for (unsigned i=0; i<files.size(); i++) {
        FileInfo* info = files[i];
        for (unsigned u=0; u<info->ast.numUses(); u++) {
            UseDecl* D = info->ast.getUse(u);
            const std::string& name = D->getName();
            if (havePackage(name)) continue;
            if (!loadPackage(name)) {
                info->Diags.Report(D->getLocation(), clang::diag::err_unknown_package) << name;
                return false;
            }
        }
    }
    return true;
}

bool C2Builder::loadPackage(const std::string& name) {
    if (options.verbose) printf(COL_VERBOSE"loading external package %s"ANSI_NORMAL"\n", name.c_str());

    assert(!havePackage(name));
    // NOTE: MEMLEAK on Types

    // TEMP use dummy packages
    if (name == "c2") {
        getPackage("c2", true, false);
        return true;
    }
    SourceLocation loc;

    if (name == "stdio") {
        unsigned file_id = filenames.add("(stdio)");
        Package* stdioPkg = getPackage("stdio", true, true);
        // int puts(const char* s);
        {
            FunctionDecl* func = new FunctionDecl("puts", loc, true, file_id, Type::Int32());
            // TODO correct arg
            QualType QT(new PointerType(Type::Int8()), QUAL_CONST);
            QT->setCanonicalType(QT);
            func->addArg(new VarDecl(VARDECL_PARAM, "s", loc, QT, 0, true, file_id));
            stdioPkg->addSymbol(func);
            // function type
            func->setFunctionType(QualType(new FunctionType(func), 0));
        }
        //int printf(const char *format, ...);
        {
            FunctionDecl* func = new FunctionDecl("printf", loc, true, file_id, Type::Int32());
            // NOTE: MEMLEAK ON TYPE, this will go away when we remove these dummy protos
            QualType QT(new PointerType(Type::Int8()), QUAL_CONST);
            QT->setCanonicalType(QT);
            func->addArg(new VarDecl(VARDECL_PARAM, "format", loc, QT, 0, true, file_id));
            func->setVariadic();
            stdioPkg->addSymbol(func);
            // function type
            func->setFunctionType(QualType(new FunctionType(func), 0));
        }
        //int sprintf(char *str, const char *format, ...);
        {
            FunctionDecl* func = new FunctionDecl("sprintf", loc, true, file_id, Type::Int32());
            // NOTE: MEMLEAK ON TYPE, this will go away when we remove these dummy protos
            QualType QT(new PointerType(Type::Int8()), QUAL_CONST);
            QT->setCanonicalType(QT);
            func->addArg(new VarDecl(VARDECL_PARAM, "str", loc, QT, 0, true, file_id));
            func->addArg(new VarDecl(VARDECL_PARAM, "format", loc, QT, 0, true, file_id));
            func->setVariadic();
            stdioPkg->addSymbol(func);
            // function type
            func->setFunctionType(QualType(new FunctionType(func), 0));
        }
        return true;
    }

    if (name == "stdlib") {
        unsigned file_id = filenames.add("(stdlib)");
        Package* stdlibPkg = getPackage("stdlib", true, true);
        //void exit(int status);
        {
            FunctionDecl* func = new FunctionDecl("exit", loc, true, file_id, Type::Void());
            // TODO correct arg
            func->addArg(new VarDecl(VARDECL_PARAM, "status", loc, Type::Int32(), 0, true, file_id));
            stdlibPkg->addSymbol(func);
            // function type
            func->setFunctionType(QualType(new FunctionType(func), 0));
        }
        return true;
    }
    return false;
}

void C2Builder::printDependencies() const {
    StringBuilder output;
    DepGenerator generator(output);
    for (unsigned i=0; i<files.size(); i++) {
        files[i]->analyser->getExternals(generator);
    }
    generator.close();

    std::string path = "output/" + recipe.name;
    std::string filename = path + '/' + "deps.xml";
    FileUtils::writeFile(path.c_str(), filename, output);
}

unsigned C2Builder::analyse() {
    unsigned errors = 0;

    for (unsigned i=0; i<files.size(); i++) {
        errors += files[i]->analyser->checkUses();
    }
    if (errors) return errors;

    for (unsigned i=0; i<files.size(); i++) {
        errors += files[i]->analyser->resolveTypes();
    }
    if (errors) return errors;

    for (unsigned i=0; i<files.size(); i++) {
        errors += files[i]->analyser->resolveTypeCanonicals();
    }
    if (errors) return errors;

    for (unsigned i=0; i<files.size(); i++) {
        errors += files[i]->analyser->resolveStructMembers();
    }
    if (options.printAST1) printASTs();
    if (errors) return errors;

    for (unsigned i=0; i<files.size(); i++) {
        errors += files[i]->analyser->resolveVars();
    }
    if (errors) return errors;

    for (unsigned i=0; i<files.size(); i++) {
        errors += files[i]->analyser->checkVarInits();
    }
    if (options.printAST2) printASTs();
    if (errors) return errors;

    for (unsigned i=0; i<files.size(); i++) {
        errors += files[i]->analyser->resolveEnumConstants();
    }
    if (errors) return errors;

    for (unsigned i=0; i<files.size(); i++) {
        errors += files[i]->analyser->checkFunctionProtos();
    }
    if (errors) return errors;

    for (unsigned i=0; i<files.size(); i++) {
        errors += files[i]->analyser->checkFunctionBodies();
    }
    if (options.printAST3) printASTs();

    for (unsigned i=0; i<files.size(); i++) {
        files[i]->analyser->checkDeclsForUsed();
    }
    return errors;
}

void C2Builder::dumpPkgs() const {
    for (PkgsConstIter iter = pkgs.begin(); iter != pkgs.end(); ++iter) {
        const Package* P = iter->second;
        P->dump();
    }
}

void C2Builder::printASTs() const {
    for (unsigned i=0; i<files.size(); i++) {
        FileInfo* info = files[i];
        info->ast.print();
    }
}

bool C2Builder::checkMainFunction(DiagnosticsEngine& Diags) {
    Decl* mainDecl = 0;
    for (PkgsIter iter = pkgs.begin(); iter != pkgs.end(); ++iter) {
        Package* P = iter->second;
        Decl* decl = P->findSymbol("main");
        if (decl) {
            if (mainDecl) {
                // TODO multiple main functions
            } else {
                mainDecl = decl;
            }
        }
    }
    if (!mainDecl) {
        Diags.Report(diag::err_main_missing);
        return false;
    }
    return true;
}

void C2Builder::generateOptionalC() {
    if (!options.generateC) return;

    bool single_module = false;
    bool no_local_prefix = false;
    for (unsigned i=0; i<recipe.cConfigs.size(); i++) {
        const std::string& conf = recipe.cConfigs[i];
        // TODO just pass struct with bools?
        if (conf == "single_module") single_module = true;
        if (conf == "no_local_prefix") no_local_prefix = true;
    }

    if (single_module) {
        u_int64_t t1 = Utils::getCurrentTime();
        // TODO
        std::string filename = recipe.name;
        CCodeGenerator gen(filename, CCodeGenerator::SINGLE_FILE, pkgs, no_local_prefix);
        for (unsigned i=0; i<files.size(); i++) {
            FileInfo* info = files[i];
            gen.addEntry(info->ast);
        }
        if (options.verbose) printf(COL_VERBOSE"generating C (single module)"ANSI_NORMAL"\n");
        gen.generate();
        u_int64_t t2 = Utils::getCurrentTime();
        if (options.printTiming) printf(COL_TIME"C code generation took %lld usec"ANSI_NORMAL"\n", t2 - t1);
        if (options.printC) gen.dump();
        gen.write(recipe.name, "");
    } else {
        for (PkgsIter iter = pkgs.begin(); iter != pkgs.end(); ++iter) {
            Package* P = iter->second;
            u_int64_t t1 = Utils::getCurrentTime();
            if (P->isPlainC()) continue;
            // for now filter out 'c2' as well
            if (P->getName() == "c2") continue;
            if (options.verbose) printf(COL_VERBOSE"generating C for package %s"ANSI_NORMAL"\n", P->getName().c_str());
            CCodeGenerator gen(P->getName(), CCodeGenerator::MULTI_FILE, pkgs, no_local_prefix);
            for (unsigned i=0; i<files.size(); i++) {
                FileInfo* info = files[i];
                if (info->ast.getPkgName() == P->getName()) {
                    gen.addEntry(info->ast);
                }
            }
            gen.generate();
            u_int64_t t2 = Utils::getCurrentTime();
            if (options.printTiming) printf(COL_TIME"C code generation took %lld usec"ANSI_NORMAL"\n", t2 - t1);
            if (options.printC) gen.dump();
            gen.write(recipe.name, P->getName());
        }
    }
}

void C2Builder::generateOptionalIR() {
    if (!options.generateIR) return;

    for (PkgsIter iter = pkgs.begin(); iter != pkgs.end(); ++iter) {
        Package* P = iter->second;
        u_int64_t t1 = Utils::getCurrentTime();
        if (P->isPlainC()) continue;
        // for now filter out 'c2' as well
        if (P->getName() == "c2") continue;
        if (options.verbose) printf(COL_VERBOSE"generating IR for package %s"ANSI_NORMAL"\n", P->getName().c_str());
        CodeGenModule cgm(P);
        for (unsigned i=0; i<files.size(); i++) {
            FileInfo* info = files[i];
            if (info->ast.getPkgName() == P->getName()) {
                cgm.addEntry(info->ast);
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
