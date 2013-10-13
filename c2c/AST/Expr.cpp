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

#include "AST/Expr.h"
#include "AST/Type.h"
#include "AST/Decl.h"
#include "AST/Package.h"
#include "Utils/StringBuilder.h"
#include "Utils/Utils.h"
#include "Utils/color.h"
#include "Utils/constants.h"

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


void IntegerLiteral::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "[IntegerLiteral " << Value.getSExtValue() << "]\n";
}


void FloatingLiteral::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    char temp[20];
    sprintf(temp, "%f", Value.convertToFloat());
    buffer << "[FloatingLiteral " << temp << "]\n";
}


void StringLiteral::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "[StringLiteral '" << value << "']\n";
}


void BooleanLiteral::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "[BooleanLiteral " << getValue() << "]\n";
}


void CharacterLiteral::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "[CharacterLiteral '" << (char)value << "']\n";
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


void CallExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "[CallExpr ";
    expr2name(Fn, buffer);
    buffer << "]\n";
    Fn->print(buffer, indent + INDENT);
    for (unsigned i=0; i<args.size(); i++) {
        args[i]->print(buffer, indent + INDENT);
    }
}


void IdentifierExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "[IdentifierExpr ";
    if (pkg) {
        buffer << ANSI_CYAN << pkg->getName() << '.' << ANSI_NORMAL;
    }
     buffer << getName() << "]";
    if (decl) buffer << " <RESOLVED>";
    buffer << '\n';
}


void TypeExpr::print(StringBuilder& buffer, unsigned indent) const {
    //QT.print(buffer, indent, QualType::RECURSE_NONE);
    QT.debugPrint(buffer, indent);
}


InitListExpr::InitListExpr(SourceLocation left, SourceLocation right, ExprList& values_)
    : Expr(EXPR_INITLIST)
    , leftBrace(left)
    , rightBrace(right)
    , values(values_)
{}

InitListExpr::~InitListExpr() {
    for (unsigned i=0; i<values.size(); i++) {
        delete values[i];
    }
}

void InitListExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "[InitListExpr]\n";
    for (unsigned i=0; i<values.size(); i++) {
        values[i]->print(buffer, indent + INDENT);
    }
}


DeclExpr::DeclExpr(VarDecl* decl_)
    : Expr(EXPR_DECL)
    , decl(decl_)
{}

DeclExpr::~DeclExpr() {
    delete decl;
}

void DeclExpr::print(StringBuilder& buffer, unsigned indent) const {
    decl->print(buffer, indent);
}

const std::string& DeclExpr::getName() const { return decl->getName(); }

clang::SourceLocation DeclExpr::getLocation() const {
    return decl->getLocation();
}

QualType DeclExpr::getType() const { return decl->getType(); }

Expr* DeclExpr::getInitValue() const { return decl->getInitValue(); }

bool DeclExpr::hasLocalQualifier() const { return decl->hasLocalQualifier(); }


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

void BinaryOperator::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "[BinaryOperator " << OpCode2str(opc) << "]\n";
    lhs->print(buffer, indent + INDENT);
    rhs->print(buffer, indent + INDENT);
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

void ConditionalOperator::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "[ConditionalOperator]\n";
    cond->print(buffer, indent + INDENT);
    lhs->print(buffer, indent + INDENT);
    rhs->print(buffer, indent + INDENT);
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

void UnaryOperator::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "[UnaryOperator " << OpCode2str(opc) << "]\n";
    val->print(buffer, indent + INDENT);
}


BuiltinExpr::~BuiltinExpr() {
    delete expr;
}

void BuiltinExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "[BuiltinExpr ";
    if (isSizeof()) buffer << "sizeof]\n";
    else buffer << "elemsof]\n";
    expr->print(buffer, indent + INDENT);
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

void ArraySubscriptExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "[ArraySubscriptExpr]\n";
    base->print(buffer, indent + INDENT);
    idx->print(buffer, indent + INDENT);
}


MemberExpr::~MemberExpr() {
    delete Base;
    delete Member;
}

void MemberExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "[MemberExpr]\n";
    Base->print(buffer, indent + INDENT);
    Member->print(buffer, indent + INDENT);
}

const char* MemberExpr::getFullName() const {
    // TODO use recursion;
    return "TODO";
}


ParenExpr::~ParenExpr() {
    delete Val;
}

void ParenExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "[ParenExpr]\n";
    Val->print(buffer, indent + INDENT);
}

