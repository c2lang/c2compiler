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

#include <stdio.h> // for dump()
#include <assert.h>
#include <string.h>

#include "Type.h"
#include "StringBuilder.h"
#include "Expr.h"
#include "Decl.h"
#include "Utils.h"
#include "color.h"

//#define TYPE_DEBUG

using namespace C2;


static void printArray(StringBuilder& buffer, Expr* expr) {
    if (expr == 0) {
        buffer << "[]";
    } else {
        buffer << '[';
        expr->print(buffer, 0);
        buffer << ']';
    }
}

static void printQualifiers(StringBuilder& buffer, unsigned flags) {
    if (flags & QUAL_VOLATILE) buffer << "volatile ";
    if (flags & QUAL_CONST) buffer << "const ";
}

#if 0
void Type::printCQualifier(StringBuilder& buffer, unsigned flags) {
    if (flags & QUAL_LOCAL) buffer << "static ";
    if (flags & QUAL_VOLATILE) buffer << "volatile ";
    if (flags & QUAL_CONST) buffer << "const ";
}
#endif

#ifdef TYPE_DEBUG
static unsigned tcount = 0;

static const char* kind2name(Type::Kind k) {
    switch (k) {
    case Type::BUILTIN: return "builtin";
    case Type::USER: return "user";
    case Type::STRUCT: return "struct";
    case Type::UNION: return "union";
    case Type::ENUM: return "enum";
    case Type::FUNC: return "func";
    case Type::POINTER: return "pointer";
    case Type::ARRAY: return "array";
    }
    return "UNKNOWN";
}
#endif


inline QualType QualType::getCanonicalType() const {
    QualType canon = type->CanonicalType;
    canon.setQualifiers(qualifiers);
    return canon;
}

C2::Type* QualType::getTypePtr() const {
    assert(!isNull() && "Cannot retrieve a NULL type pointer");
    return type;
}

bool QualType::isPointerType() const { return getTypePtr()->isPointerType(); }
bool QualType::isUserType() const { return getTypePtr()->isUserType(); }
bool QualType::isSubscriptable() const { return getTypePtr()->isSubscriptable(); }
bool QualType::isStructOrUnionType() const { return getTypePtr()->isStructOrUnionType(); }
bool QualType::isArrayType() const { return getTypePtr()->isArrayType(); }
bool QualType::isFuncType() const { return getTypePtr()->isFuncType(); }
bool QualType::isEnumType() const { return getTypePtr()->isEnumType(); }

void QualType::DiagName(StringBuilder& buffer) const {
    getTypePtr()->DiagName(buffer);
}

void QualType::print(StringBuilder& buffer, unsigned indent, RecursionType recursive) const {
    if (isNull()) {
        buffer.indent(indent);
        buffer << "NULL";
    } else {
        getTypePtr()->print(buffer, indent, RECURSE_ALL);
        if (qualifiers) {
            buffer.indent(indent);
            buffer << "qualifiers=";
            printQualifiers(buffer, qualifiers);
            buffer << '\n';
        }
    }
}

void QualType::dump() const {
    StringBuilder buffer;
    print(buffer, 0, RECURSE_ALL);
    fprintf(stderr, "[QUALTYPE] %s\n", (const char*)buffer);
}


Type::Type(Type::Kind kind_, QualType refType_)
    : kind(kind_)
    , refType(refType_)
{
    memset(initializer, 0, sizeof(initializer));

#ifdef TYPE_DEBUG
    if (kind != BUILTIN) {
        tcount++;
        printf("tcount=%d  %s\n", tcount, kind2name(kind));
    }
#endif

    switch (kind) {
    case BUILTIN:
    case USER:
    case STRUCT:
    case UNION:
    case ENUM:
    case FUNC:
        assert(refType.isNull());
        break;
    case POINTER:
    case ARRAY:
        assert(refType.isValid());
        break;
    }
}

Type::~Type() {
#ifdef TYPE_DEBUG
    if (kind != BUILTIN) {
        tcount--;
        printf("tcount=%d  %s\n", tcount, kind2name(kind));
    }
#endif
    switch (kind) {
    case BUILTIN:
        break;
    case USER:
        break;
    case STRUCT:
    case UNION:
    case ENUM:
        delete constants;
        break;
    case FUNC:
        break;
    case POINTER:
        break;
    case ARRAY:
        if (ownArrayExpr) delete arrayExpr;
        break;
    }
}

unsigned Type::getWidth() const {
    switch (kind) {
    case BUILTIN:
        return width;
    case USER:
        return refType->getWidth();
    case STRUCT:
    case UNION:
        assert(0 && "TODO");
    case ENUM:
        // TEMP
        return 4;
        break;
    case FUNC:
        return 4;
    case POINTER:
        return 4;
    case ARRAY:
        return refType->getWidth();
    }
    return 0;
}

void Type::setRefType(QualType t) {
    refType = t;
}

#if 0
Type* Type::getCanonical(TypeContext& context) {
    // TODO
    return CanonicalType;
    switch (kind) {
    case BUILTIN:
        return this;
    case USER:
        assert(refType);
        return refType->getCanonical(context);
    case STRUCT:
    case UNION:
    case ENUM:
    case FUNC:
        return this;
    case POINTER:
        {
            assert(refType);
            Type* CT = refType->getCanonical(context);
            if (CT == refType) return this;
            return context.getPointer(CT);
        }
    case ARRAY:
        {
            assert(refType);
            Type* CT = refType->getCanonical(context);
            if (CT == refType) return this;
            return context.getArray(CT, arrayExpr, false);
        }
    }
    return this;
}
#endif

void Type::addEnumMember(EnumConstantDecl* C) {
    if (constants == 0) constants = new ConstantList();
    constants->push_back(C);
}

unsigned Type::getNumMembers() const {
    assert(structDecl);
    return structDecl->getNumMembers();
}

QualType Type::getMember(unsigned index) const {
    assert(structDecl);
    Decl* D = structDecl->getMember(index);
    // TODO use TypedDecl.getType()
    if (isa<VarDecl>(D)) {
        const VarDecl* V = cast<VarDecl>(D);
        return V->getType();
    }
    if (isa<StructTypeDecl>(D)) {
        return cast<StructTypeDecl>(D)->getType();
    }
    assert(0);
    return QualType();
}

bool Type::isCompatible(const Type& t2) const {
    switch (kind) {
    case BUILTIN:
        if (t2.kind != BUILTIN) return false;
        return name == t2.name;
    case USER:
        assert(0 && "TODO");
        break;
    case STRUCT:
    case UNION:
    case ENUM:
    case FUNC:
        if (t2.kind != kind) return false;
        assert(0 && "TODO");
    case POINTER:
        if (t2.kind != POINTER) return false;
        return refType->isCompatible(*(t2.refType));
    case ARRAY:
        // array is never compatible
        if (t2.kind != ARRAY) return false;
        // TODO compare Expr
        //if (arraySize != t2.arraySize) return false;
        return refType->isCompatible(*(t2.refType));
    }
    return true;
}

void Type::printFull(StringBuilder& buffer, unsigned indent) const {
    switch (kind) {
    case USER:
        assert(0 && "TODO");
        break;
    case BUILTIN:
        buffer << name;
        break;
    case STRUCT:
        buffer.indent(indent);
        buffer << "struct " << sname << " {\n";
        buffer.indent(indent);
        buffer << '}';
        break;
    case UNION:
        buffer.indent(indent);
        buffer << "union " << sname << " {\n";
        buffer.indent(indent);
        buffer << '}';
        break;
    case ENUM:
    {
        buffer.indent(indent);
        buffer << "enum " << " {\n";
        buffer.indent(indent);
        buffer << '}';
        break;
    }
    case FUNC:
    {
        assert(refType.isValid());
        buffer.indent(indent);
        buffer << "func type";
        break;
    }
    case POINTER:
        refType->printFull(buffer, indent);
        buffer << '*';
        break;
    case ARRAY:
        refType->printFull(buffer, indent);
        printArray(buffer, arrayExpr);
        break;
    }
}

void Type::printEffective(StringBuilder& buffer, unsigned indent) const {
    switch (kind) {
    case BUILTIN:
        assert(name);
        buffer.indent(indent);
        buffer << name;
        break;
    case USER:
        refType.getTypePtr()->printEffective(buffer, indent);
        break;
    case UNION:
        buffer.indent(indent);
        buffer << "(union)";
        break;
    case ENUM:
        buffer.indent(indent);
        buffer << "(enum)";
        break;
    case STRUCT:
        buffer.indent(indent);
        buffer << "(struct)" << sname;
        break;
    case FUNC:
    {
        buffer.indent(indent);
        buffer << "(func)";
        break;
    }
    case POINTER:
        refType->printEffective(buffer, indent);
        buffer << '*';
        break;
    case ARRAY:
        refType->printEffective(buffer, indent);
        printArray(buffer, arrayExpr);
        break;
    }
}

void Type::printName(StringBuilder& buffer) const {
    switch (kind) {
    case BUILTIN:
        assert(name);
        buffer << name;
        break;
    case STRUCT:
        buffer << "(struct)" << sname;
        break;
    case UNION:
        buffer << "(union)" << sname;
        break;
    case ENUM:
        buffer << "(enum)" << sname;
        break;
    case FUNC:
        buffer << "(function)" << func->getName();
        break;
    case USER:
        assert(refType.isValid());
        refType->printName(buffer);
        break;
    case POINTER:
        refType->printName(buffer);
        buffer << '*';
        break;
    case ARRAY:
        refType->printName(buffer);
        printArray(buffer, arrayExpr);
        break;
    }
}

void Type::print(StringBuilder& buffer, unsigned indent, QualType::RecursionType recursive) const {
    buffer.indent(indent);
    buffer << "[type] ";
    switch (kind) {
    case BUILTIN:
        buffer << "(builtin) " << name << '\n';
        break;
    case USER:
        buffer << "(user)\n";
        assert(userType);
        userType->print(buffer, indent + INDENT);
        if (refType.isValid() && recursive != QualType::RECURSE_NONE) {
            buffer.indent(indent + INDENT);
            buffer << ANSI_CYAN << "resolved to:" << ANSI_NORMAL << ' ';
            refType->printName(buffer);
            buffer << '\n';
        }
        break;
    case UNION:
        buffer << "(union)" << sname << "\n";
        break;
    case ENUM:
        buffer << "(enum) " << sname << "\n";
        if (refType.isValid()) {
            buffer.indent(indent + INDENT);
            buffer << "implType=";
            refType.print(buffer, 0, QualType::RECURSE_NONE);
        }
        if (constants && recursive != QualType::RECURSE_NONE) {
            for (unsigned i=0; i<constants->size(); i++) {
                EnumConstantDecl* C = (*constants)[i];
                C->print(buffer, indent + INDENT);
            }
        }
        break;
    case STRUCT:
        buffer << "(struct) "<< sname << "\n";
        break;
    case FUNC:
        buffer << "(func)\n";
        break;
    case POINTER:
        buffer << "(pointer)\n";
        refType.print(buffer, indent + INDENT, recursive);
        break;
    case ARRAY:
        buffer << "(array)\n";
        refType.print(buffer, indent + INDENT, recursive);
        if (arrayExpr) {
            buffer.indent(indent);
            buffer << COL_ATTR << "size:" << ANSI_NORMAL << '\n';
            arrayExpr->print(buffer, indent + INDENT);
        }
        break;
    }
}

void Type::DiagName(StringBuilder& buffer) const {
    buffer << '\'';
    printEffective(buffer);
    buffer << '\'';
}

void Type::dump() const {
    StringBuilder buffer;
    //printEffective(buffer, 0);
    print(buffer, 0, QualType::RECURSE_ALL);
    fprintf(stderr, "[TYPE] %s\n", (const char*)buffer);
}

Expr* Type::getBaseUserType() const {
    switch (kind) {
    case BUILTIN:
    case STRUCT:
    case UNION:
    case ENUM:
    case FUNC:
        assert(0);
        return 0;
    case USER:
        return userType;
    case POINTER:
    case ARRAY:
        return refType->getBaseUserType();
    }
}


static C2::Type type_u8(Type::BUILTIN);
static C2::Type type_u16(Type::BUILTIN);
static C2::Type type_u32(Type::BUILTIN);
static C2::Type type_u64(Type::BUILTIN);
static C2::Type type_i8(Type::BUILTIN);
static C2::Type type_i16(Type::BUILTIN);
static C2::Type type_i32(Type::BUILTIN);
static C2::Type type_i64(Type::BUILTIN);
static C2::Type type_int(Type::BUILTIN);
static C2::Type type_string(Type::BUILTIN);
static C2::Type type_f32(Type::BUILTIN);
static C2::Type type_f64(Type::BUILTIN);
static C2::Type type_bool(Type::BUILTIN);
static C2::Type type_void(Type::BUILTIN);

BuiltinType::BuiltinType() {
    type_u8.setBuiltinName(TYPE_U8, "u8", "unsigned char", 1);
    type_u16.setBuiltinName(TYPE_U16, "u16", "unsigned short", 2);
    type_u32.setBuiltinName(TYPE_U32,"u32", "unsigned", 4);
    type_u64.setBuiltinName(TYPE_U64,"u64", "unsigned long long", 4);
    type_i8.setBuiltinName(TYPE_I8,"i8", "char", 1);
    type_i16.setBuiltinName(TYPE_I16,"i16", "short", 2);
    type_i32.setBuiltinName(TYPE_I32, "i32", "int", 4);
    type_i64.setBuiltinName(TYPE_I64, "i64", "long long", 4);
    type_int.setBuiltinName(TYPE_INT,"int", "int", 4);
    type_string.setBuiltinName(TYPE_STRING, "string", "const char*", 4);
    type_f32.setBuiltinName(TYPE_F32,"f32", "float", 4);
    type_f64.setBuiltinName(TYPE_F64, "f64", "double", 8);
    type_bool.setBuiltinName(TYPE_BOOL, "bool", "int", 1);
    type_void.setBuiltinName(TYPE_VOID, "void", "void", 0);
}

C2::Type* BuiltinType::get(C2Type t) {
    static BuiltinType types;

    switch (t) {
    case TYPE_U8:     return &type_u8;
    case TYPE_U16:    return &type_u16;
    case TYPE_U32:    return &type_u32;
    case TYPE_U64:    return &type_u64;
    case TYPE_I8:     return &type_i8;
    case TYPE_I16:    return &type_i16;
    case TYPE_I32:    return &type_i32;
    case TYPE_I64:    return &type_i64;
    case TYPE_INT:    return &type_int;
    case TYPE_STRING: return &type_string;
    case TYPE_F32:    return &type_f32;
    case TYPE_F64:    return &type_f64;
    case TYPE_BOOL:   return &type_bool;
    case TYPE_VOID:   return &type_void;
    }
    return 0;
}



TypeContext::TypeContext() {}

TypeContext::~TypeContext() {
    for (unsigned i=0; i<types.size(); i++) delete types[i];
}

Type* TypeContext::getUser() {
    Type* T = new Type(Type::USER, 0);
    types.push_back(T);
    return T;
}

QualType TypeContext::getPointer(QualType ref) {
    // just search all pointer types with refType = ref
    for (unsigned i=0; i<types.size(); i++) {
        Type* t = types[i];
        if (t->isPointerType()) {
            // check Qualifiers
            QualType refType = t->getRefType();
            if (refType == ref) return t;
        }
    }
    // create new
    Type* N = new Type(Type::POINTER, ref);
    types.push_back(N);
    return QualType(N);
}

Type* TypeContext::getStruct(bool isStruct, const char* id) {
    Type* T = new Type(isStruct ? Type::STRUCT : Type::UNION);
    types.push_back(T);
    T->setStructName(id);
    return T;
}

Type* TypeContext::getEnum() {
    Type* T = new Type(Type::ENUM);
    types.push_back(T);
    return T;
}

QualType TypeContext::getArray(QualType ref, Expr* sizeExpr, bool ownSize) {
    Type* T = new Type(Type::ARRAY, ref);
    T->setArrayExpr(sizeExpr, ownSize);
    types.push_back(T);
    return QualType(T);
}

Type* TypeContext::getFunction(FunctionDecl* F) {
    Type* proto = new Type(Type::FUNC);
    proto->setFunc(F);
    types.push_back(proto);
    return proto;
}

