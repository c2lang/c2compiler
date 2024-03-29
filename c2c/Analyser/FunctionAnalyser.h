/* Copyright 2013-2023 Bas van den Berg
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
#include "AST/Expr.h"
#include "Analyser/ExprAnalyser.h"

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
class DesignatedInitExpr;
class Stmt;
class SwitchStmt;
class Expr;
class FallthroughStmt;
class IdentifierExpr;
class InitListExpr;
class MemberExpr;
class CallExpr;
class BuiltinExpr;
class ASTContext;
class TargetInfo;
class Module;

constexpr size_t MAX_STRUCT_INDIRECTION_DEPTH = 256;
constexpr size_t MAX_EXPR_DEPTH = 64;

class FunctionAnalyser {
public:
    FunctionAnalyser(Scope& scope_,
                    TypeResolver& typeRes_,
                    ASTContext& context_,
                    c2lang::DiagnosticsEngine& Diags_,
                    const TargetInfo& target_,
                    bool isInterface_);

    void check(FunctionDecl* F);
private:
    void checkFunction(FunctionDecl* F);

    bool analyseStmt(Stmt* stmt, bool haveScope = false);
    void analyseCompoundStmt(Stmt* stmt);
    void analyseIfStmt(Stmt* stmt);
    void analyseWhileStmt(Stmt* stmt);
    void analyseDoStmt(Stmt* stmt);
    void analyseForStmt(Stmt* stmt);
    void analyseSwitchStmt(Stmt* stmt);
    void analyseSSwitchStmt(Stmt* stmt);
    bool analyseBreakStmt(Stmt* S);
    bool analyseFallthroughStmt(Stmt* S);
    bool analyseContinueStmt(Stmt* S);
    bool analyseLabelStmt(Stmt* S);
    void analyseGotoStmt(Stmt* S);
    bool analyseCaseStmt(Stmt* stmt, const EnumType* ET);
    void analyseSSwitchCaseStmt(Stmt* stmt);
    void analyseDefaultStmt(Stmt* stmt, bool isSwitch);
    bool analyseReturnStmt(Stmt* stmt);
    void analyseDeclStmt(Stmt* stmt);
    void analyseAsmStmt(Stmt* stmt);
    void analyseAssertStmt(Stmt* stmt);
    bool analyseCondition(Stmt* stmt);
    void analyseStmtExpr(Stmt* stmt);

    QualType analyseExpr(Expr** expr_ptr, unsigned side, bool need_rvalue);
    QualType inline analyseExprInner(Expr** expr, unsigned side);
    QualType analyseBinaryOperator(Expr* expr, unsigned side);
    QualType checkCondionalOperatorTypes(QualType TLeft, QualType TRight);
    QualType analyseConditionalOperator(Expr* expr);
    QualType analyseUnaryOperator(Expr** expr, unsigned side);
    QualType analyseBuiltinExpr(Expr* expr);
    QualType analyseArraySubscript(Expr* expr, unsigned side);
    QualType analyseMemberExpr(Expr** expr_ptr, unsigned side);
    QualType analyseStructMember(QualType T, Expr** expr_ptr, unsigned side, bool isStatic);
    bool exprIsType(const Expr* E) const;
    QualType analyseParenExpr(Expr* expr);
    bool analyseBitOffsetIndex(Expr** expr, llvm::APSInt* Result, BuiltinType* BaseType);
    QualType analyseBitOffsetExpr(Expr* expr, QualType BaseType, c2lang::SourceLocation base);
    QualType analyseExplicitCastExpr(Expr* expr);
    QualType analyseImplicitCastExpr(Expr* expr);
    QualType analyseCall(Expr* expr);
    QualType analyseIdentifierExpr(Expr** expr_ptr, unsigned side);
    bool checkCallArgs(FunctionDecl* func, CallExpr* call, Expr *structFunction);
    Decl* analyseIdentifier(IdentifierExpr* expr);

    void analyseInitExpr(Expr** expr, QualType expectedType);
    void analyseInitList(InitListExpr* expr, QualType expectedType);
    void analyseInitListArray(InitListExpr* expr, QualType expectedType, unsigned numValues, Expr** values);
    void analyseInitListStruct(InitListExpr* expr, QualType expectedType, unsigned numValues, Expr** values);
    void analyseDesignatorInitExpr(Expr* expr, QualType expectedType);
    typedef std::vector<Expr*> Fields;
    bool analyseFieldInDesignatedInitExpr(DesignatedInitExpr* expr,
                                          StructTypeDecl* std,
                                          QualType Q,
                                          Fields &fields,
                                          Expr* value,
                                          bool &haveDesignators);
    bool checkAddressOfOperand(Expr* expr);

    QualType analyseSizeOfExpr(BuiltinExpr* expr);
    QualType analyseElemsOfExpr(BuiltinExpr* B);
    QualType analyseEnumMinMaxExpr(BuiltinExpr* B, bool isMin);
    StructTypeDecl* builtinExprToStructTypeDecl(BuiltinExpr* B);
    QualType analyseOffsetOf(BuiltinExpr* expr);
    QualType analyseToContainer(BuiltinExpr* expr);
    QualType analyseType(QualType Q, c2lang::SourceLocation loc);
    QualType analyseRefType(QualType Q);
    bool analyseArraySizeExpr(ArrayType* AT);

    c2lang::DiagnosticBuilder Diag(c2lang::SourceLocation Loc, unsigned DiagID) const;
    void pushMode(unsigned DiagID);
    void popMode();
    const Module* currentModule() const;

    LabelDecl* LookupOrCreateLabel(const char* name, c2lang::SourceLocation loc);

    bool checkAssignment(Expr* assignee, QualType TLeft, const char* diag_msg, c2lang::SourceLocation loc);
    void checkDeclAssignment(Decl* decl, Expr* expr);
    void checkArrayDesignators(InitListExpr* expr, int64_t* size);
    void checkEnumCases(const SwitchStmt* SS, const EnumType* ET) const;
    QualType getStructType(QualType T) const;
    QualType getConditionType(const Stmt* C) const;

    void insertImplicitCast(c2lang::CastKind ck, Expr** inner, QualType Q);

    QualType analyseStaticStructMember(QualType T, Expr** M, const StructTypeDecl *S, unsigned side);

    Scope& scope;
    TypeResolver& TR;
    ASTContext& Context;
    ExprAnalyser EA;

    c2lang::DiagnosticsEngine& Diags;
    const TargetInfo& target;

    FunctionDecl* CurrentFunction;
    VarDecl* CurrentVarDecl;
    unsigned constDiagID;
    bool inConstExpr;
    bool usedPublicly;
    bool isInterface;
    FallthroughStmt* fallthrough;

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
    bool allowStaticMember;

    typedef std::vector<LabelDecl*> Labels;
    typedef Labels::iterator LabelsIter;
    Labels labels;

    FunctionAnalyser(const FunctionAnalyser&);
    FunctionAnalyser& operator= (const FunctionAnalyser&);
};

}

#endif

