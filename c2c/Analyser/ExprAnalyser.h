/* Copyright 2013-2021 Bas van den Berg
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
#include "AST/Type.h"

namespace c2lang {
class DiagnosticsEngine;
}

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

/*
 *  ExprAnalyser checks each sub-expression that is CTC_FULL or
 *  CTC_NONE with the other checkers separately.
 *  It checks Literals for Range and Symbols for Type
 *  For CTC_PARTIAL expressions (eg. 10 + a), each sub-expression
 *  should fit the LHS type.
*/
class ExprAnalyser {
public:
    ExprAnalyser(c2lang::DiagnosticsEngine& Diags_, const TargetInfo& target_);

    void check(QualType Tleft, const Expr* expr);
    bool checkExplicitCast(const ExplicitCastExpr* cast, QualType TLeft, QualType TRight);
    void error(c2lang::SourceLocation loc, QualType left, QualType right);
    bool outputStructDiagnostics(QualType T, IdentifierExpr* member, unsigned msg);

    Decl* analyseOffsetOf(BuiltinExpr* expr, const StructTypeDecl* S, Expr* member, uint64_t* off);
    QualType analyseIntegerLiteral(Expr* expr);

    bool hasError() const { return m_hasError; }
private:
    void checkUnaryOp(QualType TLeft, const UnaryOperator* op);
    void checkBinOp(QualType TLeft, const BinaryOperator* binop);

    bool checkCompatible(QualType left, const Expr* expr);
    bool checkBuiltin(QualType left, QualType right, const Expr* expr, bool first);
    bool checkStruct(QualType left, QualType right, const Expr* expr);
    bool checkPointer(QualType left, QualType right, const Expr* expr);
    bool checkFunction(QualType left, const Expr* expr);
    bool checkWidth(QualType type, c2lang::SourceLocation loc, int msg);

    bool checkNonPointerCast(const ExplicitCastExpr* expr, QualType DestType, QualType SrcType);
    bool checkBuiltinCast(const ExplicitCastExpr* expr, QualType DestType, QualType SrcType);
    bool checkEnumCast(const ExplicitCastExpr* expr, QualType DestType, QualType SrcType);
    bool checkFunctionCast(const ExplicitCastExpr* expr, QualType DestType, QualType SrcType);

    void error2(c2lang::SourceLocation loc, QualType left, QualType right, unsigned msg);

    c2lang::DiagnosticsEngine& Diags;
    const TargetInfo& target;
    bool m_hasError;

    ExprAnalyser(const ExprAnalyser&);
    ExprAnalyser& operator= (const ExprAnalyser&);
};

}

#endif

