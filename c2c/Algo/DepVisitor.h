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

#ifndef ALGO_DEP_VISITOR_H
#define ALGO_DEP_VISITOR_H

#include <stdint.h>
#include <vector>

namespace C2 {

class QualType;
class Decl;
class VarDecl;
class FunctionDecl;
class Stmt;
class CompoundStmt;
class Expr;

class DepVisitor {
public:
    DepVisitor(const Decl* D, bool checkExternals_) : decl(D), checkExternals(checkExternals_) {}

    void run();

    unsigned getNumDeps() const { return deps.size(); }
    inline const Decl* getDep(unsigned i) const {
        return reinterpret_cast<const Decl*>(deps[i] & ~0x1);
    }
    inline bool isFull(unsigned i) const { return  deps[i] & 0x1; }
private:
    // Decl
    void checkDecl(const Decl* D);
    void checkVarDecl(const VarDecl* V);
    void checkFunctionDecl(const FunctionDecl* F);
    // Type
    void checkType(QualType Q, bool isFull = true);
    // Stmt
    void checkStmt(const Stmt* S);
    void checkCompoundStmt(const CompoundStmt* C);
    // Expr
    void checkExpr(const Expr* E);

    void addDep(const Decl* D, bool isFull = true);

    // isFull is stored in lowest bit, Decl* in rest
    typedef std::vector<uintptr_t> Deps;
    Deps deps;
    const Decl* decl;
    bool checkExternals;
};

}

#endif

