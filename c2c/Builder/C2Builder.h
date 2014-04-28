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

#ifndef BUILDER_C2BUILDER_H
#define BUILDER_C2BUILDER_H

#include <vector>
#include <string>

#include "AST/Package.h"
#include "FileUtils/FileDb.h"

namespace clang {
class DiagnosticsEngine;
}

namespace C2 {

class Recipe;
class FileInfo;

struct BuildOptions {
    BuildOptions()
        : printAST0(false)
        , printAST1(false)
        , printAST2(false)
        , printAST3(false)
        , printTiming(false)
        , printSymbols(false)
        , generateIR(false)
        , printIR(false)
        , generateC(false)
        , printC(false)
        , printPackages(false)
        , printDependencies(false)
        , verbose(false)
        , testMode(false)
    {}
    bool printAST0;
    bool printAST1;
    bool printAST2;
    bool printAST3;
    bool printTiming;
    bool printSymbols;
    bool generateIR;
    bool printIR;
    bool generateC;
    bool printC;
    bool printPackages;
    bool printDependencies;
    bool verbose;
    bool testMode;
};


class C2Builder {
public:
    C2Builder(const Recipe& recipe_, const BuildOptions& opts);
    ~C2Builder();

    int checkFiles();
    int build();
private:
    bool havePackage(const std::string& name) const;
    Package* getPackage(const std::string& name, bool isExternal, bool isCLib);
    bool createPkgs();
    bool loadExternalPackages();
    bool loadPackage(const std::string& name);
    void printDependencies() const;
    unsigned analyse();
    void dumpPkgs() const;
    void printASTs() const;

    bool checkMainFunction(clang::DiagnosticsEngine& Diags);
    void generateOptionalC();
    void generateOptionalIR();

    const Recipe& recipe;
    BuildOptions options;

    typedef std::vector<FileInfo*> Files;
    Files files;
    FileDb filenames;

    Pkgs pkgs;

    C2Builder(const C2Builder&);
    C2Builder& operator= (const C2Builder&);
};

}

#endif

