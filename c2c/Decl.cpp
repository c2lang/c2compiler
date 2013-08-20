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

Decl::Decl(DeclKind k, const std::string& name_, SourceLocation loc_, bool is_public)
    : name(name_)
    , loc(loc_)
    , BitsInit(0)
{
    DeclBits.dKind = k;
    DeclBits.DeclIsPublic = is_public;
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
    print(buffer, 0);
    printf("%s\n", (const char*) buffer);
}


FunctionDecl::FunctionDecl(const std::string& name_,
                                 SourceLocation loc_,
                                 bool is_public, QualType rtype_)
    : Decl(DECL_FUNC, name_, loc_, is_public)
    , rtype(rtype_)
    , body(0)
    , IRProto(0)
{}

FunctionDecl::~FunctionDecl() {
    delete body;
}

void FunctionDecl::print(StringBuilder& buffer, unsigned indent) {
    buffer.indent(indent);
    buffer << "[FunctionDecl " << name << "]\n";
    buffer.indent(indent + INDENT);
    buffer << COL_ATTR << "returntype:" << ANSI_NORMAL << '\n';
    rtype.print(buffer, indent+INDENT, QualType::RECURSE_NONE);
    if (args.size()) {
        buffer.indent(indent + INDENT);
        buffer << COL_ATTR << "args:" << ANSI_NORMAL;
        if (isVariadic()) buffer << " <...>";
        buffer << '\n';
        for (unsigned int i=0; i<args.size(); i++) {
            args[i]->print(buffer, indent + INDENT);
        }
    }
    if (body) {
        buffer.indent(INDENT);
        buffer << COL_ATTR << "body:" << ANSI_NORMAL << '\n';
        body->print(buffer, INDENT);
    }
}

VarDecl* FunctionDecl::findArg(const std::string& name) const {
    for (unsigned i=0; i<args.size(); i++) {
        VarDecl* arg = args[i];
        if (arg->getName() == name) return arg;
    }
    return 0;
}

unsigned FunctionDecl::minArgs() const {
    if (!hasDefaultArgs()) return args.size();

    unsigned i;
    for (i=0; i<args.size(); i++) {
        VarDecl* arg = args[i];
        if (arg->getInitValue()) break;
    }
    return i;
}


VarDecl::VarDecl(const std::string& name_, SourceLocation loc_,
            QualType type_, Expr* initValue_, bool is_public)
    : Decl(DECL_VAR, name_, loc_, is_public)
    , type(type_)
    , initValue(initValue_)
{}

VarDecl::~VarDecl() {
    delete initValue;
}

void VarDecl::print(StringBuilder& buffer, unsigned indent) {
    buffer.indent(indent);
    buffer << "[VarDecl " << name;
    if (hasLocalQualifier()) buffer << " LOCAL";
    buffer << "]\n";
    indent += INDENT;
    // Dont print types for enums, otherwise we get a loop since Type have Decls etc
    if (!type->isEnumType()) {
        type.print(buffer, indent, QualType::RECURSE_ONCE);
    }
    if (initValue) {
        buffer.indent(indent);
        buffer << "initial:\n";
        initValue->print(buffer, indent+INDENT);
    }
    // TODO move
    if (initValues.size()) {
        buffer.indent(INDENT);
        buffer << ANSI_CYAN << "initvalues:" << ANSI_NORMAL << '\n';
        for (InitValuesConstIter iter=initValues.begin(); iter != initValues.end(); ++iter) {
            (*iter)->getExpr()->print(buffer, INDENT);
        }
    }
}

void VarDecl::addInitValue(ArrayValueDecl* value) {
    initValues.push_back(value);
}


EnumConstantDecl::EnumConstantDecl(const std::string& name_, SourceLocation loc_, QualType type_, Expr* Init, bool is_public)
    : Decl(DECL_ENUMVALUE, name_, loc_, is_public)
    , type(type_)
    , InitVal(Init)
    , value(0)
{
}

EnumConstantDecl::~EnumConstantDecl() {
    delete InitVal;
}

void EnumConstantDecl::print(StringBuilder& buffer, unsigned indent) {
    buffer.indent(indent);
    buffer << "[EnumConstantDecl] '" << name << "' value=" << value << '\n';
    if (InitVal) InitVal->print(buffer, indent+INDENT);
}

TypeDecl::TypeDecl(const std::string& name_, SourceLocation loc_, QualType type_, bool is_public)
    : Decl(DECL_TYPE, name_, loc_, is_public)
    , type(type_)
{}

TypeDecl::TypeDecl(DeclKind k, const std::string& name_, SourceLocation loc_, QualType type_, bool is_public)
    : Decl(k, name_, loc_, is_public)
    , type(type_)
{}

TypeDecl::~TypeDecl() {
}

void TypeDecl::print(StringBuilder& buffer, unsigned indent) {
    buffer << "[TypeDecl " << name << "]\n";
    type.print(buffer, INDENT, QualType::RECURSE_ONCE);
}


StructTypeDecl::StructTypeDecl(const std::string& name_, SourceLocation loc_,
                             QualType type_, bool is_struct, bool is_global, bool is_public)
    : TypeDecl(DECL_STRUCTTYPE, name_, loc_, type_, is_public)
{
    DeclBits.StructTypeIsStruct = is_struct;
    DeclBits.StructTypeIsGlobal = is_global;
}

void StructTypeDecl::addMember(Decl* D) {
    assert(isa<VarDecl>(D) || isa<StructTypeDecl>(D));
    members.push_back(D);
}

void StructTypeDecl::print(StringBuilder& buffer, unsigned indent) {
    buffer.indent(indent);
    buffer << "[StructTypeDecl (";
    if (isStruct()) buffer << "struct";
    else buffer << "union";
    buffer  << ") " << name << "]\n";
    for (unsigned int i=0; i<members.size(); i++) {
        members[i]->print(buffer, indent + INDENT);
    }
    type.print(buffer, indent+INDENT, QualType::RECURSE_ONCE);
}


FunctionTypeDecl::FunctionTypeDecl(FunctionDecl* F)
    : TypeDecl(DECL_FUNCTIONTYPE, F->getName(), F->getLocation(), F->getType(), F->isPublic())
    , func(F)
{
}

FunctionTypeDecl::~FunctionTypeDecl() {
    delete func;
}

void FunctionTypeDecl::print(StringBuilder& buffer, unsigned indent) {
    buffer.indent(indent);
    buffer << "[FunctionTypeDecl]\n";
    func->print(buffer, indent + INDENT);
}


ArrayValueDecl::ArrayValueDecl(const std::string& name_, SourceLocation loc_, Expr* value_)
    : Decl(DECL_ARRAYVALUE, name_, loc_, false)
    , value(value_)
{}

ArrayValueDecl::~ArrayValueDecl() {
    delete value;
}

void ArrayValueDecl::print(StringBuilder& buffer, unsigned indent) {
    buffer.indent(indent);
    buffer << "[ArrayValueDecl " << name << "]\n";
    value->print(buffer, INDENT);
}

UseDecl::UseDecl(const std::string& name_, SourceLocation loc_, bool isLocal_,
                 const char* alias_, SourceLocation aliasLoc_)
    : Decl(DECL_USE, name_, loc_, false)
    , alias(alias_)
    , aliasLoc(aliasLoc_)
{
    DeclBits.UseIsLocal = isLocal_;
}

void UseDecl::print(StringBuilder& buffer, unsigned indent) {
    buffer << "[UseDecl " << name;
    if (alias != "") buffer << " as " << alias;
    if (isLocal()) buffer << " local";
    buffer << "]\n";
}

