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

#ifndef REFACTOR_REFFINDER_H
#define REFACTOR_REFFINDER_H

#include <vector>
#include <clang/Basic/SourceLocation.h>

namespace C2 {

class AST;
class Decl;
class Stmt;
class CompoundStmt;
class Expr;

class RefFinder {
public:
    RefFinder(AST& ast_, const Decl* decl_)
        : ast(ast_)
        , decl(decl_)
    {}
    ~RefFinder() {}

    unsigned find();

    typedef std::vector<clang::SourceLocation> Locs;
    Locs locs;
private:
    void searchStmt(const Stmt* S);
    void searchCompoundStmt(const CompoundStmt* S);
    void searchExpr(const Expr* E);
    void addFileLocation(clang::SourceLocation loc);

    AST& ast;
    const Decl* decl;

    RefFinder(const RefFinder&);
    RefFinder& operator= (const RefFinder&);
};

}

#endif

