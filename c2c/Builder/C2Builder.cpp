/* Copyright 2013-2019 Bas van den Berg
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
#include "Clang/ParseDiagnostic.h"
#include "Clang/DiagnosticOptions.h"
#include "Clang/FileManager.h"
#include "Clang/FileSystemOptions.h"
#include "Clang/MacroBuilder.h"
#include "Clang/SourceManager.h"
#include "Clang/TargetOptions.h"
#include "Clang/TextDiagnosticPrinter.h"
#include "Clang/Utils.h"
#include "Clang/HeaderSearch.h"
#include "Clang/HeaderSearchOptions.h"
#include "Clang/Preprocessor.h"
#include "Clang/SemaDiagnostic.h"

#include "Builder/C2Builder.h"
#include "Builder/Recipe.h"
#include "Builder/C2ModuleLoader.h"
#include "Builder/ManifestWriter.h"
#include "AST/AST.h"
#include "AST/Module.h"
#include "AST/Decl.h"
#include "Parser/Parser.h"
#include "Parser/ASTBuilder.h"
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
#include "Utils/BuildFile.h"

using c2lang::DiagnosticOptions;
using c2lang::DiagnosticsEngine;
using c2lang::FileEntry;
using c2lang::FileManager;
using c2lang::FileSystemOptions;
using c2lang::HeaderSearch;
using c2lang::HeaderSearchOptions;
using c2lang::Preprocessor;
using c2lang::SourceManager;
using c2lang::TargetInfo;
using c2lang::TargetOptions;
using c2lang::TextDiagnosticPrinter;

using namespace C2;
using namespace c2lang;

#define OUTPUT_DIR "output/"
#define BUILD_DIR  "build/"

namespace C2 {

class ParseHelper {
public:
    ParseHelper(DiagnosticsEngine &Diags_,
                std::shared_ptr<HeaderSearchOptions> HSOpts_,
                SourceManager &SM_,
                FileManager &FileMgr_,
                const std::string &configs_,
                const TargetInfo &ti)
        : Diags(Diags_)
        , HSOpts(HSOpts_)
        , SM(SM_)
        , FileMgr(FileMgr_)
        , configs(configs_)
        , targetInfo(ti)
    {}

    bool parse(Component& component, Module* existingMod, const std::string& filename, bool printAST) {
        // NOTE: seems to get deleted by Preprocessor,
        HeaderSearch* Headers = new HeaderSearch(SM, Diags);

        Preprocessor PP(Diags, SM, *Headers);
        std::string config = configs;
        if (component.isExternal()) config = "";

        ApplyHeaderSearchOptions(PP.getHeaderSearchInfo(), *HSOpts);
        PP.setPredefines(config);
        PP.Initialize(targetInfo);

        // File stuff
        const FileEntry *pFile = FileMgr.getFile(filename);
        if (pFile == 0) {
            fprintf(stderr, "Error opening file: '%s'\n", filename.c_str());
            exit(EXIT_FAILURE);
        }
        FileID id = SM.createFileID(pFile, SourceLocation(), SrcMgr::C_User);
        PP.EnterSourceFile(id, nullptr, SourceLocation());

        // TEMP rewriter test
        //ast.fileID = id;
        // Manually set predefines (normally done in EnterMainSourceFile())
        std::unique_ptr<llvm::MemoryBuffer> SB = llvm::MemoryBuffer::getMemBufferCopy(config, "<built-in>");
        assert(SB && "Cannot create predefined source buffer");
        FileID FID = SM.createFileID(std::move(SB));

        // NOTE: setPredefines() is normally private
        PP.setPredefinesFileID(FID);
        PP.EnterSourceFile(FID, nullptr, SourceLocation());

        Diags.getClient()->BeginSourceFile(0);

        ASTBuilder astBuilder(SM, Diags, PP, component, existingMod, filename, targetInfo);
        Parser parser(PP, astBuilder, component.isInterface());
        bool ok = parser.Parse();
        if (printAST) astBuilder.printAST();
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
    std::shared_ptr<HeaderSearchOptions> HSOpts;
    SourceManager& SM;
    FileManager& FileMgr;
    const std::string& configs;
    const TargetInfo& targetInfo;
};

}


C2Builder::C2Builder(const Recipe& recipe_, const BuildFile* buildFile_, const BuildOptions& opts)
    : recipe(recipe_)
    , buildFile(buildFile_)
    , options(opts)
    , c2Mod(0)
    , mainComponent(0)
    , libLoader(components, modules, recipe)
    , useColors(true)
{
    if (buildFile && !buildFile->outputDir.empty()) {
        outputDir = buildFile->outputDir + '/' + recipe.name + '/';
    } else {
        outputDir = OUTPUT_DIR + recipe.name + '/';
    }
    if (buildFile) {
        for (StringListConstIter iter = buildFile->libDirs.begin();
                iter != buildFile->libDirs.end(); ++iter) {
            libLoader.addDir(*iter);
        }
        if (buildFile->target.empty()) {
            TargetInfo::getNative(targetInfo);
        } else {
            if (!TargetInfo::fromString(targetInfo, buildFile->target)) {
                fprintf(stderr, "c2c: error: invalid target string '%s'\n",
                    buildFile->target.c_str());
                // TODO handle (need to extract outside constructor?)
            }
        }
    } else {
        if (opts.libdir) libLoader.addDir(opts.libdir);
        TargetInfo::getNative(targetInfo);
    }
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

int C2Builder::build() {
    log(ANSI_GREEN, "building target %s", recipe.name.c_str());

    uint64_t t1_build = Utils::getCurrentTime();

    // Step 1 - create all components, read manifests, create external modules
    if (!libLoader.createComponents()) return 1;
    mainComponent = libLoader.getMainComponent();

    if (options.showLibs) {
        libLoader.showLibs(useColors, false);
        return 0;
    }

    // Step 2 - parse sources
    // Diagnostics
    // NOTE: DiagOpts is somehow deleted by Diags/TextDiagnosticPrinter below?
    DiagnosticOptions* DiagOpts = new DiagnosticOptions();
    if (!options.testMode && isatty(2)) DiagOpts->ShowColors = true;
    IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
    DiagnosticsEngine Diags(DiagID, DiagOpts,
                            // NOTE: setting ShouldOwnClient to true causes crash??
                            new TextDiagnosticPrinter(llvm::errs(), DiagOpts), false);
    DiagnosticConsumer* client = Diags.getClient();
    configDiagnostics(Diags, recipe.silentWarnings);

    // TargetInfo
    std::shared_ptr<TargetOptions> to(new TargetOptions());
    to->Triple = llvm::sys::getDefaultTargetTriple();

    std::shared_ptr<HeaderSearchOptions> HSOpts(new HeaderSearchOptions());
    // add current directory (=project root) to #include path
    char pwd[512];
    if (getcwd(pwd, 512) == 0) {
        FATAL_ERROR("Failed to get current directory");
    }
    HSOpts->AddPath(pwd, c2lang::frontend::Quoted);

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

    ParseHelper helper(Diags, HSOpts, SM, FileMgr, PredefineBuffer, targetInfo);

    // Step 2A - parse main component sources + analyse imports
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
    if (client->getNumErrors()) return report(client, t1_build);

    // Step 2B - analyse imports of all internal modules, parse (+ analyse imports of ) used external modules
    uint64_t t1_parse_libs = Utils::getCurrentTime();
    bool ok = checkImports(helper);
    if (!ok) return report(client, t1_build);
    uint64_t t2_parse_libs = Utils::getCurrentTime();
    if (options.printTiming) log(COL_TIME, "parsing libs took %" PRIu64" usec", t2_parse_libs - t1_parse_libs);

    if (options.printModules) {
        printComponents(options.printLibModules);
        return report(client, t1_build);
    }

    // Step 3 - analyze everything
    uint64_t t1_analyse = Utils::getCurrentTime();
    // components should be ordered bottom-up
    for (unsigned c=0; c<components.size(); c++) {
        if (options.verbose) log(COL_VERBOSE, "analysing component %s", components[c]->getName().c_str());
        ComponentAnalyser analyser(*components[c], modules, Diags, targetInfo, context, options.verbose, options.testMode);
        errors += analyser.analyse(options.printAST1, options.printAST2, options.printAST3, options.printASTLib);
    }
    uint64_t t2_analyse = Utils::getCurrentTime();
    if (options.printTiming) log(COL_TIME, "analysis took %" PRIu64" usec", t2_analyse - t1_analyse);

    if (options.printSymbols) printSymbols(options.printLibSymbols, options.printNonPublic);

    if (client->getNumErrors()) return report(client, t1_build);

    if (!checkExportedPackages()) return report(client, t1_build);

    // Step 4 - generation
    generateOptionalDeps();

    generateOptionalTags(SM);

    generateInterface();

    generateOptionalC();

    generateOptionalIR();

    if (options.verbose) log(COL_VERBOSE, "done");

    return report(client, t1_build);
}

int C2Builder::report(DiagnosticConsumer* client, uint64_t t1_build) {
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

void C2Builder::configDiagnostics(DiagnosticsEngine &Diags, const StringList& silentWarnings) {
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
    Diags.setSeverity(diag::warn_shift_lhs_negative, diag::Severity::Error, SourceLocation());
    Diags.setSeverity(diag::warn_shift_negative, diag::Severity::Error, SourceLocation());
    Diags.setSeverity(diag::warn_shift_gt_typewidth, diag::Severity::Error, SourceLocation());
    Diags.setSeverity(diag::ext_typecheck_convert_discards_qualifiers, diag::Severity::Error, SourceLocation());

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
        if (conf == "no-unused-import") {
            Diags.setSeverityForGroup(diag::Flavor::WarningOrError, "unused-import", diag::Severity::Ignored);
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
        exit(EXIT_FAILURE);
    }
}

bool C2Builder::checkImports(ParseHelper& helper) {

    ImportsQueue queue;

    // add main modules to libLoader, queue for import analysis
    const ModuleList& mainModules = mainComponent->getModules();
    for (unsigned i=0; i<mainModules.size(); i++) {
        const LibInfo* lib = libLoader.addModule(mainComponent, mainModules[i], "", "");
        queue.push_back(lib);
    }

    bool ok = true;
    while (!queue.empty()) {
        const LibInfo* lib = queue.front();
        queue.pop_front();
        ok &= checkModuleImports(helper, queue, lib);
    }
    return ok;
}

bool C2Builder::checkModuleImports(ParseHelper& helper, ImportsQueue& queue, const LibInfo* lib) {
    Component* component = lib->component;
    Module* module = lib->module;
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
                helper.Diags.Report(D->getLocation(), c2lang::diag::err_unknown_module) << targetModuleName;
                ok = false;
                continue;
            }
            D->setModule(target->module);
            if (target->component != component) {
                // check that imports are in directly dependent component (no indirect component)
                if (!component->hasDep(target->component)) {
                    helper.Diags.Report(D->getLocation(), c2lang::diag::err_indirect_component)
                            << component->getName() << target->component->getName() << targetModuleName;
                    ok = false;
                    continue;
                }
            }

            if (!target->module->isUsed()) {
                target->module->setUsed();
                queue.push_back(target);
            }
        }
    }
    return ok;
}

void C2Builder::createC2Module() {
    if (!c2Mod) {
        if (options.verbose) log(COL_VERBOSE, "generating module c2");
        c2Mod = new Module("c2", true, true, false);
        modules["c2"] = c2Mod;
        C2ModuleLoader::load(c2Mod, targetInfo.intWidth == 32);
    }
}

C2::Module* C2Builder::findModule(const std::string& name) const {
    ModulesConstIter iter = modules.find(name);
    if (iter == modules.end()) return 0;
    else return iter->second;
}

void C2Builder::printSymbols(bool printLibs, bool printNonPublic) const {
    assert(mainComponent);
    StringBuilder output;
    output.enableColor(true);
    if (printLibs) {
        if (c2Mod) {
            output << "Component <internal>\n";
            c2Mod->printSymbols(output, false);
        }
        for (unsigned i=0; i<components.size(); i++) {
            components[i]->printSymbols(output, false);
        }
    } else {
        mainComponent->printSymbols(output, printNonPublic);
    }
    printf("%s\n", (const char*)output);
}

void C2Builder::printComponents(bool printLibs) const {
    StringBuilder output;
    output.enableColor(true);
    if (c2Mod && printLibs) {
        output << "Component <internal>\n";
        c2Mod->printFiles(output);
    }
    for (unsigned i=0; i<components.size(); i++) {
        const Component* C = components[i];
        if (printLibs || !C->isExternal()) C->print(output);
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

    generateDeps(showFiles, showPrivate, showExternals, outputDir);
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
    generator.write(recipe.name, outputDir);
    uint64_t t2 = Utils::getCurrentTime();
    if (options.printTiming) log(COL_TIME, "refs generation took %" PRIu64" usec", t2 - t1);
}

void C2Builder::generateInterface() const {
    if (options.checkOnly) return;
    if (!options.generateC && !recipe.generateCCode &&
            !options.generateIR && !recipe.generateIR) return;
    if (!recipe.needsInterface()) return;

    if (options.verbose) log(COL_VERBOSE, "generating c2 interfaces");

    const ModuleList& mods = mainComponent->getModules();
    for (unsigned m=0; m<mods.size(); m++) {
        const Module* M = mods[m];
        if (!M->isExported()) continue;
        InterfaceGenerator gen(*M);
        gen.write(outputDir, options.printC);
    }

    ManifestWriter manifest(*mainComponent);
    manifest.write(outputDir);
}

void C2Builder::generateOptionalC() {
    if (options.checkOnly) return;
    if (!options.generateC && !recipe.generateCCode) return;

    uint64_t t1 = Utils::getCurrentTime();
    bool single_module = false;
    bool no_build = false;
    bool genChecks = false;
    for (unsigned i=0; i<recipe.cConfigs.size(); i++) {
        const std::string& conf = recipe.cConfigs[i];
        // TODO just pass struct with bools?
        if (conf == "single-module") single_module = true;
        else if (conf == "no-build") no_build = true;
        else if (conf == "check") genChecks = true;
        else {
            fprintf(stderr, ANSI_RED"invalid c-generation argument '%s'" ANSI_NORMAL"\n", conf.c_str());
        }
    }

    CGenerator::Options cgen_options(outputDir, BUILD_DIR);
    cgen_options.single_module = single_module;
    cgen_options.printC = options.printC;
    cgen_options.generateChecks = genChecks;
    CGenerator cgen(*mainComponent, modules, libLoader, cgen_options, targetInfo, buildFile);
    for (unsigned i=0; i<components.size(); i++) {
        Component* c = components[i];
        if (c->getType() == Component::SOURCE_LIB) cgen.addSourceLib(*c);
    }

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

    std::string buildDir = outputDir + BUILD_DIR;

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
        if (ok) cgm.write(buildDir, filename);
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
            if (ok) cgm.write(buildDir, M->getName());
        }
    }
}

