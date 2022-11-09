/* Copyright 2013-2022 Bas van den Berg
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

#include <stdio.h>
#include <assert.h>

#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/APInt.h>
#include "Clang/ParseDiagnostic.h"
#include "Clang/SemaDiagnostic.h"

#include "AST/Type.h"
#include "Analyser/CTVAnalyser.h"
#include "AST/Expr.h"
#include "AST/Decl.h"
#include "Utils/StringBuilder.h"
#include "Utils/Utils.h"

using namespace C2;
using namespace llvm;
using namespace c2lang;

namespace C2 {
struct Limit {
    int64_t minVal;
    uint64_t maxVal;
    const char* minStr;
    const char* maxStr;
};
}

// clang-format off
static const Limit limits [] = {
    // bool
    {         0,          1,           "0",         "1" },
    // int8
    {       -128,        127,        "-128",       "127" },
    // uint8
    {          0,        255,           "0",        "255" },
    // int16
    {      -32768,      32767,      "-32768",      "32767" },
    // uint16
    {          0,      65535,           "0",      "65535" },
    // int32
    { -2147483648, 2147483647, "-2147483648", "2147483647" },
    // uint32
    {          0, 4294967295,           "0", "4294967295" },
    // int64
    // NOTE: minimum should be -..808, but clang complains about it..
    {-9223372036854775807ll,  9223372036854775807llu, "-9223372036854775808", "9223372036854775807" },
    // uint64
    {                   0,  18446744073709551615llu, "0", "18446744073709551615" },
};
// clang-format on

static const Limit* getLimit(int width) {
    switch (width) {
    case 1: return &limits[0];
    case 7: return &limits[1];
    case 8: return &limits[2];
    case 15: return &limits[3];
    case 16: return &limits[4];
    case 31: return &limits[5];
    case 32: return &limits[6];
    case 63: return &limits[7];
    case 64: return &limits[8];
    default:
        fprintf(stderr, "UNHANDLED width %d\n", width);
        TODO;
        return 0;
    }
}

CTVAnalyser::CTVAnalyser(c2lang::DiagnosticsEngine& Diags_)
    : Diags(Diags_)
{
}

void CTVAnalyser::check(QualType TLeft, const Expr* Right) {
    if (!Right->isCTV()) return;
    // TODO assert here instead of check?


    // special case for assignments to enums
    if (TLeft.isEnumType()) {
        // dont check value if right is also same enum type
        if (TLeft.same_base_type(Right->getType())) return;

        // TODO should be done elsewhere (checking if conversion is allowed)
        //fprintf(stderr, "TODO refactor checking!!, type conversion not allowed\n");
        //TODO;
#if 0
        // this part should be used when checking casting CTC's to Enum types
        APSInt Result = checkLiterals(Right);

        // check if value has matching enum constant
        const EnumType* ET = cast<EnumType>(TLeft.getTypePtr());
        const EnumTypeDecl* ETD = ET->getDecl();
        assert(ETD);
        if (!ETD->hasConstantValue(Result)) {
            fprintf(stderr, "NO SUCH CONSTANT\n");

        }
#endif
        return;
    }

    int availableWidth = 0;
    if (!calcWidth(TLeft, Right, &availableWidth)) return;
    StringBuilder tname(128);
    TLeft.DiagName(tname, false);
    const Limit* L = getLimit(availableWidth);

    checkWidth(availableWidth, L, Right, tname);
}

void CTVAnalyser::checkWidth(int availableWidth, const Limit* L, const Expr* Right, const char* tname) {
    APSInt Result = checkLiterals(Right);

    // TEMP dont check for any 64-bit
    if (availableWidth == 64) return;
    assert(Result.isSigned() && "TEMP FOR NOW");
    int64_t value = Result.getSExtValue();
    bool overflow = false;
    if (Result.isNegative()) {
        const int64_t limit = L->minVal;
        if (value < limit) overflow = true;
    } else {
        if (availableWidth == 64) {
            // NOTE: assume for now value always fits in uint64
        } else {
            const int64_t limit = (int64_t)L->maxVal;
            if (value > limit) overflow = true;
        }
    }
    //fprintf(stderr, "VAL=%lld  width=%d signed=%d\n", value, availableWidth, Result.isSigned());
    if (overflow) {
        SmallString<20> ss;
        Result.toString(ss, 10, true);
        Diags.Report(Right->getLocStart(), diag::err_literal_outofbounds)
                << tname << L->minStr << L->maxStr << ss << Right->getSourceRange();
    }
}

bool CTVAnalyser::calcWidth(QualType TLeft, const Expr* Right, int* availableWidth) {
    const QualType QT = TLeft.getCanonicalType();
    // TODO check if type is already ok?, then skip check?
    //if (QT == Right->getType().getCanonicalType()) return;
    if (QT.isBuiltinType()) {
        const BuiltinType* TL = cast<BuiltinType>(QT);
        if (!TL->isInteger()) {
            // TODO floats
            return false;
        }
        // TODO remove const cast
        Expr* EE = const_cast<Expr*>(Right);
        QualType Canon = EE->getType().getCanonicalType();
        assert(Canon->isBuiltinType());
        const BuiltinType* BI = cast<BuiltinType>(Canon);
        if (TL->getKind() != BI->getKind()) EE->setImpCast(TL->getKind());
        if (QT == Type::Bool()) {
            // NOTE: any integer to bool is ok
            return false;
        }

        *availableWidth = TL->getIntegerWidth();
    } else if (QT.isPointerType()) {
        *availableWidth = 32;    // only 32-bit for now
        // dont ask for pointer, replace with uint32 here.
    } else {
        TLeft.dump();
        Right->dump();
        StringBuilder t1name(128);
        Right->getType().DiagName(t1name, false);
        // Q: allow FuncPtr to return 0? (or nil?)
        StringBuilder t2name(128);
        TLeft.DiagName(t2name, false);
        // TODO pass correct args (not returning .. from function with incompatible ..
        // TODO this error cannot be generated here since we dont know the situation
        Diags.Report(Right->getLocation(), diag::err_typecheck_convert_incompatible) << t1name << t2name << 2 << 0 << 0;
        return false;
        //QT.dump();
        //assert(0 && "todo");
    }

    return true;
}

APSInt CTVAnalyser::checkLiterals(const Expr* Right) {
    if (!Right->isCTV()) return APSInt(64, false);

    APSInt result(64, false);

    switch (Right->getKind()) {
    case EXPR_INTEGER_LITERAL: {
        const IntegerLiteral* I = cast<IntegerLiteral>(Right);
        result = I->Value;
        break;
    }
    case EXPR_FLOAT_LITERAL:
        TODO;
        break;
    case EXPR_BOOL_LITERAL: {
        const BooleanLiteral* B = cast<BooleanLiteral>(Right);
        result = B->getValue();
        break;
    }
    case EXPR_CHAR_LITERAL:
    {
        const CharacterLiteral* C = cast<CharacterLiteral>(Right);
        result = APInt(64, C->getValue(), true);
        break;
    }
    case EXPR_STRING_LITERAL:
        break;
    case EXPR_NIL:
        break;
    case EXPR_IDENTIFIER:
        return checkDecl(cast<IdentifierExpr>(Right)->getDecl());
    case EXPR_TYPE:
    case EXPR_CALL:
    case EXPR_INITLIST:
        break;
    case EXPR_DESIGNATOR_INIT:
        TODO;
        break;
    case EXPR_BINOP:
        return checkBinaryLiterals(Right);
    case EXPR_CONDOP:
        break;
    case EXPR_UNARYOP:
        return checkUnaryLiterals(Right);
    case EXPR_BUILTIN:
    {
        const BuiltinExpr* B = cast<BuiltinExpr>(Right);
        return B->getValue();
    }
    case EXPR_ARRAYSUBSCRIPT:
        return checkArraySubscript(Right);
    case EXPR_MEMBER:
    {
        // Q: is this correct for Struct.Member?
        const MemberExpr* M = cast<MemberExpr>(Right);
        return checkDecl(M->getDecl());
    }
    case EXPR_PAREN:
    {
        const ParenExpr* P = cast<ParenExpr>(Right);
        return checkLiterals(P->getExpr());
    }
    case EXPR_BITOFFSET:
        TODO;
        break;
    case EXPR_EXPLICIT_CAST:
    {
        // a cast may change the value without warning
        const ExplicitCastExpr* E = cast<ExplicitCastExpr>(Right);
        APSInt Result = checkLiterals(E->getInner());
        return truncateLiteral(E->getType(), Right, Result);
    }
    case EXPR_IMPLICIT_CAST: {
        const ImplicitCastExpr* ic = cast<ImplicitCastExpr>(Right);
        return checkLiterals(ic->getInner());
    }
    }
    return result;
}

void CTVAnalyser::checkBitOffset(const Expr* Left, const Expr* Right) {
    assert(isa<ArraySubscriptExpr>(Left));
    const ArraySubscriptExpr* A = cast<ArraySubscriptExpr>(Left);
    assert(isa<BitOffsetExpr>(A->getIndex()));
    const BitOffsetExpr* BO = cast<BitOffsetExpr>(A->getIndex());

    StringBuilder tname;
    tname << "unsigned:" << BO->getWidth();
    Limit L;
    L.minVal = 0;
    L.maxVal = (((uint64_t) 1)<<BO->getWidth()) -1;
    // TODO do something special for width 64?
    StringBuilder maxVal;
    maxVal << L.maxVal;
    L.minStr = "0";
    L.maxStr = (const char*)maxVal;
    checkWidth(BO->getWidth(), &L, Right, tname);
}

bool CTVAnalyser::checkRange(QualType TLeft, const Expr* Right, c2lang::SourceLocation Loc, llvm::APSInt Result) {
    // TODO refactor with check()
    const QualType QT = TLeft.getCanonicalType();
    int availableWidth = 0;
    if (QT.isBuiltinType()) {
        const BuiltinType* TL = cast<BuiltinType>(QT);
        if (!TL->isInteger()) {
            // TODO floats
            return false;
        }
        availableWidth = TL->getIntegerWidth();
    } else {
        QT.dump();
        TODO;
    }

    const Limit* L = getLimit(availableWidth);
    assert(Result.isSigned() && "TEMP FOR NOW");
    int64_t value = Result.getSExtValue();
    bool overflow = false;
    if (Result.isNegative()) {
        const int64_t limit = L->minVal;
        if (value < limit) overflow = true;
    } else {
        int64_t limit = (int64_t)L->maxVal;
        if ((int64_t)L->maxVal == -1) { // for 64-bit, TEMP just use max int64
            limit = 9223372036854775807llu;
        }
        if (value > limit) overflow = true;
    }
    if (overflow) {
        SmallString<20> ss;
        Result.toString(ss, 10, true);

        StringBuilder buf1;
        TLeft.DiagName(buf1, false);

        if (Right) {
            Diags.Report(Right->getLocStart(), diag::err_literal_outofbounds)
                    << buf1 << L->minStr << L->maxStr << ss << Right->getSourceRange();
        } else {
            Diags.Report(Loc, diag::err_literal_outofbounds)
                    << buf1 << L->minStr << L->maxStr << ss;
        }
        return false;
    }
    return true;
}

APSInt CTVAnalyser::checkUnaryLiterals(const Expr* Right) {
    const UnaryOperator* unaryop = cast<UnaryOperator>(Right);
    QualType LType;
    switch (unaryop->getOpcode()) {
    case UO_PostInc:
    case UO_PostDec:
    case UO_PreInc:
    case UO_PreDec:
        break;
    case UO_AddrOf:
    case UO_Deref:
        TODO;
        break;
    case UO_Minus:
        return -checkLiterals(unaryop->getExpr());
    case UO_Not:
    {
        APSInt Result = checkLiterals(unaryop->getExpr());
        Result = Result == 0 ? 1 : 0;
        return Result;
    }
    case UO_LNot:
        return ~checkLiterals(unaryop->getExpr());
    }
    return APSInt();
}

static inline int evaluateBinaryComparison(APSInt &lhs, APSInt &rhs, c2lang::BinaryOperatorKind opcode)
{
    switch (opcode) {
        case BINOP_LT: return lhs < rhs;
        case BINOP_GT: return lhs > rhs;
        case BINOP_LE: return lhs <= rhs;
        case BINOP_GE: return lhs >= rhs;
        case BINOP_EQ: return lhs == rhs;
        case BINOP_NE: return lhs != rhs;
        case BINOP_LAnd: return !(lhs == 0 || rhs == 0);
        case BINOP_LOr: return !(lhs == 0 && rhs == 0);
        default: FATAL_ERROR("Unreachable statement");
    }
}

static inline APSInt evaluateBinaryBitwiseOp(APSInt &lhs, APSInt &rhs, c2lang::BinaryOperatorKind opcode)
{
    switch (opcode) {
        case BINOP_And: return lhs & rhs;
        case BINOP_Xor: return lhs ^ rhs;
        case BINOP_Or: return lhs | rhs;
        default: FATAL_ERROR("Unreachable statement");
    }
}


APSInt CTVAnalyser::checkBinaryLiterals(const Expr *Right) {
    const BinaryOperator *binop = cast<BinaryOperator>(Right);
    Expr *lhs = binop->getLHS();
    Expr *rhs = binop->getRHS();
    APSInt L = checkLiterals(lhs);
    APSInt R = checkLiterals(rhs);

    switch (binop->getOpcode()) {

    case BINOP_Mul:
        return L * R;
    case BINOP_Div:
        if (R == 0) {
            Diags.Report(rhs->getLocation(), diag::warn_remainder_division_by_zero) << 1;
            break;
        }
        return L / R;
    case BINOP_Rem:
        if (R == 0) {
            Diags.Report(rhs->getLocation(), diag::warn_remainder_division_by_zero) << 0;
            break;
        }
        return L % R;
    case BINOP_Add:
        return L + R;
    case BINOP_Sub:
        return L - R;
    case BINOP_Shl:
        if (L.isNegative()) {
            Diags.Report(lhs->getLocation(), diag::warn_shift_lhs_negative) << 0;
            break;
        }
        if (R.isNegative()) {
            Diags.Report(rhs->getLocation(), diag::warn_shift_negative) << 0;
            break;
        }
        // TODO warn about overflow in the correct manner.
        return L << R.getExtValue();
    case BINOP_Shr:
        if (L.isNegative()) {
            Diags.Report(lhs->getLocation(), diag::warn_shift_lhs_negative) << 0;
            break;
        }
        if (R.isNegative()) {
            Diags.Report(rhs->getLocation(), diag::warn_shift_negative) << 0;
            break;
        }
        {
            uint64_t rightHandSide = (uint64_t)R.getExtValue();
            if (rightHandSide > L.getBitWidth()) {
                Diags.Report(rhs->getLocation(), diag::warn_shift_gt_typewidth) << 0;
                break;
            }
            return L >> rightHandSide;
        }
    case BINOP_LT:
    case BINOP_GT:
    case BINOP_LE:
    case BINOP_GE:
    case BINOP_EQ:
    case BINOP_LAnd:
    case BINOP_LOr:
    case BINOP_NE: {
        APSInt result(std::max(L.getBitWidth(), R.getBitWidth()), false);
        result = evaluateBinaryComparison(L, R, binop->getOpcode());
        return result;
    }
    case BINOP_And:
    case BINOP_Xor:
    case BINOP_Or:
        return evaluateBinaryBitwiseOp(L, R, binop->getOpcode());
    case BINOP_Comma:
        return R;
    case BINOP_Assign:
    case BINOP_MulAssign:
    case BINOP_DivAssign:
    case BINOP_RemAssign:
    case BINOP_AddAssign:
    case BINOP_SubAssign:
    case BINOP_ShlAssign:
    case BINOP_ShrAssign:
    case BINOP_AndAssign:
    case BINOP_XorAssign:
    case BINOP_OrAssign:
        Diags.Report(lhs->getLocation(), diag::err_typecheck_expression_not_modifiable_lvalue);
        break;
    }
    return APSInt();
}

APSInt CTVAnalyser::checkArraySubscript(const Expr* Right) {
    const ArraySubscriptExpr* AS = cast<ArraySubscriptExpr>(Right);
    assert(AS);
    assert(isa<BitOffsetExpr>(AS->getIndex()) && "TODO only bitoffsets for now");

    APSInt base = checkLiterals(AS->getBase());

    const BitOffsetExpr* BO = cast<BitOffsetExpr>(AS->getIndex());
    APSInt low = checkLiterals(BO->getRHS());
    unsigned width = BO->getWidth();

    // calculate result = ((base >> low) & bitmask(width));
    uint64_t result = base.getZExtValue();
    result >>= low.getZExtValue();
    result &= Utils::bitmask(width);

    APSInt Result(64, false);
    Result = result;
    return Result;
}

APSInt CTVAnalyser::checkDecl(const Decl* D) {
    assert(D);
    const EnumConstantDecl* ECD = dyncast<EnumConstantDecl>(D);
    if (ECD) return ECD->getValue();

    const VarDecl* VD = dyncast<VarDecl>(D);
    if (VD) {
        // VarDecl should be CTC_FULL here!
        // Only check value (=initial value)
        assert(VD->getInitValue());
        return checkLiterals(VD->getInitValue());
    }
    FATAL_ERROR("Unreachable");
    return APSInt();
}

APSInt CTVAnalyser::truncateLiteral(QualType TLeft, const Expr* Right, APSInt Orig) {
    int availableWidth = 0;
    // TODO needs cleanup (first check if conversions are ok, then check literal values?)
    if (!calcWidth(TLeft, Right, &availableWidth)) return APSInt(0);

    return Orig.trunc(availableWidth);
}

