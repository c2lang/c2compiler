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

#ifndef ALGO_DEP_GENERATOR_H
#define ALGO_DEP_GENERATOR_H

#include <string>
#include <vector>

#include "AST/Component.h"

namespace C2 {

class AST;
class Module;
class Decl;
class StringBuilder;

class DepGenerator {
public:
    DepGenerator(bool showFiles_, bool showPrivate_, bool showExternals_)
        : showFiles(showFiles_), showPrivate(showPrivate_), showExternals(showExternals_)
    {}

    void write(const Components& components, const std::string& title, const std::string& path) const;
private:
    void addExternal(const Module* P) const;
    void writeModule(const Module& M, StringBuilder& output, unsigned indent) const;
    void writeAST(const AST& ast, StringBuilder& output, unsigned indent, bool isExternal) const;
    void writeDecl(const Decl* D, StringBuilder& output, unsigned indent) const;

    bool showFiles;
    bool showPrivate;
    bool showExternals;

    DepGenerator(const DepGenerator&);
    DepGenerator& operator= (const DepGenerator&);
};

}

#endif

