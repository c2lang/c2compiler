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

//#define EXPR_DEBUG
#ifdef EXPR_DEBUG
#include <stdio.h>
static int creationCount;
static int deleteCount;
#endif

static const char* bin_names[] = {
    "+",
    "-",
    "*",
    "/",
    "=",
    "<",
};

static const char* binop2str(BinOp op) {
    assert(op < BOP_MAX);
    return bin_names[op];
}

static const char* unary_names[] = {
};

static const char* unary2str(UnaryOp op) {
    assert(op < OP_MAX);
    return unary_names[op];
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


CallExpr::~CallExpr() {
    // TODO delete args
}

EXPR_VISITOR_ACCEPT(CallExpr);

void CallExpr::addArg(Expr* arg) {
    args.push_back(arg);
}

void CallExpr::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[call " << Fn->getName() << "]\n";
    for (int i=0; i<args.size(); i++) {
        args[i]->print(indent + INDENT, buffer);
    }
}

void CallExpr::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << Fn->getName() << '(';
    for (int i=0; i<args.size(); i++) {
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
    if (pname.empty()) {
        buffer << name;
    } else {
        buffer << pname << '_' << name;
    }
}

const char* IdentifierExpr::getName() const {
    // TODO use several buffers
    static char buffer[128];
    if (!pname.empty()) {
        snprintf(buffer, 127, "%s::%s", pname.c_str(), name.c_str());
        return buffer;
    } else {
        return name.c_str();
    }
}


TypeExpr::~TypeExpr() {
    if (type && type->own()) delete type;
}

EXPR_VISITOR_ACCEPT(TypeExpr);

void TypeExpr::print(int indent, StringBuilder& buffer) {
    if (type) type->print(indent, buffer);
}

void TypeExpr::generateC(int indent, StringBuilder& buffer) {
    printf("%s() TODO\n", __PRETTY_FUNCTION__);
}

void TypeExpr::addArray(Expr* sizeExpr) {
    type = new Type(Type::ARRAY, type);
    type->setArrayExpr(sizeExpr);
}

void TypeExpr::addPointer() {
    type = new Type(Type::POINTER, type);
}

void TypeExpr::addQualifier(unsigned int qualifier) {
    type = new Type(Type::QUALIFIER, type);
    type->setQualifier(qualifier);
}

InitListExpr::InitListExpr(SourceLocation left, SourceLocation right, ExprList& values_)
    : leftBrace(left)
    , rightBrace(right)
    , values(values_)
{}

InitListExpr::~InitListExpr() {
    for (int i=0; i<values.size(); i++) {
        delete values[i];
    }
}

EXPR_VISITOR_ACCEPT(InitListExpr);

void InitListExpr::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[initlist]\n";
    for (int i=0; i<values.size(); i++) {
        values[i]->print(indent + INDENT, buffer);
    }
    
}

void InitListExpr::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "{ ";
    for (int i=0; i<values.size(); i++) {
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

DeclExpr::~DeclExpr() {
    if (type->own()) delete type;
}

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

