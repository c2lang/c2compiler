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
#include <assert.h>

#include "AST/Type.h"
#include "AST/Expr.h"
#include "AST/Decl.h"
#include "Utils/StringBuilder.h"
#include "Utils/Utils.h"
#include "Utils/color.h"
#include "Utils/constants.h"

using namespace C2;

// TODO insert QualType function impls
QualType QualType::getCanonicalType() const {
    QualType canon = type->canonicalType;
    canon.setQualifiers(qualifiers);
    return canon;
}

C2::Type* QualType::getTypePtr() const {
    assert(!isNull() && "Cannot retrieve a NULL type pointer");
    return type;
}

bool QualType::isBuiltinType() const { return getTypePtr()->isBuiltinType(); }
bool QualType::isPointerType() const { return getTypePtr()->isPointerType(); }
bool QualType::isArrayType() const { return getTypePtr()->isArrayType(); }
bool QualType::isAliasType() const { return getTypePtr()->isAliasType(); }
bool QualType::isStructType() const { return getTypePtr()->isStructType(); }
bool QualType::isFunctionType() const { return getTypePtr()->isFunctionType(); }
bool QualType::isSubscriptable() const { return getTypePtr()->isSubscriptable(); }

void QualType::DiagName(StringBuilder& buffer) const {
    if (isNull()) {
        buffer << "NULL";
    } else {
        getTypePtr()->DiagName(buffer);
    }
}

void QualType::printName(StringBuilder& buffer) const {
    if (isNull()) {
        buffer << "NULL";
    } else {
        getTypePtr()->printName(buffer);
    }
}

void QualType::print(StringBuilder& buffer) const {
    buffer << '\'';
    debugPrint(buffer);
    buffer << '\'';
}

void QualType::debugPrint(StringBuilder& buffer) const {
    if (isNull()) {
        buffer << ANSI_RED << "??" << ANSI_NORMAL;;
    } else {
        printQualifiers(buffer);
        getTypePtr()->debugPrint(buffer);
    }
}

void QualType::dump() const {
    if (isNull()) {
        fprintf(stderr, "NULL\n");
    } else {
        StringBuilder output;
        debugPrint(output);
        fprintf(stderr, "[TYPE] '%s'\n", (const char*)output);
    }
}

void QualType::printQualifiers(StringBuilder& buffer) const {
    if (hasQualifiers()) {
        if (isConstQualified()) buffer << "const ";
        if (isVolatileQualified()) buffer << "volatile ";
        if (isRestrictQualified()) buffer << "restrict ";
    }
}



void Type::setCanonicalType(QualType qt) const {
    assert(canonicalType.isNull());
    canonicalType = qt;
}

void Type::DiagName(StringBuilder& buf) const {
    buf << '\'';
    printName(buf);
    buf << '\'';
    if (canonicalType.isNull()) {
        buf << " canonical?";
    } else {
        const Type* canon = canonicalType.getTypePtr();
        if (this != canon) {
            buf << " (aka '";
            canon->printName(buf);
            buf << "')";
        }
    }

}

void Type::debugPrint(StringBuilder& buffer) const {
    // only used to print canonical type (called by Sub-Class::debugPrint())
    buffer << "  canonical=";
    if (canonicalType.isNull()) {
        buffer << ANSI_RED << "???";
    } else {
        const Type* Canon = canonicalType.getTypePtr();
        buffer << ANSI_CYAN;
        if (Canon == this) {
            buffer << "this";
        } else {
            Canon->printName(buffer);
        }
    }
    buffer << ANSI_NORMAL << '\n';
}

void Type::dump() const {
    StringBuilder output;
    debugPrint(output);
    fprintf(stderr, "[TYPE] '%s'\n", (const char*)output);
}


static BuiltinType _Int8(BuiltinType::Int8);
static BuiltinType _Int16(BuiltinType::Int16);
static BuiltinType _Int32(BuiltinType::Int32);
static BuiltinType _Int64(BuiltinType::Int64);
static BuiltinType _UInt8(BuiltinType::UInt8);
static BuiltinType _UInt16(BuiltinType::UInt16);
static BuiltinType _UInt32(BuiltinType::UInt32);
static BuiltinType _UInt64(BuiltinType::UInt64);
static BuiltinType _Float32(BuiltinType::Float32);
static BuiltinType _Float64(BuiltinType::Float64);
static BuiltinType _Bool(BuiltinType::Bool);
static BuiltinType _Void(BuiltinType::Void);

QualType Type::Int8() { return QualType(&_Int8); }
QualType Type::Int16() { return QualType(&_Int16); }
QualType Type::Int32() { return QualType(&_Int32); }
QualType Type::Int64() { return QualType(&_Int64); }
QualType Type::UInt8() { return QualType(&_UInt8); }
QualType Type::UInt16() { return QualType(&_UInt16); }
QualType Type::UInt32() { return QualType(&_UInt32); }
QualType Type::UInt64() { return QualType(&_UInt64); }
QualType Type::Float32() { return QualType(&_Float32); }
QualType Type::Float64() { return QualType(&_Float64); }
QualType Type::Bool() { return QualType(&_Bool); }
QualType Type::Void() { return QualType(&_Void); }

BuiltinType* BuiltinType::get(Kind k) {
    switch (k) {
        case Int8:      return &_Int8;
        case Int16:     return &_Int16;
        case Int32:     return &_Int32;
        case Int64:     return &_Int64;
        case UInt8:     return &_UInt8;
        case UInt16:    return &_UInt16;
        case UInt32:    return &_UInt32;
        case UInt64:    return &_UInt64;
        case Float32:   return &_Float32;
        case Float64:   return &_Float64;
        case Bool:      return &_Bool;
        case Void:      return &_Void;
    }
    return 0;       // to satisfy compiler
}

unsigned BuiltinType::getWidth() const {
    switch (kind) {
        case Int8:      return 8;
        case Int16:     return 16;
        case Int32:     return 32;
        case Int64:     return 64;
        case UInt8:     return 8;
        case UInt16:    return 16;
        case UInt32:    return 32;
        case UInt64:    return 64;
        case Float32:   return 32;
        case Float64:   return 64;
        case Bool:      return 1;
        case Void:      return 0;
    }
    return 0;       // to satisfy compiler
}

unsigned BuiltinType::getIntegerWidth() const {
    switch (kind) {
        case Int8:      return 7;
        case Int16:     return 15;
        case Int32:     return 31;
        case Int64:     return 63;
        case UInt8:     return 8;
        case UInt16:    return 16;
        case UInt32:    return 32;
        case UInt64:    return 64;
        case Float32:   return 0;
        case Float64:   return 0;
        case Bool:      return 1;
        case Void:      return 0;
    }
    return 0;       // to satisfy compiler
}

const char* BuiltinType::kind2name(Kind k) {
    switch (k) {
        case Int8:      return "int8";
        case Int16:     return "int16";
        case Int32:     return "int32";
        case Int64:     return "int64";
        case UInt8:     return "uint8";
        case UInt16:    return "uint16";
        case UInt32:    return "uint32";
        case UInt64:    return "uint64";
        case Float32:   return "float32";
        case Float64:   return "float64";
        case Bool:      return "bool";
        case Void:      return "void";
    }
    return "";      // to satisfy compiler

}
const char* BuiltinType::getName() const {
    return kind2name(kind);
}

const char* BuiltinType::getCName() const {
    switch (kind) {
        case Int8:      return "char";
        case Int16:     return "short";
        case Int32:     return "int";
        case Int64:     return "long long";
        case UInt8:     return "unsigned char";
        case UInt16:    return "unsigned short";
        case UInt32:    return "unsigned";
        case UInt64:    return "unsigned long long";
        case Float32:   return "float";
        case Float64:   return "double";
        case Bool:      return "int";
        case Void:      return "void";
    }
    return "";      // to satisfy compiler
}

bool BuiltinType::isInteger() const {
    switch (kind) {
        case Int8:      return true;
        case Int16:     return true;
        case Int32:     return true;
        case Int64:     return true;
        case UInt8:     return true;
        case UInt16:    return true;
        case UInt32:    return true;
        case UInt64:    return true;
        case Float32:   return false;
        case Float64:   return false;
        case Bool:      return false;
        case Void:      return false;
    }
    return false;       // to satisfy compiler
}

bool BuiltinType::isSignedInteger() const {
    switch (kind) {
        case Int8:      return true;
        case Int16:     return true;
        case Int32:     return true;
        case Int64:     return true;
        case UInt8:     return false;
        case UInt16:    return false;
        case UInt32:    return false;
        case UInt64:    return false;
        case Float32:   return false;
        case Float64:   return false;
        case Bool:      return false;
        case Void:      return false;
    }
    return false;       // to satisfy compiler
}

bool BuiltinType::isUnsignedInteger() const {
    switch (kind) {
        case Int8:      return false;
        case Int16:     return false;
        case Int32:     return false;
        case Int64:     return false;
        case UInt8:     return true;
        case UInt16:    return true;
        case UInt32:    return true;
        case UInt64:    return true;
        case Float32:   return false;
        case Float64:   return false;
        case Bool:      return true;
        case Void:      return false;
    }
    return false;       // to satisfy compiler
}

bool BuiltinType::isFloatingPoint() const {
    switch (kind) {
        case Int8:      return false;
        case Int16:     return false;
        case Int32:     return false;
        case Int64:     return false;
        case UInt8:     return false;
        case UInt16:    return false;
        case UInt32:    return false;
        case UInt64:    return false;
        case Float32:   return true;
        case Float64:   return true;
        case Bool:      return false;
        case Void:      return false;
    }
    return false;       // to satisfy compiler
}

void BuiltinType::printName(StringBuilder& buffer) const {
    buffer << getName();
}

void BuiltinType::debugPrint(StringBuilder& buffer) const {
    buffer << getName();
}


void PointerType::printName(StringBuilder& buffer) const {
    PointeeType.printName(buffer);
    buffer << '*';
}

void PointerType::debugPrint(StringBuilder& buffer) const {
    PointeeType.debugPrint(buffer);
    buffer << '*';
}


ArrayType::~ArrayType() {
    // TODO only if have ownership (Canonical Types may not have it)
    if (ownSize) delete sizeExpr;
}

void ArrayType::printName(StringBuilder& buffer) const {
    ElementType.printName(buffer);
    buffer << "[]";
}

void ArrayType::debugPrint(StringBuilder& buffer) const {
    ElementType.debugPrint(buffer);
    buffer << '[' << (unsigned)Size.getZExtValue() << ']';
// TEMP
    buffer << "OWN=" << ownSize;
    // TODO size
    if (sizeExpr) {
        buffer << COL_ATTR << "size=\n" << ANSI_NORMAL;
        sizeExpr->print(buffer, 0);
    }
}


UnresolvedType::~UnresolvedType() {
    delete expr;
}

void UnresolvedType::printName(StringBuilder& buffer) const {
    if (decl) {
        buffer << decl->getName();
    } else {
        // TODO print Expr's name (need function for that)
        buffer << "TODO_EXPR_NAME" << "(unresolved!)";
    }
}

void UnresolvedType::debugPrint(StringBuilder& buffer) const {
    if (decl) {
        buffer << decl->getName();
    } else {
        buffer << ANSI_RED;
        expr->printLiteral(buffer);
        buffer << ANSI_NORMAL;
    }
}


void AliasType::printName(StringBuilder& buffer) const {
    buffer << name;
}

void AliasType::debugPrint(StringBuilder& buffer) const {
    buffer << '\'' << name << '\'';
    buffer << name << "(alias: ";
    refType.debugPrint(buffer);
    buffer << ')';
}


void StructType::printName(StringBuilder& buffer) const {
    buffer << "(struct)" << decl->getName();
}

void StructType::debugPrint(StringBuilder& buffer) const {
    const std::string& name = decl->getName();
    if (name.empty()) {
        buffer << "<anonymous>";
    } else {
        buffer << "'" << name << '\'';
    }
    buffer << "(struct)";
}


void EnumType::printName(StringBuilder& buffer) const {
    buffer << "(enum)" << decl->getName();
}

void EnumType::debugPrint(StringBuilder& buffer) const {
    buffer << '\'' << decl->getName() << "'(enum)";
    // TODO canonical?
}


void FunctionType::printName(StringBuilder& buffer) const {
    // print something like int (int, int)
    QualType Q = func->getReturnType();
    Q = Q.getCanonicalType();
    Q.printName(buffer);
    buffer << " (";
    for (int i=0; i<func->numArgs(); i++) {
        if (i != 0) buffer << ", ";
        VarDecl* A = func->getArg(i);
        Q = A->getType();
        Q = Q.getCanonicalType();
        Q.printName(buffer);
    }
    if (func->isVariadic()) buffer << ", ...";
    buffer << ')';
}

void FunctionType::debugPrint(StringBuilder& buffer) const {
    printName(buffer);
}


TypeContext::TypeContext() {}

TypeContext::~TypeContext() {
    for (unsigned i=0; i<types.size(); i++) delete types[i];
}

QualType TypeContext::getPointerType(QualType ref) {
    assert(ref.isValid());
    for (unsigned i=0; i<types.size(); i++) {
        Type* t = types[i];
        if (isa<PointerType>(t)) {
            PointerType* P = cast<PointerType>(t);
            if (P->getPointeeType() == ref) return t;
        }
    }
    Type* N = new PointerType(ref);
    return add(N);
}

QualType TypeContext::getArrayType(QualType element, Expr* size, bool ownSize) {
    // Q: check for Expr*? (might be needed to resolve canonical type?)
    // always unique
    Type* N = new ArrayType(element, size, ownSize);
    return add(N);
}

QualType TypeContext::getUnresolvedType(Expr* E) {
    // Q: check for Expr*? (might be needed to resolve canonical type?)
    return add(new UnresolvedType(E));
}

QualType TypeContext::getAliasType(QualType refType, const std::string& name) {
    return add(new AliasType(refType, name));
}

QualType TypeContext::getStructType() {
    return add(new StructType());
}

QualType TypeContext::getEnumType() {
    return add(new EnumType());
}

QualType TypeContext::getFunctionType(FunctionDecl* F) {
    return add(new FunctionType(F));
}

QualType TypeContext::add(Type* T) {
    types.push_back(T);
    return QualType(T);
}


