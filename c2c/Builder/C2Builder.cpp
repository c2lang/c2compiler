/* Copyright 2013-2023 Bas van den Berg
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

#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
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
#include "Analyser/AnalyserUtils.h"
#include "IRGenerator/IRGenerator.h"
#include "IRGenerator/InterfaceGenerator.h"
#include "CGenerator/CGenerator.h"
#include "Refactor/RefFinder.h"
#include "FileUtils/FileUtils.h"
#include "Utils/color.h"
#include "Utils/Utils.h"
#include "Utils/Log.h"
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


C2Builder::C2Builder(Recipe& recipe_, const BuildFile* buildFile_, const BuildOptions& opts, PluginHandler* pluginHandler_)
    : recipe(recipe_)
    , buildFile(buildFile_)
    , options(opts)
    , pluginHandler(pluginHandler_)
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
    if (options.verbose) Log::log(COL_VERBOSE, "Target: %s", Str(targetInfo));

    AnalyserUtils::init(targetInfo);

    if (pluginHandler) pluginHandler->beginTarget(*this);
}

C2Builder::~C2Builder()
{
    for (unsigned i=0; i<components.size(); i++) {
        delete components[i];
    }
    delete c2Mod;
}

int C2Builder::build() {
    Log::log(ANSI_GREEN, "building target %s", recipe.name.c_str());

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
    configDiagnostics(Diags);

    // TargetInfo
    std::shared_ptr<TargetOptions> to(new TargetOptions());
    //to->Triple = llvm::sys::getDefaultTargetTriple();

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
        for (ConfigsIter iter = recipe.configs.begin(); iter != recipe.configs.end(); ++iter) {
            mbuilder.defineMacro(iter->first, iter->second);
        }
    }

    // FileManager
    FileSystemOptions FileSystemOpts;
    FileManager FileMgr(FileSystemOpts);
    SourceManager SM(Diags, FileMgr);

    ParseHelper helper(Diags, HSOpts, SM, FileMgr, PredefineBuffer, targetInfo);

    // create output dir here (plugins could generate files)
    // TODO

    if (pluginHandler) pluginHandler->build(*this);

    // Step 2A - parse main component sources + analyse imports
    uint64_t t1_parse = Utils::getCurrentTime();
    unsigned errors = 0;
    for (unsigned i=0; i<recipe.size(); i++) {
        const std::string& filename = recipe.get(i);

        if (options.verbose) Log::log(COL_VERBOSE, "parsing (%s) %s", mainComponent->getName().c_str(), filename.c_str());
        bool ok = helper.parse(*mainComponent, 0, filename, options.printAST0);
        errors += !ok;
    }
    uint64_t t2_parse = Utils::getCurrentTime();
    if (options.printTiming) Log::log(COL_TIME, "parsing took %" PRIu64" usec", t2_parse - t1_parse);
    if (client->getNumErrors()) return report(client, t1_build);

    // Step 2B - analyse imports of all internal modules, parse (+ analyse imports of ) used external modules
    uint64_t t1_parse_libs = Utils::getCurrentTime();
    bool ok = checkImports(helper);
    if (!ok) return report(client, t1_build);
    uint64_t t2_parse_libs = Utils::getCurrentTime();
    if (options.printTiming) Log::log(COL_TIME, "parsing libs took %" PRIu64" usec", t2_parse_libs - t1_parse_libs);

    if (options.printModules) {
        printComponents(options.printLibModules);
        return report(client, t1_build);
    }

    // Step 3 - analyze everything
    uint64_t t1_analyse = Utils::getCurrentTime();
    // components should be ordered bottom-up
    for (unsigned c=0; c<components.size(); c++) {
        Component* C = components[c];
        if (C->getType() == Component::INTERNAL) continue;
        if (options.verbose) Log::log(COL_VERBOSE, "analysing component %s", C->getName().c_str());
        ComponentAnalyser analyser(*C, modules, Diags, targetInfo, context, options.verbose, options.testMode);
        errors += analyser.analyse(options.printAST1, options.printAST2, options.printAST3, options.printASTLib);
    }
    uint64_t t2_analyse = Utils::getCurrentTime();
    if (options.printTiming) Log::log(COL_TIME, "analysis took %" PRIu64" usec", t2_analyse - t1_analyse);

    if (options.printSymbols) printSymbols(options.printLibSymbols, options.printNonPublic);

    if (client->getNumErrors()) return report(client, t1_build);

    if (!checkExportedPackages()) return report(client, t1_build);

    // Step 4 - generation
    if (pluginHandler) pluginHandler->generate(*this, SM);

    generateInterface();

    generateOptionalC();

    generateOptionalIR();

    if (recipe.writeAST) writeAST();

    if (options.verbose) Log::log(COL_VERBOSE, "done");

    return report(client, t1_build);
}

int C2Builder::report(DiagnosticConsumer* client, uint64_t t1_build) {
    //SM.PrintStats();
    uint64_t t2_build = Utils::getCurrentTime();
    if (options.printTiming) Log::log(COL_TIME, "total build took %" PRIu64" usec", t2_build - t1_build);
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

void C2Builder::configDiagnostics(DiagnosticsEngine &Diags) {
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
    Diags.setSeverity(diag::warn_unreachable_default, diag::Severity::Error, SourceLocation());
    Diags.setSeverity(diag::warn_remainder_division_by_zero, diag::Severity::Error, SourceLocation());
    Diags.setSeverity(diag::warn_int_to_pointer_cast, diag::Severity::Error, SourceLocation());
    Diags.setSeverity(diag::warn_shift_lhs_negative, diag::Severity::Error, SourceLocation());
    Diags.setSeverity(diag::warn_shift_negative, diag::Severity::Error, SourceLocation());
    Diags.setSeverity(diag::warn_shift_gt_typewidth, diag::Severity::Error, SourceLocation());
    Diags.setSeverity(diag::ext_typecheck_convert_discards_qualifiers, diag::Severity::Error, SourceLocation());

    // set recipe warning options
    if (recipe.WarningFlags.no_unused) {
        Diags.setSeverityForGroup(diag::Flavor::WarningOrError, "unused", diag::Severity::Ignored);
        Diags.setSeverityForGroup(diag::Flavor::WarningOrError, "unused-parameter", diag::Severity::Ignored);
    }
    if (recipe.WarningFlags.no_unused_variable) {
        Diags.setSeverityForGroup(diag::Flavor::WarningOrError, "unused-variable", diag::Severity::Ignored);
    }
    if (recipe.WarningFlags.no_unused_function) {
        Diags.setSeverityForGroup(diag::Flavor::WarningOrError, "unused-function", diag::Severity::Ignored);
    }
    if (recipe.WarningFlags.no_unused_parameter) {
        Diags.setSeverityForGroup(diag::Flavor::WarningOrError, "unused-parameter", diag::Severity::Ignored);
    }
    if (recipe.WarningFlags.no_unused_type) {
        Diags.setSeverityForGroup(diag::Flavor::WarningOrError, "unused-type", diag::Severity::Ignored);
    }
    if (recipe.WarningFlags.no_unused_module) {
        Diags.setSeverityForGroup(diag::Flavor::WarningOrError, "unused-module", diag::Severity::Ignored);
    }
    if (recipe.WarningFlags.no_unused_import) {
        Diags.setSeverityForGroup(diag::Flavor::WarningOrError, "unused-import", diag::Severity::Ignored);
    }
    if (recipe.WarningFlags.no_unused_public) {
        Diags.setSeverityForGroup(diag::Flavor::WarningOrError, "unused-public", diag::Severity::Ignored);
    }
    if (recipe.WarningFlags.no_unused_label) {
        Diags.setSeverityForGroup(diag::Flavor::WarningOrError, "unused-label", diag::Severity::Ignored);
    }
}

bool C2Builder::checkImports(ParseHelper& helper) {

    ImportsQueue queue;

    // add main modules to libLoader, queue for import analysis
    bool ok = true;
    const ModuleList& mainModules = mainComponent->getModules();
    for (unsigned i=0; i<mainModules.size(); i++) {
        Module* m = mainModules[i];
        ok &= libLoader.checkMainModule(m);
        const LibInfo* lib = libLoader.addModule(mainComponent, m, "", "");
        queue.push_back(lib);
    }

    if (!ok) return false;

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
        if (options.verbose) Log::log(COL_VERBOSE, "parsing (%s) %s", component->getName().c_str(), lib->c2file.c_str());
        if (!helper.parse(*component, module, lib->c2file, (options.printAST0 && options.printASTLib))) {
            return false;
        }
    }
    if (options.verbose) Log::log(COL_VERBOSE, "checking imports for module (%s) %s", component->getName().c_str(), module->getName().c_str());
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
                if (!component->hasDep(target->component) && !target->component->isInternalOrPlugin()) {
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
        // TODO add to internal component
        if (options.verbose) Log::log(COL_VERBOSE, "generating module c2");
        c2Mod = new Module("c2", true, true, false, true);
        modules["c2"] = c2Mod;
        C2ModuleLoader::load(c2Mod, targetInfo.intWidth == 32);
    }
}

const std::string& C2Builder::getRecipeName() const {
    return recipe.name;
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
        c2Mod->printFiles(output, true);
    }
    for (unsigned i=0; i<components.size(); i++) {
        const Component* C = components[i];
        if (printLibs || !C->isExternal()) C->print(output);
    }
    printf("%s\n", (const char*)output);
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

void C2Builder::generateInterface() const {
    if (options.checkOnly) return;
    if (!options.generateC && !recipe.generateCCode &&
            !options.generateIR && !recipe.generateIR) return;
    if (!recipe.needsInterface()) return;

    if (options.verbose) Log::log(COL_VERBOSE, "generating c2 interfaces");

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

    CGenerator::Options cgen_options(outputDir, BUILD_DIR);
    cgen_options.single_module = recipe.CGenFlags.single_module;
    cgen_options.printC = options.printC;
    cgen_options.generateChecks = recipe.CGenFlags.gen_checks;
    cgen_options.generateAsserts = recipe.enableAsserts;
    CGenerator cgen(*mainComponent, modules, libLoader, cgen_options, targetInfo, buildFile);
    for (unsigned i=0; i<components.size(); i++) {
        Component* c = components[i];
        if (c->getType() == Component::MAIN_SOURCE_LIB) cgen.addSourceLib(*c);
        if (c->getType() == Component::PLUGIN) cgen.addSourceLib(*c);
    }

    // generate headers for external libraries
    if (options.verbose) Log::log(COL_VERBOSE, "generating external headers");
    cgen.generateExternalHeaders();

    // generate C interface files
    if (recipe.needsInterface()) {
        if (options.verbose) Log::log(COL_VERBOSE, "generating interface headers");
        cgen.generateInterfaceFiles();
    }

    // use C-backend
    if (options.verbose) Log::log(COL_VERBOSE, "generating C code");
    cgen.generate();

    uint64_t t2 = Utils::getCurrentTime();
    if (options.printTiming) Log::log(COL_TIME, "C code generation took %" PRIu64" usec", t2 - t1);

    if (!recipe.CGenFlags.no_build) {
        if (options.verbose) Log::log(COL_VERBOSE, "building C code");
        uint64_t t3 = Utils::getCurrentTime();
        cgen.build();
        uint64_t t4 = Utils::getCurrentTime();
        if (options.printTiming) Log::log(COL_TIME, "C code compilation took %" PRIu64" usec", t4 - t3);
    }
}

void C2Builder::generateOptionalIR() {
    if (options.checkOnly) return;
    if (!options.generateIR && !recipe.generateIR) return;

    // TODO also add source libraries!
    std::string buildDir = outputDir + BUILD_DIR;
    IRGenerator ir(recipe.name,
                   buildDir,
                   mainComponent->getModules(),
                   recipe.IrGenFlags.single_module,
                   recipe.IrGenFlags.single_threaded,
                   recipe.IrGenFlags.keep_intermediates,
                   options.verbose,
                   options.printTiming,
                   options.printIR,
                   useColors);
    ir.build();
}

void C2Builder::writeAST() const {
    const ModuleList& mainModules = mainComponent->getModules();
    for (unsigned i=0; i<mainModules.size(); i++) {
        const AstList& files = mainModules[i]->getFiles();
        for (unsigned f=0; f<files.size(); f++) {
            const AST* ast = files[f];
            StringBuilder out;
            ast->print(out);
            FileUtils::writeFile(outputDir.c_str(), outputDir + "out.ast", out);
        }
    }
}

// TODO should know current plugin that calls this code
C2::Module* C2Builder::addPluginModule(const std::string& name) {
    Module* old = findModule(name);
    if (old) {
        fprintf(stderr, "module %s already exists\n", name.c_str());
        return NULL;
    }


    Component* pluginComponent = libLoader.getPluginComponent();
    if (!pluginComponent) {
        fprintf(stderr, "NO PLUGIN COMPONENT\n");
        exit(EXIT_FAILURE);
    }

    Module* m = pluginComponent->getModule(name);
    if (!m) {
        fprintf(stderr, "NO PLUGIN MODULE\n");
        exit(EXIT_FAILURE);
    }
    modules[name] = m;
    libLoader.addModule(pluginComponent, m, "", "");
    return m;
}

