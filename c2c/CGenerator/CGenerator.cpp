/* Copyright 2013-2015 Bas van den Berg
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

#include "CGenerator/CGenerator.h"
#include "CGenerator/CCodeGenerator.h"
#include "CGenerator/MakefileGenerator.h"
#include "AST/AST.h"
#include "AST/Decl.h"
#include "FileUtils/FileUtils.h"
#include "Utils/StringBuilder.h"
#include "Utils/ProcessUtils.h"
#include "Utils/color.h"

using namespace C2;


CGenerator::CGenerator(const std::string& name_, GenUtils::TargetType type_, const Modules& modules_, HeaderNamer& namer_, const Options& options_)
    : targetName(name_)
    , targetType(type_)
    , modules(modules_)
    , includeNamer(namer_)
    , options(options_)
{}

void CGenerator::generate() {
    std::string outdir = options.outputDir + targetName + options.buildDir;

    MakefileGenerator makeGen(outdir, targetName, targetType);
    if (options.single_module) {
        makeGen.add(targetName);
        CCodeGenerator gen(targetName, CCodeGenerator::SINGLE_FILE, modules, includeNamer);
        for (EntriesIter iter = entries.begin(); iter != entries.end(); ++iter) {
            gen.addEntry(*(*iter));
        }
        gen.generate(options.printC);
        gen.write(outdir);
    } else {
        for (ModulesConstIter iter = modules.begin(); iter != modules.end(); ++iter) {
            const Module* M = iter->second;
            if (M->isExternal()) continue;
            makeGen.add(M->getName());
            CCodeGenerator gen(M->getName(), CCodeGenerator::MULTI_FILE, modules, includeNamer);
            for (EntriesIter iter = entries.begin(); iter != entries.end(); ++iter) {
                AST* ast = *iter;
                if (ast->getModuleName() == M->getName()) {
                    gen.addEntry(*ast);
                }
            }
            gen.generate(options.printC);
            gen.write(outdir);
        }
    }
    makeGen.write();

    // generate external library header
    bool generateHeader = false;
    switch (targetType) {
    case GenUtils::EXECUTABLE:
        break;
    case GenUtils::SHARED_LIB:
    case GenUtils::STATIC_LIB:
        generateHeader = true;
        break;
    }
    if (generateHeader) {
        CCodeGenerator gen(targetName, CCodeGenerator::MULTI_FILE, modules, includeNamer);
        for (ModulesConstIter iter = modules.begin(); iter != modules.end(); ++iter) {
            const Module* M = iter->second;
            // TODO dont look at Module, but at exported Decls
            if (!M->isExported()) continue;
            for (EntriesIter iter = entries.begin(); iter != entries.end(); ++iter) {
                AST* ast = *iter;
                if (ast->getModuleName() == M->getName()) {
                    gen.addEntry(*ast);
                }
            }
        }
        gen.createLibHeader(options.printC, options.outputDir + targetName + "/");
    }

    // generate exports.version
    if (targetType == GenUtils::SHARED_LIB) {
        StringBuilder expmap;
        expmap << "LIB_1.0 {\n";
        expmap << "\tglobal:\n";
        for (ModulesConstIter iter = modules.begin(); iter != modules.end(); ++iter) {
            const Module* M = iter->second;
            if (M->isExternal()) continue;
            const std::string& modname = M->getName();
            const Module::Symbols& syms = M->getSymbols();
            for (Module::SymbolsConstIter iter = syms.begin(); iter != syms.end(); ++iter) {
                const Decl* D = iter->second;
                if (!D->isExported()) continue;
                if (!isa<FunctionDecl>(D) && !isa<VarDecl>(D)) continue;
                expmap << "\t\t";
                GenUtils::addName(modname, iter->first, expmap);
                expmap << ";\n";
            }
        }
        expmap << "\tlocal:\n\t\t*;\n";
        expmap << "};\n";
        std::string outfile = outdir + "exports.version";
        FileUtils::writeFile(outdir.c_str(), outfile.c_str(), expmap);
    }

}

void CGenerator::build() {
    std::string outdir = options.outputDir + targetName + options.buildDir;
    // execute generated makefile
    int retval = ProcessUtils::run(outdir, "/usr/bin/make");
    if (retval != 0) {
        fprintf(stderr, ANSI_RED"error during external c compilation" ANSI_NORMAL"\n");
    }
}

