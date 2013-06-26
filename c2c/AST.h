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

#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <clang/Basic/SourceLocation.h>

namespace C2 {

class Decl;
class ASTVisitor;

class AST {
public:
    AST() {}
    ~AST();

    // analysis
    void visitAST(ASTVisitor& visitor);

    // debugging
    void print(const std::string& filename) const;

    // codegen
    // TODO use functions of direct member access?
    unsigned getNumDecls() const { return decls.size(); }
    Decl* getDecl(unsigned index) const { return decls[index]; }

    const std::string& getPkgName() const { return pkgName; }

    std::string pkgName;
    clang::SourceLocation pkgLoc;

    typedef std::vector<Decl*> DeclList;
    typedef DeclList::const_iterator DeclListConstIter;
    typedef DeclList::iterator DeclListIter;
    DeclList decls;
};

}

#endif

