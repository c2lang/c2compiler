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
#include "Type.h"
#include "Utils.h"

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
                                 bool is_public_, Type* rtype_)
    : Decl(is_public_)
    , name(name_)
    , loc(loc_)
    , rtype(rtype_)
    , body(0)
    , m_isVariadic(false)
{
}

FunctionDecl::~FunctionDecl() {
    delete body;
}

DECL_VISITOR_ACCEPT(FunctionDecl);

void FunctionDecl::print(StringBuilder& buffer) {
    buffer << "[function " << name << "]\n";
    rtype->print(INDENT, buffer);
    for (unsigned int i=0; i<args.size(); i++) {
        args[i]->print(INDENT, buffer);
    }
    assert(body);
    body->print(INDENT, buffer);
}

void FunctionDecl::generateC(StringBuilder& buffer, const std::string& pkgName) {
    if (!is_public) buffer << "static ";
    rtype->generateC_PreName(buffer);
    rtype->generateC_PostName(buffer);
    buffer << ' ';
    Utils::addName(pkgName, name, buffer);
    buffer << '(';
    int count = args.size();
    for (unsigned int i=0; i<args.size(); i++) {
        args[i]->generateC(0, buffer);
        if (count != 1) buffer << ", ";
        count--;
    }
    if (m_isVariadic) buffer << "...";
    buffer << ")\n";
    assert(body);
    body->generateC(0, buffer);
    buffer << '\n';
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

Type* FunctionDecl::getProto() const {
    Type* proto = new Type(Type::FUNC);
    // TODO use TypeContext
    //proto->setReturnType(rtype);
    //proto->addArgument(Type*)
    return proto;
}



#define VARDECL_INEXPR   0x1

VarDecl::VarDecl(DeclExpr* decl_, bool is_public_, bool inExpr)
    : Decl(is_public_)
    , decl(decl_)
    , flags(0)
{
    if (inExpr) flags |= VARDECL_INEXPR;
}

VarDecl::~VarDecl() {
    delete decl;
}

DECL_VISITOR_ACCEPT(VarDecl);

void VarDecl::print(StringBuilder& buffer) {
    buffer << "[var]\n";
    decl->print(INDENT, buffer);
}

void VarDecl::generateC(StringBuilder& buffer, const std::string& pkgName) {
    if (isPublic()) buffer << "static ";
    decl->generateC(buffer, pkgName);
    // TODO semicolon not needed when ending initlist with '}'
    // Q: add bool return value to Expr.generateC()?
    buffer << ";\n";
}

bool VarDecl::isInExpr() const { return ((flags & VARDECL_INEXPR) != 0); }

const std::string& VarDecl::getName() const { return decl->getName(); }

clang::SourceLocation VarDecl::getLocation() const {
    return decl->getLocation();
}

Type* VarDecl::getType() const { return decl->getType(); }


TypeDecl::TypeDecl(const std::string& name_, SourceLocation loc_, Type* type_, bool is_public_)
    : Decl(is_public_)
    , name(name_)
    , loc(loc_)
    , type(type_)
{}

TypeDecl::~TypeDecl() {
}

DECL_VISITOR_ACCEPT(TypeDecl);

void TypeDecl::print(StringBuilder& buffer) {
    buffer << "[type " << name << "]\n";
    type->print(INDENT, buffer);
}

void TypeDecl::generateC(StringBuilder& buffer, const std::string& pkgName) {
    buffer << "typedef ";
    type->generateC_PreName(buffer);
    buffer << ' ';
    Utils::addName(pkgName, name, buffer);
    type->generateC_PostName(buffer);
    buffer << ";\n";
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

void ArrayValueDecl::generateC(StringBuilder& buffer, const std::string& pkgName) {
    value->generateC(INDENT, buffer);
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

void UseDecl::generateC(StringBuilder& buffer, const std::string& pkgName) {
    // Temp hardcoded for stdio
    if (name == "stdio") {
        buffer << "#include <stdio.h>\n";
    }
}

