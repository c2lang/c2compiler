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

#ifndef FUNCTION_ANALYSER_H
#define FUNCTION_ANALYSER_H

#include "ASTVisitor.h"
#include "Package.h"
#include "Scope.h"

#define MAX_SCOPE_DEPTH 15

namespace clang {
class DiagnosticsEngine;
}

namespace C2 {

class Type;
class FileScope;
class Decl;
class FunctionDecl;
class Stmt;
class Expr;
class IdentifierExpr;
class TypeContext;

class FunctionAnalyser : public ASTVisitor {
public:
    FunctionAnalyser(FileScope& scope_,
                         TypeContext& tc,
                         clang::DiagnosticsEngine& Diags_);
    virtual ~FunctionAnalyser();

    virtual bool handle(Decl* decl);
    unsigned int getErrors() const { return errors; }
private:
    void EnterScope(unsigned int flags);
    void ExitScope();

    void analyseStmt(Stmt* stmt, bool haveScope = false);
    void analyseCompoundStmt(Stmt* stmt);
    void analyseIfStmt(Stmt* stmt);
    void analyseWhileStmt(Stmt* stmt);
    void analyseDoStmt(Stmt* stmt);
    void analyseForStmt(Stmt* stmt);
    void analyseSwitchStmt(Stmt* stmt);
    void analyseBreakStmt(Stmt* S);
    void analyseContinueStmt(Stmt* S);
    void analyseCaseStmt(Stmt* stmt);
    void analyseDefaultStmt(Stmt* stmt);
    void analyseReturnStmt(Stmt* stmt);

    void analyseStmtExpr(Stmt* stmt);
    Type* analyseExpr(Expr* expr);
    void analyseDeclExpr(Expr* expr);
    Type* analyseBinOpExpr(Expr* expr);
    Type* analyseUnaryOpExpr(Expr* expr);
    void analyseSizeofExpr(Expr* expr);
    Type* analyseArraySubscript(Expr* expr);
    Type* analyseMemberExpr(Expr* expr);
    Type* analyseParenExpr(Expr* expr);
    Type* analyseCall(Expr* expr);
    ScopeResult analyseIdentifier(Expr* expr);

    Type* checkAssignmentOperands(Type* left, Type* right);

    static Type* resolveUserType(Type* T);
    Type* resolveCanonicalType(Type* T);
    Type* Decl2Type(Decl* decl);

    FileScope& globalScope;
    TypeContext& typeContext;
    Scope scopes[MAX_SCOPE_DEPTH];
    unsigned scopeIndex;    // first free scope (= count of scopes)
    Scope* curScope;
    clang::DiagnosticsEngine& Diags;
    unsigned int errors;

    FunctionDecl* func;     // current function

    FunctionAnalyser(const FunctionAnalyser&);
    FunctionAnalyser& operator= (const FunctionAnalyser&);
};

}

#endif

