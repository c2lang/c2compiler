/* Copyright 2013-2018 Bas van den Berg
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

#include "AST/Expr.h"
#include "AST/Type.h"
#include "AST/Decl.h"
#include "Utils/StringBuilder.h"
#include "Utils/color.h"
#include "Utils/UtilsConstants.h"

using namespace C2;
using namespace std;
using namespace c2lang;


Expr::Expr(ExprKind k, c2lang::SourceLocation loc_, bool isConstant_)
    : Stmt(STMT_EXPR)
    , exprLoc(loc_)
{
    exprBits.eKind = k;
    exprBits.ImpCast = BuiltinType::Void;
    exprBits.IsCTC = 0;
    exprBits.IsConstant = isConstant_;
}

static const char* ctc_strings[] = { "none", "partial", "full" };

void Expr::print(StringBuilder& buffer) const {
    QualType Q = getType();
    Q.print(buffer);
    buffer.setColor(COL_ATTR);
    buffer << " ctc=" << ctc_strings[exprBits.IsCTC];
    buffer << ", constant=" << isConstant();
    if (getImpCast() != BuiltinType::Void) {
        buffer << ", cast=" << BuiltinType::kind2name(getImpCast());
    }
}

void Expr::print(StringBuilder& buffer, unsigned indent) const {
    switch (getKind()) {
    case EXPR_INTEGER_LITERAL:
        return cast<IntegerLiteral>(this)->print(buffer, indent);
    case EXPR_FLOAT_LITERAL:
        return cast<FloatingLiteral>(this)->print(buffer, indent);
    case EXPR_BOOL_LITERAL:
        return cast<BooleanLiteral>(this)->print(buffer, indent);
    case EXPR_CHAR_LITERAL:
        return cast<CharacterLiteral>(this)->print(buffer, indent);
    case EXPR_STRING_LITERAL:
        return cast<StringLiteral>(this)->print(buffer, indent);
    case EXPR_NIL:
        return cast<NilExpr>(this)->print(buffer, indent);
    case EXPR_IDENTIFIER:
        return cast<IdentifierExpr>(this)->print(buffer, indent);
    case EXPR_TYPE:
        return cast<TypeExpr>(this)->print(buffer, indent);
    case EXPR_CALL:
        return cast<CallExpr>(this)->print(buffer, indent);
    case EXPR_INITLIST:
        return cast<InitListExpr>(this)->print(buffer, indent);
    case EXPR_DESIGNATOR_INIT:
        return cast<DesignatedInitExpr>(this)->print(buffer, indent);
    case EXPR_BINOP:
        return cast<BinaryOperator>(this)->print(buffer, indent);
    case EXPR_CONDOP:
        return cast<ConditionalOperator>(this)->print(buffer, indent);
    case EXPR_UNARYOP:
        return cast<UnaryOperator>(this)->print(buffer, indent);
    case EXPR_BUILTIN:
        return cast<BuiltinExpr>(this)->print(buffer, indent);
    case EXPR_ARRAYSUBSCRIPT:
        return cast<ArraySubscriptExpr>(this)->print(buffer, indent);
    case EXPR_MEMBER:
        return cast<MemberExpr>(this)->print(buffer, indent);
    case EXPR_PAREN:
        return cast<ParenExpr>(this)->print(buffer, indent);
    case EXPR_BITOFFSET:
        return cast<BitOffsetExpr>(this)->print(buffer, indent);
    case EXPR_CAST:
        return cast<ExplicitCastExpr>(this)->print(buffer, indent);
    }
}

void Expr::printLiteral(StringBuilder& buffer) const {
    switch (getKind()) {
    case EXPR_INTEGER_LITERAL:
        return cast<IntegerLiteral>(this)->printLiteral(buffer);
    case EXPR_FLOAT_LITERAL:
        return cast<FloatingLiteral>(this)->printLiteral(buffer);
    case EXPR_BOOL_LITERAL:
        break;
    case EXPR_CHAR_LITERAL:
        return cast<CharacterLiteral>(this)->printLiteral(buffer);
    case EXPR_STRING_LITERAL:
        return cast<StringLiteral>(this)->printLiteral(buffer);
    case EXPR_NIL:
        break;
    case EXPR_IDENTIFIER:
        return cast<IdentifierExpr>(this)->printLiteral(buffer);
    case EXPR_TYPE:
    case EXPR_CALL:
    case EXPR_INITLIST:
    case EXPR_DESIGNATOR_INIT:
        break;
    case EXPR_BINOP:
        return cast<BinaryOperator>(this)->printLiteral(buffer);
    case EXPR_CONDOP:
    case EXPR_UNARYOP:
    case EXPR_BUILTIN:
    case EXPR_ARRAYSUBSCRIPT:
        break;
    case EXPR_MEMBER:
        return cast<MemberExpr>(this)->printLiteral(buffer);
    case EXPR_PAREN:
        break;
    case EXPR_BITOFFSET:
        return cast<BitOffsetExpr>(this)->printLiteral(buffer);
    case EXPR_CAST:
        break;
    }
}

SourceLocation Expr::getLocation() const {
    switch (getKind()) {
    case EXPR_INTEGER_LITERAL:
    case EXPR_FLOAT_LITERAL:
    case EXPR_BOOL_LITERAL:
    case EXPR_CHAR_LITERAL:
    case EXPR_STRING_LITERAL:
    case EXPR_NIL:
    case EXPR_IDENTIFIER:
        return exprLoc;
    case EXPR_TYPE:
        return SourceLocation();
    case EXPR_CALL:
        return cast<CallExpr>(this)->getLocation();
    case EXPR_INITLIST:
        return cast<InitListExpr>(this)->getLocation();
    case EXPR_DESIGNATOR_INIT:
    case EXPR_BINOP:
        return exprLoc;
    case EXPR_CONDOP:
        return cast<ConditionalOperator>(this)->getLocation();
    case EXPR_UNARYOP:
    case EXPR_BUILTIN:
        return exprLoc;
    case EXPR_ARRAYSUBSCRIPT:
        return cast<ArraySubscriptExpr>(this)->getLocation();
    case EXPR_MEMBER:
        return cast<MemberExpr>(this)->getLocation();
    case EXPR_PAREN:
        return cast<ParenExpr>(this)->getLocation();
    case EXPR_BITOFFSET:
    case EXPR_CAST:
        return exprLoc;
    }
}

SourceLocation Expr::getLocStart() const {
    switch (getKind()) {
    case EXPR_INTEGER_LITERAL:
    case EXPR_FLOAT_LITERAL:
    case EXPR_BOOL_LITERAL:
    case EXPR_CHAR_LITERAL:
    case EXPR_STRING_LITERAL:
    case EXPR_NIL:
    case EXPR_IDENTIFIER:
    case EXPR_TYPE:
        break;
    case EXPR_CALL:
        return cast<CallExpr>(this)->getLocStart();
    case EXPR_INITLIST:
        return cast<InitListExpr>(this)->getLocStart();
    case EXPR_DESIGNATOR_INIT:
        return cast<DesignatedInitExpr>(this)->getLocStart();
    case EXPR_BINOP:
        return cast<BinaryOperator>(this)->getLocStart();
    case EXPR_CONDOP:
        break;
    case EXPR_UNARYOP:
        return cast<UnaryOperator>(this)->getLocStart();
    case EXPR_BUILTIN:
        break;
    case EXPR_ARRAYSUBSCRIPT:
        return cast<ArraySubscriptExpr>(this)->getLocStart();
    case EXPR_MEMBER:
        return cast<MemberExpr>(this)->getLocStart();
    case EXPR_PAREN:
        return cast<ParenExpr>(this)->getLocStart();
    case EXPR_BITOFFSET:
        return cast<BitOffsetExpr>(this)->getLocStart();
    case EXPR_CAST:
        break;
    }
    return getLocation();
}

SourceLocation Expr::getLocEnd() const {
    switch (getKind()) {
    case EXPR_INTEGER_LITERAL:
    case EXPR_FLOAT_LITERAL:
    case EXPR_BOOL_LITERAL:
    case EXPR_CHAR_LITERAL:
    case EXPR_STRING_LITERAL:
    case EXPR_NIL:
    case EXPR_IDENTIFIER:
    case EXPR_TYPE:
        break;
    case EXPR_CALL:
        return cast<CallExpr>(this)->getLocEnd();
    case EXPR_INITLIST:
        return cast<InitListExpr>(this)->getLocEnd();
    case EXPR_DESIGNATOR_INIT:
        return cast<DesignatedInitExpr>(this)->getLocEnd();
    case EXPR_BINOP:
        return cast<BinaryOperator>(this)->getLocEnd();
    case EXPR_CONDOP:
        break;
    case EXPR_UNARYOP:
        return cast<UnaryOperator>(this)->getLocEnd();
    case EXPR_BUILTIN:
        break;
    case EXPR_ARRAYSUBSCRIPT:
        return cast<ArraySubscriptExpr>(this)->getLocEnd();
    case EXPR_MEMBER:
        return cast<MemberExpr>(this)->getLocEnd();
    case EXPR_PAREN:
        return cast<ParenExpr>(this)->getLocEnd();
    case EXPR_BITOFFSET:
        return cast<BitOffsetExpr>(this)->getLocEnd();
    case EXPR_CAST:
        return cast<ExplicitCastExpr>(this)->getLocEnd();
    }
    return getLocation();
}

void IntegerLiteral::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "IntegerLiteral ";
    Expr::print(buffer);
    buffer.setColor(COL_VALUE);
    buffer << ' ';
    buffer.number(getRadix(), Value.getSExtValue());
    buffer << '\n';
}

void IntegerLiteral::printLiteral(StringBuilder& buffer) const {
    buffer << Value.getSExtValue();
}

void FloatingLiteral::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "FloatingLiteral ";
    Expr::print(buffer);
    char temp[20];
    sprintf(temp, "%f", Value.convertToFloat());
    buffer.setColor(COL_VALUE);
    buffer << ' ' << temp << '\n';
}


void BooleanLiteral::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "BooleanLiteral ";
    Expr::print(buffer);
    buffer.setColor(COL_VALUE);
    buffer << ' ' << getValue() << '\n';
}


void CharacterLiteral::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "CharacterLiteral ";
    Expr::print(buffer);
    buffer << ' ';
    buffer.setColor(COL_VALUE);
    printLiteral(buffer);
    buffer << '\n';
}

void CharacterLiteral::printLiteral(StringBuilder& buffer) const {
    buffer << '\'';
    switch (value) {
    case '\0':
        buffer << "\\0";
        break;
    case '\n':
        buffer << "\\n";
        break;
    case '\r':
        buffer << "\\r";
        break;
    case '\t':
        buffer << "\\t";
        break;
    case '\'':
        buffer << "\\'";
        break;
    case '\\':
        buffer << "\\\\";
        break;
    default:
        buffer << (char)value;
        break;
    }
    buffer << '\'';
}


void StringLiteral::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "StringLiteral ";
    Expr::print(buffer);
    buffer << " len=" << len;
    buffer.setColor(COL_VALUE);
    buffer << ' ';
    StringLiteral::printLiteral(buffer);
    buffer << '\n';
}

void StringLiteral::printLiteral(StringBuilder& buffer) const {
    StringBuilder escaped;
    const char* cp = value;
    buffer << '"';
    for (unsigned i=0; i<len; ++i) {
        switch (*cp) {
        case '\0':
            buffer << "\\0";
            break;
        case '\n':
            buffer << "\\n";
            break;
        case '\r':
            buffer << "\\r";
            break;
        case '\t':
            buffer << "\\t";
            break;
        case '\\':
            buffer << "\\\\";
            break;
        case '"':
            buffer << "\\\"";
            break;
        case '\033':
            buffer << "\\033";
            break;
        default:
            buffer << *cp;
            break;
        }
        cp++;
    }
    buffer << '"';
}


void NilExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "NilExpr\n";
}


static const char* reftype2str(IdentifierExpr::RefType ref) {
    switch(ref) {
    case IdentifierExpr::REF_UNRESOLVED:    return "unresolved";
    case IdentifierExpr::REF_MODULE:        return "module";
    case IdentifierExpr::REF_FUNC:          return "func";
    case IdentifierExpr::REF_TYPE:          return "type";
    case IdentifierExpr::REF_VAR:           return "var";
    case IdentifierExpr::REF_ENUM_CONSTANT: return "enum_constant";
    case IdentifierExpr::REF_STRUCT_MEMBER: return "struct_mem";
    case IdentifierExpr::REF_STRUCT_FUNC:   return "struct_func";
    case IdentifierExpr::REF_LABEL:         return "label";
    }
    FATAL_ERROR("should not come here");
    return "?";
}

const char* IdentifierExpr::getName() const {
    if (getRefType() != REF_UNRESOLVED) return decl->getName();
    return name;
}

void IdentifierExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "IdentifierExpr ";
    Expr::print(buffer);
    buffer.setColor(COL_VALUE);
    buffer << ' ' << getName();
    if (getRefType() == REF_UNRESOLVED) buffer.setColor(COL_INVALID);
    else buffer.setColor(COL_INFO);
    buffer << ' ' << reftype2str(getRefType());
    buffer << '\n';
}

void IdentifierExpr::printLiteral(StringBuilder& buffer) const {
    buffer << getName();
}


void TypeExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "TypeExpr ";
    getType().print(buffer);
}


void CallExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "CallExpr ";
    Expr::print(buffer);
    buffer.setColor(COL_VALUE);
    buffer << ' ';
    Fn->printLiteral(buffer);
    buffer << '\n';
    Fn->print(buffer, indent + INDENT);
    for (unsigned i=0; i<numArgs(); i++) {
        args[i]->print(buffer, indent + INDENT);
    }
}


InitListExpr::InitListExpr(SourceLocation left, SourceLocation right, Expr** values_, unsigned num)
    : Expr(EXPR_INITLIST, SourceLocation(), false)
    , leftBrace(left)
    , rightBrace(right)
    , values(values_)
{
    initListExprBits.HasDesignators = 0;
    numValues_ = num;
}

void InitListExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "InitListExpr ";
    Expr::print(buffer);
    if (hasDesignators()) {
        buffer << " designators=1";
    }
    buffer << '\n';
    for (unsigned i=0; i<numValues(); i++) {
        values[i]->print(buffer, indent + INDENT);
    }
}


void DesignatedInitExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "DesignatedInitExpr ";
    Expr::print(buffer);
    if (getDesignatorKind() == ARRAY_DESIGNATOR) {
        buffer << " array";
    } else {
        buffer << " field";
    }
    buffer << '\n';
    buffer.indent(indent);
    buffer.setColor(COL_ATTR);
    if (getDesignatorKind() == ARRAY_DESIGNATOR) {
        buffer << "Designator = [" << index.getSExtValue() << "]\n";
        designator->print(buffer, indent + INDENT);
    } else {
        buffer << "field=\n";
        field->print(buffer, indent + INDENT);
    }
    buffer.indent(indent);
    buffer.setColor(COL_ATTR);
    buffer << "InitValue=\n";
    initValue->print(buffer, indent + INDENT);
}


BinaryOperator::BinaryOperator(Expr* lhs_, Expr* rhs_, Opcode opc_, SourceLocation opLoc_)
    : Expr(EXPR_BINOP, opLoc_, false)
    , lhs(lhs_)
    , rhs(rhs_)
{
    binaryOperatorBits.opcode = opc_;
}

const char* BinaryOperator::OpCode2str(c2lang::BinaryOperatorKind opc_) {
    switch (opc_) {
    case BINOP_Mul: return "*";
    case BINOP_Div: return "/";
    case BINOP_Rem: return "%";
    case BINOP_Add: return "+";
    case BINOP_Sub: return "-";
    case BINOP_Shl: return "<<";
    case BINOP_Shr: return ">>";
    case BINOP_LT: return "<";
    case BINOP_GT: return ">";
    case BINOP_LE: return "<=";
    case BINOP_GE: return ">=";
    case BINOP_EQ: return "==";
    case BINOP_NE: return "!=";
    case BINOP_And: return "&";
    case BINOP_Xor: return "^";
    case BINOP_Or: return "|";
    case BINOP_LAnd: return "&&";
    case BINOP_LOr: return "||";
    case BINOP_Assign: return "=";
    case BINOP_MulAssign: return "*=";
    case BINOP_DivAssign: return "/=";
    case BINOP_RemAssign: return "%=";
    case BINOP_AddAssign: return "+=";
    case BINOP_SubAssign: return "-=";
    case BINOP_ShlAssign: return "<<=";
    case BINOP_ShrAssign: return ">>=";
    case BINOP_AndAssign: return "&=";
    case BINOP_XorAssign: return "^=";
    case BINOP_OrAssign: return "|=";
    case BINOP_Comma: return ",";
    }
    FATAL_ERROR("should not come here");
}

bool BinaryOperator::requiresParensForC() const
{
    switch (getOpcode()) {
    case BINOP_Mul:
    case BINOP_Div:
    case BINOP_Rem:
    case BINOP_Add:
    case BINOP_Sub:
    case BINOP_Shl:
    case BINOP_Shr:
    case BINOP_LT:
    case BINOP_GT:
    case BINOP_LE:
    case BINOP_GE:
    case BINOP_EQ:
    case BINOP_NE:
    case BINOP_And:
    case BINOP_Xor:
    case BINOP_Or:
    case BINOP_LAnd:
    case BINOP_LOr:
        return true;
    default:
        return false;
    }
}

void BinaryOperator::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "BinaryOperator ";
    Expr::print(buffer);
    buffer.setColor(COL_VALUE);
    buffer << " '" << OpCode2str(getOpcode()) << '\'';
    buffer << '\n';

    buffer.indent(indent + INDENT);
    buffer.setColor(COL_ATTR);
    buffer << "LHS=\n";
    lhs->print(buffer, indent + INDENT);
    buffer.indent(indent + INDENT);
    buffer.setColor(COL_ATTR);
    buffer << "RHS=\n";
    rhs->print(buffer, indent + INDENT);
}

void BinaryOperator::printLiteral(StringBuilder& buffer) const {
    lhs->printLiteral(buffer);
    buffer << OpCode2str(getOpcode());
    rhs->printLiteral(buffer);
}


ConditionalOperator::ConditionalOperator(SourceLocation questionLoc, SourceLocation colonLoc,
        Expr* cond_, Expr* lhs_, Expr* rhs_)
    : Expr(EXPR_CONDOP, SourceLocation(), false)
    , QuestionLoc(questionLoc)
    , ColonLoc(colonLoc)
    , cond(cond_)
    , lhs(lhs_)
    , rhs(rhs_)
{}

void ConditionalOperator::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "ConditionalOperator ";
    Expr::print(buffer);
    buffer << '\n';
    cond->print(buffer, indent + INDENT);
    lhs->print(buffer, indent + INDENT);
    rhs->print(buffer, indent + INDENT);
}


const char* UnaryOperator::OpCode2str(c2lang::UnaryOperatorKind opc_) {
    switch (opc_) {
    case UO_PostInc:    return "++";
    case UO_PostDec:    return "--";
    case UO_PreInc:     return "++";
    case UO_PreDec:     return "--";
    case UO_AddrOf:     return "&";
    case UO_Deref:      return "*";
    case UO_Minus:      return "-";
    case UO_Not:        return "~";
    case UO_LNot:       return "!";
    }
}

void UnaryOperator::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "UnaryOperator ";
    Expr::print(buffer);
    buffer.setColor(COL_ATTR);
    switch (getOpcode()) {
    case UO_PostInc:
    case UO_PostDec:
        buffer << " postfix";
        break;
    case UO_PreInc:
    case UO_PreDec:
        buffer << " prefix";
        break;
    default:
        break;
    }
    buffer.setColor(COL_VALUE);
    buffer << " '" << OpCode2str(getOpcode()) << "'\n";
    val->print(buffer, indent + INDENT);
}


void BuiltinExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "BuiltinExpr ";
    Expr::print(buffer);
    buffer.setColor(COL_ATTR);
    buffer << ' ' << Str(getBuiltinKind());
    buffer << " value=";
    buffer.number(10, value.getSExtValue());
    buffer << '\n';
    expr->print(buffer, indent + INDENT);
}

const char* BuiltinExpr::Str(BuiltinExpr::BuiltinKind kind) {
    switch (kind) {
    case BUILTIN_SIZEOF:        return "sizeof";
    case BUILTIN_ELEMSOF:       return "elemsof";
    case BUILTIN_ENUM_MIN:      return "enum_min";
    case BUILTIN_ENUM_MAX:      return "enum_max";
    }
    return "";
}

void ArraySubscriptExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "ArraySubscriptExpr ";
    Expr::print(buffer);
    buffer << '\n';
    base->print(buffer, indent + INDENT);
    idx->print(buffer, indent + INDENT);
}


void MemberExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "MemberExpr";
    buffer.setColor(COL_ATTR);
    if (isModulePrefix()) buffer << " mod-prefix";
    if (isStructFunction()) buffer << " struct-function";
    if (isStaticStructFunction()) buffer << " static-struct-function";
    buffer << ' ';
    Expr::print(buffer);
    buffer.setColor(COL_VALUE);
    buffer << ' ';
    Base->printLiteral(buffer);
    buffer << ' ';
    member->printLiteral(buffer);
    buffer << '\n';
    Base->print(buffer, indent + INDENT);
    member->print(buffer, indent + INDENT);
    buffer.indent(indent + INDENT);
    buffer.setColor(COL_ATTR);
    buffer << "decl=";
    if (decl) {
        buffer << decl->getName();
    } else {
        buffer.setColor(ANSI_RED);
        buffer << "NULL";
    }
    buffer << '\n';
}

void MemberExpr::printLiteral(StringBuilder& buffer) const {
    Base->printLiteral(buffer);
    buffer << '.';
    member->printLiteral(buffer);
}


void ParenExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "ParenExpr ";
    Expr::print(buffer);
    buffer << '\n';
    Val->print(buffer, indent + INDENT);
}


void BitOffsetExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "BitOffsetExpr ";
    Expr::print(buffer);
    buffer << '\n';

    buffer.indent(indent + INDENT);
    buffer.setColor(COL_ATTR);
    buffer << "LHS=\n";
    lhs->print(buffer, indent + INDENT);
    buffer.indent(indent + INDENT);
    buffer.setColor(COL_ATTR);
    buffer << "RHS=\n";
    rhs->print(buffer, indent + INDENT);
}


void BitOffsetExpr::printLiteral(StringBuilder& buffer) const {
    lhs->printLiteral(buffer);
    buffer << " : ";
    rhs->printLiteral(buffer);
}


void ExplicitCastExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "ExplicitCastExpr ";
    Expr::print(buffer);
    buffer << '\n';
    buffer.setColor(COL_ATTR);
    buffer.indent(indent + INDENT);
    buffer << "DEST: ";
    destType.print(buffer);
    buffer << '\n';
    inner->print(buffer, indent + INDENT);
}

