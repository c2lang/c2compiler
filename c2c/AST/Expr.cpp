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
static int creationCount;
static int deleteCount;
#endif


static void expr2name(Expr* expr, StringBuilder& buffer) {
    switch (expr->getKind()) {
    case EXPR_INTEGER_LITERAL:
    case EXPR_FLOAT_LITERAL:
    case EXPR_BOOL_LITERAL:
    case EXPR_CHAR_LITERAL:
    case EXPR_STRING_LITERAL:
    case EXPR_NIL:
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
    if (Q.isValid()) {
        Q->DiagName(buffer);
    } else {
        buffer << ANSI_RED"<INVALID>"ANSI_NORMAL;
    }
    buffer << " ctc=" << ctc_strings[StmtBits.ExprIsCTC];
    buffer << ", constant=" << isConstant();
    if (getImpCast() != BuiltinType::Void) {
        buffer << ", cast=" << BuiltinType::kind2name(getImpCast());
    }
}

void IntegerLiteral::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "IntegerLiteral ";
    Expr::print(buffer, 0);
    buffer << ' ' << Value.getSExtValue() << '\n';
}


void FloatingLiteral::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "FloatingLiteral ";
    Expr::print(buffer, 0);
    char temp[20];
    sprintf(temp, "%f", Value.convertToFloat());
    buffer << ' ' << temp << '\n';
}


void BooleanLiteral::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "BooleanLiteral ";
    Expr::print(buffer, 0);
    buffer << ' ' << getValue() << '\n';
}


void CharacterLiteral::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "CharacterLiteral ";
    Expr::print(buffer, 0);
    buffer << " '" << (char)value << "'\n";
}


void StringLiteral::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "StringLiteral ";
    Expr::print(buffer, 0);
    buffer << " '" << value << "'\n";
}


void NilExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "NilExpr\n";
}


void IdentifierExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "IdentifierExpr ";
    Expr::print(buffer, 0);
    buffer << ' ' << getName();
    if (decl) buffer << " <RESOLVED>";
    else buffer << " <UNRESOLVED>";
    buffer << '\n';
}


void TypeExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "TypeExpr ";
    getType().debugPrint(buffer, 0);
}


CallExpr::~CallExpr() {}

void CallExpr::addArg(Expr* arg) {
    args.push_back(arg);
}

void CallExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "CallExpr ";
    Expr::print(buffer, 0);
    buffer << ' ';
    expr2name(Fn, buffer);
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
    buffer << "InitListExpr ";
    Expr::print(buffer, 0);
    buffer << '\n';
    for (unsigned i=0; i<values.size(); i++) {
        values[i]->print(buffer, indent + INDENT);
    }
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
    buffer << "BinaryOperator ";
    Expr::print(buffer, 0);
    buffer << " '" << OpCode2str(opc) << '\'';
    buffer << '\n';

    lhs->print(buffer, indent + INDENT);
    rhs->print(buffer, indent + INDENT);
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
    buffer << "UnaryOperator ";
    Expr::print(buffer, 0);
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
    buffer << " '" << OpCode2str(opc) << "'\n";
    val->print(buffer, indent + INDENT);
}


BuiltinExpr::~BuiltinExpr() {
    delete expr;
}

void BuiltinExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "BuiltinExpr ";
    Expr::print(buffer, 0);
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
    buffer << "ArraySubscriptExpr ";
    Expr::print(buffer, 0);
    buffer << '\n';
    base->print(buffer, indent + INDENT);
    idx->print(buffer, indent + INDENT);
}


MemberExpr::~MemberExpr() {
    delete Base;
    delete Member;
}

void MemberExpr::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer << "MemberExpr";
    if (isPkgPrefix()) buffer << " (pkg-prefix)";
    buffer << ' ';
    Expr::print(buffer, 0);
    buffer << '\n';
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
    buffer << "ParenExpr ";
    Expr::print(buffer, 0);
    buffer << '\n';
    Val->print(buffer, indent + INDENT);
}

