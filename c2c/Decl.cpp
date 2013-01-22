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

Decl::Decl() {
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


FunctionDecl::FunctionDecl(const std::string& name_,
                                 SourceLocation loc_,
                                 bool is_public_, Type* rtype_)
    : name(name_)
    , loc(loc_)
    , is_public(is_public_)
    , rtype(rtype_)
    , body(0)
{
}

FunctionDecl::~FunctionDecl() {
    if (rtype->own()) delete rtype;
    for (int i=0; i<args.size(); i++) {
        delete args[i];
    }
    delete body;
}

DECL_VISITOR_ACCEPT(FunctionDecl);

void FunctionDecl::print(StringBuilder& buffer) {
    buffer << "[function " << name << "]\n";
    rtype->print(INDENT, buffer);
    for (int i=0; i<args.size(); i++) {
        args[i]->print(INDENT, buffer);
    }
    assert(body);
    body->print(INDENT, buffer);
}

void FunctionDecl::generateC(StringBuilder& buffer) {
    if (!is_public) buffer << "static ";
    rtype->generateC_PreName(buffer);
    rtype->generateC_PostName(buffer);
    buffer << ' ' << name << '(';
    int count = args.size();
    for (int i=0; i<args.size(); i++) {
        args[i]->generateC(0, buffer);
        if (count != 1) buffer << ", ";
        count--;
    }
    buffer << ")\n";
    assert(body);
    body->generateC(0, buffer);
    buffer << '\n';
}



#define VARDECL_ISPUBLIC 0x1
#define VARDECL_INEXPR   0x2

VarDecl::VarDecl(DeclExpr* decl_, bool is_public, bool inExpr)
    : decl(decl_)
    , flags(0)
{
    if (is_public) flags |= VARDECL_ISPUBLIC;
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

void VarDecl::generateC(StringBuilder& buffer) {
    if (isPublic()) buffer << "static ";
    decl->generateC(0, buffer);
    // TODO semicolon not needed when ending initlist with '}'
    // Q: add bool return value to Expr.generateC()?
    buffer << ";\n";
}

bool VarDecl::isPublic() const { return ((flags & VARDECL_ISPUBLIC) != 0); }

bool VarDecl::isInExpr() const { return ((flags & VARDECL_INEXPR) != 0); }

const std::string& VarDecl::getName() const { return decl->getName(); }

clang::SourceLocation VarDecl::getLocation() const {
    return decl->getLocation();
}

TypeDecl::TypeDecl(const std::string& name_, SourceLocation loc_, Type* type_, bool is_public_)
    : name(name_)
    , loc(loc_)
    , type(type_)
    , is_public(is_public_)
{}

TypeDecl::~TypeDecl() {
    if (type->own()) delete type;
}

DECL_VISITOR_ACCEPT(TypeDecl);

void TypeDecl::print(StringBuilder& buffer) {
    buffer << "[type " << name << "]\n";
    type->print(INDENT, buffer);
}

void TypeDecl::generateC(StringBuilder& buffer) {
    buffer << "typedef ";
    type->generateC_PreName(buffer);
    buffer << ' ' << name;
    type->generateC_PostName(buffer);
    buffer << ";\n";
}


ArrayValueDecl::ArrayValueDecl(const std::string& name_, SourceLocation loc_, Expr* value_)
    : name(name_)
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

void ArrayValueDecl::generateC(StringBuilder& buffer) {
    fprintf(stderr, "TODO SHOULD NOT BE CALLED\n");
}


UseDecl::UseDecl(const std::string& name_, SourceLocation loc_)
    : name(name_)
    , loc(loc_)
{}

DECL_VISITOR_ACCEPT(UseDecl);

void UseDecl::print(StringBuilder& buffer) {
    buffer << "[use " << name << "]\n";
}

void UseDecl::generateC(StringBuilder& buffer) {
    // Temp hardcoded for stdio
    if (name == "stdio") {
        buffer << "#include <stdio.h>\n";
    }
}


