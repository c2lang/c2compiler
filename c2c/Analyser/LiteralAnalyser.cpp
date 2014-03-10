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

#include <stdio.h>

#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/APInt.h>
#include <clang/Parse/ParseDiagnostic.h>
#include <clang/Sema/SemaDiagnostic.h>

#include "Analyser/LiteralAnalyser.h"
#include "AST/Expr.h"
#include "AST/Decl.h"
#include "Utils/StringBuilder.h"

using namespace C2;
using namespace llvm;
using namespace clang;

struct Limit {
    int64_t minVal;
    uint64_t maxVal;
    const char* minStr;
    const char* maxStr;
};

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
};

static const Limit* getLimit(int width) {
    switch (width) {
    case 1: return &limits[0];
    case 7: return &limits[1];
    case 8: return &limits[2];
    case 15: return &limits[3];
    case 16: return &limits[4];
    case 31: return &limits[5];
    case 32: return &limits[6];
    default:
        fprintf(stderr, "UNHANDLED width %d\n", width);
        assert(0 && "todo");
        return 0;
    }
}

LiteralAnalyser::LiteralAnalyser(clang::DiagnosticsEngine& Diags_)
    : Diags(Diags_)
{
}

void LiteralAnalyser::check(QualType TLeft, Expr* Right) {
    if (Right->getCTC() == CTC_NONE) return;

    const QualType QT = TLeft->getCanonicalType();
    int availableWidth = 0;
    //bool isSigned = false;
    QualType wanted = TLeft;
    if (QT.isBuiltinType()) {
        const BuiltinType* TL = cast<BuiltinType>(QT);
        availableWidth = TL->getIntegerWidth();
        //isSigned = TL->isSignedInteger();
    } else if (QT.isPointerType()) {
        availableWidth = 32;    // only 32-bit for now
        //isSigned = false;
        // dont ask for pointer, replace with uint32 here.
        wanted = BuiltinType::get(BuiltinType::UInt32);
    } else {
        QT.dump();
        assert(0 && "todo");
    }

    APSInt Result = checkLiterals(wanted, Right);

    const Limit* L = getLimit(availableWidth);
    assert(Result.isSigned() && "TEMP FOR NOW");
    int64_t value = Result.getSExtValue();
    bool overflow = false;
    if (Result.isNegative()) {
        const int64_t limit = L->minVal;
        if (value < limit) overflow = true;
    } else {
        const int64_t limit = (int64_t)L->maxVal;
        if (value > limit) overflow = true;
    }
    //fprintf(stderr, "VAL=%lld  width=%d signed=%d\n", value, availableWidth, Result.isSigned());
    if (overflow) {
        SmallString<20> ss;
        Result.toString(ss, 10, true);

        StringBuilder buf1;
        TLeft->DiagName(buf1);

        Diags.Report(Right->getLocStart(), diag::err_literal_outofbounds)
            << buf1 << L->minStr << L->maxStr << ss << Right->getSourceRange();
    }
}

APSInt LiteralAnalyser::checkLiterals(QualType TLeft, Expr* Right) {
    if (Right->getCTC() == CTC_NONE) return APSInt();

    switch (Right->getKind()) {
    case EXPR_INTEGER_LITERAL:
        return checkIntegerLiterals(TLeft, Right);
    case EXPR_FLOAT_LITERAL:
    case EXPR_BOOL_LITERAL:
    case EXPR_CHAR_LITERAL:
    case EXPR_STRING_LITERAL:
        break;
    case EXPR_NIL:
        break;
    case EXPR_IDENTIFIER:
        return checkIdentifier(TLeft, Right);
    case EXPR_TYPE:
    case EXPR_CALL:
    case EXPR_INITLIST:
        break;
    case EXPR_DECL:
        break;
    case EXPR_BINOP:
        return checkBinaryLiterals(TLeft, Right);
    case EXPR_CONDOP:
        break;
    case EXPR_UNARYOP:
        return checkUnaryLiterals(TLeft, Right);
    case EXPR_BUILTIN:
    case EXPR_ARRAYSUBSCRIPT:
    case EXPR_MEMBER:
        break;
    case EXPR_PAREN:
        {
            ParenExpr* P = cast<ParenExpr>(Right);
            return checkLiterals(TLeft, P->getExpr());
        }
    }
    return APSInt();
}

APSInt LiteralAnalyser::checkIntegerLiterals(QualType TLeft, Expr* Right) {
    IntegerLiteral* I = cast<IntegerLiteral>(Right);

    APSInt Result(64, false);      // always take signed 64 as base for checking
    Result = I->Value;
    return Result;
}

APSInt LiteralAnalyser::checkUnaryLiterals(QualType TLeft, Expr* Right) {
    UnaryOperator* unaryop = cast<UnaryOperator>(Right);
    QualType LType;
    switch (unaryop->getOpcode()) {
    case UO_PostInc:
    case UO_PostDec:
    case UO_PreInc:
    case UO_PreDec:
        break;
    case UO_AddrOf:
    case UO_Deref:
    case UO_Plus:
        // TODO
        break;
    case UO_Minus:
        {
            APSInt Result = checkLiterals(TLeft, unaryop->getExpr());
            APInt invert(64, -1, true);
            APSInt I(invert, false);
            Result *= I;
            return Result;
        }
    case UO_Not:
    case UO_LNot:
        // TODO
        break;
    default:
        assert(0 && "TODO");
        break;
    }
    return APSInt();
}

APSInt LiteralAnalyser::checkBinaryLiterals(QualType TLeft, Expr* Right) {
    BinaryOperator* binop = cast<BinaryOperator>(Right);
    QualType LType;
    switch (binop->getOpcode()) {
    case BO_PtrMemD:
    case BO_PtrMemI:
        // TODO
        break;
    case BO_Mul:
    case BO_Div:
    case BO_Rem:
        // TODO
        break;
    case BO_Add:
        {
            APSInt L = checkLiterals(TLeft, binop->getLHS());
            APSInt R = checkLiterals(TLeft, binop->getRHS());
            return L + R;
        }
    case BO_Sub:
        {
            APSInt L = checkLiterals(TLeft, binop->getLHS());
            APSInt R = checkLiterals(TLeft, binop->getRHS());
            return L - R;
        }
    case BO_Shl:
    case BO_Shr:
        break;
    case BO_LT:
    case BO_GT:
    case BO_LE:
    case BO_GE:
    case BO_EQ:
    case BO_NE:
    {
        // TODO check left/right values + set QualType
        // TEMP always return 1 (!false)
        APSInt Result(64, false);
        Result = 1;

        return Result;
    }
    case BO_And:
    case BO_Xor:
    case BO_Or:
    case BO_LAnd:
    case BO_LOr:
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
    case BO_Comma:
        // TODO
        break;
    }
    return APSInt();
}

APSInt LiteralAnalyser::checkIdentifier(QualType TLeft, Expr* Right) {
    IdentifierExpr* I = cast<IdentifierExpr>(Right);
    const Decl* D = I->getDecl();
    assert(D);
    const EnumConstantDecl* ECD = dyncast<EnumConstantDecl>(D);
    if (ECD) {
        APSInt Result;
        Result.setIsSigned(false);         // TODO set depending on enum type
        Result = ECD->getValue();
        return Result;
    }
    return APSInt();
}

