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

#include "Decl.h"
#include "Stmt.h"
#include "Expr.h"
#include "StringBuilder.h"
#include "Utils.h"
#include "color.h"

using namespace C2;
using namespace std;

//#define DECL_DEBUG

#ifdef DECL_DEBUG
static int creationCount;
static int deleteCount;
#endif

bool Decl::isSymbol(DeclType d) {
    switch (d) {
    case DECL_FUNC:
    case DECL_VAR:
    case DECL_TYPE:
    case DECL_ENUMVALUE:
        return true;
    default:
        return false;
    }
}

Decl::Decl(bool is_public_)
    : is_public(is_public_)
{
#ifdef DECL_DEBUG
    creationCount++;
    fprintf(stderr, "[DECL] create %p  created %d deleted %d\n", this, creationCount, deleteCount);
#endif
}

Decl::~Decl() {
#ifdef DECL_DEBUG
    deleteCount++;
    fprintf(stderr, "[DECL] delete %p  created %d deleted %d\n", this, creationCount, deleteCount);
#endif
}

void Decl::dump() {
    StringBuilder buffer;
    print(buffer);
    printf("%s\n", (const char*) buffer);
}


FunctionDecl::FunctionDecl(const std::string& name_,
                                 SourceLocation loc_,
                                 bool is_public_, QualType rtype_)
    : Decl(is_public_)
    , name(name_)
    , loc(loc_)
    , rtype(rtype_)
    , body(0)
    , m_isVariadic(false)
    , IRProto(0)
{
}

FunctionDecl::~FunctionDecl() {
    delete body;
}

DECL_VISITOR_ACCEPT(FunctionDecl);

void FunctionDecl::print(StringBuilder& buffer) {
    buffer << "[function " << name << "]\n";
    buffer.indent(INDENT);
    buffer << COL_ATTR << "returntype:" << ANSI_NORMAL << '\n';
    rtype.print(INDENT, buffer, QualType::RECURSE_NONE);
    if (args.size()) {
        buffer.indent(INDENT);
        buffer << COL_ATTR << "args:" << ANSI_NORMAL << '\n';
    }
    for (unsigned int i=0; i<args.size(); i++) {
        args[i]->print(INDENT, buffer);
    }
//    if (canonicalType) {
//        buffer.indent(INDENT);
//        buffer << ANSI_CYAN << "canonical:" << ANSI_NORMAL << '\n';
//        canonicalType->print(INDENT, buffer, QualType::RECURSE_NONE);
//    }
    if (body) {
        buffer.indent(INDENT);
        buffer << COL_ATTR << "body:" << ANSI_NORMAL << '\n';
        body->print(INDENT, buffer);
    }
}

DeclExpr* FunctionDecl::findArg(const std::string& name) const {
    for (unsigned i=0; i<args.size(); i++) {
        // TEMP
        DeclExpr* arg = ExprCaster<DeclExpr>::getType(args[i]);
        assert(arg);
        if (arg->getName() == name) return arg;
    }
    return 0;
}

void FunctionDecl::addArg(DeclExpr* arg) {
    args.push_back(arg);
}


VarDecl::VarDecl(DeclExpr* decl_, bool is_public_, bool inExpr)
    : Decl(is_public_)
    , decl(decl_)
{
}

VarDecl::~VarDecl() {
    delete decl;
}

DECL_VISITOR_ACCEPT(VarDecl);

void VarDecl::print(StringBuilder& buffer) {
    buffer << "[var]\n";
    decl->print(INDENT, buffer);
    if (initValues.size()) {
        buffer.indent(INDENT);
        buffer << ANSI_CYAN << "initvalues:" << ANSI_NORMAL << '\n';
        for (InitValuesConstIter iter=initValues.begin(); iter != initValues.end(); ++iter) {
            (*iter)->getExpr()->print(INDENT, buffer);
        }
    }
}

const std::string& VarDecl::getName() const { return decl->getName(); }

clang::SourceLocation VarDecl::getLocation() const {
    return decl->getLocation();
}

QualType VarDecl::getType() const { return decl->getType(); }

Expr* VarDecl::getInitValue() const { return decl->getInitValue(); }

void VarDecl::addInitValue(ArrayValueDecl* value) {
    initValues.push_back(value);
}


EnumConstantDecl::EnumConstantDecl(DeclExpr* decl_, bool is_public)
    : Decl(is_public)
    , decl(decl_)
{
}

EnumConstantDecl::~EnumConstantDecl() {
    delete decl;
}

DECL_VISITOR_ACCEPT(EnumConstantDecl);

void EnumConstantDecl::print(StringBuilder& buffer) {
    buffer << "[enum constant] value " << value << '\n';
    decl->print(INDENT, buffer);
}

const std::string& EnumConstantDecl::getName() const { return decl->getName(); }

clang::SourceLocation EnumConstantDecl::getLocation() const {
    return decl->getLocation();
}

QualType EnumConstantDecl::getType() const { return decl->getType(); }

Expr* EnumConstantDecl::getInitValue() const { return decl->getInitValue(); }


TypeDecl::TypeDecl(const std::string& name_, SourceLocation loc_, QualType type_, bool is_public_)
    : Decl(is_public_)
    , name(name_)
    , loc(loc_)
    , type(type_)
{}

TypeDecl::~TypeDecl() {
}

DECL_VISITOR_ACCEPT(TypeDecl);

void TypeDecl::print(StringBuilder& buffer) {
    buffer << "[typedef " << name << "]\n";
    type.print(INDENT, buffer, QualType::RECURSE_ONCE);
}


ArrayValueDecl::ArrayValueDecl(const std::string& name_, SourceLocation loc_, Expr* value_)
    : Decl(false)
    , name(name_)
    , loc(loc_)
    , value(value_)
{}

ArrayValueDecl::~ArrayValueDecl() {
    delete value;
}

DECL_VISITOR_ACCEPT(ArrayValueDecl);

void ArrayValueDecl::print(StringBuilder& buffer) {
    buffer << "[+= " << name << "]\n";
    value->print(INDENT, buffer);
}

UseDecl::UseDecl(const std::string& name_, SourceLocation loc_, bool isLocal_,
                 const char* alias_, SourceLocation aliasLoc_)
    : Decl(false)
    , name(name_)
    , alias(alias_)
    , loc(loc_)
    , aliasLoc(aliasLoc_)
    , is_local(isLocal_)
{}

DECL_VISITOR_ACCEPT(UseDecl);

void UseDecl::print(StringBuilder& buffer) {
    buffer << "[use " << name;
    if (alias != "") buffer << " as " << alias;
    if (is_local) buffer << " local";
    buffer << "]\n";
}

