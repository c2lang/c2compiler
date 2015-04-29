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

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <errno.h>

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <llvm/Support/Host.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticIDs.h>
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
#define private public
#include <clang/Lex/Preprocessor.h>
#undef private
#include <clang/Lex/PreprocessorOptions.h>
#include <clang/Sema/SemaDiagnostic.h>

#include "Builder/C2Builder.h"
#include "Builder/Recipe.h"
#include "Builder/C2ModuleLoader.h"
#include "AST/AST.h"
#include "AST/Module.h"
#include "AST/Decl.h"

#include "Parser/C2Parser.h"
#include "Parser/C2Sema.h"
#include "Analyser/FileAnalyser.h"
#include "Algo/DepGenerator.h"
#include "CodeGen/CodeGenModule.h"
#include "CGenerator/CGenerator.h"
#include "Utils/color.h"
#include "Utils/Utils.h"
#include "Utils/GenUtils.h"

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
using namespace clang;

#define OUTPUT_DIR "output/"
#define BUILD_DIR  "/build/"

namespace C2 {
class DummyLoader : public ModuleLoader {
public:
    DummyLoader() {}
    virtual ~DummyLoader () {}

    virtual ModuleLoadResult loadModule(SourceLocation ImportLoc,
                                        ModuleIdPath Path,
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

    virtual GlobalModuleIndex* loadGlobalModuleIndex(SourceLocation TriggerLoc)
    {
        fprintf(stderr, "MODULE LOADER: loadGlobalModuleIndex\n");
        return 0;
    }

    virtual bool lookupMissingImports(StringRef Name,
                                      SourceLocation TriggerLoc)
    {
        fprintf(stderr, "MODULE LOADER: lookupMissingImports\n");
        return false;
    }
private:
    DummyLoader(const DummyLoader& rhs);
    DummyLoader& operator= (const DummyLoader& rhs);
};

class FileInfo {
public:
    FileInfo(DiagnosticsEngine& Diags_,
             LangOptions& LangOpts_,
             TargetInfo* pti,
             HeaderSearchOptions* HSOpts,
             FileManager& FileMgr,
             SourceManager& SM_,
             const std::string& filename_,
             const std::string& configs)
        : filename(filename_)
    // TODO note: Diags makes copy constr, pass DiagnosticIDs?
        , Diags(Diags_)
        , SM(SM_)
        , Headers(HSOpts, SM, Diags, LangOpts_, pti)
        , PPOpts(new PreprocessorOptions())
        , PP(PPOpts, Diags, LangOpts_, SM, Headers, loader)
        , ast(filename_)
        , analyser(0)
    {
        ApplyHeaderSearchOptions(PP.getHeaderSearchInfo(), *HSOpts, LangOpts_, pti->getTriple());

        PP.setPredefines(configs);
        PP.Initialize(*pti);


        // File stuff
        const FileEntry *pFile = FileMgr.getFile(filename);
        if (pFile == 0) {
            fprintf(stderr, "Error opening file: '%s'\n", filename.c_str());
            exit(-1);
        }
        FileID id = SM.createFileID(pFile, SourceLocation(), SrcMgr::C_User);
        PP.EnterSourceFile(id, nullptr, SourceLocation());

        // Manually set predefines (normally done in EnterMainSourceFile())
        std::unique_ptr<llvm::MemoryBuffer> SB = llvm::MemoryBuffer::getMemBufferCopy(configs, "<built-in>");
        assert(SB && "Cannot create predefined source buffer");
        FileID FID = SM.createFileID(std::move(SB));

        // NOTE: setPredefines() is normally private
        PP.setPredefinesFileID(FID);
        PP.EnterSourceFile(FID, nullptr, SourceLocation());

        Diags.getClient()->BeginSourceFile(LangOpts_, 0);
    }
    ~FileInfo() {
        //PP.EndSourceFile();
        //Diags.getClient()->EndSourceFile();

        //llvm::errs() << "\nSTATISTICS FOR '" << filename << "':\n";
        //PP.PrintStats();
        //PP.getIdentifierTable().PrintStats();
        //PP.getHeaderSearchInfo().PrintStats();
        //llvm::errs() << "\n";
    }

    bool parse(const BuildOptions& options) {
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
    DiagnosticsEngine& Diags;

    // SourceManager
    SourceManager& SM;

    // HeaderSearch
    HeaderSearch Headers;

    DummyLoader loader;

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
    , useColors(true)
{
    if (!isatty(1)) useColors = false;
}

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
    log(ANSI_GREEN, "building target %s", recipe.name.c_str());

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
    if (!options.testMode && isatty(2)) DiagOpts->ShowColors = true;
    IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
    DiagnosticsEngine Diags(DiagID, DiagOpts,
            // NOTE: setting ShouldOwnClient to true causes crash??
            new TextDiagnosticPrinter(llvm::errs(), DiagOpts), false);
    DiagnosticConsumer* client = Diags.getClient();

    // add these diagnostic groups by default
    Diags.setSeverityForGroup(diag::Flavor::WarningOrError, "conversion", diag::Severity::Warning);
    Diags.setSeverityForGroup(diag::Flavor::WarningOrError, "all", diag::Severity::Warning);
    Diags.setSeverityForGroup(diag::Flavor::WarningOrError, "extra", diag::Severity::Warning);
    assert (!Diags.setSeverityForGroup(diag::Flavor::WarningOrError, "covered-switch-default", diag::Severity::Warning));
    //Diags.setDiagnosticWarningAsError(diag::warn_integer_too_large, true);

    Diags.setSeverity(diag::warn_falloff_nonvoid_function, diag::Severity::Error, SourceLocation());
    Diags.setSeverity(diag::warn_falloff_nonvoid_function, diag::Severity::Error, SourceLocation());
    Diags.setSeverity(diag::warn_duplicate_attribute_exact, diag::Severity::Error, SourceLocation());
    Diags.setSeverity(diag::warn_not_in_enum, diag::Severity::Error, SourceLocation());
    Diags.setSeverity(diag::warn_missing_case1, diag::Severity::Error, SourceLocation());
    Diags.setSeverity(diag::warn_missing_case2, diag::Severity::Error, SourceLocation());
    Diags.setSeverity(diag::warn_missing_case3, diag::Severity::Error, SourceLocation());
    Diags.setSeverity(diag::warn_missing_cases, diag::Severity::Error, SourceLocation());

    // set recipe warning options
    for (unsigned i=0; i<recipe.silentWarnings.size(); i++) {
        const std::string& conf = recipe.silentWarnings[i];

        if (conf == "no-unused") {
            Diags.setSeverityForGroup(diag::Flavor::WarningOrError, "unused", diag::Severity::Ignored);
            Diags.setSeverityForGroup(diag::Flavor::WarningOrError, "unused-parameter", diag::Severity::Ignored);
            continue;
        }
        if (conf == "no-unused-variable") {
            Diags.setSeverityForGroup(diag::Flavor::WarningOrError, "unused-variable", diag::Severity::Ignored);
            continue;
        }
        if (conf == "no-unused-function") {
            Diags.setSeverityForGroup(diag::Flavor::WarningOrError, "unused-function", diag::Severity::Ignored);
            continue;
        }
        if (conf == "no-unused-parameter") {
            Diags.setSeverityForGroup(diag::Flavor::WarningOrError, "unused-parameter", diag::Severity::Ignored);
            continue;
        }
        if (conf == "no-unused-type") {
            Diags.setSeverityForGroup(diag::Flavor::WarningOrError, "unused-type", diag::Severity::Ignored);
            continue;
        }
        if (conf == "no-unused-module") {
            Diags.setSeverityForGroup(diag::Flavor::WarningOrError, "unused-module", diag::Severity::Ignored);
            continue;
        }
        if (conf == "no-unused-public") {
            Diags.setSeverityForGroup(diag::Flavor::WarningOrError, "unused-public", diag::Severity::Ignored);
            continue;
        }
        if (conf == "no-unused-label") {
            Diags.setSeverityForGroup(diag::Flavor::WarningOrError, "unused-label", diag::Severity::Ignored);
            continue;
        }
        fprintf(stderr, "recipe: unknown warning: '%s'\n", conf.c_str());
        exit(-1);
    }

    // TargetInfo
    std::shared_ptr<TargetOptions> to(new TargetOptions());
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

    // FileManager
    FileSystemOptions FileSystemOpts;
    FileManager FileMgr(FileSystemOpts);
    SourceManager SM(Diags, FileMgr);

    // phase 1: parse and local analyse
    unsigned errors = 0;
    u_int64_t t1_parse = Utils::getCurrentTime();
    for (int i=0; i<recipe.size(); i++) {
        const std::string& filename = recipe.get(i);
        FileInfo* info = new FileInfo(Diags, LangOpts, pti, HSOpts, FileMgr, SM, filename, PredefineBuffer);
        files.push_back(info);
        if (options.verbose) log(COL_VERBOSE, "parsing %s", filename.c_str());
        bool ok = info->parse(options);
        if (options.printAST0) info->ast.print(true, true);
        errors += !ok;
    }
    u_int64_t t2_parse = Utils::getCurrentTime();
    u_int64_t t1_analyse, t2_analyse;
    if (options.printTiming) log(COL_TIME, "parsing took %" PRIu64" usec", t2_parse - t1_parse);
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
    if (options.printTiming) log(COL_TIME, "analysis took %" PRIu64" usec", t2_analyse - t1_analyse);
    if (client->getNumErrors()) goto out;

    if (!checkMainFunction(Diags)) goto out;

    if (!checkExportedPackages()) goto out;

    generateOptionsDeps();

    generateOptionalC();

    generateOptionalIR();

    if (options.verbose) log(COL_VERBOSE, "done");
out:
    //SM.PrintStats();
    u_int64_t t2_build = Utils::getCurrentTime();
    if (options.printTiming) log(COL_TIME, "total build took %" PRIu64" usec", t2_build - t1_build);
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
        C2::Module* M = new C2::Module(name, isExternal, isCLib);
        modules[name] = M;
        if (recipe.hasExported(name)) M->setExported();
        return M;
    } else {
        // TODO check that isCLib/isExternal matches returned module? otherwise give error
        return iter->second;
    }
}

C2::Module* C2Builder::findModule(const std::string& name) const {
    ModulesConstIter iter = modules.find(name);
    if (iter == modules.end()) return 0;
    else return iter->second;
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
                info->Diags.Report(New->getLocation(), diag::err_redefinition) << New->getName();
                info->Diags.Report(Old->getLocation(), diag::note_previous_definition);
            } else {
                mod->addSymbol(New);
            }
            if (New->isPublic() && mod->isExported()) New->setExported();
        }
        // setModule() for ArrayValueDecls since they're not symbols
        for (unsigned i=0; i<info->ast.numArrayValues(); i++) {
            info->ast.getArrayValue(i)->setModule(mod);
        }
        // merge attributes
        mod->addAttributes(info->ast.getAttributes());
    }
    return true;
}

bool C2Builder::loadExternalModules() {
    // collect all external modules
    C2ModuleLoader loader;
    bool haveErrors = false;
    for (unsigned i=0; i<files.size(); i++) {
        FileInfo* info = files[i];
        for (unsigned u=0; u<info->ast.numImports(); u++) {
            ImportDecl* D = info->ast.getImport(u);
            const std::string& name = D->getModuleName();
            if (haveModule(name)) continue;
            if (options.verbose) log(COL_VERBOSE, "loading external module %s", name.c_str());
            Module* M = loader.load(name);
            if (!M) {
                info->Diags.Report(D->getLocation(), clang::diag::err_unknown_module) << name;
                haveErrors = true;
            }
            modules[name] = M;
        }
    }
    return !haveErrors;
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
        errors += files[i]->analyser->checkArrayValues();
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

void C2Builder::log(const char* color, const char* format, ...) const {
    char buffer[256];
    va_list(Args);
    va_start(Args, format);
    vsprintf(buffer, format, Args);
    va_end(Args);

    if (useColors) printf("%s%s" ANSI_NORMAL "\n", color, buffer);
    else printf("%s\n", buffer);
}

bool C2Builder::checkMainFunction(DiagnosticsEngine& Diags) {

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

    if (recipe.type == GenUtils::EXECUTABLE) {
        // bin: must have main
        if (options.testMode) return true;
        if (!mainDecl) {
            Diags.Report(diag::err_main_missing);
            return false;
        }
    } else {
        // lib: cannot have main
        if (mainDecl) {
            Diags.Report(mainDecl->getLocation(), diag::err_lib_has_main);
            return false;
        }
    }
    return true;
}

bool C2Builder::checkExportedPackages() const {
    for (unsigned i=0; i<recipe.exported.size(); i++) {
        const std::string& modName = recipe.exported[i];
        const Module* M = findModule(modName);
        if (!M) {
            fprintf(stderr, "cannot export '%s', no such module\n", modName.c_str());
            exit(-1);
        }
        if (M->isExternal()) {
            fprintf(stderr, "cannot export external module '%s'\n", modName.c_str());
            exit(-1);
        }
    }
    return true;
}

void C2Builder::generateOptionalC() {
    if (options.checkOnly) return;
    if (!options.generateC && !recipe.generateCCode) return;

    u_int64_t t1 = Utils::getCurrentTime();
    bool single_module = false;
    bool no_build = false;
    for (unsigned i=0; i<recipe.cConfigs.size(); i++) {
        const std::string& conf = recipe.cConfigs[i];
        // TODO just pass struct with bools?
        if (conf == "single-module") single_module = true;
        else if (conf == "no-build") no_build = true;
        else {
            fprintf(stderr, ANSI_RED"invalid c-generation argument '%s'" ANSI_NORMAL"\n", conf.c_str());
        }
    }

    CGenerator::Options cgen_options(OUTPUT_DIR, BUILD_DIR);
    cgen_options.single_module = single_module;
    cgen_options.printC = options.printC;
    CGenerator cgen(recipe.name, recipe.type, modules, cgen_options);
    for (unsigned i=0; i<files.size(); i++) {
        FileInfo* info = files[i];
        cgen.addFile(info->ast);
    }
    if (options.verbose) log(COL_VERBOSE, "generating C code");
    cgen.generate();

    u_int64_t t2 = Utils::getCurrentTime();
    if (options.printTiming) log(COL_TIME, "C code generation took %" PRIu64" usec", t2 - t1);

    if (!no_build) {
        if (options.verbose) log(COL_VERBOSE, "building C code");
        u_int64_t t3 = Utils::getCurrentTime();
        cgen.build();
        u_int64_t t4 = Utils::getCurrentTime();
        if (options.printTiming) log(COL_TIME, "C code compilation took %" PRIu64" usec", t4 - t3);
    }
}

void C2Builder::generateOptionalIR() {
    if (options.checkOnly) return;
    if (!options.generateIR && !recipe.generateIR) return;

    bool single_module = false;
    for (unsigned i=0; i<recipe.genConfigs.size(); i++) {
        const std::string& conf = recipe.genConfigs[i];
        // TODO just pass struct with bools?
        if (conf == "single-module") single_module = true;
        else {
            fprintf(stderr, ANSI_RED"invalid code generation argument '%s'" ANSI_NORMAL"\n", conf.c_str());
        }
    }

    std::string outdir = OUTPUT_DIR + recipe.name + BUILD_DIR;

    if (single_module) {
        u_int64_t t1 = Utils::getCurrentTime();
        std::string filename = recipe.name;
        //  HMM dont have Module P here, refactor
        CodeGenModule cgm(filename, true);
        if (options.verbose) log(COL_VERBOSE, "generating IR for single module %s", filename.c_str());
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
        if (options.printTiming) log(COL_TIME, "IR generation took %" PRIu64" usec", t2 - t1);
        if (options.printIR) cgm.dump();
        bool ok = cgm.verify();
        if (ok) cgm.write(outdir, filename);
    } else {
        for (ModulesIter iter = modules.begin(); iter != modules.end(); ++iter) {
            Module* P = iter->second;
            u_int64_t t1 = Utils::getCurrentTime();
            if (P->isPlainC()) continue;
            if (P->getName() == "c2") continue;

            if (options.verbose) log(COL_VERBOSE, "generating IR for module %s", P->getName().c_str());
            CodeGenModule cgm(P->getName(), false);
            for (unsigned i=0; i<files.size(); i++) {
                FileInfo* info = files[i];
                if (info->ast.getModuleName() == P->getName()) {
                    cgm.addEntry(info->ast);
                }
            }
            cgm.generate();
            u_int64_t t2 = Utils::getCurrentTime();
            if (options.printTiming) log(COL_TIME, "IR generation took %" PRIu64" usec", t2 - t1);
            if (options.printIR) cgm.dump();
            bool ok = cgm.verify();
            if (ok) cgm.write(outdir, P->getName());
        }
    }
}

void C2Builder::generateOptionsDeps() const {
    if (options.checkOnly) return;
    if (!options.printDependencies && !recipe.generateDeps) return;

    if (options.verbose) log(COL_VERBOSE, "generating dependencies");

    u_int64_t t1 = Utils::getCurrentTime();
    bool showFiles = false;
    bool showExternals = false;
    bool showPrivate = true;
    for (unsigned i=0; i<recipe.depConfigs.size(); i++) {
        const std::string& conf = recipe.depConfigs[i];
        // TODO just pass struct with bools?
        if (conf == "show-files") showFiles = true;
        if (conf == "show-externals") showExternals = true;
    }

    DepGenerator generator(showFiles, showPrivate, showExternals);
    for (unsigned i=0; i<files.size(); i++) {
        generator.analyse(files[i]->ast);
    }

    std::string path = OUTPUT_DIR + recipe.name + '/';
    generator.write(recipe.name, path);
    u_int64_t t2 = Utils::getCurrentTime();
    if (options.printTiming) log(COL_TIME, "dep generation took %" PRIu64" usec", t2 - t1);
}

