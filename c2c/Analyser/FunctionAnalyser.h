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

#include <llvm/ADT/APSInt.h>

#include "Analyser/Scope.h"
#include "Analyser/ExprTypeAnalyser.h"
#include "AST/Type.h"

namespace clang {
class DiagnosticsEngine;
}

namespace C2 {

class TypeChecker;
class Scope;
class Decl;
class VarDecl;
class FunctionDecl;
class Stmt;
class Expr;
class IdentifierExpr;
class InitListExpr;

class FunctionAnalyser {
public:
    FunctionAnalyser(Scope& scope_,
                    TypeChecker& typeRes_,
                    TypeContext& tc,
                    clang::DiagnosticsEngine& Diags_);

    unsigned check(FunctionDecl* F);
    unsigned checkVarInit(VarDecl* V);
private:
    void checkFunction(FunctionDecl* F);

    void analyseStmt(Stmt* stmt, bool haveScope = false);
    void analyseCompoundStmt(Stmt* stmt);
    void analyseIfStmt(Stmt* stmt);
    void analyseWhileStmt(Stmt* stmt);
    void analyseDoStmt(Stmt* stmt);
    void analyseForStmt(Stmt* stmt);
    void analyseSwitchStmt(Stmt* stmt);
    void analyseBreakStmt(Stmt* S);
    void analyseContinueStmt(Stmt* S);
    void analyseLabelStmt(Stmt* S);
    void analyseCaseStmt(Stmt* stmt);
    void analyseDefaultStmt(Stmt* stmt);
    void analyseReturnStmt(Stmt* stmt);
    void analyseStmtExpr(Stmt* stmt);

    QualType analyseExpr(Expr* expr, unsigned side);
    void analyseDeclExpr(Expr* expr);
    QualType analyseIntegerLiteral(Expr* expr);
    QualType analyseBinaryOperator(Expr* expr, unsigned side);
    QualType analyseConditionalOperator(Expr* expr);
    QualType analyseUnaryOperator(Expr* expr, unsigned side);
    QualType analyseBuiltinExpr(Expr* expr);
    QualType analyseArraySubscript(Expr* expr);
    QualType analyseMemberExpr(Expr* expr, unsigned side);
    QualType analyseMember(QualType T, IdentifierExpr* member, unsigned side);
    QualType analyseParenExpr(Expr* expr);
    QualType analyseCall(Expr* expr);
    ScopeResult analyseIdentifier(IdentifierExpr* expr);

    void analyseInitExpr(Expr* expr, QualType expectedType);
    void analyseInitList(InitListExpr* expr, QualType expectedType);
    void analyseSizeofExpr(Expr* expr);

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

    bool checkAssignee(Expr* expr) const;
    void checkAssignment(Expr* assignee, QualType TLeft);
    void checkDeclAssignment(Decl* decl, Expr* expr);

    static QualType resolveUserType(QualType T);
    QualType Decl2Type(Decl* decl);

    Scope& scope;
    TypeChecker& TC;
    TypeContext& typeContext;
    ExprTypeAnalyser EA;

    clang::DiagnosticsEngine& Diags;

    unsigned errors;
    FunctionDecl* CurrentFunction;
    VarDecl* CurrentVarDecl;
    unsigned constDiagID;
    bool inConstExpr;

    FunctionAnalyser(const FunctionAnalyser&);
    FunctionAnalyser& operator= (const FunctionAnalyser&);
};

}

#endif

