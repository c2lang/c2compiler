/* Copyright 2013-2018 Bas van den Berg
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

#ifndef ANALYSER_FUNCTION_ANALYSER_H
#define ANALYSER_FUNCTION_ANALYSER_H

#include <string>
#include <vector>
#include <llvm/ADT/APSInt.h>
#include "Clang/SourceLocation.h"
#include "AST/Type.h"
#include "Analyser/ExprTypeAnalyser.h"

namespace c2lang {
class DiagnosticsEngine;
class DiagnosticBuilder;
}

namespace C2 {

class TypeResolver;
class Scope;
class Decl;
class VarDecl;
class FunctionDecl;
class EnumConstantDecl;
class LabelDecl;
class Stmt;
class SwitchStmt;
class Expr;
class IdentifierExpr;
class InitListExpr;
class MemberExpr;
class CallExpr;
class BuiltinExpr;
class ASTContext;


constexpr size_t MAX_STRUCT_INDIRECTION_DEPTH = 256;

class FunctionAnalyser {
public:
    FunctionAnalyser(Scope& scope_,
                    TypeResolver& typeRes_,
                    ASTContext& context_,
                    c2lang::DiagnosticsEngine& Diags_,
                    bool isInterface_);

    void check(FunctionDecl* F);
    void checkVarInit(VarDecl* V);
    void checkArraySizeExpr(VarDecl* V);
    unsigned checkEnumValue(EnumConstantDecl* E, llvm::APSInt& nextValue);
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
    void analyseGotoStmt(Stmt* S);
    void analyseCaseStmt(Stmt* stmt);
    void analyseDefaultStmt(Stmt* stmt);
    void analyseReturnStmt(Stmt* stmt);
    void analyseDeclStmt(Stmt* stmt);
    void analyseAsmStmt(Stmt* stmt);
    bool analyseCondition(Stmt* stmt);
    void analyseStmtExpr(Stmt* stmt);

    QualType analyseExpr(Expr* expr, unsigned side);
    QualType analyseIntegerLiteral(Expr* expr);
    QualType analyseBinaryOperator(Expr* expr, unsigned side);
    QualType analyseConditionalOperator(Expr* expr);
    QualType analyseUnaryOperator(Expr* expr, unsigned side);
    QualType analyseBuiltinExpr(Expr* expr);
    QualType analyseArraySubscript(Expr* expr, unsigned side);
    QualType analyseMemberExpr(Expr* expr, unsigned side);
    QualType analyseStructMember(QualType T, MemberExpr* M, unsigned side, bool isStatic);
    bool checkStructTypeArg(QualType T,  FunctionDecl* func) const;
    Decl* getStructDecl(QualType T) const;
    bool exprIsType(const Expr* E) const;
    QualType analyseParenExpr(Expr* expr);
    bool analyseBitOffsetIndex(Expr* expr, llvm::APSInt* Result, BuiltinType* BaseType);
    QualType analyseBitOffsetExpr(Expr* expr, QualType BaseType, c2lang::SourceLocation base);
    QualType analyseExplicitCastExpr(Expr* expr);
    QualType analyseCall(Expr* expr);
    bool checkCallArgs(FunctionDecl* func, CallExpr* call, Expr *structFunction);
    Decl* analyseIdentifier(IdentifierExpr* expr);

    void analyseInitExpr(Expr* expr, QualType expectedType);
    void analyseInitList(InitListExpr* expr, QualType expectedType);
    void analyseDesignatorInitExpr(Expr* expr, QualType expectedType);
    void analyseSizeOfExpr(BuiltinExpr* expr);
    QualType analyseElemsOfExpr(BuiltinExpr* B);
    QualType analyseEnumMinMaxExpr(BuiltinExpr* B, bool isMin);
    void analyseArrayType(VarDecl* V, QualType T);
    void analyseArraySizeExpr(ArrayType* AT);

    c2lang::DiagnosticBuilder Diag(c2lang::SourceLocation Loc, unsigned DiagID) const;
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

    LabelDecl* LookupOrCreateLabel(const char* name, c2lang::SourceLocation loc);

    bool checkAssignee(Expr* expr) const;
    void checkAssignment(Expr* assignee, QualType TLeft);
    void checkDeclAssignment(Decl* decl, Expr* expr);
    void checkArrayDesignators(InitListExpr* expr, int64_t* size);
    void checkEnumCases(const SwitchStmt* SS, const EnumType* ET) const;
    QualType getStructType(QualType T) const;
    QualType getConditionType(const Stmt* C) const;

    void outputStructDiagnostics(QualType T, IdentifierExpr *member, unsigned msg);
    QualType analyseStaticStructFunction(QualType T, MemberExpr *M, const StructTypeDecl *S, unsigned side);

    // conversions
    QualType UsualUnaryConversions(Expr* expr) const;

    Scope& scope;
    TypeResolver& TR;
    ASTContext& Context;
    ExprTypeAnalyser EA;

    c2lang::DiagnosticsEngine& Diags;

    FunctionDecl* CurrentFunction;
    VarDecl* CurrentVarDecl;
    unsigned constDiagID;
    bool inConstExpr;
    bool usedPublicly;
    bool isInterface;

    // Our callstack (statically allocated)
    struct CallStack {
        Expr *structFunction[MAX_STRUCT_INDIRECTION_DEPTH];
        unsigned callDepth;
        void push() { structFunction[callDepth++] = 0; }
        Expr *pop() { return structFunction[--callDepth]; }
        bool reachedMax() { return callDepth == MAX_STRUCT_INDIRECTION_DEPTH; };
        void setStructFunction(Expr* expr) { structFunction[callDepth - 1] = expr; }
    };

    CallStack callStack;

    typedef std::vector<LabelDecl*> Labels;
    typedef Labels::iterator LabelsIter;
    Labels labels;

    FunctionAnalyser(const FunctionAnalyser&);
    FunctionAnalyser& operator= (const FunctionAnalyser&);
};

}

#endif

