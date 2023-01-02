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

#include <assert.h>
#include "Clang/SemaDiagnostic.h"

#include "Analyser/ExprAnalyser.h"
#include "Analyser/CTVAnalyser.h"
#include "Analyser/AnalyserConstants.h"
#include "Analyser/AnalyserUtils.h"
#include "AST/Expr.h"
#include "AST/Decl.h"
#include "Utils/TargetInfo.h"
#include "Utils/Errors.h"

using namespace C2;
using namespace llvm;
using namespace c2lang;

// clang-format off

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
// clang-format on


ExprAnalyser::ExprAnalyser(c2lang::DiagnosticsEngine& Diags_,
                           const TargetInfo& target_,
                           ASTContext& context_)
    : Diags(Diags_)
    , target(target_)
    , Context(context_)
    , m_hasError(false)
    , buf1(MAX_LEN_TYPENAME)
    , buf2(MAX_LEN_TYPENAME)
{}

void ExprAnalyser::check(QualType TLeft, Expr* expr) {
    m_hasError = false;

    if (expr->isCTV()) {
        CTVAnalyser LA(Diags);
        LA.check(TLeft, expr);
        // TODO add if (!LA.hasError()) m_hasError = true;
        return;
    }

    checkCompatible(TLeft, expr);
}

bool ExprAnalyser::checkExplicitCast(const ExplicitCastExpr* expr, QualType DestType, QualType SrcType) {
    m_hasError = false;
    // C99 6.5.4p2: the cast type needs to be void or scalar and the expression

    if (!DestType.isScalarType()) {
        // Dont allow any cast to non-scalar
        DestType.DiagName(buf1, true);
        SrcType.DiagName(buf2, true);
        Diags.Report(expr->getLocation(), diag::err_typecheck_cond_expect_scalar)
                << buf1 << buf2 << expr->getSourceRange();
        m_hasError = true;
        return false;
    }

    // If either type is a pointer, the other type has to be either an
    // integer or a pointer
    // TODO decide if Enums are arithmatic types or not (they are in C99, not is C++0x)
    if (DestType.isPointerType()) {
        if (SrcType.isPointerType()) {
            // allow all pointer casts
            return true;
        } else {
            return checkWidth(SrcType.getCanonicalType(), expr->getLocation(), diag::err_cast_nonword_to_pointer);
        }
    } else {
        if (SrcType.isPointerType()) {
            return checkWidth(DestType.getCanonicalType(), expr->getLocation(), diag::err_cast_pointer_to_nonword);
        } else {
            // check non-pointer to non-pointer type
            // TODO make this top level function? (switch on src-type)
            return checkNonPointerCast(expr, DestType, SrcType);
        }
    }
    return false;
}

bool ExprAnalyser::checkNonPointerCast(const ExplicitCastExpr* expr, QualType DestType, QualType SrcType) {
    // by now: DestType isScalar(): Bool, Arithmetic, Function or Enum

    QualType C = SrcType.getCanonicalType();
    switch (C->getTypeClass()) {
    case TC_BUILTIN:
        return checkBuiltinCast(expr, DestType, SrcType);
    case TC_POINTER:
        FATAL_ERROR("Unreachable");
        return false;
    case TC_ARRAY:
        TODO;
        break;
    case TC_REF:
        TODO;
        break;
    case TC_ALIAS:
        C.dump();
        FATAL_ERROR("Unreachable");
        return false;
    case TC_STRUCT:
        // no casts allowed
        break;
    case TC_ENUM:
        return checkEnumCast(expr, DestType, SrcType);
    case TC_FUNCTION:
        return checkFunctionCast(expr, DestType, SrcType);
    case TC_MODULE:
        // no casts allowed
        break;
    }

    // TODO refactor duplicate code (after completion)
    SrcType.DiagName(buf1, true);
    DestType.DiagName(buf2, true);
    Diags.Report(expr->getLocation(), diag::err_illegal_cast)
            << buf1 << buf2 << expr->getSourceRange();
    m_hasError = true;
    return false;
}

bool ExprAnalyser::checkBuiltinCast(const ExplicitCastExpr* expr, QualType DestType, QualType SrcType) {
    // by now: DestType isScalar(): Bool, Arithmetic, Function or Enum
    const BuiltinType* Right = cast<BuiltinType>(SrcType.getCanonicalType());

    QualType C = DestType.getCanonicalType();
    switch (C->getTypeClass()) {
    case TC_BUILTIN:
    {
        const BuiltinType* Left = cast<BuiltinType>(DestType.getCanonicalType());
        int rule = type_conversions[Right->getKind()][Left->getKind()];
        switch (rule) {
        case 0:
        case 1: // loss of precision
        case 2: // sign-conversion
        case 3: // float->integer
            break;
        case 4: // incompatible
        {
            DestType.DiagName(buf1, true);
            SrcType.DiagName(buf2, true);
            Diags.Report(expr->getLocation(), diag::err_illegal_cast)
                    << buf1 << buf2 << expr->getSourceRange();
            m_hasError = true;
            return false;
        }
        case 5: // loss of fp-precision
            break;
        default:
            FATAL_ERROR("Unreachable");
        }
        return true;
    }
    case TC_POINTER:
        FATAL_ERROR("Unreachable");
        return false;
    case TC_ARRAY:
        TODO;
        break;
    case TC_REF:
    case TC_ALIAS:
    case TC_STRUCT:
    case TC_ENUM:
        FATAL_ERROR("Unreachable");
        return false;
    case TC_FUNCTION:
        // only allow if uint32/64 (ptr size)
        if (((target.intWidth == 64) && (Right->getKind() != BuiltinType::UInt64)) ||
            ((target.intWidth == 32) && (Right->getKind() != BuiltinType::UInt32))) {
            SrcType.DiagName(buf1, true);
            DestType.DiagName(buf2, true);
            Diags.Report(expr->getLocation(), diag::warn_int_to_pointer_cast) << buf1 << buf2;
            m_hasError = true;
            return false;
        }
        break;
    case TC_MODULE:
        FATAL_ERROR("Unreachable");
        return false;
    }
    return true;
}

bool ExprAnalyser::checkEnumCast(const ExplicitCastExpr* expr, QualType DestType, QualType SrcType) {
    // by now: DestType is: Bool, Integer, Function or Enum
    switch (DestType->getTypeClass()) {
    case TC_BUILTIN:
        return true;    // allow
    case TC_POINTER:
    case TC_ARRAY:
    case TC_REF:
    case TC_ALIAS:
    case TC_STRUCT:
        FATAL_ERROR("Unreachable");
        return false;
    case TC_ENUM:
        return true;    // allow
    case TC_FUNCTION:
        break;          // deny
    case TC_MODULE:
        FATAL_ERROR("Unreachable");
        return false;
    }
    SrcType.DiagName(buf1, true);
    DestType.DiagName(buf2, true);
    Diags.Report(expr->getLocation(), diag::err_illegal_cast)
            << buf1 << buf2 << expr->getSourceRange();
    m_hasError = true;
    return false;
}

bool ExprAnalyser::checkFunctionCast(const ExplicitCastExpr* expr, QualType DestType, QualType SrcType) {
    switch (DestType->getTypeClass()) {
    case TC_BUILTIN:
        // TODO use  warn_int_to_void_pointer_cast, remove err_cast_pointer_to_nonword
        return checkWidth(DestType.getCanonicalType(), expr->getLocation(), diag::err_cast_pointer_to_nonword);
    case TC_POINTER:
    case TC_ARRAY:
    case TC_REF:
    case TC_ALIAS:
    case TC_STRUCT:
        FATAL_ERRORF("Test %d", 1);
        FATAL_ERROR("Unreachable");
        return false;
    case TC_ENUM:
        break;          // deny
    case TC_FUNCTION:
    {
        // Always allow TEMP
        return true;
/*
        // check other function proto, allow if same
        const FunctionType* src = cast<FunctionType>(SrcType);
        const FunctionType* dest = cast<FunctionType>(DestType);
        if (FunctionType::sameProto(src, dest)) return true;
        break;  // deny
*/
    }
    case TC_MODULE:
        FATAL_ERROR("Unreachable");
        return false;
    }
    SrcType.DiagName(buf1, true);
    DestType.DiagName(buf2, true);
    Diags.Report(expr->getLocation(), diag::err_illegal_cast)
            << buf1 << buf2 << expr->getSourceRange();
    m_hasError = true;
    return false;
}

void ExprAnalyser::checkUnaryOp(QualType TLeft, const UnaryOperator* op) {
        switch (op->getOpcode()) {
        case c2lang::UO_PostInc:
        case c2lang::UO_PostDec:
        case c2lang::UO_PreInc:
        case c2lang::UO_PreDec:
        case c2lang::UO_AddrOf:
        case c2lang::UO_Deref:
        case c2lang::UO_Minus:
        case c2lang::UO_Not:
        case c2lang::UO_LNot:
            // TODO
            break;
        }
}

void ExprAnalyser::checkBinOp(QualType TLeft, Expr* binop_ptr) {
    BinaryOperator* binop = cast<BinaryOperator>(binop_ptr);
    // NOTE we check Left / Right separately if CTC's are not the same
    switch (binop->getOpcode()) {
    case BINOP_Mul:
    case BINOP_Div:
    case BINOP_Rem:
    case BINOP_Add:
    case BINOP_Sub:
    case BINOP_Shl:
    case BINOP_Shr:
    case BINOP_LE:
    case BINOP_LT:
    case BINOP_GE:
    case BINOP_GT:
    case BINOP_NE:
    case BINOP_EQ:
    case BINOP_And:
    case BINOP_Xor:
    case BINOP_Or:
    case BINOP_LAnd:
    case BINOP_LOr:
        check(TLeft, binop->getLHS());
        check(TLeft, binop->getRHS());
        break;
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
        TODO;
        break;
    case BINOP_Comma:
        TODO;
        break;
    }
}

// TODO change return type to void, it's never used
bool ExprAnalyser::checkCompatible(QualType left, Expr* expr) {
    QualType right = expr->getType();
    assert(left.isValid());
    const Type* canon = left.getCanonicalType();
    assert(canon);
    switch (canon->getTypeClass()) {
    case TC_BUILTIN:
        return checkBuiltin(left, right, expr, true);
    case TC_POINTER:
        return checkPointer(left, right, expr);
    case TC_ARRAY:
        break;
    case TC_REF:
        break;
    case TC_ALIAS:
        break;
    case TC_STRUCT:
        return checkStruct(left, right, expr);
    case TC_ENUM:
        break;
    case TC_FUNCTION:
        return checkFunction(left, expr);
    case TC_MODULE:
        TODO;
        break;
    }
    return false;
}

bool ExprAnalyser::checkBuiltin(QualType left, QualType right, const Expr* expr, bool first) {
    const BuiltinType* Left = cast<BuiltinType>(left.getCanonicalType());

    expr = AnalyserUtils::ignoreParenEpr(expr);

    // left is builtin
    bool showQualifiers = true;
    QualType C = right.getCanonicalType();
    switch (C->getTypeClass()) {
    case TC_BUILTIN:
    {
        // NOTE: canonical is builtin, var itself my be RefType etc
        const BuiltinType* Right = cast<BuiltinType>(right.getCanonicalType());
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
                //QualType Q = TypeFinder::findType(expr);
                // BB TEMP
                QualType Q = expr->getType();
                return checkBuiltin(left, Q, expr, false);
            }
        }

        switch (rule) {
        case 0:
            return true;
        case 1: // loss of precision
            errorMsg = diag::warn_impcast_integer_precision;
            showQualifiers = false;
            break;
        case 2: // sign-conversion
            errorMsg = diag::warn_impcast_integer_sign;
            showQualifiers = false;
            break;
        case 3: // float->integer
            errorMsg = diag::warn_impcast_float_integer;
            showQualifiers = false;
            break;
        case 4: // incompatible
            errorMsg = diag::err_illegal_type_conversion;
            break;
        case 5: // loss of fp-precision
            errorMsg = diag::warn_impcast_float_precision;
            showQualifiers = false;
            break;
        default:
            FATAL_ERROR("Unreachable");
            break;
        }
        right.DiagName(buf1, showQualifiers);
        left.DiagName(buf2, showQualifiers);
        // TODO error msg depends on conv type (see clang errors)
        Diags.Report(expr->getLocation(), errorMsg) << buf1 << buf2
                << expr->getSourceRange();
        m_hasError = true;
        return false;
    }
    case TC_POINTER:
        // allow implicit cast to bool if(ptr), TODO other cases
        if (Left->getKind() == BuiltinType::Bool) return true;
        break;
    case TC_ARRAY:
        break;
    case TC_REF:
        break;
    case TC_ALIAS:
        break;
    case TC_STRUCT:
        break;
    case TC_ENUM:
        break;
    case TC_FUNCTION:
        break;
    case TC_MODULE:
        break;
    }
    error(expr, left, right);
    return false;
}

bool ExprAnalyser::checkStruct(QualType left, QualType right, const Expr* expr) {
    QualType C = right.getCanonicalType();
    switch (C->getTypeClass()) {
    case TC_BUILTIN:
    case TC_POINTER:
    case TC_ARRAY:
    case TC_REF:
    case TC_ALIAS:
        break;
    case TC_STRUCT:
        // TODO check if same struct
        return true;
    case TC_ENUM:
    case TC_FUNCTION:
    case TC_MODULE:
        break;
    }
    error(expr, left, right);
    return false;
}

bool ExprAnalyser::checkPointer(QualType left, QualType right, Expr* expr) {
    QualType LP = cast<PointerType>(left)->getPointeeType();

    if (right->isPointerType()) {
        QualType RP = cast<PointerType>(right)->getPointeeType();
        if (RP.isConstQualified() && !LP.isConstQualified()) {
            // TODO need to know what error is:
            // TODO the ExprAnalyser should return a ConversionType (clang AssignConvertType)
            //initializing 'S *' with an expression of type 'const S *' discards qualifiers
            //assigning to 'S *' from 'const S *' discards qualifiers
            //NOTE: also needs args
            error2(expr->getLocation(), left, right, diag::ext_typecheck_convert_discards_qualifiers);
            return false;
        }

        if (LP.isVoidType()) {
            // dont allow implicit ptr-ptr to void*
            if (RP->isPointerType()) {
                error(expr, left, right);
                return false;
            }

            return true;
        }
        // TODO check if allowed (either same or to/from void* etc)
        return true;
    }
    if (right->isFunctionType()) {
        // TODO check if correct one, for now allow all to void*
        if (LP.isVoidType()) return true;

        return true;
    }
    error(expr, left, right);
    return false;
}

bool ExprAnalyser::checkFunction(QualType L, const Expr* expr) {
    QualType R = expr->getType();

    if (isa<NilExpr>(expr)) return true;

    // NOTE: for now only allow FunctionTypes
    if (!R->isFunctionType()) {
        error(expr, L, R);
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

bool ExprAnalyser::checkWidth(QualType type, SourceLocation loc, int msg) {
    // only allow cast to pointer from uint32/64 (pointer size)
    const BuiltinType* BT = dyncast<BuiltinType>(type);
    if (BT) {
        if (target.intWidth == 32 && BT->getKind() == BuiltinType::UInt32) return true;
        if (target.intWidth == 64 && BT->getKind() == BuiltinType::UInt64) return true;
    }

    QualType expected = target.intWidth == 64 ? Type::UInt64() : Type::UInt32();
    expected.DiagName(buf1, false);
    Diags.Report(loc, msg) << buf1;
    m_hasError = true;
    return false;
}

void ExprAnalyser::error(const Expr* expr, QualType left, QualType right) {
    SourceLocation loc = expr->getLocation();

    expr = stripImplicitCast(expr);

    expr->getType().DiagName(buf1);
    left.DiagName(buf2);

    // TODO error msg depends on conv type (see clang errors)
    Diags.Report(loc, diag::err_illegal_type_conversion)
            << buf1 << buf2;
    m_hasError = true;
}

QualType ExprAnalyser::outputStructDiagnostics(QualType T, IdentifierExpr* member, unsigned msg)
{
    buf2.clear();
    T.DiagName(buf1);
    buf2 << '\'' << member->getName() << '\'';
    Diags.Report(member->getLocation(), msg) << buf2 << buf1;
    return QualType();
}

void ExprAnalyser::error2(SourceLocation loc, QualType left, QualType right, unsigned msg) {
    right.DiagName(buf1);
    left.DiagName(buf2);

    Diags.Report(loc, msg) << buf1 << buf2;
    m_hasError = true;
}

Decl* ExprAnalyser::analyseOffsetOf(BuiltinExpr* expr, const StructTypeDecl* S, Expr* member, uint64_t* off) {
    IdentifierExpr* I = dyncast<IdentifierExpr>(member);
    if (I) {
        int index = S->findMemberIndex(I->getName());
        if (index == -1) {
            outputStructDiagnostics(S->getType(), I, diag::err_no_member);
            return 0;
        }
        Decl* field = S->getMember(index);
        uint64_t offset = AnalyserUtils::offsetOfStructMember(S, index);
        if (field->hasEmptyName()) {    // anonymous sub-struct/union
            const StructTypeDecl* anon = cast<StructTypeDecl>(field);
            int sub_index = anon->findMemberIndex(I->getName());
            uint64_t sub_offset = AnalyserUtils::offsetOfStructMember(anon, sub_index);
            offset += sub_offset;
        }

        expr->setValue(llvm::APSInt::getUnsigned(offset));
        *off = offset;
        field->setUsed();
        I->setType(field->getType());
        I->setDecl(field, IdentifierExpr::REF_STRUCT_MEMBER);
        return field;
    }

    assert(isa<MemberExpr>(member));
    MemberExpr* M = cast<MemberExpr>(member);

    uint64_t offset = 0;
    Decl* subStruct = analyseOffsetOf(expr, S, M->getBase(), &offset);
    if (!subStruct) return 0;
    StructTypeDecl* sub = dyncast<StructTypeDecl>(subStruct);
    if (!sub) {
        // Can also be variable of another struct type
        VarDecl* var = dyncast<VarDecl>(subStruct);
        if (var) {
            QualType T = var->getType();
            if (T.isStructType()) {
                const StructType* ST = cast<StructType>(T);
                sub = ST->getDecl();
            }
        }

        if (!sub) {
            QualType LType = subStruct->getType();
            LType.DiagName(buf1);
            Diags.Report(M->getLocation(), diag::err_typecheck_member_reference_struct_union)
                        << buf1 << M->getSourceRange() << M->getMember()->getLocation();
            return 0;
        }
    }
    uint64_t offset2 = 0;
    Decl* field = analyseOffsetOf(expr, sub, M->getMember(), &offset2);
    if (field) {
        M->setDecl(field);
        M->setType(field->getType());
        expr->setValue(llvm::APSInt::getUnsigned(offset + offset2));
        *off = offset + offset2;
    }
    return field;
}

QualType ExprAnalyser::analyseIntegerLiteral(Expr* expr) {
    IntegerLiteral* I = cast<IntegerLiteral>(expr);
    // Fit smallest Type: int32 > uint32 > int64 > uint64
    // TODO unsigned types

    // TEMP for now assume signed
    // Q: we can determine size, but don't know if we need signed/unsigned
    //unsigned numbits = I->Value.getMinSignedBits();  // signed
    unsigned numbits = I->Value.getActiveBits();   // unsigned
    //if (numbits <= 8) return Type::Int8();
    //if (numbits <= 16) return Type::Int16();
    expr->setType(Type::Int32());
    if (numbits <= 32) return Type::Int32();
    expr->setType(Type::Int64());
    return Type::Int64();
}

QualType ExprAnalyser::getBinOpType(const BinaryOperator* binop) {
    const Expr* lhs = binop->getLHS();
    const Expr* rhs = binop->getRHS();
    QualType TLeft = lhs->getType();
    QualType TRight = rhs->getType();

    if (TLeft.isVoidType() || TRight.isVoidType()) {
        Diag2(binop->getLocation(), diag::err_typecheck_invalid_operands, TLeft, TRight);
        return QualType();
    }

    switch (binop->getOpcode()) {
    case BINOP_Mul:
    case BINOP_Div:
    case BINOP_Rem:
    case BINOP_Add:
    case BINOP_Sub: {
        bool left_is_ptr = TLeft->isPointerType();
        bool right_is_ptr = TRight->isPointerType();
        if (left_is_ptr || right_is_ptr) {
            if (left_is_ptr && right_is_ptr) {
                if (binop->getOpcode() == BINOP_Sub) {
                    switch (target.intWidth) {
                    case 32:
                        return Type::Int32();
                    case 64:
                        return Type::Int64();
                    default:
                        assert(0 && "should not come here");
                        break;
                    }
                } else {
                    Diag2(binop->getLocation(), diag::err_typecheck_invalid_operands, TLeft, TRight);
                    return QualType();
                }
            } else {
                if (left_is_ptr) {
                    return TLeft;
                } else {
                    return TRight;
                }
            }
        } else {
            if (rhs->isCTV()) return TLeft;
            if (lhs->isCTV()) return TRight;
            return LargestType(TLeft, TRight);
        }
        break;
    }
    case BINOP_Shl:
    case BINOP_Shr:
        return TLeft;
    case BINOP_LE:
    case BINOP_LT:
    case BINOP_GE:
    case BINOP_GT:
    case BINOP_NE:
    case BINOP_EQ:
    case BINOP_And:
    case BINOP_Or:
        return Type::Bool();
    case BINOP_Xor:
    case BINOP_LAnd:
    case BINOP_LOr:
        return TLeft;    // TODO valid?
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
        return TLeft;
    case BINOP_Comma:
        TODO;
        break;
    }

    return binop->getType();
}

const Expr* ExprAnalyser::stripImplicitCast(const Expr* E) {
    const ImplicitCastExpr* ic = dyncast<ImplicitCastExpr>(E);
    if (ic) return ic->getInner();
    return E;
}

QualType ExprAnalyser::LargestType(QualType TL, QualType TR) {
    // TODO cleanup
    QualType Lcanon = TL.getCanonicalType();
    QualType Rcanon = TR.getCanonicalType();
    assert(Lcanon.isBuiltinType());
    assert(Rcanon.isBuiltinType());
    const BuiltinType* Lbi = cast<BuiltinType>(Lcanon);
    const BuiltinType* Rbi = cast<BuiltinType>(Rcanon);
    if (Lbi->getWidth() > Rbi->getWidth()) {
        return TL;
    }
    return TR;
}

bool ExprAnalyser::arePointersCompatible(QualType L, QualType R) {
    // TODO
    return true;
}

DiagnosticBuilder ExprAnalyser::Diag2(SourceLocation loc, int diag_id, QualType T1, QualType T2) {
    T1.DiagName(buf1);
    T2.DiagName(buf2);
    return Diags.Report(loc, diag_id) << buf1 << buf2;
}

