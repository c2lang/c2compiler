/* Copyright 2013-2017 Bas van den Berg
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

using namespace C2;
using namespace llvm;
using namespace clang;


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
        assert(0 && "should not come here");
        break;
    case EXPR_CALL:
        break;
    case EXPR_INITLIST:
    case EXPR_DESIGNATOR_INIT:
        assert(0 && "should not come here");
        break;
    case EXPR_BINOP:
        return getBinOpType(cast<BinaryOperator>(expr));
    case EXPR_CONDOP:
        return getCondOpType(cast<ConditionalOperator>(expr));
    case EXPR_UNARYOP:
        return getUnaryOpType(cast<UnaryOperator>(expr));
    case EXPR_BUILTIN:
        assert(0 && "should not come here");
        break;
    case EXPR_ARRAYSUBSCRIPT:
    case EXPR_MEMBER:
        break;
    case EXPR_PAREN:
        return findType(cast<ParenExpr>(expr)->getExpr());
    case EXPR_BITOFFSET:
        break;
    case EXPR_CAST:
        assert(0 && "TODO");
        break;
    }
    return expr->getType();
}

QualType TypeFinder::getBinOpType(const BinaryOperator* binop) {
    switch (binop->getOpcode()) {
    case BO_PtrMemD:
    case BO_PtrMemI:
        assert(0 && "TODO?");
        break;
    case BO_Mul:
    case BO_Div:
    case BO_Rem:
        return LargestType(binop->getLHS(), binop->getRHS());
    case BO_Add:
    case BO_Sub:
        return LargestType(binop->getLHS(), binop->getRHS());
    case BO_Shl:
    case BO_Shr:
        assert(0 && "TODO");
        break;
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
        // should be bool
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
        // return LHS type
        return findType(binop->getLHS());
    case BO_Comma:
        assert(0 && "TODO?");
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
    case UO_Plus:
    case UO_Minus:
    case UO_Not:
        return findType(unaryop->getExpr());
    case UO_LNot:
        // should be bool already
        break;
    default:
        assert(0 && "TODO");
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

