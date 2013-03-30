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

using namespace C2;
using namespace std;
using namespace clang;

//#define EXPR_DEBUG
#ifdef EXPR_DEBUG
#include <stdio.h>
static int creationCount;
static int deleteCount;
#endif

// TODO doesn't clang have a function for this?
static const char* BinOpCode2str(clang::BinaryOperatorKind opc) {
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

static const char* UnaryOpCode2str(clang::UnaryOperatorKind opc) {
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
}


Expr::Expr()
    : isStatement(false)
{
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

STMT_VISITOR_ACCEPT(Expr);

EXPR_VISITOR_ACCEPT(NumberExpr);

void NumberExpr::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[number " << (int)value << "]\n";
}

void NumberExpr::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << (int)value;
}


EXPR_VISITOR_ACCEPT(StringExpr);

void StringExpr::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[text '" << value << "']\n";
}

void StringExpr::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << '"' << value << '"';
}


EXPR_VISITOR_ACCEPT(BoolLiteralExpr);

void BoolLiteralExpr::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[bool " << value << "]\n";
}

void BoolLiteralExpr::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << (int)value;
}


EXPR_VISITOR_ACCEPT(CharLiteralExpr);

void CharLiteralExpr::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[char '" << (char)value << "']\n";
}

void CharLiteralExpr::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << '\'';
    buffer << (char)value;
    buffer << '\'';
}


CallExpr::~CallExpr() {}

EXPR_VISITOR_ACCEPT(CallExpr);

void CallExpr::addArg(Expr* arg) {
    args.push_back(arg);
}

void CallExpr::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    //buffer << "[call " << expr2name(Fn) << "]\n";
    buffer << "[call " << "TODO" << "]\n";
    for (unsigned int i=0; i<args.size(); i++) {
        args[i]->print(indent + INDENT, buffer);
    }
}

void CallExpr::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    Fn->generateC(0, buffer);
    buffer << '(';
    for (unsigned int i=0; i<args.size(); i++) {
        if (i != 0) buffer << ", ";
        args[i]->generateC(0, buffer);
    }
    buffer << ')';
    if (isStmt()) buffer << ";\n";
}


EXPR_VISITOR_ACCEPT(IdentifierExpr);

void IdentifierExpr::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[identifier " << getName() << "]\n";
}

void IdentifierExpr::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << name;
}

TypeExpr::~TypeExpr() {
}

EXPR_VISITOR_ACCEPT(TypeExpr);

void TypeExpr::print(int indent, StringBuilder& buffer) {
    if (type) type->print(indent, buffer);
}

void TypeExpr::generateC(int indent, StringBuilder& buffer) {
    printf("%s() TODO\n", __PRETTY_FUNCTION__);
}



InitListExpr::InitListExpr(SourceLocation left, SourceLocation right, ExprList& values_)
    : leftBrace(left)
    , rightBrace(right)
    , values(values_)
{}

InitListExpr::~InitListExpr() {
    for (unsigned int i=0; i<values.size(); i++) {
        delete values[i];
    }
}

EXPR_VISITOR_ACCEPT(InitListExpr);

void InitListExpr::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[initlist]\n";
    for (unsigned int i=0; i<values.size(); i++) {
        values[i]->print(indent + INDENT, buffer);
    }
    
}

void InitListExpr::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "{ ";
    for (unsigned int i=0; i<values.size(); i++) {
        if (i != 0) buffer << ", ";
        values[i]->generateC(0, buffer);
    }
    buffer << " }";
}


DeclExpr::DeclExpr(const std::string& name_, SourceLocation& loc_,
            Type* type_, Expr* initValue_)
    : name(name_)
    , loc(loc_)
    , type(type_)
    , initValue(initValue_)
{}

DeclExpr::~DeclExpr() {}

EXPR_VISITOR_ACCEPT(DeclExpr);

void DeclExpr::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[decl " << name << "]\n";
    indent += INDENT;
    type->print(indent, buffer);
    if (initValue) {
        buffer.indent(indent);
        buffer << "initial:\n";
        initValue->print(indent+INDENT, buffer);
    }
}

void DeclExpr::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    type->generateC_PreName(buffer);
    buffer << ' ' << name;
    type->generateC_PostName(buffer);
    if (initValue) {
        buffer << " = ";
        initValue->generateC(0, buffer);
    }
    if (isStmt()) buffer << ";\n";
}

void DeclExpr::generateC(StringBuilder& buffer, const std::string& pkgName) {
    type->generateC_PreName(buffer);
    buffer << ' ';
    Utils::addName(pkgName, name, buffer);
    type->generateC_PostName(buffer);
    if (initValue) {
        buffer << " = ";
        initValue->generateC(0, buffer);
    }
    if (isStmt()) buffer << ";\n";
}

BinOpExpr::BinOpExpr(Expr* lhs_, Expr* rhs_, Opcode opc_, SourceLocation opLoc_)
    : opLoc(opLoc_)
    , opc(opc_)
    , lhs(lhs_)
    , rhs(rhs_)
{}

BinOpExpr::~BinOpExpr() {
    delete lhs;
    delete rhs;
}

EXPR_VISITOR_ACCEPT(BinOpExpr);

void BinOpExpr::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[binop " << BinOpCode2str(opc) << "]\n";
    lhs->print(indent + INDENT, buffer);
    rhs->print(indent + INDENT, buffer);
}

void BinOpExpr::generateC(int indent, StringBuilder& buffer) {
    lhs->generateC(indent, buffer);
    buffer << ' ' << BinOpCode2str(opc) << ' ';
    rhs->generateC(0, buffer);
    if (isStmt()) buffer << ";\n";
}


UnaryOpExpr::UnaryOpExpr(SourceLocation opLoc_, Opcode opc_, Expr* val_)
    : opLoc(opLoc_)
    , opc(opc_)
    , val(val_)
{}

UnaryOpExpr::~UnaryOpExpr() {
    delete val;
}

EXPR_VISITOR_ACCEPT(UnaryOpExpr);

void UnaryOpExpr::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[unaryop " << UnaryOpCode2str(opc) << "]\n";
    val->print(indent + INDENT, buffer);
}

void UnaryOpExpr::generateC(int indent, StringBuilder& buffer) {
    switch (opc) {
    case UO_PostInc:
    case UO_PostDec:
        val->generateC(indent, buffer);
        buffer << UnaryOpCode2str(opc);
        break;
    case UO_PreInc:
    case UO_PreDec:
    case UO_AddrOf:
    case UO_Deref:
    case UO_Plus:
    case UO_Minus:
    case UO_Not:
    case UO_LNot:
        buffer.indent(indent);
        buffer << UnaryOpCode2str(opc);
        val->generateC(0, buffer);
        break;
    default:
        assert(0);
    }
}


SizeofExpr::SizeofExpr(SourceLocation Loc_, Expr* expr_)
    : Loc(Loc_)
    , expr(expr_)
{}

SizeofExpr::~SizeofExpr() {
    delete expr;
}

EXPR_VISITOR_ACCEPT(SizeofExpr);

void SizeofExpr::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[sizeof]\n";
    expr->print(indent + INDENT, buffer);
}

void SizeofExpr::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "sizeof(";
    expr->generateC(0, buffer);
    buffer << ")";
    if (isStmt()) buffer << ";\n";
}


ArraySubscriptExpr::ArraySubscriptExpr(SourceLocation RLoc_, Expr* Base_, Expr* Idx_)
    : RLoc(RLoc_)
    , base(Base_)
    , idx(Idx_)
{}

ArraySubscriptExpr::~ArraySubscriptExpr() {
    delete base;
    delete idx;
}

EXPR_VISITOR_ACCEPT(ArraySubscriptExpr);

void ArraySubscriptExpr::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[arraysubscript]\n";
    base->print(indent + INDENT, buffer);
    idx->print(indent + INDENT, buffer);
}

void ArraySubscriptExpr::generateC(int indent, StringBuilder& buffer) {
    base->generateC(indent, buffer);
    buffer << '[';
    idx->generateC(0, buffer);
    buffer << ']';
}


MemberExpr::~MemberExpr() {
    delete Base;
    delete Member;
}

EXPR_VISITOR_ACCEPT(MemberExpr);

void MemberExpr::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[member expr]\n";
    Base->print(indent + INDENT, buffer);
    Member->print(indent + INDENT, buffer);
}

void MemberExpr::generateC(int indent, StringBuilder& buffer) {
    Base->generateC(indent, buffer);
    if (isArrow) buffer << "->";
    else buffer << '.';
    Member->generateC(0, buffer);
}

const char* MemberExpr::getFullName() const {
    // TODO use recursion;
    return "TODO";
}


ParenExpr::~ParenExpr() {
    delete Val;
}

EXPR_VISITOR_ACCEPT(ParenExpr);

void ParenExpr::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[paren expr]\n";
    Val->print(indent + INDENT, buffer);
}

void ParenExpr::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << '(';
    Val->generateC(0, buffer);
    buffer << ')';
}

