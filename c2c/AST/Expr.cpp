/* Copyright 2013,2014 Bas van den Berg
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
#include "Utils/Utils.h"
#include "Utils/color.h"
#include "Utils/constants.h"

using namespace C2;
using namespace std;
using namespace clang;

//#define EXPR_DEBUG
#ifdef EXPR_DEBUG
static int creationCount;
static int deleteCount;
#endif


Expr::Expr(ExprKind k, bool isConstant_)
    : Stmt(STMT_EXPR)
{
    StmtBits.eKind = k;
    StmtBits.ExprIsConstant = isConstant_;
    StmtBits.ExprImpCast = BuiltinType::Void;
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

static const char* ctc_strings[] = { "none", "partial", "full" };

void Expr::print(StringBuilder& buffer, unsigned indent) const {
    QualType Q = getType();
    Q.print(buffer);
    buffer.setColor(COL_ATTR);
    buffer << " ctc=" << ctc_strings[StmtBits.ExprIsCTC];
    buffer << ", constant=" << isConstant();
    if (getImpCast() != BuiltinType::Void) {
        buffer << ", cast=" << BuiltinType::kind2name(getImpCast());
    }
}

void IntegerLiteral::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "IntegerLiteral ";
    Expr::print(buffer, 0);
    buffer.setColor(COL_VALUE);
    buffer << ' ' << Value.getSExtValue() << '\n';
}

void IntegerLiteral::printLiteral(StringBuilder& buffer) const {
    buffer << Value.getSExtValue();
}


void FloatingLiteral::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "FloatingLiteral ";
    Expr::print(buffer, 0);
    char temp[20];
    sprintf(temp, "%f", Value.convertToFloat());
    buffer.setColor(COL_VALUE);
    buffer << ' ' << temp << '\n';
}


void BooleanLiteral::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "BooleanLiteral ";
    Expr::print(buffer, 0);
    buffer.setColor(COL_VALUE);
    buffer << ' ' << getValue() << '\n';
}


void CharacterLiteral::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "CharacterLiteral ";
    Expr::print(buffer, 0);
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
        buffer << "\\n";
        break;
    case '\t':
        buffer << "\\n";
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
    Expr::print(buffer, 0);
    buffer.setColor(COL_VALUE);
    buffer << " '" << value << "'\n";
}


void NilExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "NilExpr\n";
}



const std::string& IdentifierExpr::getName() const {
    if (decl) return decl->getName();
    return name;
}

void IdentifierExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "IdentifierExpr ";
    Expr::print(buffer, 0);
    buffer.setColor(COL_VALUE);
    buffer << ' ' << getName();
    buffer.setColor(COL_ATTR);
    if (decl) buffer << " <RESOLVED>";
    else buffer << " <UNRESOLVED>";
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


CallExpr::~CallExpr() {}

void CallExpr::addArg(Expr* arg) {
    args.push_back(arg);
}

void CallExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "CallExpr ";
    Expr::print(buffer, 0);
    buffer.setColor(COL_VALUE);
    buffer << ' ';
    Fn->printLiteral(buffer);
    buffer << '\n';
    Fn->print(buffer, indent + INDENT);
    for (unsigned i=0; i<args.size(); i++) {
        args[i]->print(buffer, indent + INDENT);
    }
}


InitListExpr::InitListExpr(SourceLocation left, SourceLocation right, ExprList& values_)
    : Expr(EXPR_INITLIST, false)
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
    buffer.setColor(COL_EXPR);
    buffer << "InitListExpr ";
    Expr::print(buffer, 0);
    if (hasDesignators()) {
        buffer << " designators=1";
    }
    buffer << '\n';
    for (unsigned i=0; i<values.size(); i++) {
        values[i]->print(buffer, indent + INDENT);
    }
}


DesignatedInitExpr::~DesignatedInitExpr() {
    delete designator;
    delete initValue;
}

void DesignatedInitExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "DesignatedInitExpr ";
    Expr::print(buffer, 0);
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
        buffer << "field = ";
        buffer.setColor(COL_VALUE);
        buffer << '\'' << field << '\'' << '\n';
    }
    buffer.indent(indent);
    buffer.setColor(COL_ATTR);
    buffer << "InitValue=\n";
    initValue->print(buffer, indent + INDENT);
}


DeclExpr::DeclExpr(VarDecl* decl_)
    : Expr(EXPR_DECL, true)
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

QualType DeclExpr::getDeclType() const { return decl->getType(); }

Expr* DeclExpr::getInitValue() const { return decl->getInitValue(); }

bool DeclExpr::hasLocalQualifier() const { return decl->hasLocalQualifier(); }


BinaryOperator::BinaryOperator(Expr* lhs_, Expr* rhs_, Opcode opc_, SourceLocation opLoc_)
    : Expr(EXPR_BINOP, false)
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
    buffer.setColor(COL_EXPR);
    buffer << "BinaryOperator ";
    Expr::print(buffer, 0);
    buffer.setColor(COL_VALUE);
    buffer << " '" << OpCode2str(opc) << '\'';
    buffer << '\n';

    buffer.indent(indent);
    buffer.setColor(COL_ATTR);
    buffer << "LHS=\n";
    lhs->print(buffer, indent + INDENT);
    buffer.indent(indent);
    buffer.setColor(COL_ATTR);
    buffer << "RHS=\n";
    rhs->print(buffer, indent + INDENT);
}

void BinaryOperator::printLiteral(StringBuilder& buffer) const {
    lhs->printLiteral(buffer);
    buffer << OpCode2str(opc);
    rhs->printLiteral(buffer);
}


ConditionalOperator::ConditionalOperator(SourceLocation questionLoc, SourceLocation colonLoc,
                Expr* cond_, Expr* lhs_, Expr* rhs_)
    : Expr(EXPR_CONDOP, false)
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
    buffer.setColor(COL_EXPR);
    buffer << "ConditionalOperator ";
    Expr::print(buffer, 0);
    buffer << '\n';
    cond->print(buffer, indent + INDENT);
    lhs->print(buffer, indent + INDENT);
    rhs->print(buffer, indent + INDENT);
}


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
    buffer.setColor(COL_EXPR);
    buffer << "UnaryOperator ";
    Expr::print(buffer, 0);
    buffer.setColor(COL_ATTR);
    switch (opc) {
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
    buffer << " '" << OpCode2str(opc) << "'\n";
    val->print(buffer, indent + INDENT);
}


BuiltinExpr::~BuiltinExpr() {
    delete expr;
}

void BuiltinExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "BuiltinExpr ";
    Expr::print(buffer, 0);
    buffer.setColor(COL_ATTR);
    if (isSizeof()) buffer << " sizeof";
    else buffer << " elemsof";
    buffer << '\n';
    expr->print(buffer, indent + INDENT);
}


ArraySubscriptExpr::~ArraySubscriptExpr() {
    delete base;
    delete idx;
}

void ArraySubscriptExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "ArraySubscriptExpr ";
    Expr::print(buffer, 0);
    buffer << '\n';
    base->print(buffer, indent + INDENT);
    idx->print(buffer, indent + INDENT);
}


MemberExpr::~MemberExpr() {
    delete Base;
}

void MemberExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "MemberExpr";
    buffer.setColor(COL_ATTR);
    if (isModulePrefix()) buffer << " mod-prefix";
    buffer << ' ';
    Expr::print(buffer, 0);
    buffer << '\n';
    buffer.indent(indent);
    buffer.setColor(COL_ATTR);
    buffer << "LHS=\n";
    Base->print(buffer, indent + INDENT);
    buffer.indent(indent);
    buffer.setColor(COL_ATTR);
    buffer << "RHS=";
    buffer.setColor(COL_VALUE);
    buffer << member << '\n';
    buffer.indent(indent);
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
    buffer << '.' << member;
}


ParenExpr::~ParenExpr() {
    delete Val;
}

void ParenExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "ParenExpr ";
    Expr::print(buffer, 0);
    buffer << '\n';
    Val->print(buffer, indent + INDENT);
}

