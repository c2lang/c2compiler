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

#include <clang/Basic/SourceLocation.h>
#include "Scope.h"
#include "Type.h"

using clang::SourceLocation;

#define MAX_SCOPE_DEPTH 15

namespace clang {
class DiagnosticsEngine;
}

namespace C2 {

class Type;
class TypeContext;
class FileScope;
class Decl;
class VarDecl;
class FunctionDecl;
class Stmt;
class Expr;
class IdentifierExpr;

class FunctionAnalyser {
public:
    FunctionAnalyser(FileScope& scope_,
                         TypeContext& tc,
                         clang::DiagnosticsEngine& Diags_);

    unsigned check(FunctionDecl* F);
private:
    void EnterScope(unsigned flags);
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

    QualType analyseExpr(Expr* expr);
    void analyseDeclExpr(Expr* expr);
    QualType analyseBinaryOperator(Expr* expr);
    QualType analyseConditionalOperator(Expr* expr);
    QualType analyseUnaryOperator(Expr* expr);
    void analyseBuiltinExpr(Expr* expr);
    QualType analyseArraySubscript(Expr* expr);
    QualType analyseMemberExpr(Expr* expr);
    QualType analyseMember(QualType T, IdentifierExpr* member);
    QualType analyseParenExpr(Expr* expr);
    QualType analyseCall(Expr* expr);
    ScopeResult analyseIdentifier(IdentifierExpr* expr);

    void analyseInitExpr(Expr* expr, QualType expectedType);
    void analyseInitList(Expr* expr, QualType expectedType);

    void pushMode(unsigned DiagID);
    void popMode();

    class ConstModeSetter {
    public:
        ConstModeSetter (FunctionAnalyser& analyser_, unsigned DiagID)
            : analyser(analyser_)
        {
            analyser.pushMode(DiagID);
        }
        ~ConstModeSetter()
        {
            analyser.popMode();
        }
    private:
        FunctionAnalyser& analyser;
    };

    void checkConversion(SourceLocation Loc, QualType from, QualType to);
    void checkAssignment(Expr* assignee, QualType TLeft);

    static QualType resolveUserType(QualType T);
    QualType Decl2Type(Decl* decl);
    unsigned checkType(QualType Q, bool used_public);
    QualType resolveCanonical(QualType Q, bool set);

    FileScope& globalScope;
    TypeContext& typeContext;
    Scope scopes[MAX_SCOPE_DEPTH];
    unsigned scopeIndex;    // first free scope (= count of scopes)
    Scope* curScope;
    clang::DiagnosticsEngine& Diags;
    unsigned errors;

    FunctionDecl* Function;     // current function

    unsigned constDiagID;
    bool inConstExpr;
    unsigned CRASH_AVOIDER[4];    // NEEDED, OTHERWISE CLANG CAUSES SEGFAULT IN COMPILED CODE!

    FunctionAnalyser(const FunctionAnalyser&);
    FunctionAnalyser& operator= (const FunctionAnalyser&);
};

}

#endif

