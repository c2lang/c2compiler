/* Copyright 2013-2019 Bas van den Berg
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
#include "AST/Decl.h"
#include "Utils/StringBuilder.h"

using namespace C2;

const char* AnalyserUtils::fullName(const std::string& modName, const char* symname) {
    static char buffer[128];
    sprintf(buffer, "%s.%s", modName.c_str(), symname);
    return buffer;
}

QualType AnalyserUtils::getStructType(QualType Q) {
    // Q: use CanonicalType to get rid of AliasTypes?

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
        FATAL_ERROR("Unreachable");
        return QualType();
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

    if (const BuiltinType* BI = cast<BuiltinType>(canon)) {
        if (BI->isPromotableIntegerType()) {
            // TODO keep flags (const, etc)?
            expr->setImpCast(BuiltinType::Int32);
            return Type::Int32();
        }
    }

    if (canon->isPointerType()) {
        // TODO need targetInfo
        //return (target.intWidth == 64) ? Type::UInt64() : Type::UInt32();
        return Type::UInt64();
    }

    return expr->getType();
}

void AnalyserUtils::SetConstantFlags(Decl* D, Expr* expr) {
    switch (D->getKind()) {
    case DECL_FUNC:
        expr->setConstant();
        break;
    case DECL_VAR:
    {
        VarDecl* VD = cast<VarDecl>(D);
        QualType T = VD->getType();
        if (T.isConstQualified()) {
            Expr* Init = VD->getInitValue();
            if (Init) {
                // Copy CTC status of Init Expr
                expr->setCTC(Init->getCTC());
            }
            expr->setConstant();
            return;
        }
        break;
    }
    case DECL_ENUMVALUE:
        expr->setCTC(CTC_FULL);
        expr->setConstant();
        return;
    case DECL_ALIASTYPE:
    case DECL_STRUCTTYPE:
    case DECL_ENUMTYPE:
    case DECL_FUNCTIONTYPE:
        expr->setConstant();
        break;
    case DECL_ARRAYVALUE:
    case DECL_IMPORT:
    case DECL_LABEL:
    case DECL_STATIC_ASSERT:
        break;
    }
    // TODO needed?
    expr->setCTC(CTC_NONE);
}

ExprCTC AnalyserUtils::combineCtc(Expr* Result, const Expr* L, const Expr* R) {
    const ExprCTC left =  L->getCTC();
    const ExprCTC right = R->getCTC();
    switch (left + right) {
    case 0:
        Result->setCTC(CTC_NONE);
        return CTC_NONE;
    case 1:
    case 2:
    case 3:
        Result->setCTC(CTC_PARTIAL);
        return CTC_PARTIAL;
    case 4:
        Result->setCTC(CTC_FULL);
        return CTC_FULL;
    }
    FATAL_ERROR("Unreachable");
    return CTC_NONE;
}

bool AnalyserUtils::isConstantBitOffset(const Expr* E) {
    if (const ArraySubscriptExpr* A = dyncast<ArraySubscriptExpr>(E)) {
        if (const BitOffsetExpr* B = dyncast<BitOffsetExpr>(A->getIndex())) {
            return B->isConstant();
        }
    }
    return false;
}

StringBuilder& AnalyserUtils::quotedField(StringBuilder &builder, IdentifierExpr *field) {
    return builder << '\'' << field->getName() << '\'';
}

uint64_t AnalyserUtils::sizeOfUnion(StructTypeDecl* S, uint32_t* align) {
    //bool packed = S->isPacked();
    uint32_t alignment = S->getAttrAlignment();
    *align = alignment;
    //printf("SIZEOF  UNION %s packed=%u  aligned=%u\n", S->getName(), packed, alignment);
    uint64_t size = 0;
    // TODO handle packed

    unsigned num_members = S->numMembers();
    for (unsigned i=0; i<num_members; i++) {
        Decl* D = S->getMember(i);
        unsigned m_align = 0;
        uint64_t m_size = AnalyserUtils::sizeOfType(D->getType(), &m_align);
        // TODO handle un-packed sub-structs
        if (m_size > size) size = m_size;
        if (m_align > *align) *align = m_align;
    }
    return size;
}

uint64_t AnalyserUtils::sizeOfStruct(StructTypeDecl* S, uint32_t* align) {
    if (!S->isStruct()) return AnalyserUtils::sizeOfUnion(S, align);

    bool packed = S->isPacked();
    uint32_t alignment = S->getAttrAlignment();
    *align = alignment;
    //printf("SIZEOF STRUCT %s packed=%u  aligned=%u\n", S->getName(), packed, alignment);
    uint64_t size = 0;
    if (packed) {
        unsigned num_members = S->numMembers();
        for (unsigned i=0; i<num_members; i++) {
            Decl* D = S->getMember(i);
            unsigned m_align = 0;
            uint64_t m_size = AnalyserUtils::sizeOfType(D->getType(), &m_align);
            // TODO handle un-packed sub-structs
            size += m_size;
        }
    } else {
        unsigned num_members = S->numMembers();
        for (unsigned i=0; i<num_members; i++) {
            Decl* D = S->getMember(i);
            unsigned m_align = 0;
            uint64_t m_size = AnalyserUtils::sizeOfType(D->getType(), &m_align);
            if (m_align != 1) {
                if (m_align > alignment) alignment = m_align;
                unsigned rest = size % m_align;
                if (rest != 0) {
                    unsigned pad = m_align - rest;
                    //printf("  @%03lu pad %u\n", size, pad);
                    size += pad;
                }
            }
            //printf("  @%03lu member %s   size %lu  align = %u\n", size, D->getName(), m_size, m_align);
            size += m_size;
        }
        unsigned rest = size % alignment;
        if (rest != 0) {
            unsigned pad = alignment - rest;
            //printf("  @%03lu pad %u\n", size, pad);
            size += pad;
        }
    }
    //printf("  -> size %lu  align %u\n", size, alignment);
    *align = alignment;
    return size;
}

uint64_t AnalyserUtils::sizeOfType(QualType type, unsigned* alignment) {

    // TODO not a constant.
    constexpr unsigned POINTER_SIZE = 8;
    *alignment = 1;

    if (type.isNull()) return 0;

    type = type.getCanonicalType();
    switch (type->getTypeClass()) {
    case TC_REF:
    case TC_ALIAS:
        FATAL_ERROR("Should be resolved");
        return 1;
    case TC_BUILTIN:
    {
        const BuiltinType* BI = cast<BuiltinType>(type.getTypePtr());
        unsigned size = (BI->getWidth() + 7) / 8;
        *alignment = size;
        return size;
    }
    case TC_POINTER:
        *alignment = POINTER_SIZE;
        return POINTER_SIZE;
    case TC_ARRAY:
    {
        ArrayType *arrayType = cast<ArrayType>(type.getTypePtr());
        return sizeOfType(arrayType->getElementType(), alignment) * arrayType->getSize().getZExtValue();
    }
    case TC_STRUCT:
    {
        // NOTE: should already be filled in
        StructType* structType = cast<StructType>(type.getTypePtr());
        StructTypeDecl* D = structType->getDecl();
        *alignment = D->getAlignment();
        return D->getSize();
    }
    case TC_ENUM:
        // TODO
        return 0;
    case TC_FUNCTION:
        *alignment = POINTER_SIZE;  // TODO arch dependent
        return POINTER_SIZE;
    case TC_MODULE:
        FATAL_ERROR("Cannot occur here");
    }
}

