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

#include "Analyser/ExprTypeAnalyser.h"
#include "Analyser/LiteralAnalyser.h"
#include "Analyser/TypeFinder.h"
#include "Analyser/TypeChecker.h"
#include "AST/Expr.h"

using namespace C2;
using namespace llvm;
using namespace clang;


ExprTypeAnalyser::ExprTypeAnalyser(TypeChecker& TC_, DiagnosticsEngine& Diags_)
    : TC(TC_)
    , Diags(Diags_)
{}

void ExprTypeAnalyser::check(QualType TLeft, const Expr* expr) {
    switch (expr->getCTC()) {
    case CTC_NONE:
    {
        // TODO only do findType if doing actual Implicit Cast
        QualType Q = TypeFinder::findType(expr);
        TC.checkCompatible(TLeft, Q, expr);
        return;
    }
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

