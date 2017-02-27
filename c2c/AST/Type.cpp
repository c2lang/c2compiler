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

#include "AST/Type.h"
#include "AST/Expr.h"
#include "AST/Decl.h"
#include "AST/ASTContext.h"
#include "Utils/StringBuilder.h"
#include "Utils/UtilsConstants.h"
#include "Utils/color.h"

using namespace C2;

// TODO insert QualType function impls
QualType QualType::getCanonicalType() const {
    QualType canon = getTypePtr()->canonicalType;
    canon.setQualifiers(getQualifiers());
    return canon;
}

bool QualType::isBuiltinType() const {
    return getTypePtr()->isBuiltinType();
}
bool QualType::isPointerType() const {
    return getTypePtr()->isPointerType();
}
bool QualType::isArrayType() const {
    return getTypePtr()->isArrayType();
}
bool QualType::isAliasType() const {
    return getTypePtr()->isAliasType();
}
bool QualType::isStructType() const {
    return getTypePtr()->isStructType();
}
bool QualType::isFunctionType() const {
    return getTypePtr()->isFunctionType();
}
bool QualType::isSubscriptable() const {
    return getTypePtr()->isSubscriptable();
}
bool QualType::isEnumType() const {
    return isa<EnumType>(getTypePtr());
}
bool QualType::isIntegerType() const {
    QualType Canon = getTypePtr()->getCanonicalType();
    if (BuiltinType* BI = dyncast<BuiltinType>(Canon.getTypePtr())) {
        return BI->isInteger();
    }
    return false;
}
bool QualType::isArithmeticType() const {
    QualType Canon = getTypePtr()->getCanonicalType();
    if (BuiltinType* BI = dyncast<BuiltinType>(Canon.getTypePtr())) {
        return BI->isArithmetic();
    }
    return false;
}
bool QualType::isScalarType() const {
    QualType Canon = getTypePtr()->getCanonicalType();
    if (Canon == Type::Bool()) return true;
    if (isArithmeticType()) return true;
    if (isPointerType()) return true;
    if (isFunctionType()) return true;
    if (isa<EnumType>(Canon)) return true;
    return false;
}

bool QualType::isIncompleteType() const {
    const Type* T = getCanonicalType();
    switch (T->getTypeClass()) {
    case TC_BUILTIN:
        return cast<BuiltinType>(T)->isVoid();
    case TC_ARRAY:
        return cast<ArrayType>(T)->getElementType().isIncompleteType();
    default:
        return false;
    }
}

bool QualType::isConstant() const {
    const Type* T = getCanonicalType();
    switch (T->getTypeClass()) {
    case TC_BUILTIN:
        return isConstQualified();
    case TC_POINTER:
        // both pointer and pointee must be const qualified
        return isConstQualified() && cast<PointerType>(T)->getPointeeType().isConstQualified();
    case TC_ARRAY:
        return cast<ArrayType>(T)->getElementType().isConstant();
    case TC_UNRESOLVED:
    case TC_ALIAS:
        assert(0);
        break;
    case TC_STRUCT:
        return isConstQualified();
    case TC_ENUM:
        assert(0);
        break;
    case TC_FUNCTION:
        return isConstQualified();
    case TC_MODULE:
        assert(0);
        break;
    }
    return false;

}

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
    buffer.setColor(COL_TYPE);
    buffer << '\'';
    debugPrint(buffer);
    buffer.setColor(COL_TYPE);
    buffer << '\'';
    const Type* T = getTypePtrOrNull();
    if (T && T != T->canonicalType.getTypePtrOrNull()) {
        buffer.setColor(COL_CANON);
        buffer << "=>";
        getCanonicalType().debugPrint(buffer);
    }
}

void QualType::debugPrint(StringBuilder& buffer) const {
    if (isNull()) {
        buffer.setColor(ANSI_RED);
        buffer << "??";
        buffer.setColor(ANSI_NORMAL);
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
#ifdef TYPE_DEBUG
        output.enableColor(true);
        output << "TYPE:\n";
        fullDebug(output, INDENT);
        output.setColor(COL_NORM);
#else
        debugPrint(output);
#endif
        fprintf(stderr, "%s\n", output.c_str());
    }
}

#ifdef TYPE_DEBUG
void QualType::fullDebug(StringBuilder& buffer, int indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_EXPR);
    buffer << "[QualType] " << (void*)this;
    buffer.setColor(COL_ATTR);
    buffer << " qualifiers=";
    printQualifiers(buffer);
    buffer << '\n';
    if (isNull()) {
        buffer.indent(indent);
        buffer.setColor(ANSI_RED);
        buffer << "type=NULL\n";
    } else {
        getTypePtr()->fullDebug(buffer, indent);
    }
}
#endif

void QualType::printQualifiers(StringBuilder& buffer) const {
    if (hasQualifiers()) {
        if (isConstQualified()) buffer << "const ";
        if (isVolatileQualified()) buffer << "volatile ";
    }
}


void* Type::operator new(size_t bytes, const C2::ASTContext& C, unsigned alignment) {
    return ::operator new(bytes, C, alignment);
}

void Type::setCanonicalType(QualType qt) const {
    assert(canonicalType.isNull());
    canonicalType = qt;
}

void Type::printName(StringBuilder& buffer) const {
    switch (getTypeClass()) {
    case TC_BUILTIN:
        cast<BuiltinType>(this)->printName(buffer);
        break;
    case TC_POINTER:
        cast<PointerType>(this)->printName(buffer);
        break;
    case TC_ARRAY:
        cast<ArrayType>(this)->printName(buffer);
        break;
    case TC_UNRESOLVED:
        cast<UnresolvedType>(this)->printName(buffer);
        break;
    case TC_ALIAS:
        cast<AliasType>(this)->printName(buffer);
        break;
    case TC_STRUCT:
        cast<StructType>(this)->printName(buffer);
        break;
    case TC_ENUM:
        cast<EnumType>(this)->printName(buffer);
        break;
    case TC_FUNCTION:
        cast<FunctionType>(this)->printName(buffer);
        break;
    case TC_MODULE:
        cast<ModuleType>(this)->printName(buffer);
        break;
    }
}

void Type::debugPrint(StringBuilder& buffer) const {
    switch (getTypeClass()) {
    case TC_BUILTIN:
        cast<BuiltinType>(this)->debugPrint(buffer);
        break;
    case TC_POINTER:
        cast<PointerType>(this)->debugPrint(buffer);
        break;
    case TC_ARRAY:
        cast<ArrayType>(this)->debugPrint(buffer);
        break;
    case TC_UNRESOLVED:
        cast<UnresolvedType>(this)->debugPrint(buffer);
        break;
    case TC_ALIAS:
        cast<AliasType>(this)->debugPrint(buffer);
        break;
    case TC_STRUCT:
        cast<StructType>(this)->debugPrint(buffer);
        break;
    case TC_ENUM:
        cast<EnumType>(this)->debugPrint(buffer);
        break;
    case TC_FUNCTION:
        cast<FunctionType>(this)->debugPrint(buffer);
        break;
    case TC_MODULE:
        cast<ModuleType>(this)->debugPrint(buffer);
        break;
    }
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
#if 0
void Type::debugPrint(StringBuilder& buffer) const {
    // NOTE: never used
    assert(0);
    // only used to print canonical type (called by Sub-Class::debugPrint())
    buffer << "  canonical=";
    if (canonicalType.isNull()) {
        buffer.setColor(ANSI_RED);
        buffer << "???";
    } else {
        const Type* Canon = canonicalType.getTypePtr();
        buffer.setColor(COL_ATTR);
        if (Canon == this) {
            buffer << "this";
        } else {
            Canon->printName(buffer);
        }
    }
    buffer << '\n';
}
#endif

#ifdef TYPE_DEBUG
void Type::fullDebug(StringBuilder& buffer, int indent) const {
    switch (getTypeClass()) {
    case TC_BUILTIN:
        cast<BuiltinType>(this)->fullDebugImpl(buffer, indent);
        break;
    case TC_POINTER:
        cast<PointerType>(this)->fullDebugImpl(buffer, indent);
        break;
    case TC_ARRAY:
        cast<ArrayType>(this)->fullDebugImpl(buffer, indent);
        break;
    case TC_UNRESOLVED:
        cast<UnresolvedType>(this)->fullDebugImpl(buffer, indent);
        break;
    case TC_ALIAS:
        cast<AliasType>(this)->fullDebugImpl(buffer, indent);
        break;
    case TC_STRUCT:
        cast<StructType>(this)->fullDebugImpl(buffer, indent);
        break;
    case TC_ENUM:
        cast<EnumType>(this)->fullDebugImpl(buffer, indent);
        break;
    case TC_FUNCTION:
        cast<FunctionType>(this)->fullDebugImpl(buffer, indent);
        break;
    case TC_MODULE:
        cast<ModuleType>(this)->fullDebugImpl(buffer, indent);
        break;
    }
}

void Type::fullDebugImpl(StringBuilder& buffer, int indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_ATTR);
    buffer << "canonical=";
    if (canonicalType.isNull()) {
        buffer.setColor(ANSI_RED);
        buffer << "???\n";
    } else {
        const Type* Canon = canonicalType.getTypePtr();
        buffer.setColor(COL_ATTR);
        if (Canon == this) {
            buffer << "this\n";
        } else {
            buffer << '\n';
            canonicalType.fullDebug(buffer, indent+INDENT);
        }
    }
}
#endif

void Type::dump() const {
    StringBuilder output;
#ifdef TYPE_DEBUG
    output.enableColor(true);
    output << "TYPE:\n";
    fullDebug(output, INDENT);
    output.setColor(COL_NORM);
#else
    debugPrint(output);
#endif
    fprintf(stderr, "[TYPE] '%s'\n", output.c_str());
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

QualType Type::Int8() {
    return QualType(&_Int8);
}
QualType Type::Int16() {
    return QualType(&_Int16);
}
QualType Type::Int32() {
    return QualType(&_Int32);
}
QualType Type::Int64() {
    return QualType(&_Int64);
}
QualType Type::UInt8() {
    return QualType(&_UInt8);
}
QualType Type::UInt16() {
    return QualType(&_UInt16);
}
QualType Type::UInt32() {
    return QualType(&_UInt32);
}
QualType Type::UInt64() {
    return QualType(&_UInt64);
}
QualType Type::Float32() {
    return QualType(&_Float32);
}
QualType Type::Float64() {
    return QualType(&_Float64);
}
QualType Type::Bool() {
    return QualType(&_Bool);
}
QualType Type::Void() {
    return QualType(&_Void);
}

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
    switch (getKind()) {
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
    switch (getKind()) {
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
    return kind2name(getKind());
}

const char* BuiltinType::getCName() const {
    switch (getKind()) {
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
    switch (getKind()) {
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

bool BuiltinType::isArithmetic() const {
    switch (getKind()) {
    case Int8:      return true;
    case Int16:     return true;
    case Int32:     return true;
    case Int64:     return true;
    case UInt8:     return true;
    case UInt16:    return true;
    case UInt32:    return true;
    case UInt64:    return true;
    case Float32:   return true;
    case Float64:   return true;
    case Bool:      return false;
    case Void:      return false;
    }
    return false;       // to satisfy compiler
}

bool BuiltinType::isSignedInteger() const {
    switch (getKind()) {
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
    switch (getKind()) {
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
    switch (getKind()) {
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

void BuiltinType::debugPrint(StringBuilder& buffer) const {
    buffer << getName();
}
#ifdef TYPE_DEBUG
void BuiltinType::fullDebugImpl(StringBuilder& buffer, int indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "[BuiltinType] " << (void*)this << ' '<< getName() << '\n';
    Type::fullDebugImpl(buffer, indent);
}
#endif


void BuiltinType::printName(StringBuilder& buffer) const {
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
#ifdef TYPE_DEBUG
void PointerType::fullDebugImpl(StringBuilder& buffer, int indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "[PointerType] " << (void*)this << '\n';
    buffer.indent(indent);
    buffer.setColor(COL_ATTR);
    buffer << "pointee=\n";
    PointeeType.fullDebug(buffer, indent+INDENT);
    Type::fullDebugImpl(buffer, indent);
}
#endif


void ArrayType::printName(StringBuilder& buffer) const {
    ElementType.printName(buffer);
    buffer << '[';
    if (arrayTypeBits.hasSize) buffer << (unsigned)Size.getZExtValue();
    buffer << ']';
}

void ArrayType::debugPrint(StringBuilder& buffer) const {
    ElementType.debugPrint(buffer);
    buffer << '[';
    if (arrayTypeBits.hasSize) {
        buffer << (unsigned)Size.getZExtValue();
    } else {
        if (arrayTypeBits.incremental) buffer << '+';
        if (sizeExpr) {
            buffer.setColor(COL_ATTR);
            buffer << "(expr)";
            sizeExpr->printLiteral(buffer);
        }
    }
    buffer << ']';
}
#ifdef TYPE_DEBUG
void ArrayType::fullDebugImpl(StringBuilder& buffer, int indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "[ArrayType] " << (void*)this;
    buffer.setColor(COL_ATTR);
    buffer << " hasSize=" << arrayTypeBits.hasSize;
    buffer << " size=" << (int)Size.getZExtValue();
    buffer << " isIncremental=" << arrayTypeBits.incremental << '\n';
    buffer.indent(indent);
    buffer << "sizeExpr=";
    if (sizeExpr) {
        buffer << '\n';
        sizeExpr->print(buffer, indent+INDENT);
    } else {
        buffer << "NULL\n";
    }
    buffer.indent(indent);
    buffer.setColor(COL_ATTR);
    buffer << "element=\n";
    ElementType.fullDebug(buffer, indent+INDENT);
    Type::fullDebugImpl(buffer, indent);
}
#endif

void ArrayType::setSize(const llvm::APInt& value) {
    Size = value;
    arrayTypeBits.hasSize = 1;
    // also set on Canonical
    QualType canonical = getCanonicalType();
    assert(canonical.isValid());
    Type* T = canonical.getTypePtr();
    if (T != this) {
        ArrayType* AT = cast<ArrayType>(T);
        AT->setSize(value);
    }
}


TypeDecl* UnresolvedType::getDecl() const {
    Decl* decl = typeName->getDecl();
    if (!decl) return 0;
    assert(isa<TypeDecl>(decl));
    return cast<TypeDecl>(decl);
}

void UnresolvedType::printName(StringBuilder& buffer) const {
    const Decl* decl = typeName->getDecl();
    if (decl) {
        buffer << decl->getName();
    } else {
        buffer << "(UnresolvedType)";
        printLiteral(buffer);
    }
}

void UnresolvedType::debugPrint(StringBuilder& buffer) const {
    const Decl* decl = typeName->getDecl();
    if (decl) {
        buffer << "(Unresolved)" << decl->getName();
    } else {
        buffer.setColor(ANSI_RED);
        printLiteral(buffer);
    }
}

#ifdef TYPE_DEBUG
void UnresolvedType::fullDebugImpl(StringBuilder& buffer, int indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "[UnresolvedType] " << (void*)this << '\n';
    buffer.indent(indent);
    buffer.setColor(COL_ATTR);
    buffer << "decl=";
    const Decl* decl = typeName->getDecl();
    if (decl) {
        buffer << decl->getName() << '\n';
    } else {
        buffer << "NULL\n";
    }
    buffer.indent(indent);
    buffer.setColor(COL_ATTR);
    buffer << "expr=";
    printLiteral(buffer);
    buffer << '\n';
}
#endif
void UnresolvedType::printLiteral(StringBuilder& output) const {
    if (moduleName) {
        moduleName->printLiteral(output);
        output << '.';
    }
    typeName->printLiteral(output);
}


void AliasType::printName(StringBuilder& buffer) const {
    buffer << decl->getName();
}

void AliasType::debugPrint(StringBuilder& buffer) const {
    buffer << "(alias)" << decl->getName();
}
#ifdef TYPE_DEBUG
void AliasType::fullDebugImpl(StringBuilder& buffer, int indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "[AliasType] " << (void*)this << '\n';
    Type::fullDebugImpl(buffer, indent);
    buffer.indent(indent);
    buffer.setColor(COL_ATTR);
    buffer << "decl=";
    if (decl) {
        buffer << decl->getName() << '\n';
    } else {
        buffer << "NULL\n";
    }
    buffer.indent(indent);
    buffer.setColor(COL_ATTR);
    buffer << "refType=\n";
    // Dont print fullDebugImpl() to avoid possible circular deps
    refType.fullDebug(buffer, indent+INDENT);
    //buffer.indent(indent+INDENT);
    //refType.debugPrint(buffer);
}
#endif


void StructType::printName(StringBuilder& buffer) const {
    buffer << "(struct)" << decl->getName();
}

void StructType::debugPrint(StringBuilder& buffer) const {
    const std::string& name = decl->getName();
    buffer << "(struct)";
    if (name.empty()) {
        buffer << "<anonymous>";
    } else {
        buffer << name;
    }
}
#ifdef TYPE_DEBUG
void StructType::fullDebugImpl(StringBuilder& buffer, int indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "[StructType] " << (void*)this << " TODO" << '\n';
    Type::fullDebugImpl(buffer, indent);
}
#endif


void EnumType::printName(StringBuilder& buffer) const {
    buffer << "(enum)" << decl->getName();
}

void EnumType::debugPrint(StringBuilder& buffer) const {
    buffer << '\'' << decl->getName() << "'(enum)";
    // TODO canonical?
}
#ifdef TYPE_DEBUG
void EnumType::fullDebugImpl(StringBuilder& buffer, int indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "[EnumType] " << (void*)this << '\n';
    buffer << "TODO\n";
    Type::fullDebugImpl(buffer, indent);
}
#endif


void FunctionType::printName(StringBuilder& buffer) const {
    // print something like int (int, int)
    QualType Q = func->getReturnType();
    Q.printName(buffer);
    buffer << " (";
    for (unsigned i=0; i<func->numArgs(); i++) {
        if (i != 0) buffer << ", ";
        VarDecl* A = func->getArg(i);
        Q = A->getType();
        Q.printName(buffer);
    }
    if (func->isVariadic()) buffer << ", ...";
    buffer << ')';
}

void FunctionType::debugPrint(StringBuilder& buffer) const {
    printName(buffer);
}
#ifdef TYPE_DEBUG
void FunctionType::fullDebugImpl(StringBuilder& buffer, int indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "[FunctionType] " << (void*)this << '\n';
    buffer << "TODO\n";
    Type::fullDebugImpl(buffer, indent);
}
#endif

bool FunctionType::sameProto(const FunctionType* lhs, const FunctionType* rhs) {
    return FunctionDecl::sameProto(lhs->getDecl(), rhs->getDecl());
}

const Module* ModuleType::getModule() const {
    return decl->getModule();
}

void ModuleType::printName(StringBuilder& buffer) const {
    buffer << "module";
}

void ModuleType::debugPrint(StringBuilder& buffer) const {
    buffer << "(module)" << decl->getName();
}

#ifdef TYPE_DEBUG
void ModuleType::fullDebugImpl(StringBuilder& buffer, int indent) const {
    buffer << "TODO ModuleType\n";
}
#endif

