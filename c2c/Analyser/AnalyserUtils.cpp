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
#include <ctype.h>

#include "Analyser/AnalyserUtils.h"
#include "Analyser/CTVAnalyser.h"
#include "AST/Decl.h"
#include "AST/ASTContext.h"
#include "Utils/StringBuilder.h"
#include "Utils/TargetInfo.h"

using namespace C2;

static const TargetInfo* target;
// TODO make target dependent
static unsigned POINTER_SIZE = 8;

void AnalyserUtils::init(const TargetInfo& target_) {
    target = &target_;
    POINTER_SIZE = target->intWidth / 8;
}

const char* AnalyserUtils::fullName(const std::string& modName, const char* symname) {
    static char buffer[128];
    sprintf(buffer, "%s.%s", modName.c_str(), symname);
    return buffer;
}

QualType AnalyserUtils::getStructType(QualType Q) {
    // Q: use CanonicalType to get rid of AliasTypes?

    // TODO pass const?
    Type* T = Q.getTypePtr();
    switch (T->getTypeClass()) {
    case TC_BUILTIN:
        return QualType();
    case TC_POINTER:
    {
        // Could be pointer to structtype
        PointerType* PT = cast<PointerType>(T);
        return getStructType(PT->getPointeeType());
    }
    case TC_ARRAY:
        return QualType();
    case TC_REF:
    {
        RefType* RT = cast<RefType>(T);
        return RT->getDecl()->getType();
    }
    case TC_ALIAS:
    {
        AliasType* AT = cast<AliasType>(T);
        return getStructType(AT->getRefType());
    }
    case TC_STRUCT:
        return Q;
    case TC_ENUM:
    case TC_FUNCTION:
    case TC_MODULE:
        return QualType();
    }
    FATAL_ERROR("Unreachable");
}

QualType AnalyserUtils::getPointerFromArray(ASTContext& context, QualType Q) {
    Q = Q.getCanonicalType();
    const ArrayType* A = cast<ArrayType>(Q);
    QualType ET = A->getElementType();
    QualType ptr = context.getPointerType(A->getElementType());
    if (Q.isConstant()) ptr.addConst();
    return ptr;
}

QualType AnalyserUtils::getMinusType(QualType Q) {
    Q = Q.getCanonicalType();
    // TODO aliasTypes?

    if (const BuiltinType* BI = dyncast<BuiltinType>(Q)) {
        switch (BI->getKind()) {
        case BuiltinType::Int8:
        case BuiltinType::Int16:
        case BuiltinType::Int32:
        case BuiltinType::Int64:
            return Q;
        case BuiltinType::UInt8:
        case BuiltinType::UInt16:
        case BuiltinType::UInt32:
            return Type::Int32();
        case BuiltinType::UInt64:
            return Type::Int64();
        case BuiltinType::Float32:
        case BuiltinType::Float64:
            return Q;
        case BuiltinType::Bool:
            return Type::Int32();
        case BuiltinType::Void:
            break;
        }
    }

    return QualType();
}

bool AnalyserUtils::exprIsType(const Expr* E) {
    // can be IdentifierExpr or MemberExpr
    switch (E->getKind()) {
    case EXPR_IDENTIFIER:
    {
        const IdentifierExpr* I = cast<IdentifierExpr>(E);
        return I->isType();
    }
    case EXPR_MEMBER:
    {
        const MemberExpr* M = cast<MemberExpr>(E);
        return M->getMember()->isType();
    }
    default:
        break;
    }
    return false;
}

// Convert smaller types to int, others remain the same
// This function should only be called if Expr's type is ok for unary operator
QualType AnalyserUtils::UsualUnaryConversions(Expr* expr) {
    const Type* canon = expr->getType().getCanonicalType();

    if (const BuiltinType* BI = dyncast<BuiltinType>(canon)) {
        if (BI->isPromotableIntegerType()) {
            // TODO keep flags (const, etc)?
            expr->setImpCast(BuiltinType::Int32);
            return Type::Int32();
        }
    }

    if (canon->isPointerType()) {
        return (target->intWidth == 64) ? Type::UInt64() : Type::UInt32();
    }

    return expr->getType();
}

void AnalyserUtils::SetConstantFlags(const Decl* D, Expr* expr) {
    switch (D->getKind()) {
    case DECL_FUNC:
        expr->setCTC();
        expr->setIsRValue();
        return;
    case DECL_VAR:
    {
        const VarDecl* VD = cast<VarDecl>(D);
        QualType T = VD->getType();
        expr->setCTC();
        Expr* Init = VD->getInitValue();
        if (Init && T.isConstQualified()) {
            expr->setCTV(Init->isCTV());
            return;
        }
        return;
    }
    case DECL_ENUMVALUE:
        expr->setCTV(true);
        expr->setCTC();
        expr->setIsRValue();
        return;
    case DECL_ALIASTYPE:
        break;
    case DECL_STRUCTTYPE:
        expr->setIsRValue();
        break;
    case DECL_ENUMTYPE:
        expr->setCTC();
        expr->setIsRValue();
        break;
    case DECL_FUNCTIONTYPE:
        expr->setCTC();
        break;
    case DECL_ARRAYVALUE:
    case DECL_IMPORT:
    case DECL_LABEL:
    case DECL_STATIC_ASSERT:
        break;
    }
    // TODO needed?
    expr->setCTV(false);
}

bool AnalyserUtils::isConstantBitOffset(const Expr* E) {
    if (const ArraySubscriptExpr* A = dyncast<ArraySubscriptExpr>(E)) {
        if (const BitOffsetExpr* B = dyncast<BitOffsetExpr>(A->getIndex())) {
            return B->isCTC();
        }
    }
    return false;
}

StringBuilder& AnalyserUtils::quotedField(StringBuilder &builder, IdentifierExpr *field) {
    return builder << '\'' << field->getName() << '\'';
}

TypeSize AnalyserUtils::sizeOfUnion(StructTypeDecl* S) {
    TypeSize result = { 0, 1, 0, 0 };
    //bool packed = S->isPacked();
    result.align = S->getAttrAlignment();
    //printf("SIZEOF  UNION %s packed=%u  aligned=%u\n", S->getName(), packed, alignment);
    // TODO handle packed

    unsigned num_members = S->numMembers();
    for (unsigned i=0; i<num_members; i++) {
        Decl* D = S->getMember(i);
        TypeSize m_size = AnalyserUtils::sizeOfType(D->getType());
        // TODO handle un-packed sub-structs
        if (m_size.size > result.size) result.size = m_size.size;
        if (m_size.align > result.align) result.align = m_size.align;
    }
    return result;
}

TypeSize AnalyserUtils::sizeOfStruct(StructTypeDecl* S, CTVAnalyser& ca) {
    if (!S->isStruct()) return AnalyserUtils::sizeOfUnion(S);

    // TODO refactor to struct we can pass along and do operations on (like end-bitfield)
    TypeSize result = { 0, 1, 0, 0 };

    bool packed = S->isPacked();
    result.align = S->getAttrAlignment();
    //printf("SIZEOF STRUCT %s packed=%u  aligned=%u\n", S->getName(), packed, result.align);
    uint32_t bitfieldsLeft = 0;
    if (packed) {
        unsigned num_members = S->numMembers();
        for (unsigned i=0; i<num_members; i++) {
            Decl* D = S->getMember(i);
            TypeSize member = AnalyserUtils::sizeOfType(D->getType());
            // TODO handle un-packed sub-structs?
            //printf("  @%03u member %s   size %u  align %u (bitfield %u/%u)\n", result.size, D->getName(), member.size, member.align, member.bitfield_size, member.bitfield_width);
            result.size += member.size;
        }
    } else {
        unsigned num_members = S->numMembers();
        for (unsigned i=0; i<num_members; i++) {
            Decl* D = S->getMember(i);
            // Note: doesn't analyze the bit-field
            TypeSize member = AnalyserUtils::sizeOfType(D->getType());
            VarDecl* vd = dyncast<VarDecl>(D);
            if (vd && vd->getBitfield()) {
                const Expr* bitfield = vd->getBitfield();
                llvm::APSInt value = ca.checkLiterals(bitfield);
                member.bitfield_size = value.getZExtValue();
                member.bitfield_width = member.size * 8;
                member.size = 0;
                member.align = 0;   // calculate at end
/*
                // NOTE: should already been checked!
                if (member.bitfield_size > member.bitfield_width) {
                    error: width of anonymous bit-field (11 bits) exceeds the width of its type (8 bits)
                }
*/
            }
            if (result.bitfield_width && member.align != 0) {   // no bitfield anymore, finalize last one
                // TODO end last bitfield, add size etc
                //printf("    end bitfield\n");
                uint32_t bytesize = (result.bitfield_size + 7) / 8;
                // TODO padding
                result.size += bytesize;
                if (bytesize > result.align) result.align = bytesize;
                //printf("    setting bitfields size %u  align %u\n", result.size, result.align);
                result.bitfield_width = 0;
                result.bitfield_size = 0;
            }
            if (member.align > 1) {
                if (member.align > result.align) result.align = member.align;
                unsigned rest = result.size % member.align;
                if (rest != 0) {
                    unsigned pad = member.align - rest;
                    //printf("  @%03u pad %u\n", result.size, pad);
                    result.size += pad;
                }
            }
            //printf("  @%03u member %s   size %u  align %u (bitfield %u/%u)\n", result.size, D->getName(), member.size, member.align, member.bitfield_size, member.bitfield_width);
            if (member.bitfield_width) {
                //printf("    bit-fields %u / %u\n", result.bitfield_size, result.bitfield_width);
                uint32_t total_bitsize = result.bitfield_size + member.bitfield_size;
                if (total_bitsize > member.bitfield_width) {
                    //printf("    new field\n");
                    uint32_t bytesize = (result.bitfield_size + 7) / 8;
                    member.align = bytesize;
                    if (bytesize > 1) {
                        unsigned rest = result.size % bytesize;
                        if (rest != 0) {
                            unsigned pad = member.align - rest;
                            //printf("  @%03u pad %u\n", result.size, pad);
                            result.size += pad;
                        }
                    }
                    result.size += bytesize;
                    //if (bytesize > result.align) result.align = bytesize;
                    result.bitfield_size = member.bitfield_size;
                    result.bitfield_width = member.bitfield_width;
                } else {
                    result.bitfield_size = total_bitsize;
                    result.bitfield_width = member.bitfield_width;
                }
            } else {
                result.size += member.size;
            }
            //printf("  -> size %u  align %u bitfield (%u/%u)\n", result.size, result.align, result.bitfield_size, result.bitfield_width);
        }
        if (result.bitfield_width) {
            //printf("  done, bit-fields %u / %u\n", result.bitfield_size, result.bitfield_width);
            uint32_t bytesize = (result.bitfield_size + 7) / 8;
            // TODO padding before
            result.size += bytesize;
            //if (bytesize > result.align) result.align = bytesize;
            //printf("    setting size %u  align %u\n", result.size, result.align);
        }
        unsigned rest = result.size % result.align;
        if (rest != 0) {
            unsigned pad = result.align - rest;
            //printf("  @%03u pad %u\n", result.size, pad);
            result.size += pad;
        }
    }
    //printf("  -> FINAL size %u  align %u\n", result.size, result.align);
    return result;
}

TypeSize AnalyserUtils::sizeOfType(QualType type) {

    TypeSize result = { 0, 1, 0, 0 };

    if (type.isNull()) return result;

    type = type.getCanonicalType();
    switch (type->getTypeClass()) {
    case TC_REF:
    case TC_ALIAS:
        FATAL_ERROR("Should be resolved");
        break;
    case TC_BUILTIN:
    {
        const BuiltinType* BI = cast<BuiltinType>(type.getTypePtr());
        // NOTE: alignment is also size
        result.size = BI->getAlignment();
        result.align = result.size;
        break;
    }
    case TC_POINTER:
        result.size = POINTER_SIZE;
        result.align = result.size;
        break;
    case TC_ARRAY:
    {
        ArrayType *arrayType = cast<ArrayType>(type.getTypePtr());
        result = sizeOfType(arrayType->getElementType());
        result.size *= arrayType->getSize().getZExtValue();
        break;
    }
    case TC_STRUCT:
    {
        // NOTE: should already be filled in
        StructType* structType = cast<StructType>(type.getTypePtr());
        StructTypeDecl* D = structType->getDecl();
        result.size = D->getSize();
        result.align = D->getAlignment();
        break;
    }
    case TC_ENUM:
        FATAL_ERROR("Cannot come here");
        break;
    case TC_FUNCTION:
        result.size = POINTER_SIZE;
        result.align = POINTER_SIZE;
        break;
    case TC_MODULE:
        FATAL_ERROR("Cannot occur here");
        break;
    }
    return result;
}

uint64_t AnalyserUtils::offsetOfStructMember(const StructTypeDecl* S, unsigned index) {
    if (S->isUnion()) return 0;
    if (index == 0) return 0;

    TypeSize result = { 0, 1, 0, 0 };
    result.align = S->getAttrAlignment();

    if (S->isPacked()) {
        for (unsigned i=0; i<index; i++) {
            const Decl* D = S->getMember(i);
            TypeSize member = AnalyserUtils::sizeOfType(D->getType());
            result.size += member.size;
        }
    } else {
        for (unsigned i=0; i<=index; i++) {
            const Decl* D = S->getMember(i);
            TypeSize member = AnalyserUtils::sizeOfType(D->getType());
            if (member.align != 1) {
                if (member.align > result.align) result.align = member.align;
                unsigned rest = result.size % member.align;
                if (rest != 0) {
                    unsigned pad = member.align - rest;
                    result.size += pad;
                }
            }
            //printf("  @%03u member %s   size %u  align %u (bitfield %u/%u)\n", member.size, D->getName(), member.size, member.align, member.bitfield_size, member.bitfield_width);
            if (i == index) return result.size;
            result.size += member.size;
        }
    }
    return result.size;
}

Expr* AnalyserUtils::getInnerExprAddressOf(Expr* expr) {
    // TODO can be MemberExpr with ArraySubScript: &foo[1].x
    // TODO can also be ArraySubScript with emberExpr with: &foo.x[2]
    // strip MemberExpr, ArraySubscriptExpr and Paren
    while (1) {
        switch (expr->getKind()) {
        case EXPR_INTEGER_LITERAL:
        case EXPR_FLOAT_LITERAL:
        case EXPR_BOOL_LITERAL:
        case EXPR_CHAR_LITERAL:
        case EXPR_STRING_LITERAL:
        case EXPR_NIL:
        case EXPR_IDENTIFIER:
        case EXPR_TYPE:
        case EXPR_CALL:
        case EXPR_INITLIST:
        case EXPR_DESIGNATOR_INIT:
            return expr;
        case EXPR_BINOP:
            // TODO
            return expr;
        case EXPR_CONDOP:
            // TODO
            return expr;
        case EXPR_UNARYOP:
            // TODO
            return expr;
        case EXPR_BUILTIN:
            return expr;
        case EXPR_ARRAYSUBSCRIPT: {
            ArraySubscriptExpr* sub = cast<ArraySubscriptExpr>(expr);
            return getInnerExprAddressOf(sub->getBase());
        }
        case EXPR_MEMBER: {
            MemberExpr* member = cast<MemberExpr>(expr);
            return member->getMember();
        }
        case EXPR_PAREN: {
            ParenExpr* paren = cast<ParenExpr>(expr);
            return getInnerExprAddressOf(paren->getExpr());
        }
        case EXPR_BITOFFSET:
            return expr;
        case EXPR_EXPLICIT_CAST: {
            ExplicitCastExpr* ec = cast<ExplicitCastExpr>(expr);
            return getInnerExprAddressOf(ec->getInner());
        }
        case EXPR_IMPLICIT_CAST: {
            ImplicitCastExpr* ic = cast<ImplicitCastExpr>(expr);
            return getInnerExprAddressOf(ic->getInner());
        }
        }
    }
    return expr;
}

const Expr* AnalyserUtils::ignoreParenEpr(const Expr* expr) {
    while (1) {
        const ParenExpr* paren = dyncast<ParenExpr>(expr);
        if (!paren) return expr;
        expr = paren->getExpr();
    }
}

// TODO refactor to remove duplicates
IdentifierExpr::RefKind AnalyserUtils::globalDecl2RefKind(const Decl* D) {
    switch (D->getKind()) {
    case DECL_FUNC:         return IdentifierExpr::REF_FUNC;
    case DECL_VAR:          return IdentifierExpr::REF_VAR;
    case DECL_ENUMVALUE:    return IdentifierExpr::REF_ENUM_CONSTANT;
    case DECL_ALIASTYPE:
    case DECL_STRUCTTYPE:
    case DECL_ENUMTYPE:
    case DECL_FUNCTIONTYPE:
                            return IdentifierExpr::REF_TYPE;
    case DECL_ARRAYVALUE:
                            return IdentifierExpr::REF_VAR;
    case DECL_IMPORT:
                            return IdentifierExpr::REF_MODULE;
    case DECL_LABEL:        return IdentifierExpr::REF_LABEL;
    case DECL_STATIC_ASSERT:
        FATAL_ERROR("cannot come here");
    }
}

