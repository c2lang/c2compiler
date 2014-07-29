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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
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
#include "Builder/DepGenerator.h"
#include "AST/AST.h"
#include "AST/Module.h"
#include "AST/Decl.h"
// only for Dummy Modules
#include "AST/Expr.h"

#include "Parser/C2Parser.h"
#include "Parser/C2Sema.h"
#include "Analyser/FileAnalyser.h"
#include "CodeGen/CodeGenModule.h"
#include "CGenerator/CCodeGenerator.h"
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
                               clang::Module::NameVisibilityKind Visibility,
                               bool IsInclusionDirective)
    {
        fprintf(stderr, "MODULE LOADER: loadModule\n");
        return ModuleLoadResult();
    }

    virtual void makeModuleVisible(clang::Module *Mod,
                                   clang::Module::NameVisibilityKind Visibility,
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
        , Headers(HSOpts, SM, Diags, LangOpts_, pti)
        , PPOpts(new PreprocessorOptions())
        , PP(PPOpts, Diags, LangOpts_, pti, SM, Headers, loader)
        , ast(filename_, file_id)
        , analyser(0)
    {
        ApplyHeaderSearchOptions(PP.getHeaderSearchInfo(), *HSOpts, LangOpts_, pti->getTriple());

        PP.setPredefines(configs);

        // File stuff
        const FileEntry *pFile = FileMgr.getFile(filename);
        if (pFile == 0) {
            fprintf(stderr, "Error opening file: '%s'\n", filename.c_str());
            exit(-1);
        }
        SM.createMainFileID(pFile);
        PP.EnterMainSourceFile();
        // NOTE: filling zero instead of PP works
        //Diags.getClient()->BeginSourceFile(LangOpts_, &PP);
        Diags.getClient()->BeginSourceFile(LangOpts_, 0);
    }
    ~FileInfo() {
        //Diags.getClient()->EndSourceFile();
    }

    bool parse(const BuildOptions& options) {
        if (options.verbose) printf(COL_VERBOSE"parsing %s"ANSI_NORMAL"\n", filename.c_str());
        C2Sema sema(SM, Diags, typeContext, ast, PP);
        C2Parser parser(PP, sema);
        parser.Initialize();
        return parser.Parse();
    }

    void createAnalyser(const Modules& modules, bool verbose) {
        analyser = new FileAnalyser(modules, Diags, ast, typeContext, verbose);
    }
    void deleteAnalyser() {
        delete analyser;
        analyser = 0;
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
    for (ModulesIter iter = modules.begin(); iter != modules.end(); ++iter) {
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
    if (!options.testMode) DiagOpts->ShowColors = true;
    IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
    DiagnosticsEngine Diags(DiagID, DiagOpts,
            // NOTE: setting ShouldOwnClient to true causes crash??
            new TextDiagnosticPrinter(llvm::errs(), DiagOpts), false);
    DiagnosticConsumer* client = Diags.getClient();

    // add these diagnostic groups by default
    Diags.setDiagnosticGroupMapping("conversion", diag::MAP_WARNING);
    Diags.setDiagnosticGroupMapping("all", diag::MAP_WARNING);
    Diags.setDiagnosticGroupMapping("extra", diag::MAP_WARNING);
    //Diags.setDiagnosticWarningAsError(diag::warn_integer_too_large, true);
    Diags.setDiagnosticWarningAsError(diag::warn_falloff_nonvoid_function, true);
    // Workaround
    //Diags.setDiagnosticErrorAsFatal(diag::warn_integer_too_large, true);
    //Diags.setDiagnosticErrorAsFatal(diag::warn_integer_too_large, false);
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
        if (conf == "no-unused-module") {
            Diags.setDiagnosticGroupMapping("unused-module", diag::MAP_IGNORE);
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
        if (options.printAST0) info->ast.print(true);
        errors += !ok;
    }
    u_int64_t t2_parse = Utils::getCurrentTime();
    u_int64_t t1_analyse, t2_analyse;
    if (options.printTiming) printf(COL_TIME"parsing took %"PRIu64" usec"ANSI_NORMAL"\n", t2_parse - t1_parse);
    if (client->getNumErrors()) goto out;
    t1_analyse = Utils::getCurrentTime();

    // phase 1b: merge file's symbol tables to module symbols tables
    errors = !createModules();
    if (options.printSymbols) dumpModules();

    if (client->getNumErrors()) goto out;

    // phase 1c: load required external modules
    if (!loadExternalModules()) goto out;

    if (options.printModules) {
        printf("List of modules:\n");
        for (ModulesConstIter iter = modules.begin(); iter != modules.end(); ++iter) {
            const Module* P = iter->second;
            printf("  %s %s\n", P->getName().c_str(), P->isExternal() ? "(external)" : "");
        }
    }

    // create analysers/scopes
    for (unsigned i=0; i<files.size(); i++) {
        files[i]->createAnalyser(modules, options.verbose);
    }

    // phase 2: run analysing on all files
    errors += analyse();

    for (unsigned i=0; i<files.size(); i++) {
        files[i]->deleteAnalyser();
    }
    t2_analyse = Utils::getCurrentTime();
    if (options.printTiming) printf(COL_TIME"analysis took %"PRIu64" usec"ANSI_NORMAL"\n", t2_analyse - t1_analyse);
    if (client->getNumErrors()) goto out;

    if (!checkMainFunction(Diags)) goto out;

    generateOptionsDeps();

    generateOptionalC();

    generateOptionalIR();

    if (options.verbose) printf(COL_VERBOSE"done"ANSI_NORMAL"\n");
out:
    u_int64_t t2_build = Utils::getCurrentTime();
    if (options.printTiming) printf(COL_TIME"total build took %"PRIu64" usec"ANSI_NORMAL"\n", t2_build - t1_build);
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

bool C2Builder::haveModule(const std::string& name) const {
    ModulesConstIter iter = modules.find(name);
    return iter != modules.end();
}

C2::Module* C2Builder::getModule(const std::string& name, bool isExternal, bool isCLib) {
    ModulesIter iter = modules.find(name);
    if (iter == modules.end()) {
        C2::Module* P = new C2::Module(name, isExternal, isCLib);
        modules[name] = P;
        return P;
    } else {
        // TODO check that isCLib/isExternal matches returned module? otherwise give error
        return iter->second;
    }
}

// merges symbols of all files of each module
bool C2Builder::createModules() {
    for (unsigned i=0; i<files.size(); i++) {
        FileInfo* info = files[i];
        Module* mod = getModule(info->ast.getModuleName(), false, false);
        const AST::Symbols& symbols = info->ast.getSymbols();
        for (AST::SymbolsConstIter iter = symbols.begin(); iter != symbols.end(); ++iter) {
            Decl* New = iter->second;
            if (isa<ImportDecl>(New)) continue;
            Decl* Old = mod->findSymbol(iter->first);
            if (Old) {
                fprintf(stderr, "MULTI_FILE: duplicate symbol %s\n", New->getName().c_str());
                fprintf(stderr, "TODO NASTY: locs have to be resolved in different SourceManagers..\n");
                // TODO: continue/return?
                return false;
            } else {
                mod->addSymbol(New);
            }
        }
    }
    return true;
}

bool C2Builder::loadExternalModules() {
    // collect all external modules
    bool haveErrors = false;
    for (unsigned i=0; i<files.size(); i++) {
        FileInfo* info = files[i];
        for (unsigned u=0; u<info->ast.numImports(); u++) {
            ImportDecl* D = info->ast.getImport(u);
            const std::string& name = D->getModuleName();
            if (haveModule(name)) continue;
            if (!loadModule(name)) {
                info->Diags.Report(D->getLocation(), clang::diag::err_unknown_module) << name;
                haveErrors = true;
            }
        }
    }
    return !haveErrors;
}

bool C2Builder::loadModule(const std::string& name) {
    if (options.verbose) printf(COL_VERBOSE"loading external module %s"ANSI_NORMAL"\n", name.c_str());

    assert(!haveModule(name));
    // NOTE: MEMLEAK on Types

    SourceLocation loc;

    if (name == "stdio") {
        unsigned file_id = filenames.add("(stdio)");
        Module* stdioMod = getModule("stdio", true, true);
        // int puts(const char* s);
        {
            FunctionDecl* func = new FunctionDecl("puts", loc, true, file_id, Type::Int32());
            // TODO correct arg
            QualType QT(new PointerType(Type::Int8()), QUAL_CONST);
            QT->setCanonicalType(QT);
            VarDecl* Arg1 = new VarDecl(VARDECL_PARAM, "s", loc, QT, 0, true, file_id);
            Arg1->setType(QT);
            func->addArg(Arg1);
            stdioMod->addSymbol(func);
            // function type
            func->setType(QualType(new FunctionType(func), 0));
        }
        //int printf(const char *format, ...);
        {
            FunctionDecl* func = new FunctionDecl("printf", loc, true, file_id, Type::Int32());
            // NOTE: MEMLEAK ON TYPE, this will go away when we remove these dummy protos
            QualType QT(new PointerType(Type::Int8()), QUAL_CONST);
            QT->setCanonicalType(QT);
            VarDecl* Arg1 = new VarDecl(VARDECL_PARAM, "format", loc, QT, 0, true, file_id);
            Arg1->setType(QT);
            func->addArg(Arg1);
            func->setVariadic();
            stdioMod->addSymbol(func);
            // function type
            func->setType(QualType(new FunctionType(func), 0));
        }
        //int sprintf(char *str, const char *format, ...);
        {
            FunctionDecl* func = new FunctionDecl("sprintf", loc, true, file_id, Type::Int32());
            // NOTE: MEMLEAK ON TYPE, this will go away when we remove these dummy protos
            QualType QT(new PointerType(Type::Int8()), QUAL_CONST);
            QT->setCanonicalType(QT);
            VarDecl* Arg1 = new VarDecl(VARDECL_PARAM, "str", loc, QT, 0, true, file_id);
            Arg1->setType(QT);
            func->addArg(Arg1);
            VarDecl* Arg2 = new VarDecl(VARDECL_PARAM, "format", loc, QT, 0, true, file_id);
            Arg2->setType(QT);
            func->addArg(Arg2);
            func->setVariadic();
            stdioMod->addSymbol(func);
            // function type
            func->setType(QualType(new FunctionType(func), 0));
        }
        return true;
    }
    if (name == "string_h") {
        // TODO
        unsigned file_id = filenames.add("(string_h)");
        Module* stringMod = getModule("string_h", true, true);
        // void *memset(void *s, int c, size_t n);
        {
            QualType VP(new PointerType(Type::Void()));
            VP->setCanonicalType(VP);
            FunctionDecl* func = new FunctionDecl("memset", loc, true, file_id, VP);
            // NOTE: MEMLEAK ON TYPE, this will go away when we remove these dummy protos
            VarDecl* Arg1 = new VarDecl(VARDECL_PARAM, "s", loc, VP, 0, true, file_id);
            Arg1->setType(VP);
            func->addArg(Arg1);
            VarDecl* Arg2 = new VarDecl(VARDECL_PARAM, "c", loc, Type::Int32(), 0, true, file_id);
            Arg2->setType(Type::Int32());
            func->addArg(Arg2);
            // TEMP size_t -> uint32
            VarDecl* Arg3 = new VarDecl(VARDECL_PARAM, "n", loc, Type::UInt32(), 0, true, file_id);
            Arg3->setType(Type::UInt32());
            func->addArg(Arg3);
            stringMod->addSymbol(func);
            // function type
            func->setType(QualType(new FunctionType(func), 0));
        }
        return true;

    }
    if (name == "stdlib") {
        unsigned file_id = filenames.add("(stdlib)");
        Module* stdlibMod = getModule("stdlib", true, true);
        //void exit(int status);
        {
            FunctionDecl* func = new FunctionDecl("exit", loc, true, file_id, Type::Void());
            // TODO correct arg
            VarDecl* Arg1 = new VarDecl(VARDECL_PARAM, "status", loc, Type::Int32(), 0, true, file_id);
            Arg1->setType(Type::Int32());
            func->addArg(Arg1);
            stdlibMod->addSymbol(func);
            // function type
            func->setType(QualType(new FunctionType(func), 0));
        }
        return true;
    }
    if (name == "c2") {
        unsigned file_id = filenames.add("(c2)");
        Module* c2Mod = getModule("c2", true, false);
        {
            // make constant, CTC_NONE
            QualType QT = Type::UInt64();
            QT.addConst();
            uint64_t value = time(0);
            Expr* init = new IntegerLiteral(loc, llvm::APInt(64, value, false));
            // TODO get error without value if CTC_NONE, CTC_FULL gives out-of-bounds for value 123?!!
            init->setCTC(CTC_NONE); // Don't check range, only type
            init->setConstant();
            init->setType(QT);
            VarDecl* var = new VarDecl(VARDECL_GLOBAL, "buildtime", loc, QT, init, true, file_id);
            var->setType(QT);
            c2Mod->addSymbol(var);
        }
        return true;
    }
    return false;
}

unsigned C2Builder::analyse() {
    unsigned errors = 0;

    for (unsigned i=0; i<files.size(); i++) {
        errors += files[i]->analyser->checkImports();
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
        errors += files[i]->analyser->resolveEnumConstants();
    }
    if (errors) return errors;

    for (unsigned i=0; i<files.size(); i++) {
        errors += files[i]->analyser->checkVarInits();
    }
    if (options.printAST2) printASTs();
    if (errors) return errors;

    for (unsigned i=0; i<files.size(); i++) {
        errors += files[i]->analyser->checkFunctionProtos();
    }
    if (errors) return errors;

    for (unsigned i=0; i<files.size(); i++) {
        errors += files[i]->analyser->checkFunctionBodies();
    }

    for (unsigned i=0; i<files.size(); i++) {
        files[i]->analyser->checkDeclsForUsed();
    }

    if (options.printAST3) printASTs();
    return errors;
}

void C2Builder::dumpModules() const {
    for (ModulesConstIter iter = modules.begin(); iter != modules.end(); ++iter) {
        const Module* P = iter->second;
        P->dump();
    }
}

void C2Builder::printASTs() const {
    for (unsigned i=0; i<files.size(); i++) {
        FileInfo* info = files[i];
        info->ast.print(true);
    }
}

bool C2Builder::checkMainFunction(DiagnosticsEngine& Diags) {
    if (!recipe.isExec) return false;
    if (options.testMode) return true;

    Decl* mainDecl = 0;
    for (ModulesIter iter = modules.begin(); iter != modules.end(); ++iter) {
        Module* P = iter->second;
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
    if (!options.generateC && !recipe.generateCCode) return;

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
        CCodeGenerator gen(filename, CCodeGenerator::SINGLE_FILE, modules, no_local_prefix);
        for (unsigned i=0; i<files.size(); i++) {
            FileInfo* info = files[i];
            gen.addEntry(info->ast);
        }
        if (options.verbose) printf(COL_VERBOSE"generating C (single module)"ANSI_NORMAL"\n");
        gen.generate();
        u_int64_t t2 = Utils::getCurrentTime();
        if (options.printTiming) printf(COL_TIME"C code generation took %"PRIu64" usec"ANSI_NORMAL"\n", t2 - t1);
        if (options.printC) gen.dump();
        gen.write(recipe.name, "");
    } else {
        for (ModulesIter iter = modules.begin(); iter != modules.end(); ++iter) {
            Module* P = iter->second;
            u_int64_t t1 = Utils::getCurrentTime();
            if (P->isPlainC()) continue;
            // for now filter out 'c2' as well
            if (P->getName() == "c2") continue;
            if (options.verbose) printf(COL_VERBOSE"generating C for module %s"ANSI_NORMAL"\n", P->getName().c_str());
            CCodeGenerator gen(P->getName(), CCodeGenerator::MULTI_FILE, modules, no_local_prefix);
            for (unsigned i=0; i<files.size(); i++) {
                FileInfo* info = files[i];
                if (info->ast.getModuleName() == P->getName()) {
                    gen.addEntry(info->ast);
                }
            }
            gen.generate();
            u_int64_t t2 = Utils::getCurrentTime();
            if (options.printTiming) printf(COL_TIME"C code generation took %"PRIu64" usec"ANSI_NORMAL"\n", t2 - t1);
            if (options.printC) gen.dump();
            gen.write(recipe.name, P->getName());
        }
    }
}

void C2Builder::generateOptionalIR() {
    if (!options.generateIR) return;

    bool single_module = false;
    for (unsigned i=0; i<recipe.genConfigs.size(); i++) {
        const std::string& conf = recipe.genConfigs[i];
        // TODO just pass struct with bools?
        if (conf == "single_module") single_module = true;
    }

    if (single_module) {
        u_int64_t t1 = Utils::getCurrentTime();
        std::string filename = recipe.name;
        //  HMM dont have Module P here, refactor
        CodeGenModule cgm(filename, true);
        if (options.verbose) printf(COL_VERBOSE"generating IR for single module %s"ANSI_NORMAL"\n", filename.c_str());
        for (ModulesIter iter = modules.begin(); iter != modules.end(); ++iter) {
            Module* P = iter->second;
            if (P->isPlainC()) continue;
            if (P->getName() == "c2") continue;

            for (unsigned i=0; i<files.size(); i++) {
                FileInfo* info = files[i];
                if (info->ast.getModuleName() == P->getName()) {
                    cgm.addEntry(info->ast);
                }
            }
        }
        cgm.generate();
        u_int64_t t2 = Utils::getCurrentTime();
        if (options.printTiming) printf(COL_TIME"IR generation took %"PRIu64" usec"ANSI_NORMAL"\n", t2 - t1);
        if (options.printIR) cgm.dump();
        bool ok = cgm.verify();
        if (ok) cgm.write(recipe.name, filename);
    } else {
        for (ModulesIter iter = modules.begin(); iter != modules.end(); ++iter) {
            Module* P = iter->second;
            u_int64_t t1 = Utils::getCurrentTime();
            if (P->isPlainC()) continue;
            if (P->getName() == "c2") continue;

            if (options.verbose) printf(COL_VERBOSE"generating IR for module %s"ANSI_NORMAL"\n", P->getName().c_str());
            CodeGenModule cgm(P->getName(), false);
            for (unsigned i=0; i<files.size(); i++) {
                FileInfo* info = files[i];
                if (info->ast.getModuleName() == P->getName()) {
                    cgm.addEntry(info->ast);
                }
            }
            cgm.generate();
            u_int64_t t2 = Utils::getCurrentTime();
            if (options.printTiming) printf(COL_TIME"IR generation took %"PRIu64" usec"ANSI_NORMAL"\n", t2 - t1);
            if (options.printIR) cgm.dump();
            bool ok = cgm.verify();
            if (ok) cgm.write(recipe.name, P->getName());
        }
    }
}

void C2Builder::generateOptionsDeps() const {
    if (!options.printDependencies && !recipe.generateDeps) return;

    u_int64_t t1 = Utils::getCurrentTime();
    bool showFiles = false;
    bool showExternals = false;
    bool showPrivate = true;
    for (unsigned i=0; i<recipe.depConfigs.size(); i++) {
        const std::string& conf = recipe.depConfigs[i];
        // TODO just pass struct with bools?
        if (conf == "show_files") showFiles = true;
        if (conf == "show_externals") showExternals = true;
    }

    DepGenerator generator(showFiles, showPrivate, showExternals);
    for (unsigned i=0; i<files.size(); i++) {
        generator.analyse(files[i]->ast);
    }
    StringBuilder output;
    generator.write(output, recipe.name);
    u_int64_t t2 = Utils::getCurrentTime();
    std::string path = "output/" + recipe.name;
    std::string filename = path + '/' + "deps.xml";
    FileUtils::writeFile(path.c_str(), filename, output);
    if (options.printTiming) printf(COL_TIME"dep generation took %"PRIu64" usec"ANSI_NORMAL"\n", t2 - t1);
}

