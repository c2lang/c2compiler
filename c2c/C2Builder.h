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
#include <map>
#include <string>

namespace C2 {

class Recipe;
class FileInfo;
class Package;

struct BuildOptions {
    BuildOptions()
        : printAST(false)
        , printTiming(false)
        , mode(NORMAL)
    {}
    bool printAST;
    bool printTiming;
    enum Mode {
        NORMAL,
        GENERATE_C,
        SYNTAX_ONLY,
    };
    Mode mode;
};


class C2Builder {
public:
    C2Builder(const Recipe& recipe_, const BuildOptions& opts);
    ~C2Builder();

    void build();
private:
    Package* getPackage(const std::string& name);
    bool createPkgs();
    void dumpPkgs();

    const Recipe&  recipe;
    BuildOptions options;

    typedef std::vector<FileInfo*> Files;
    Files files;

    typedef std::map<std::string, Package*> Pkgs;
    typedef Pkgs::iterator PkgsIter;
    Pkgs pkgs;

    C2Builder(const C2Builder&);
    C2Builder& operator= (const C2Builder&);
};

}

#endif

