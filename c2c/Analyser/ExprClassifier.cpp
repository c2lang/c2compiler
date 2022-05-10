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

#include <assert.h>

#include "Analyser/ExprClassifier.h"
#include "AST/Expr.h"
//#include "AST/Decl.h"

using namespace C2;
using namespace llvm;
using namespace c2lang;

ExprValueKind ExprClassifier::classifyExpr(const Expr* expr) {
    switch (expr->getKind()) {
    case EXPR_INTEGER_LITERAL:
    case EXPR_FLOAT_LITERAL:
    case EXPR_BOOL_LITERAL:
    case EXPR_CHAR_LITERAL:
    case EXPR_STRING_LITERAL:
    case EXPR_NIL:
        break;
    case EXPR_IDENTIFIER:
        return VK_LValue;
    case EXPR_TYPE:
    case EXPR_CALL:
        break;
    case EXPR_INITLIST:
    case EXPR_DESIGNATOR_INIT:
        break;
    case EXPR_BINOP:
    case EXPR_CONDOP:
        break;
    case EXPR_UNARYOP:
        return ExprClassifier::classifyUnaryOperator(cast<UnaryOperator>(expr));
    case EXPR_BUILTIN:
        break;
    case EXPR_ARRAYSUBSCRIPT:
        // TODO unless it still points to an array
        return VK_LValue;
    case EXPR_MEMBER:
        return VK_LValue;
    case EXPR_PAREN:
        return ExprClassifier::classifyExpr(cast<ParenExpr>(expr)->getExpr());
    case EXPR_BITOFFSET:
    case EXPR_EXPL_CAST:
        break;
    }
    return VK_RValue;
}

ExprValueKind ExprClassifier::classifyUnaryOperator(const UnaryOperator* uo) {
    if (uo->getOpcode() == UO_Deref) return VK_LValue;
    return VK_RValue;
}
        // only deref is L_value
