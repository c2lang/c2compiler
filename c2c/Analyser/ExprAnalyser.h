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

#ifndef ANALYSER_EXPR_ANALYSER_H
#define ANALYSER_EXPR_ANALYSER_H

#include "Clang/SourceLocation.h"
#include "Clang/OperationKinds.h"
#include "Clang/ParseDiagnostic.h"
#include "AST/Type.h"
#include "Utils/StringBuilder.h"

namespace C2 {

class Expr;
class BuiltinExpr;
class ExplicitCastExpr;
class IdentifierExpr;
class UnaryOperator;
class BinaryOperator;
class TargetInfo;
class Decl;
class StructTypeDecl;
class ASTContext;

/*
 *  ExprAnalyser checks each sub-expression that is CTC_FULL or
 *  CTC_NONE with the other checkers separately.
 *  It checks Literals for Range and Symbols for Type
*/
class ExprAnalyser {
public:
    ExprAnalyser(c2lang::DiagnosticsEngine& Diags_,
                 const TargetInfo& target_,
                 ASTContext& context_);

    void check(QualType Tleft, Expr* expr);
    bool checkExplicitCast(const ExplicitCastExpr* cast, QualType TLeft, QualType TRight);
    void error(const Expr* expr, QualType left, QualType right);
    QualType outputStructDiagnostics(QualType T, IdentifierExpr* member, unsigned msg);

    Decl* analyseOffsetOf(BuiltinExpr* expr, const StructTypeDecl* S, Expr* member, uint64_t* off);
    QualType analyseIntegerLiteral(Expr* expr);

    QualType getBinOpType(const BinaryOperator* binop);

    bool hasError() const { return m_hasError; }

    c2lang::DiagnosticBuilder Diag2(c2lang::SourceLocation loc, int diag_id, QualType T1, QualType T2);
    QualType LargestType(QualType TL, QualType TR);

    bool arePointersCompatible(QualType L, QualType R);
private:
    void checkUnaryOp(QualType TLeft, const UnaryOperator* op);
    void checkBinOp(QualType TLeft, Expr* binop_ptr);

    bool checkCompatible(QualType left, Expr* expr);
    bool checkBuiltin(QualType left, QualType right, const Expr* expr, bool first);
    bool checkStruct(QualType left, QualType right, const Expr* expr);
    bool checkPointer(QualType left, QualType right, Expr* expr);
    bool checkFunction(QualType left, const Expr* expr);
    bool checkWidth(QualType type, c2lang::SourceLocation loc, int msg);

    bool checkNonPointerCast(const ExplicitCastExpr* expr, QualType DestType, QualType SrcType);
    bool checkBuiltinCast(const ExplicitCastExpr* expr, QualType DestType, QualType SrcType);
    bool checkEnumCast(const ExplicitCastExpr* expr, QualType DestType, QualType SrcType);
    bool checkFunctionCast(const ExplicitCastExpr* expr, QualType DestType, QualType SrcType);


    void error2(c2lang::SourceLocation loc, QualType left, QualType right, unsigned msg);
    const Expr* stripImplicitCast(const Expr* E);

    c2lang::DiagnosticsEngine& Diags;
    const TargetInfo& target;
    ASTContext& Context;
    bool m_hasError;

    StringBuilder buf1;
    StringBuilder buf2;

    ExprAnalyser(const ExprAnalyser&);
    ExprAnalyser& operator= (const ExprAnalyser&);
};

}

#endif

