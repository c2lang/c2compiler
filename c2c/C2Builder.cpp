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

#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/FileSystemOptions.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/HeaderSearchOptions.h"
#include "clang/Lex/ModuleLoader.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Host.h"

#include "C2Builder.h"
#include "C2Parser.h"
#include "C2Sema.h"
#include "CodeGenerator.h"
#include "color.h"
#include "FileAnalyser.h"
#include "Recipe.h"
#include "Utils.h"

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

#if 1
class FileInfo {
public:
    FileInfo(DiagnosticsEngine& Diags_, LangOptions& LangOpts_, TargetInfo* pti, const std::string& filename_)
        : filename(filename_)
        , Diags(Diags_)
        , FileMgr(FileSystemOpts)
        , SM(Diags, FileMgr)
        , HSOpts(new HeaderSearchOptions())
        , Headers(HSOpts, FileMgr, Diags, LangOpts_, pti)
        , PPOpts(new PreprocessorOptions())
        , PP(PPOpts, Diags, LangOpts_, pti, SM, Headers, loader)
        , sema(SM)
        , parser(PP, sema)
    {
        // File stuff
        const FileEntry *pFile = FileMgr.getFile(filename);
        SM.createMainFileID(pFile);
        PP.EnterMainSourceFile();
        Diags.getClient()->BeginSourceFile(LangOpts_, &PP);

        parser.Initialize();
    }

    void parse(BuildOptions& options) {
        printf("------ parsing %s ------\n", filename.c_str());
        u_int64_t totalStart = Utils::getCurrentTime();
        // parse the file into AST
        u_int64_t tt = Utils::getCurrentTime();
        if (options.printTiming) printf(COL_TIME"setup took %lld usec"ANSI_NORMAL"\n", tt - totalStart);
        {
            u_int64_t t1 = Utils::getCurrentTime();
            bool ok = parser.Parse();
            u_int64_t t2 = Utils::getCurrentTime();
            if (options.printTiming) printf(COL_TIME"parsing took %lld usec"ANSI_NORMAL"\n", t2 - t1);
            if (!ok) return;
        }
        if (options.printAST) sema.printAST();

        // initial per-file analysis
        FileAnalyser analyser(Diags);
        sema.visitAST(analyser);
        if (analyser.hasErrors()) return;

        switch (options.mode) {
        case BuildOptions::NORMAL:
            {
                printf("------ generating code for %s ------\n", filename.c_str());
                CodeGenerator codegen(sema);
                u_int64_t t1 = Utils::getCurrentTime();
                codegen.generate();
                u_int64_t t2 = Utils::getCurrentTime();
                codegen.dump();
                if (options.printTiming) printf(COL_TIME"generation took %lld usec"ANSI_NORMAL"\n", t2 - t1);
            }
            break;
        case BuildOptions::GENERATE_C:
            sema.generateC();
            break;
        case BuildOptions::SYNTAX_ONLY:
            break;
        }
        u_int64_t totalEnd = Utils::getCurrentTime();
        if (options.printTiming) printf(COL_TIME"total took %lld usec"ANSI_NORMAL"\n", totalEnd - totalStart);
    }
private:
    std::string filename;

    // Diagnostics
    DiagnosticOptions DiagOpts;
    IntrusiveRefCntPtr<DiagnosticIDs> DiagID;
    DiagnosticsEngine Diags;

    // FileManager
    FileSystemOptions FileSystemOpts;
    FileManager FileMgr;

    // SourceManager
    SourceManager SM;

    TargetOptions* to;

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
};
#endif

}


C2Builder::C2Builder(const Recipe& recipe_, const BuildOptions& opts)
    : recipe(recipe_)
    , options(opts)
{}

C2Builder::~C2Builder()
{}

void C2Builder::build() {
    printf("------ Building target %s ------\n", recipe.name.c_str());

    // LangOptions
    LangOptions LangOpts;
    LangOpts.C2 = 1;
    LangOpts.Bool = 1;

    for (int i=0; i<recipe.size(); i++) {
        const char* filename = recipe.get(i).c_str();

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
        
        FileInfo info(Diags, LangOpts, pti, filename);
        info.parse(options);
    }
}

void C2Builder::buildFile(const char* filename) {
}

