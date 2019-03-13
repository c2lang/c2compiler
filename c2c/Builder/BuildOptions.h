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

#ifndef BUILDER_BUILD_OPTIONS_H
#define BUILDER_BUILD_OPTIONS_H

namespace C2 {

struct BuildOptions {
    BuildOptions()
        : printAST0(false)
        , printAST1(false)
        , printAST2(false)
        , printAST3(false)
        , printASTLib(false)
        , printTiming(false)
        , printSymbols(false)
        , printLibSymbols(false)
        , generateIR(false)
        , printIR(false)
        , generateC(false)
        , printC(false)
        , checkOnly(false)
        , showLibs(false)
        , printModules(false)
        , printDependencies(false)
        , generateRefs(false)
        , verbose(false)
        , testMode(false)
        , libdir(0)
    {}
    bool printAST0;
    bool printAST1;
    bool printAST2;
    bool printAST3;
    bool printASTLib;
    bool printTiming;
    bool printSymbols;
    bool printLibSymbols;
    bool generateIR;
    bool printIR;
    bool generateC;
    bool printC;
    bool checkOnly;
    bool showLibs;
    bool printModules;
    bool printDependencies;
    bool generateRefs;
    bool verbose;
    bool testMode;

    const char* libdir;
};

}

#endif

