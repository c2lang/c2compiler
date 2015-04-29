/* Copyright 2013,2014,2015 Bas van den Berg
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

#include "AST/Decl.h"
#include "AST/Stmt.h"
#include "AST/Expr.h"
#include "AST/Attr.h"
#include "AST/Module.h"
#include "Utils/color.h"
#include "Utils/StringBuilder.h"
#include "Utils/UtilsConstants.h"

using namespace C2;
using namespace std;

//#define DECL_DEBUG

#ifdef DECL_DEBUG
static int creationCount;
static int deleteCount;
#endif

Decl::Decl(DeclKind k, const std::string& name_, SourceLocation loc_, QualType type_, bool is_public)
    : name(name_)
    , loc(loc_)
    , type(type_)
    , BitsInit(0)
    , mod(0)
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

string Decl::DiagName() const {
    StringBuilder tmp(128);
    tmp << "'" << name << "'";
    return (const char*)tmp;
}

void Decl::printAttributes(StringBuilder& buffer, unsigned indent) const {
    if (isExported()) {
        buffer.indent(indent);
        buffer.setColor(COL_ATTRIBUTES);
        buffer << "exported\n";
    }
    if (!hasAttributes()) return;
    buffer.indent(indent);
    buffer.setColor(COL_ATTRIBUTES);
    buffer << "Attributes: ";
    if (mod) {
        const AttrList& AL = mod->getAttributes(this);
        for (AttrListConstIter iter = AL.begin(); iter != AL.end(); ++iter) {
            if (iter != AL.begin()) buffer << ", ";
            (*iter)->print(buffer);
        }
    } else {
        buffer << "<NO MODULE>";
    }
    buffer << '\n';
}

void Decl::dump() const {
    StringBuilder buffer;
    print(buffer, 0);
    printf("%s\n", (const char*) buffer);
}

void Decl::printPublic(StringBuilder& buffer) const {
    if (isPublic()) {
        buffer.setColor(COL_ATTR);
        buffer << " public";
    }
}

bool Decl::hasAttribute(AttrKind kind) const {
    if (!hasAttributes()) return false;
    const AttrList& AL = getAttributes();
    for (AttrListConstIter iter = AL.begin(); iter != AL.end(); ++iter) {
        const Attr* A = (*iter);
        if (A->getKind() == kind) return true;
    }
    return false;
}

const AttrList& Decl::getAttributes() const {
    assert(hasAttributes());
    return mod->getAttributes(this);
}

FunctionDecl::FunctionDecl(const std::string& name_, SourceLocation loc_,
                           bool is_public, QualType rtype_)
    : Decl(DECL_FUNC, name_, loc_, QualType(), is_public)
    , rtype(rtype_)
    , body(0)
    , IRProto(0)
{}

FunctionDecl::~FunctionDecl() {
    delete body;
}

void FunctionDecl::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_DECL);
    buffer << "FunctionDecl ";
    type.print(buffer);
    printPublic(buffer);
    buffer.setColor(COL_VALUE);
    buffer << ' ' << name;
    buffer << '\n';
    for (unsigned i=0; i<args.size(); i++) {
        args[i]->print(buffer, indent + INDENT);
    }
    printAttributes(buffer, indent + INDENT);
    if (body) body->print(buffer, INDENT);
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


static const char* VarDeclKind2Str(VarDeclKind k) {
    switch (k) {
    case VARDECL_GLOBAL: return "global";
    case VARDECL_LOCAL:  return "local";
    case VARDECL_PARAM:  return "param";
    case VARDECL_MEMBER: return "member";
    }
    assert(0);
}


VarDecl::VarDecl(VarDeclKind k_, const std::string& name_, SourceLocation loc_,
            QualType type_, Expr* initValue_, bool is_public)
    : Decl(DECL_VAR, name_, loc_, type_, is_public)
    , initValue(initValue_)
    , IRValue(0)
{
    DeclBits.varDeclKind = k_;
}

VarDecl::~VarDecl() {
    delete initValue;
}

void VarDecl::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_DECL);
    buffer << "VarDecl ";
    type.print(buffer);
    buffer.setColor(COL_ATTR);
    buffer << ' ' << VarDeclKind2Str(getVarKind());
    printPublic(buffer);
    buffer.setColor(COL_VALUE);
    buffer << ' ' << name << '\n';
    printAttributes(buffer, indent + INDENT);

    if (hasLocalQualifier()) buffer << " LOCAL";
    indent += INDENT;
    if (initValue) initValue->print(buffer, indent);
#if 0
    // TODO move
    if (initValues.size()) {
        buffer.indent(INDENT);
        for (InitValuesConstIter iter=initValues.begin(); iter != initValues.end(); ++iter) {
            (*iter)->getExpr()->print(buffer, INDENT);
        }
    }
#endif
}

#if 0
void VarDecl::addInitValue(ArrayValueDecl* value) {
    initValues.push_back(value);
}
#endif


EnumConstantDecl::EnumConstantDecl(const std::string& name_, SourceLocation loc_,
                                   QualType type_, Expr* Init,
                                   bool is_public)
    : Decl(DECL_ENUMVALUE, name_, loc_, type_, is_public)
    , InitVal(Init)
    , Val(64, false)
{
}

EnumConstantDecl::~EnumConstantDecl() {
    delete InitVal;
}

void EnumConstantDecl::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_DECL);
    buffer << "EnumConstantDecl ";
    buffer.setColor(COL_VALUE);
    buffer << name;
    buffer.setColor(COL_ATTR);
    buffer << ' ' << Val.getSExtValue() << '\n';
    if (InitVal) InitVal->print(buffer, indent+INDENT);
}


TypeDecl::TypeDecl(DeclKind k, const std::string& name_, SourceLocation loc_, QualType type_,
                   bool is_public)
    : Decl(k, name_, loc_, type_, is_public)
{}


void AliasTypeDecl::print(StringBuilder& buffer, unsigned indent) const {
    buffer.setColor(COL_DECL);
    buffer << "AliasTypeDecl ";
    type.print(buffer);
    printPublic(buffer);
    buffer.setColor(COL_VALUE);
    buffer << ' ' << name;
    buffer.setColor(COL_ATTR); buffer << " refType: "; refType.print(buffer);
    buffer << '\n';
    printAttributes(buffer, indent + INDENT);
}


StructTypeDecl::StructTypeDecl(const std::string& name_, SourceLocation loc_,
                             QualType type_, bool is_struct, bool is_global,
                             bool is_public)
    : TypeDecl(DECL_STRUCTTYPE, name_, loc_, type_, is_public)
{
    DeclBits.StructTypeIsStruct = is_struct;
    DeclBits.StructTypeIsGlobal = is_global;
}

void StructTypeDecl::addMember(Decl* D) {
    assert(isa<VarDecl>(D) || isa<StructTypeDecl>(D));
    members.push_back(D);
}

void StructTypeDecl::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_DECL);
    buffer << "StructTypeDecl (";
    if (isStruct()) buffer << "struct";
    else buffer << "union";
    buffer << ") ";
    printPublic(buffer);
    buffer.setColor(COL_VALUE);
    buffer << name << '\n';
    printAttributes(buffer, indent + INDENT);
    for (unsigned i=0; i<members.size(); i++) {
        members[i]->print(buffer, indent + INDENT);
    }
}


void EnumTypeDecl::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_DECL);
    buffer << "EnumTypeDecl ";
    type.print(buffer);
    printPublic(buffer);
    buffer.setColor(COL_VALUE);
    buffer << ' ' << name;
    buffer << '\n';
    printAttributes(buffer, indent + INDENT);
    for (unsigned i=0; i<constants.size(); i++) {
        constants[i]->print(buffer, indent + INDENT);
    }
}

int EnumTypeDecl::getIndex(const EnumConstantDecl* c) const {
    for (unsigned i=0; i<constants.size(); i++) {
        if (constants[i] == c) return i;
    }
    return -1;
}

FunctionTypeDecl::FunctionTypeDecl(FunctionDecl* F)
    : TypeDecl(DECL_FUNCTIONTYPE, F->getName(), F->getLocation(), F->getType(), F->isPublic())
    , func(F)
{}

FunctionTypeDecl::~FunctionTypeDecl() {
    delete func;
}

void FunctionTypeDecl::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_DECL);
    buffer << "FunctionTypeDecl\n";
    printAttributes(buffer, indent + INDENT);
    func->print(buffer, indent + INDENT);
}


ArrayValueDecl::ArrayValueDecl(const std::string& name_, SourceLocation loc_,
                               Expr* value_)
    : Decl(DECL_ARRAYVALUE, name_, loc_, QualType(), false)
    , value(value_)
{
    DeclBits.arrayDeclOwnsExpr = true;
}

ArrayValueDecl::~ArrayValueDecl() {
    if (DeclBits.arrayDeclOwnsExpr) delete value;
}

void ArrayValueDecl::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_DECL);
    buffer << "ArrayValueDecl ";
    buffer.setColor(COL_VALUE);
    buffer << name << '\n';
    value->print(buffer, INDENT);
}


ImportDecl::ImportDecl(const std::string& name_, SourceLocation loc_, bool isLocal_,
                 const std::string& modName_, SourceLocation aliasLoc_)
    : Decl(DECL_IMPORT, name_, loc_, QualType(), false)
    , modName(modName_)
    , aliasLoc(aliasLoc_)
{
    DeclBits.ImportIsLocal = isLocal_;
}

void ImportDecl::print(StringBuilder& buffer, unsigned indent) const {
    buffer.setColor(COL_DECL);
    buffer << "ImportDecl ";
    buffer.setColor(COL_VALUE);
    buffer << modName;
    if (aliasLoc.isValid()) buffer << " as " << name;
    if (isLocal()) buffer << " local";
    buffer << '\n';
}


LabelDecl::LabelDecl(const std::string& name_, SourceLocation loc_)
    : Decl(DECL_LABEL, name_, loc_, QualType(), false)
    , TheStmt(0)
{}

void LabelDecl::print(StringBuilder& buffer, unsigned indent) const {
    buffer.setColor(COL_DECL);
    buffer << "LabelDecl ";
    buffer.setColor(COL_VALUE);
    buffer << name << '\n';
    if (TheStmt) TheStmt->print(buffer, indent+INDENT);
}

