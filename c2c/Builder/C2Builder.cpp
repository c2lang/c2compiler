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

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <errno.h>
#include <memory.h>

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <llvm/Support/Host.h>
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
// for Rewriter
#include <clang/Rewrite/Core/Rewriter.h>

#include "Builder/C2Builder.h"
#include "Builder/Recipe.h"
#include "Builder/C2ModuleLoader.h"
#include "Builder/ManifestWriter.h"
#include "AST/AST.h"
#include "AST/Module.h"
#include "AST/Decl.h"

#include "Parser/C2Parser.h"
#include "Parser/C2Sema.h"
#include "Analyser/ComponentAnalyser.h"
#include "Algo/DepGenerator.h"
#include "Algo/TagWriter.h"
#include "IRGenerator/CodeGenModule.h"
#include "IRGenerator/InterfaceGenerator.h"
#include "CGenerator/CGenerator.h"
#include "Refactor/RefFinder.h"
#include "Utils/color.h"
#include "Utils/Utils.h"
#include "Utils/StringBuilder.h"

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
                                   SourceLocation ImportLoc)
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

class ParseHelper {
public:
    ParseHelper(DiagnosticsEngine& Diags_,
                LangOptions& LangOpts_,
                clang::TargetInfo* pti_,
                std::shared_ptr<HeaderSearchOptions> HSOpts_,
                SourceManager& SM_,
                FileManager& FileMgr_,
                const std::string& configs_)
        : Diags(Diags_)
        , LangOpts(LangOpts_)
        , pti(pti_)
        , HSOpts(HSOpts_)
        , SM(SM_)
        , FileMgr(FileMgr_)
        , configs(configs_)
    {}

    bool parse(Component& component, Module* existingMod, const std::string& filename, bool printAST) {
        // NOTE: seems to get deleted by Preprocessor
        HeaderSearch* Headers = new HeaderSearch(HSOpts, SM, Diags, LangOpts, pti);
        DummyLoader loader;

        std::shared_ptr<PreprocessorOptions> PPOpts(new PreprocessorOptions());
        Preprocessor PP(PPOpts, Diags, LangOpts, SM, *Headers, loader);

        ApplyHeaderSearchOptions(PP.getHeaderSearchInfo(), *HSOpts, LangOpts, pti->getTriple());
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

        // TEMP rewriter test
        //ast.fileID = id;
        // Manually set predefines (normally done in EnterMainSourceFile())
        std::unique_ptr<llvm::MemoryBuffer> SB = llvm::MemoryBuffer::getMemBufferCopy(configs, "<built-in>");
        assert(SB && "Cannot create predefined source buffer");
        FileID FID = SM.createFileID(std::move(SB));

        // NOTE: setPredefines() is normally private
        PP.setPredefinesFileID(FID);
        PP.EnterSourceFile(FID, nullptr, SourceLocation());

        Diags.getClient()->BeginSourceFile(LangOpts, 0);

        C2Sema sema(SM, Diags, PP, component, existingMod, filename);
        C2Parser parser(PP, sema, component.isExternal());
        bool ok = parser.Parse();
        if (printAST) sema.printAST();
#if 0
        PP.EndSourceFile();

        llvm::errs() << "\nSTATISTICS FOR '" << ast.getFileName() << "':\n";
        PP.PrintStats();
        PP.getIdentifierTable().PrintStats();
        llvm::errs() << "\n";
#endif
        return ok;
    }

    DiagnosticsEngine& Diags;
    LangOptions& LangOpts;
    clang::TargetInfo* pti;
    std::shared_ptr<HeaderSearchOptions> HSOpts;
    SourceManager& SM;
    FileManager& FileMgr;
    const std::string& configs;
};

}


C2Builder::C2Builder(const Recipe& recipe_, const BuildOptions& opts)
    : recipe(recipe_)
    , options(opts)
    , c2Mod(0)
    , mainComponent(0)
    , libLoader(components, modules, options.libdir, recipe.getExports())
    , useColors(true)
{
    TargetInfo::getNative(targetInfo);
    if (options.verbose) log(COL_VERBOSE, "Target: %s", Str(targetInfo));
    if (!isatty(1)) useColors = false;
}

C2Builder::~C2Builder()
{
    for (unsigned i=0; i<components.size(); i++) {
        delete components[i];
    }
    delete c2Mod;
}

int C2Builder::checkFiles() {
    int errors = 0;
    for (unsigned i=0; i<recipe.size(); i++) {
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
    // TODO refactor this function to 'Work'-framework
    log(ANSI_GREEN, "building target %s", recipe.name.c_str());

    uint64_t t1_build = Utils::getCurrentTime();
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
    Diags.setSeverity(diag::warn_missing_case, diag::Severity::Error, SourceLocation());
    Diags.setSeverity(diag::warn_unreachable_default, diag::Severity::Warning, SourceLocation());
    Diags.setSeverity(diag::warn_remainder_division_by_zero, diag::Severity::Error, SourceLocation());
    Diags.setSeverity(diag::warn_int_to_pointer_cast, diag::Severity::Error, SourceLocation());

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
    clang::TargetInfo *pti = clang::TargetInfo::CreateTargetInfo(Diags, to);
    IntrusiveRefCntPtr<clang::TargetInfo> Target(pti);

    std::shared_ptr<HeaderSearchOptions> HSOpts(new HeaderSearchOptions());
    // add current directory (=project root) to #include path
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

    // create main Component
    mainComponent = new Component(recipe.name, recipe.type, false, false, recipe.getExports());
    components.push_back(mainComponent);
    // NOTE: libc always SHARED_LIB for now
    if (!recipe.noLibC) {
        libLoader.addDep(mainComponent, "libc", Component::SHARED_LIB);
    }
    for (unsigned i=0; i<recipe.libraries.size(); i++) {
        libLoader.addDep(mainComponent, recipe.libraries[i].name, recipe.libraries[i].type);
    }

    ParseHelper helper(Diags, LangOpts, pti, HSOpts, SM, FileMgr, PredefineBuffer);
    // phase 1a: parse and local analyse
    uint64_t t1_parse = Utils::getCurrentTime();
    unsigned errors = 0;
    for (unsigned i=0; i<recipe.size(); i++) {
        const std::string& filename = recipe.get(i);

        if (options.verbose) log(COL_VERBOSE, "parsing (%s) %s", mainComponent->getName().c_str(), filename.c_str());
        bool ok = helper.parse(*mainComponent, 0, filename, options.printAST0);
        errors += !ok;
    }
    uint64_t t2_parse = Utils::getCurrentTime();
    if (options.printTiming) log(COL_TIME, "parsing took %" PRIu64" usec", t2_parse - t1_parse);
    if (client->getNumErrors()) goto out;

    uint64_t t1_analyse, t2_analyse;
    // phase 1b: load required external Components/Modules
    {
        uint64_t t1_parse_libs = Utils::getCurrentTime();
        if (!libLoader.scan()) goto out;
        if (options.showLibs) libLoader.showLibs(useColors);
        bool ok = checkImports(helper);
        uint64_t t2_parse_libs = Utils::getCurrentTime();
        if (options.printTiming) log(COL_TIME, "parsing libs took %" PRIu64" usec", t2_parse_libs - t1_parse_libs);
        if (!ok) goto out;
    }
    if (options.printModules) printComponents();
    if (options.printSymbols) printSymbols(options.printLibSymbols);

    // phase 2: analyse all files
    t1_analyse = Utils::getCurrentTime();
    for (unsigned c=0; c<components.size(); c++) {
        if (options.verbose) log(COL_VERBOSE, "analysing component %s", components[c]->getName().c_str());
        ComponentAnalyser analyser(*components[c], modules, Diags, context, options.verbose);
        errors += analyser.analyse(options.printAST1, options.printAST2, options.printAST3, options.printASTLib);
    }
    t2_analyse = Utils::getCurrentTime();
    if (options.printTiming) log(COL_TIME, "analysis took %" PRIu64" usec", t2_analyse - t1_analyse);
    if (client->getNumErrors()) goto out;

    if (!checkMainFunction(Diags)) goto out;

    if (!checkExportedPackages()) goto out;

    rewriterTest(SM, LangOpts);

    generateOptionalDeps();

    generateOptionalTags(SM);

    generateInterface();

    generateOptionalC();

    generateOptionalIR();

    if (options.verbose) log(COL_VERBOSE, "done");
out:
    //SM.PrintStats();
    uint64_t t2_build = Utils::getCurrentTime();
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

bool C2Builder::checkImports(ParseHelper& helper) {
    ImportsQueue queue;

    bool ok = true;
    const ModuleList& mainModules = mainComponent->getModules();
    for (unsigned i=0; i<mainModules.size(); i++) {
        ok &= checkModuleImports(helper, mainComponent, mainModules[i], queue);
    }
    if (!ok) return false;

    while (!queue.empty()) {
        std::string currentModuleName = queue.front();
        queue.pop_front();
        const LibInfo* lib = libLoader.findModuleLib(currentModuleName);
        assert(lib);

        ok &= checkModuleImports(helper, lib->component, lib->module, queue, lib);
    }
    return ok;
}

bool C2Builder::checkModuleImports(ParseHelper& helper, Component* component, Module* module, ImportsQueue& queue, const LibInfo* lib) {
    if (!module->isLoaded()) {
        assert(lib);
        if (options.verbose) log(COL_VERBOSE, "parsing (%s) %s", component->getName().c_str(), lib->c2file.c_str());
        if (!helper.parse(*component, module, lib->c2file, (options.printAST0 && options.printASTLib))) {
            return false;
        }
    }
    if (options.verbose) log(COL_VERBOSE, "checking imports for module (%s) %s", component->getName().c_str(), module->getName().c_str());
    bool ok = true;
    const AstList& files = module->getFiles();
    for (unsigned a=0; a<files.size(); a++) {
        AST* ast = files[a];
        for (unsigned u=1; u<ast->numImports(); u++) {  // NOTE: first import is module decl
            ImportDecl* D = ast->getImport(u);
            const std::string& targetModuleName = D->getModuleName();
            // handle c2 pseudo-module
            if (targetModuleName == "c2") {
                createC2Module();
                D->setModule(c2Mod);
                continue;
            }
            const LibInfo* target = libLoader.findModuleLib(targetModuleName);
            if (!target) {
                helper.Diags.Report(D->getLocation(), clang::diag::err_unknown_module) << targetModuleName;
                ok = false;
                continue;
            }
            D->setModule(target->module);
            if (target->component != component) {
                // check that imports are in directly dependent component (no indirect component)
                if (!component->hasDep(target->component)) {
                    helper.Diags.Report(D->getLocation(), clang::diag::err_indirect_component)
                            << component->getName() << target->component->getName() << targetModuleName;
                    ok = false;
                    continue;
                }
            }

            if (target->module->isLoaded()) continue;
            queue.push_back(targetModuleName);
        }
    }
    return ok;
}

void C2Builder::createC2Module() {
    if (!c2Mod) {
        if (options.verbose) log(COL_VERBOSE, "generating module c2");
        c2Mod = new Module("c2", true, false);
        modules["c2"] = c2Mod;
        C2ModuleLoader::load(c2Mod);
    }
}

C2::Module* C2Builder::findModule(const std::string& name) const {
    ModulesConstIter iter = modules.find(name);
    if (iter == modules.end()) return 0;
    else return iter->second;
}

void C2Builder::printSymbols(bool printLibs) const {
    assert(mainComponent);
    StringBuilder output;
    output.enableColor(true);
    if (printLibs) {
        if (c2Mod) {
            output << "Component <internal>\n";
            c2Mod->printSymbols(output);
        }
        for (unsigned i=0; i<components.size(); i++) {
            components[i]->printSymbols(output);
        }
    } else {
        mainComponent->printSymbols(output);
    }
    printf("%s\n", (const char*)output);
}

void C2Builder::printComponents() const {
    StringBuilder output;
    output.enableColor(true);
    if (c2Mod) {
        output << "Component <internal>\n";
        c2Mod->printFiles(output);
    }
    for (unsigned i=0; i<components.size(); i++) {
        components[i]->print(output);
    }
    printf("%s\n", (const char*)output);
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
    assert(mainComponent);

    Decl* mainDecl = 0;
    const ModuleList& mods = mainComponent->getModules();
    for (unsigned m=0; m<mods.size(); m++) {
        const Module* M = mods[m];
        Decl* decl = M->findSymbol("main");
        if (decl) {
            if (mainDecl) {
                // TODO multiple main functions
            } else {
                mainDecl = decl;
            }
        }
    }

    if (recipe.type == Component::EXECUTABLE) {
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
            return false;
        }
        if (M->isExternal()) {
            fprintf(stderr, "cannot export external module '%s'\n", modName.c_str());
            return false;
        }
    }
    return true;
}

void C2Builder::rewriterTest(SourceManager& SM, LangOptions& LangOpts) {
#if 0
    // FOR TESTING rename global test.aa -> bb
    const std::string modName = "test";
    const std::string oldName = "aa";
    const std::string newName = "bb";

    // Step 1a: find Module
    const Module* M = 0;
    const Module* mod = findModule(modName);
    assert(M && "unknown module");
    assert(!M->isExternal() && "cannot replace symbol in external module");

    // Step 1b: find Decl
    Decl* D = M->findSymbol(oldName);
    assert(D && "unknown decl");

    // Step 2a: replace Decl itself
    Rewriter rewriter;
    rewriter.setSourceMgr(SM, LangOpts);
    rewriter.ReplaceText(D->getLocation(), oldName.size(), newName);

    // Step 2b: replace all references
    // TODO only in mainComponent
    const ModuleList& mods = mainComponent->getModules();
    for (unsigned m=0; m<mods.size(); m++) {
        const AstList& files = mods[m]->getFiles();
        for (unsigned a=0; a<files.size(); a++) {
            AST* ast = files[a];

            RefFinder finder(*ast, D);
            unsigned count = finder.find();
            if (count) printf("replaced %d references in %s\n", count, files[i]->getFileName().c_str());
            for (unsigned i=0; i<count; i++) {
                std::string temp = finder.locs[i].printToString(SM);
                printf("loc %d -> %s\n", finder.locs[i].getRawEncoding(), temp.c_str());
                PresumedLoc loc = SM.getPresumedLoc(finder.locs[i]);
                assert(!loc.isInvalid() && "Invalid location");
                printf(" -> %s:%d:%d\n", loc.getFilename(), loc.getLine(), loc.getColumn());
                std::pair<FileID, unsigned> Off = SM.getDecomposedExpansionLoc(finder.locs[i]);
                printf("-> offset %d\n", Off.second);
                rewriter.ReplaceText(finder.locs[i], oldName.size(), newName);
            }
        }
    }

    // Step 3: reparse and check
    // TODO

    // print output
    for (unsigned m=0; m<mods.size(); m++) {
        const AstList& files = mods[m]->getFiles();
        for (unsigned a=0; a<files.size(); a++) {
            AST* ast = files[a];
            const RewriteBuffer *RewriteBuf =
                rewriter.getRewriteBufferFor(ast->fileID);
            if (RewriteBuf) {
                printf("====== %s ======\n", ast->getFileName().c_str());
                llvm::outs() << std::string(RewriteBuf->begin(), RewriteBuf->end());
            }
        }
    }
    // also works!
    //bool err = rewriter.overwriteChangedFiles();
    //printf("errors = %d\n", err);
#endif
}

void C2Builder::generateOptionalDeps() {
    if (!options.printDependencies && !recipe.generateDeps) return;

    if (options.verbose) log(COL_VERBOSE, "generating dependencies");

    uint64_t t1 = Utils::getCurrentTime();
    bool showFiles = false;
    bool showExternals = false;
    bool showPrivate = true;
    for (unsigned i=0; i<recipe.depConfigs.size(); i++) {
        const std::string& conf = recipe.depConfigs[i];
        if (conf == "show-files") showFiles = true;
        if (conf == "show-externals") showExternals = true;
    }

    std::string path = OUTPUT_DIR + recipe.name + '/';
    generateDeps(showFiles, showPrivate, showExternals, path);
    uint64_t t2 = Utils::getCurrentTime();
    if (options.printTiming) log(COL_TIME, "dep generation took %" PRIu64" usec", t2 - t1);
}

void C2Builder::generateDeps(bool showFiles, bool showPrivate, bool showExternals, const std::string& path) const {
    DepGenerator generator(showFiles, showPrivate, showExternals);
    generator.write(components, recipe.name, path);
}

void C2Builder::generateOptionalTags(const SourceManager& SM) const {
    if (!options.generateRefs && !recipe.generateRefs) return;

    if (options.verbose) log(COL_VERBOSE, "generating refs");

    uint64_t t1 = Utils::getCurrentTime();
    TagWriter generator(SM, components);
    std::string path = OUTPUT_DIR + recipe.name + '/';
    generator.write(recipe.name, path);
    uint64_t t2 = Utils::getCurrentTime();
    if (options.printTiming) log(COL_TIME, "refs generation took %" PRIu64" usec", t2 - t1);
}

void C2Builder::generateInterface() const {
    if (options.checkOnly) return;
    if (!options.generateC && !recipe.generateCCode &&
            !options.generateIR && !recipe.generateIR) return;
    if (!recipe.needsInterface()) return;

    if (options.verbose) log(COL_VERBOSE, "generating c2 interfaces");

    std::string outdir = OUTPUT_DIR + recipe.name + '/';
    const ModuleList& mods = mainComponent->getModules();
    for (unsigned m=0; m<mods.size(); m++) {
        const Module* M = mods[m];
        if (!M->isExported()) continue;
        InterfaceGenerator gen(*M);
        gen.write(outdir, options.printC);
    }

    ManifestWriter manifest(*mainComponent);
    manifest.write(outdir);
}

void C2Builder::generateOptionalC() {
    if (options.checkOnly) return;
    if (!options.generateC && !recipe.generateCCode) return;

    uint64_t t1 = Utils::getCurrentTime();
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

    CGenerator::Options cgen_options(OUTPUT_DIR, BUILD_DIR, options.libdir);
    cgen_options.single_module = single_module;
    cgen_options.printC = options.printC;
    CGenerator cgen(*mainComponent, modules, libLoader, cgen_options, targetInfo);

    // generate headers for external libraries
    if (options.verbose) log(COL_VERBOSE, "generating external headers");
    cgen.generateExternalHeaders();

    // generate C interface files
    if (recipe.needsInterface()) {
        if (options.verbose) log(COL_VERBOSE, "generating interface headers");
        cgen.generateInterfaceFiles();
    }

    // use C-backend
    if (options.verbose) log(COL_VERBOSE, "generating C code");
    cgen.generate();

    uint64_t t2 = Utils::getCurrentTime();
    if (options.printTiming) log(COL_TIME, "C code generation took %" PRIu64" usec", t2 - t1);

    if (!no_build) {
        if (options.verbose) log(COL_VERBOSE, "building C code");
        uint64_t t3 = Utils::getCurrentTime();
        cgen.build();
        uint64_t t4 = Utils::getCurrentTime();
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

    // TODO move all this to some generic Codegen class
    // Q: use single context or one-per-module?
    llvm::LLVMContext context;

    const ModuleList& mods = mainComponent->getModules();
    if (single_module) {
        uint64_t t1 = Utils::getCurrentTime();
        std::string filename = recipe.name;
        if (options.verbose) log(COL_VERBOSE, "generating IR for single module %s", filename.c_str());
        CodeGenModule cgm(filename, true, mods, context);
        cgm.generate();
        uint64_t t2 = Utils::getCurrentTime();
        if (options.printTiming) log(COL_TIME, "IR generation took %" PRIu64" usec", t2 - t1);
        if (options.printIR) cgm.dump();
        bool ok = cgm.verify();
        if (ok) cgm.write(outdir, filename);
    } else {
        for (unsigned m=0; m<mods.size(); m++) {
            Module* M = mods[m];
            uint64_t t1 = Utils::getCurrentTime();
            if (M->isPlainC()) continue;
            if (M->getName() == "c2") continue;

            if (options.verbose) log(COL_VERBOSE, "generating IR for module %s", M->getName().c_str());
            ModuleList single;
            single.push_back(M);
            CodeGenModule cgm(M->getName(), false, single, context);
            cgm.generate();
            uint64_t t2 = Utils::getCurrentTime();
            if (options.printTiming) log(COL_TIME, "IR generation took %" PRIu64" usec", t2 - t1);
            if (options.printIR) cgm.dump();
            bool ok = cgm.verify();
            if (ok) cgm.write(outdir, M->getName());
        }
    }
}

