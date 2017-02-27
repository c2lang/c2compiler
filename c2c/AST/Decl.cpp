/* Copyright 2013-2017 Bas van den Berg
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
#include "AST/ASTContext.h"
#include "AST/Module.h"
#include "Utils/color.h"
#include "Utils/StringBuilder.h"
#include "Utils/UtilsConstants.h"

using namespace C2;
using namespace std;


Decl::Decl(DeclKind k, const char* name_, SourceLocation loc_, QualType type_, bool is_public)
    : loc(loc_)
    , type(type_)
    , name(name_)
    , mod(0)
{
    declBits.dKind = k;
    declBits.IsExported = 0;
    declBits.IsPublic = is_public;
    declBits.IsUsed = 0;
    declBits.IsUsedPublic = 0;
    declBits.HasAttributes = 0;
}

void Decl::fullName(StringBuilder& output) const {
    assert(mod);
    output << mod->getName() << '_' << name;
}

string Decl::DiagName() const {
    StringBuilder tmp(128);
    tmp << '\'';
    if (mod) {
        tmp << mod->getName() << '.';
    }
    tmp << name;
    tmp << '\'';
    return tmp.c_str();
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

void Decl::print(StringBuilder& buffer, unsigned indent) const {
    switch (getKind()) {
    case DECL_FUNC:
        cast<FunctionDecl>(this)->print(buffer, indent);
        break;
    case DECL_VAR:
        cast<VarDecl>(this)->print(buffer, indent);
        break;
    case DECL_ENUMVALUE:
        cast<EnumConstantDecl>(this)->print(buffer, indent);
        break;
    case DECL_ALIASTYPE:
        cast<AliasTypeDecl>(this)->print(buffer, indent);
        break;
    case DECL_STRUCTTYPE:
        cast<StructTypeDecl>(this)->print(buffer, indent);
        break;
    case DECL_ENUMTYPE:
        cast<EnumTypeDecl>(this)->print(buffer, indent);
        break;
    case DECL_FUNCTIONTYPE:
        cast<FunctionTypeDecl>(this)->print(buffer, indent);
        break;
    case DECL_ARRAYVALUE:
        cast<ArrayValueDecl>(this)->print(buffer, indent);
        break;
    case DECL_IMPORT:
        cast<ImportDecl>(this)->print(buffer, indent);
        break;
    case DECL_LABEL:
        cast<LabelDecl>(this)->print(buffer, indent);
        break;
    }
}

void* Decl::operator new(size_t bytes, const C2::ASTContext& C, unsigned alignment) {
    return ::operator new(bytes, C, alignment);
}

void Decl::dump() const {
    StringBuilder buffer;
    print(buffer, 0);
    printf("%s\n", buffer.c_str());
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

FunctionDecl::FunctionDecl(const char* name_, SourceLocation loc_,
                           bool is_public, QualType rtype_)
    : Decl(DECL_FUNC, name_, loc_, QualType(), is_public)
    , rtype(rtype_)
    , origRType(rtype_)
    , args(0)
    , body(0)
    , IRProto(0)
{
    functionDeclBits.structFuncNameOffset= 0;
    functionDeclBits.numArgs = 0;
    functionDeclBits.IsVariadic = 0;
    functionDeclBits.HasDefaultArgs = 0;
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
    for (unsigned i=0; i<numArgs(); i++) {
        args[i]->print(buffer, indent + INDENT);
    }
    printAttributes(buffer, indent + INDENT);
    if (body) body->print(buffer, INDENT);
}

unsigned FunctionDecl::minArgs() const {
    if (!hasDefaultArgs()) return numArgs();

    unsigned i;
    for (i=0; i<numArgs(); i++) {
        VarDecl* arg = args[i];
        if (arg->getInitValue()) break;
    }
    return i;
}

bool FunctionDecl::sameProto(const FunctionDecl* lhs, const FunctionDecl* rhs) {
    if (lhs->rtype != rhs->rtype) return false;
    if (lhs->numArgs() != rhs->numArgs()) return false;
    if (lhs->isVariadic() != rhs->isVariadic()) return false;

    for (unsigned i=0; i<lhs->numArgs(); i++) {
        const VarDecl* leftArg = lhs->getArg(i);
        const VarDecl* rightArg = rhs->getArg(i);
        if (leftArg->getType() != rightArg->getType()) return false;
    }
    return true;
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


VarDecl::VarDecl(VarDeclKind k_, const char* name_, SourceLocation loc_,
                 QualType type_, Expr* initValue_, bool is_public)
    : Decl(DECL_VAR, name_, loc_, type_, is_public)
    , origType(type_)
    , initValue(initValue_)
    , IRValue(0)
{
    varDeclBits.Kind = k_;
    varDeclBits.HasLocalQualifier = 0;
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
}


EnumConstantDecl::EnumConstantDecl(const char* name_, SourceLocation loc_,
                                   QualType type_, Expr* Init,
                                   bool is_public)
    : Decl(DECL_ENUMVALUE, name_, loc_, type_, is_public)
    , InitVal(Init)
    , Val(64, false)
{}

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


TypeDecl::TypeDecl(DeclKind k, const char* name_, SourceLocation loc_, QualType type_,
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


StructTypeDecl::StructTypeDecl(const char* name_, SourceLocation loc_,
                               QualType type_, bool is_struct, bool is_global,
                               bool is_public)
    : TypeDecl(DECL_STRUCTTYPE, name_, loc_, type_, is_public)
    , members(0)
    , structFunctions(0)
{
    structTypeDeclBits.numMembers = 0;
    structTypeDeclBits.numStructFunctions = 0;
    structTypeDeclBits.IsStruct = is_struct;
    structTypeDeclBits.IsGlobal = is_global;
}

Decl* StructTypeDecl::find(const char* name_) const {
    // normal members
    for (unsigned i=0; i<numMembers(); i++) {
        Decl* D = members[i];
        if (strcmp(D->getName(), name_) == 0) return D;
        if (D->hasEmptyName()) {      // empty string
            assert(isa<StructTypeDecl>(D));
            StructTypeDecl* sub = cast<StructTypeDecl>(D);
            D = sub->find(name_);
            if (D) return D;
        }
    }

    return findFunction(name_);
}

Decl* StructTypeDecl::findFunction(const char* name_) const {
    // struct-functions
    for (unsigned i=0; i<numStructFunctions(); i++) {
        FunctionDecl* F = structFunctions[i];
        if (F->matchesStructFuncName(name_)) return F;
    }
    return 0;
}

int StructTypeDecl::findIndex(const char*  name_) const {
    for (unsigned i=0; i<numMembers(); i++) {
        Decl* D = members[i];
        if (strcmp(D->getName(), name_) == 0) return i;
    }
    return -1;
}

void StructTypeDecl::setOpaqueMembers() {
    for (unsigned i=0; i<numMembers(); i++) {
        Decl* D = members[i];
        D->setPublic(false);
        StructTypeDecl* subStruct = dyncast<StructTypeDecl>(D);
        if (subStruct) subStruct->setOpaqueMembers();
    }
}

void StructTypeDecl::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_DECL);
    buffer << "StructTypeDecl (";
    if (isStruct()) buffer << "struct";
    else buffer << "union";
    buffer << ")";
    printPublic(buffer);
    buffer.setColor(COL_VALUE);
    buffer << ' ';
    if (name[0] != 0) buffer << name;
    else buffer << "<anonymous>";
    buffer << '\n';
    printAttributes(buffer, indent + INDENT);
    for (unsigned i=0; i<numMembers(); i++) {
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
    if (isIncremental()) buffer << " incremental";
    buffer << '\n';
    printAttributes(buffer, indent + INDENT);
    for (unsigned i=0; i<numConstants(); i++) {
        constants[i]->print(buffer, indent + INDENT);
    }
}

int EnumTypeDecl::getIndex(const EnumConstantDecl* c) const {
    for (unsigned i=0; i<numConstants(); i++) {
        if (constants[i] == c) return i;
    }
    return -1;
}

bool EnumTypeDecl::hasConstantValue(llvm::APSInt Val) const {
    for (unsigned i=0; i<numConstants(); i++) {
        if (constants[i]->getValue() == Val) return true;
    }
    return false;
}

llvm::APSInt EnumTypeDecl::getMinValue() const {
    assert(numConstants() != 0);
    llvm::APSInt min = constants[0]->getValue();
    for (unsigned i=1; i<numConstants(); i++) {
        llvm::APSInt cur = constants[i]->getValue();
        if (cur < min) min = cur;
    }
    return min;
}

llvm::APSInt EnumTypeDecl::getMaxValue() const {
    assert(numConstants() != 0);
    llvm::APSInt max = constants[0]->getValue();
    for (unsigned i=1; i<numConstants(); i++) {
        llvm::APSInt cur = constants[i]->getValue();
        if (cur > max) max = cur;
    }
    return max;
}


FunctionTypeDecl::FunctionTypeDecl(FunctionDecl* F)
    : TypeDecl(DECL_FUNCTIONTYPE, F->getName(), F->getLocation(), F->getType(), F->isPublic())
    , func(F)
{
}

void FunctionTypeDecl::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_DECL);
    buffer << "FunctionTypeDecl\n";
    printAttributes(buffer, indent + INDENT);
    func->print(buffer, indent + INDENT);
}


ArrayValueDecl::ArrayValueDecl(const char* name_, SourceLocation loc_, Expr* value_)
    : Decl(DECL_ARRAYVALUE, name_, loc_, QualType(), false)
    , value(value_)
{}

void ArrayValueDecl::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_DECL);
    buffer << "ArrayValueDecl ";
    buffer.setColor(COL_VALUE);
    buffer << name << '\n';
    value->print(buffer, INDENT);
}


ImportDecl::ImportDecl(const char* name_, SourceLocation loc_, bool isLocal_,
                       const char* modName_, SourceLocation aliasLoc_)
    : Decl(DECL_IMPORT, name_, loc_, QualType(), false)
    , modName(modName_)
    , aliasLoc(aliasLoc_)
{
    importDeclBits.IsLocal = isLocal_;
}

void ImportDecl::print(StringBuilder& buffer, unsigned indent) const {
    buffer.setColor(COL_DECL);
    buffer << "ImportDecl ";
    buffer.setColor(COL_VALUE);
    buffer << modName;
    if (aliasLoc.isValid()) buffer << " as " << name;
    if (isLocal()) buffer << " local";
    buffer.setColor(COL_ATTRIBUTES);
    buffer << "  used=" << isUsed() << '/' << isUsedPublic();
    if (getModule()) {
        buffer << " module=" << getModule()->getName();
    } else {
        buffer << " NO MODULE";
    }
    buffer << '\n';
}


LabelDecl::LabelDecl(const char* name_, SourceLocation loc_)
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

