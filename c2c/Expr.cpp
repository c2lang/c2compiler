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

#include "Expr.h"
#include "StringBuilder.h"
#include "Utils.h"
#include "Type.h"
#include "Decl.h"
#include "Package.h"
#include "color.h"

using namespace C2;
using namespace std;
using namespace clang;

//#define EXPR_DEBUG
#ifdef EXPR_DEBUG
#include <stdio.h>
static int creationCount;
static int deleteCount;
#endif


Expr::Expr(ExprKind k)
    : Stmt(STMT_EXPR)
{
    StmtBits.eKind = k;
#ifdef EXPR_DEBUG
    creationCount++;
    fprintf(stderr, "[EXPR] create %p  created %d deleted %d\n", this, creationCount, deleteCount);
#endif
}

Expr::~Expr() {
#ifdef EXPR_DEBUG
    deleteCount++;
    fprintf(stderr, "[EXPR] delete %p  created %d deleted %d\n", this, creationCount, deleteCount);
#endif
}


void IntegerLiteral::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[integer " << Value.getSExtValue() << "]\n";
}


void FloatingLiteral::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    char temp[20];
    sprintf(temp, "%f", Value.convertToFloat());
    buffer << "[float " << temp << "]\n";
}


void StringLiteral::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[text '" << value << "']\n";
}


void BooleanLiteral::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[bool " << getValue() << "]\n";
}


void CharacterLiteral::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[char '" << (char)value << "']\n";
}


CallExpr::~CallExpr() {}

void CallExpr::addArg(Expr* arg) {
    args.push_back(arg);
}


static void expr2name(Expr* expr, StringBuilder& buffer) {
    switch (expr->getKind()) {
    case EXPR_INTEGER_LITERAL:
    case EXPR_STRING_LITERAL:
    case EXPR_BOOL_LITERAL:
    case EXPR_CHAR_LITERAL:
    case EXPR_FLOAT_LITERAL:
    case EXPR_CALL:
        break;
    case EXPR_IDENTIFIER:
        {
            IdentifierExpr* id = cast<IdentifierExpr>(expr);
            buffer << id->getName();
            return;
        }
    case EXPR_INITLIST:
    case EXPR_TYPE:
    case EXPR_DECL:
    case EXPR_BINOP:
    case EXPR_CONDOP:
    case EXPR_UNARYOP:
    case EXPR_BUILTIN:
    case EXPR_ARRAYSUBSCRIPT:
        break;
    case EXPR_MEMBER:
        {
            MemberExpr* member = cast<MemberExpr>(expr);
            expr2name(member->getBase(), buffer);
            buffer << '.';
            buffer << member->getMember()->getName();
            return;
        }
    case EXPR_PAREN:
        break;
    };
    buffer << "?";
}


void CallExpr::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[call ";
    expr2name(Fn, buffer);
    buffer << "]\n";
    Fn->print(indent + INDENT, buffer);
    for (unsigned int i=0; i<args.size(); i++) {
        args[i]->print(indent + INDENT, buffer);
    }
}


void IdentifierExpr::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[identifier ";
    if (pkg) {
        buffer << ANSI_CYAN << pkg->getName() << '.' << ANSI_NORMAL;
    }
     buffer << getName() << "]";
    if (decl) buffer << " <RESOLVED>";
    buffer << '\n';
}


TypeExpr::~TypeExpr() {
}

void TypeExpr::print(int indent, StringBuilder& buffer) const {
    QT.print(indent, buffer, QualType::RECURSE_NONE);
}


InitListExpr::InitListExpr(SourceLocation left, SourceLocation right, ExprList& values_)
    : Expr(EXPR_INITLIST)
    , leftBrace(left)
    , rightBrace(right)
    , values(values_)
{}

InitListExpr::~InitListExpr() {
    for (unsigned int i=0; i<values.size(); i++) {
        delete values[i];
    }
}

void InitListExpr::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[initlist]\n";
    for (unsigned int i=0; i<values.size(); i++) {
        values[i]->print(indent + INDENT, buffer);
    }
}


DeclExpr::DeclExpr(const std::string& name_, SourceLocation& loc_,
            QualType type_, Expr* initValue_)
    : Expr(EXPR_DECL)
    , name(name_)
    , loc(loc_)
    , type(type_)
    , initValue(initValue_)
{}

DeclExpr::~DeclExpr() {}

void DeclExpr::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[decl " << name;
    if (hasLocalQualifier()) buffer << " LOCAL";
    buffer << "]\n";
    indent += INDENT;
    // Dont print types for enums, otherwise we get a loop since Type have Decls etc
    if (!type->isEnumType()) {
        type->print(indent, buffer, QualType::RECURSE_ONCE);
    }
    if (initValue) {
        buffer.indent(indent);
        buffer << "initial:\n";
        initValue->print(indent+INDENT, buffer);
    }
}


BinaryOperator::BinaryOperator(Expr* lhs_, Expr* rhs_, Opcode opc_, SourceLocation opLoc_)
    : Expr(EXPR_BINOP)
    , opLoc(opLoc_)
    , opc(opc_)
    , lhs(lhs_)
    , rhs(rhs_)
{}

BinaryOperator::~BinaryOperator() {
    delete lhs;
    delete rhs;
}

const char* BinaryOperator::OpCode2str(clang::BinaryOperatorKind opc) {
    switch (opc) {
        case BO_PtrMemD: return ".";
        case BO_PtrMemI: return "->";
        case BO_Mul: return "*";
        case BO_Div: return "/";
        case BO_Rem: return "%";
        case BO_Add: return "+";
        case BO_Sub: return "-";
        case BO_Shl: return "<<";
        case BO_Shr: return ">>";
        case BO_LT: return "<";
        case BO_GT: return ">";
        case BO_LE: return "<=";
        case BO_GE: return ">=";
        case BO_EQ: return "==";
        case BO_NE: return "!=";
        case BO_And: return "&";
        case BO_Xor: return "^";
        case BO_Or: return "|";
        case BO_LAnd: return "&&";
        case BO_LOr: return "||";
        case BO_Assign: return "=";
        case BO_MulAssign: return "*=";
        case BO_DivAssign: return "/=";
        case BO_RemAssign: return "%=";
        case BO_AddAssign: return "+=";
        case BO_SubAssign: return "-+";
        case BO_ShlAssign: return "<<=";
        case BO_ShrAssign: return ">>=";
        case BO_AndAssign: return "&=";
        case BO_XorAssign: return "^=";
        case BO_OrAssign: return "|=";
        case BO_Comma: return ",";
    }
}

void BinaryOperator::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[binop " << OpCode2str(opc) << "]\n";
    lhs->print(indent + INDENT, buffer);
    rhs->print(indent + INDENT, buffer);
}


ConditionalOperator::ConditionalOperator(SourceLocation questionLoc, SourceLocation colonLoc,
                Expr* cond_, Expr* lhs_, Expr* rhs_)
    : Expr(EXPR_CONDOP)
    , QuestionLoc(questionLoc)
    , ColonLoc(colonLoc)
    , cond(cond_)
    , lhs(lhs_)
    , rhs(rhs_)
{}

ConditionalOperator::~ConditionalOperator() {
    delete cond;
    delete lhs;
    delete rhs;
}

void ConditionalOperator::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[condop]\n";
    cond->print(indent + INDENT, buffer);
    lhs->print(indent + INDENT, buffer);
    rhs->print(indent + INDENT, buffer);
}


UnaryOperator::UnaryOperator(SourceLocation opLoc_, Opcode opc_, Expr* val_)
    : Expr(EXPR_UNARYOP)
    , opLoc(opLoc_)
    , opc(opc_)
    , val(val_)
{}

UnaryOperator::~UnaryOperator() {
    delete val;
}

const char* UnaryOperator::OpCode2str(clang::UnaryOperatorKind opc) {
    switch (opc) {
    case UO_PostInc:    return "++";
    case UO_PostDec:    return "--";
    case UO_PreInc:     return "++";
    case UO_PreDec:     return "--";
    case UO_AddrOf:     return "&";
    case UO_Deref:      return "*";
    case UO_Plus:       return "+";
    case UO_Minus:      return "-";
    case UO_Not:        return "~";
    case UO_LNot:       return "!";
    default:
        assert(0);
        break;
    }
    return "";
}

void UnaryOperator::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[unaryop " << OpCode2str(opc) << "]\n";
    val->print(indent + INDENT, buffer);
}


BuiltinExpr::~BuiltinExpr() {
    delete expr;
}

void BuiltinExpr::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    if (isSizeof()) buffer << "[sizeof]\n";
    else buffer << "[elemsof]\n";
    expr->print(indent + INDENT, buffer);
}


ArraySubscriptExpr::ArraySubscriptExpr(SourceLocation RLoc_, Expr* Base_, Expr* Idx_)
    : Expr(EXPR_ARRAYSUBSCRIPT)
    , RLoc(RLoc_)
    , base(Base_)
    , idx(Idx_)
{}

ArraySubscriptExpr::~ArraySubscriptExpr() {
    delete base;
    delete idx;
}

void ArraySubscriptExpr::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[arraysubscript]\n";
    base->print(indent + INDENT, buffer);
    idx->print(indent + INDENT, buffer);
}


MemberExpr::~MemberExpr() {
    delete Base;
    delete Member;
}

void MemberExpr::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[member expr]\n";
    Base->print(indent + INDENT, buffer);
    Member->print(indent + INDENT, buffer);
}

const char* MemberExpr::getFullName() const {
    // TODO use recursion;
    return "TODO";
}


ParenExpr::~ParenExpr() {
    delete Val;
}

void ParenExpr::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[paren expr]\n";
    Val->print(indent + INDENT, buffer);
}

