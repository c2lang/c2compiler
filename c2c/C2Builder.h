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

#ifndef C2BUILDER_H
#define C2BUILDER_H

#include <vector>
#include <string>

#include "Package.h"

namespace C2 {

class Recipe;
class FileInfo;

struct BuildOptions {
    BuildOptions()
        : printAST(false)
        , printASTAfter(false)
        , printTiming(false)
        , printSymbols(false)
        , generateIR(false)
        , single_module(false)
        , printIR(false)
        , generateC(false)
        , printC(false)
        , printPackages(false)
        , verbose(false)
    {}
    bool printAST;
    bool printASTAfter;
    bool printTiming;
    bool printSymbols;
    bool generateIR;
    bool single_module;
    bool printIR;
    bool generateC;
    bool printC;
    bool printPackages;
    bool verbose;
};


class C2Builder {
public:
    C2Builder(const Recipe& recipe_, const BuildOptions& opts);
    ~C2Builder();

    int checkFiles();
    void build();
private:
    Package* getPackage(const std::string& name, bool isCLib);
    bool createPkgs();
    void addDummyPackages();
    void dumpPkgs();

    void generateOptionalC();
    void generateOptionalIR();

    const Recipe& recipe;
    BuildOptions options;

    typedef std::vector<FileInfo*> Files;
    Files files;

    Pkgs pkgs;

    C2Builder(const C2Builder&);
    C2Builder& operator= (const C2Builder&);
};

}

#endif

