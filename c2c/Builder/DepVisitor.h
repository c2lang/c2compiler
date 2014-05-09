/* Copyright 2013,2014 Bas van den Berg
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

#ifndef BUILDER_DEP_VISITOR_H
#define BUILDER_DEP_VISITOR_H

#include <vector>

#include "AST/Type.h"

namespace C2 {

class Decl;
class VarDecl;
class FunctionDecl;
class Stmt;
class CompoundStmt;
class Expr;

class DepVisitor {
public:
    DepVisitor(const Decl* D) : decl(D) {}

    void run();

    unsigned getNumDeps() const { return deps.size(); }
    const Decl* getDep(unsigned i) const { return deps[i]; }
private:
    // Decl
    void checkDecl(const Decl* D);
    void checkVarDecl(const VarDecl* V);
    void checkFunctionDecl(const FunctionDecl* F);
    // Type
    void checkType(QualType Q);
    // Stmt
    void checkStmt(const Stmt* S);
    void checkCompoundStmt(const CompoundStmt* C);
    // Expr
    void checkExpr(const Expr* E);

    void addDep(const Decl* D);

    typedef std::vector<const Decl*> Deps;
    Deps deps;
    const Decl* decl;
};

}

#endif

