/* Copyright 2013-2020 Bas van den Berg
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
#include "Analyser/TypeFinder.h"
#include "AST/Expr.h"
#include "Utils/Errors.h"

using namespace C2;
using namespace llvm;
using namespace c2lang;


QualType TypeFinder::findType(const Expr* expr) {
    assert(expr->getCTC() == CTC_NONE);
    switch (expr->getKind()) {
    case EXPR_INTEGER_LITERAL:
    case EXPR_FLOAT_LITERAL:
    case EXPR_BOOL_LITERAL:
    case EXPR_CHAR_LITERAL:
    case EXPR_STRING_LITERAL:
    case EXPR_NIL:
    case EXPR_IDENTIFIER:
        break;
    case EXPR_TYPE:
        FATAL_ERROR("Unreachable");
        break;
    case EXPR_CALL:
        break;
    case EXPR_INITLIST:
    case EXPR_DESIGNATOR_INIT:
        FATAL_ERROR("Unreachable");
        break;
    case EXPR_BINOP:
        return getBinOpType(cast<BinaryOperator>(expr));
    case EXPR_CONDOP:
        return getCondOpType(cast<ConditionalOperator>(expr));
    case EXPR_UNARYOP:
        return getUnaryOpType(cast<UnaryOperator>(expr));
    case EXPR_BUILTIN:
        FATAL_ERROR("Unreachable");
        break;
    case EXPR_ARRAYSUBSCRIPT:
    case EXPR_MEMBER:
        break;
    case EXPR_PAREN:
        return findType(cast<ParenExpr>(expr)->getExpr());
    case EXPR_BITOFFSET:
        break;
    case EXPR_CAST:
        TODO;
        break;
    }
    return expr->getType();
}

QualType TypeFinder::getBinOpType(const BinaryOperator* binop) {
    switch (binop->getOpcode()) {
    case BINOP_Mul:
    case BINOP_Div:
    case BINOP_Rem:
        return LargestType(binop->getLHS(), binop->getRHS());
    case BINOP_Add:
    case BINOP_Sub:
        return LargestType(binop->getLHS(), binop->getRHS());
    case BINOP_Shl:
    case BINOP_Shr:
        TODO;
        break;
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
        // should be bool
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
        // return LHS type
        return findType(binop->getLHS());
    case BINOP_Comma:
        TODO;
        break;
    }
    return binop->getType();
}

QualType TypeFinder::getUnaryOpType(const UnaryOperator* unaryop) {
    switch (unaryop->getOpcode()) {
    case UO_PostInc:
    case UO_PostDec:
    case UO_PreInc:
    case UO_PreDec:
        return findType(unaryop->getExpr());
    case UO_AddrOf:
        // just return type
        break;
    case UO_Deref:
    case UO_Minus:
    case UO_Not:
        return findType(unaryop->getExpr());
    case UO_LNot:
        // should be bool already
        break;
    }
    return unaryop->getType();
}

QualType TypeFinder::getCondOpType(const ConditionalOperator* condop) {
    return LargestType(condop->getLHS(), condop->getRHS());
}

QualType TypeFinder::LargestType(const Expr* Left, const Expr* Right) {
    QualType TL = findType(Left);
    QualType TR = findType(Right);
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

