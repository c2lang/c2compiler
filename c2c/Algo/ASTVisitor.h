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

#ifndef ALGO_AST_VISITOR_H
#define ALGO_AST_VISITOR_H

/*
    This class is the beginning of a generic AST-Visitor class that can be
    extended for various purposes. Currently it only supports visiting all
    IdentifierExpr's in the code, for a Decl
*/

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
class IdentifierExpr;

class ASTVisitor {
public:
    ASTVisitor(const Decl* D) : decl(D) {}
    virtual ~ASTVisitor() {}

    void run();

    virtual void visitIdentifierExpr(const IdentifierExpr* I) = 0;
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

    const Decl* decl;
};

}

#endif

