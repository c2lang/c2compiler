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

#include <clang/Parse/ParseDiagnostic.h>
#include <clang/Sema/SemaDiagnostic.h>

#include "Analyser/ExprTypeAnalyser.h"
#include "Analyser/LiteralAnalyser.h"
#include "Analyser/TypeFinder.h"
#include "Analyser/constants.h"
#include "AST/Expr.h"
#include "Utils/StringBuilder.h"

using namespace C2;
using namespace llvm;
using namespace clang;

// 0 = ok,
// 1 = loss of integer precision,
// 2 = sign-conversion,
// 3 = float->integer,
// 4 = incompatible,
// 5 = loss of FP precision
static int type_conversions[14][14] = {
    // I8  I16  I32  I64   U8  U16  U32  U64  F32  F64  Bool  Void
    // I8 ->
    {   0,   0,   0,   0,   2,   2,   2,   2,   0,   0,    0,   4},
    // I16 ->
    {   1,   0,   0,   0,   2,   2,   2,   2,   0,   0,    0,   4},
    // I32 ->
    {   1,   1,   0,   0,   2,   2,   2,   2,   0,   0,    0,   4},
    // I64 ->
    {   1,   1,   1,   0,   2,   2,   2,   2,   0,   0,    0,   4},
    // U8 ->
    {   2,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,   4},
    // U16 ->
    {   1,   2,   0,   0,   1,   0,   0,   0,   0,   0,    0,   4},
    // U32 ->
    {   1,   1,   2,   0,   1,   1,   0,   0,   0,   0,    0,   4},
    // U64 ->
    {   1,   1,   1,   2,   1,   1,   1,   0,   0,   0,    0,   4},
    // F32 ->
    {   3,   3,   3,   3,   3,   3,   3,   3,   0,   0,    4,   4},
    // F64 ->
    {   3,   3,   3,   3,   3,   3,   3,   3,   5,   0,    4,   4},
    // BOOL ->
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,   4},
    // VOID ->
    {  4,    4,   4,   4,  4,   4,   4,   4,   4,   4,    4,    0},
};



ExprTypeAnalyser::ExprTypeAnalyser(DiagnosticsEngine& Diags_)
    : Diags(Diags_)
{}

void ExprTypeAnalyser::check(QualType TLeft, const Expr* expr) {
    switch (expr->getCTC()) {
    case CTC_NONE:
        checkCompatible(TLeft, expr);
        return;
    case CTC_PARTIAL:
        break;
    case CTC_FULL:
        LiteralAnalyser LA(Diags);
        LA.check(TLeft, expr);
        return;
    }

    switch (expr->getKind()) {
    case EXPR_INTEGER_LITERAL:
    case EXPR_FLOAT_LITERAL:
    case EXPR_BOOL_LITERAL:
    case EXPR_CHAR_LITERAL:
    case EXPR_STRING_LITERAL:
    case EXPR_NIL:
        // always CTC_FULL
    case EXPR_IDENTIFIER:
        // can be CTC_NONE or CTC_FULL
    case EXPR_TYPE:
    case EXPR_CALL:
        // always CTC_NONE
    case EXPR_INITLIST:
    case EXPR_DESIGNATOR_INIT:
    case EXPR_DECL:
        assert(0 && "should not come here");
        break;
    case EXPR_BINOP:
        checkBinOp(TLeft, cast<BinaryOperator>(expr));
        return;
    case EXPR_CONDOP:
        {
            // NOTE: Cond -> Bool has already been checked
            const ConditionalOperator* C = cast<ConditionalOperator>(expr);
            check(TLeft, C->getLHS());
            check(TLeft, C->getRHS());
            return;
        }
    case EXPR_UNARYOP:
        break;
    case EXPR_BUILTIN:
        // always CTC_FULL
    case EXPR_ARRAYSUBSCRIPT:
    case EXPR_MEMBER:
        // can be CTC_NONE or CTC_FULL
        assert(0 && "should not come here");
        break;
    case EXPR_PAREN:
        check(TLeft, cast<ParenExpr>(expr)->getExpr());
        return;
    }
    assert(0 && "should not come here");
}

void ExprTypeAnalyser::checkBinOp(QualType TLeft, const BinaryOperator* binop) {
    // NOTE we check Left / Right separately if CTC's are not the same
    switch (binop->getOpcode()) {
    case BO_PtrMemD:
    case BO_PtrMemI:
        assert(0 && "TODO");
        break;
    case BO_Mul:
    case BO_Div:
    case BO_Rem:
    case BO_Add:
    case BO_Sub:
    case BO_Shl:
    case BO_Shr:
    case BO_LE:
    case BO_LT:
    case BO_GE:
    case BO_GT:
    case BO_NE:
    case BO_EQ:
    case BO_And:
    case BO_Xor:
    case BO_Or:
    case BO_LAnd:
    case BO_LOr:
        check(TLeft, binop->getLHS());
        check(TLeft, binop->getRHS());
        break;
    case BO_Assign:
    case BO_MulAssign:
    case BO_DivAssign:
    case BO_RemAssign:
    case BO_AddAssign:
    case BO_SubAssign:
    case BO_ShlAssign:
    case BO_ShrAssign:
    case BO_AndAssign:
    case BO_XorAssign:
    case BO_OrAssign:
        assert(0 && "TODO");
        break;
    case BO_Comma:
        assert(0 && "TODO?");
        break;
    }
}

bool ExprTypeAnalyser::checkCompatible(QualType left, const Expr* expr) const {
    QualType right = expr->getType();
    //right = TypeFinder::findType(expr);
    assert(left.isValid());
    const Type* canon = left.getCanonicalType();
    switch (canon->getTypeClass()) {
    case TC_BUILTIN:
        return checkBuiltin(left, right, expr, true);
    case TC_POINTER:
        return checkPointer(left, right, expr);
    case TC_ARRAY:
        break;
    case TC_UNRESOLVED:
        break;
    case TC_ALIAS:
        break;
    case TC_STRUCT:
        break;
    case TC_ENUM:
        break;
    case TC_FUNCTION:
        return checkFunction(left, expr);
    case TC_PACKAGE:
        assert(0 && "TODO");
        break;
    }
    return false;
}

bool ExprTypeAnalyser::checkBuiltin(QualType left, QualType right, const Expr* expr, bool first) const {
    if (right->isBuiltinType()) {
        // NOTE: canonical is builtin, var itself my be UnresolvedType etc
        const BuiltinType* Right = cast<BuiltinType>(right.getCanonicalType());
        const BuiltinType* Left = cast<BuiltinType>(left.getCanonicalType());
        int rule = type_conversions[Right->getKind()][Left->getKind()];
        // 0 = ok, 1 = loss of precision, 2 sign-conversion, 3=float->integer, 4 incompatible, 5 loss of FP prec.
        // TODO use matrix with allowed conversions: 3 options: ok, error, warn
        int errorMsg = 0;

        if (first) {
            if (Right->getKind() != Left->getKind()) {
                // add Implicit Cast
                // TODO remove const cast
                Expr* E = const_cast<Expr*>(expr);
                E->setImpCast(Left->getKind());
            }
            if (rule == 1) {
                QualType Q = TypeFinder::findType(expr);
                return checkBuiltin(left, Q, expr, false);
            }
        }

        switch (rule) {
        case 0:
            return true;
        case 1: // loss of precision
            errorMsg = diag::warn_impcast_integer_precision;
            break;
        case 2: // sign-conversion
            errorMsg = diag::warn_impcast_integer_sign;
            break;
        case 3: // float->integer
            errorMsg = diag::warn_impcast_float_integer;
            break;
        case 4: // incompatible
            errorMsg = diag::err_illegal_type_conversion;
            break;
        case 5: // loss of fp-precision
            errorMsg = diag::warn_impcast_float_precision;
            break;
        default:
            assert(0 && "should not come here");
        }
        StringBuilder buf1(MAX_LEN_TYPENAME);
        StringBuilder buf2(MAX_LEN_TYPENAME);
        right.DiagName(buf1);
        left.DiagName(buf2);
        // TODO error msg depends on conv type (see clang errors)
        Diags.Report(expr->getLocation(), errorMsg) << buf1 << buf2
            << expr->getSourceRange();
        return false;
    }
    error(expr->getLocation(), left, right);
    return false;
}

bool ExprTypeAnalyser::checkPointer(QualType left, QualType right, const Expr* expr) const {
    if (right->isPointerType()) {
        // TODO
        return true;
    }
    if (right->isArrayType()) {
        // TODO
        return true;
    }
    error(expr->getLocation(), left, right);
    return false;
}

bool ExprTypeAnalyser::checkFunction(QualType L, const Expr* expr) const {
    QualType R = expr->getType();
    // NOTE: for now only allow FunctionTypes
    if (!R->isFunctionType()) {
        error(expr->getLocation(), L, R);
        return false;
    }
#if 0
    const FunctionType* FLeft = cast<FunctionType>(L.getTypePtr());
    const FunctionType* FRight = cast<FunctionType>(R.getTypePtr());
    const FunctionDecl* DL = FLeft->getDecl();
    const FunctionDecl* DR = FRight->getDecl();
    // TODO compare
#endif


    return true;
}

void ExprTypeAnalyser::error(SourceLocation loc, QualType left, QualType right) const {
    StringBuilder buf1(MAX_LEN_TYPENAME);
    StringBuilder buf2(MAX_LEN_TYPENAME);
    right.DiagName(buf1);
    left.DiagName(buf2);
    // TODO error msg depends on conv type (see clang errors)
    Diags.Report(loc, diag::err_illegal_type_conversion)
            << buf1 << buf2;

}
