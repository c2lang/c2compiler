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

#ifndef AST_TYPE_H
#define AST_TYPE_H

#include <assert.h>
#include <stddef.h>

#include <llvm/ADT/APInt.h>

#define QUAL_CONST      (0x1)
#define QUAL_VOLATILE   (0x2)
#define QUALS_MASK (0x3)

#define TYPE_DEBUG
//#define TYPE_MEMSIZE

namespace C2 {

class StringBuilder;
class Type;
class Expr;
class IdentifierExpr;
class StructTypeDecl;
class EnumTypeDecl;
class FunctionDecl;
class AliasTypeDecl;
class TypeDecl;
class ImportDecl;
class Module;
class ASTContext;


class QualType {
public:
    QualType()
        : Value(0) {}
    QualType(Type* Ptr, unsigned Quals = 0)
        : Value((uintptr_t)Ptr | (Quals & QUALS_MASK)) {}

    Type* getTypePtr() const {
        Type* T = getTypePtrOrNull();
        assert(T && "Cannot retrieve a NULL type pointer");
        return T;
    }

    Type* getTypePtrOrNull() const { return reinterpret_cast<Type*>(Value & ~QUALS_MASK); }
    unsigned getQualifiers() const { return Value & QUALS_MASK; }

    const Type& operator*() const { return *getTypePtr(); }
    const Type* operator->() const { return getTypePtr(); }
    operator const Type*() const { return getTypePtr(); }
    bool operator==(const QualType& rhs) {
        // Q: use Type equal operator?
        return Value == rhs.Value;
    }
    bool isNull() const { return getTypePtrOrNull() == NULL; }
    bool isValid() const { return getTypePtrOrNull() != NULL; }
    bool isConstQualified() const { return (Value & QUAL_CONST); }
    bool isVolatileQualified() const { return (Value & QUAL_VOLATILE); }
    bool hasQualifiers() const { return (Value & QUALS_MASK) != 0; }
    QualType getCanonicalType() const;

    void addConst() { Value |= QUAL_CONST; }
    void addVolatile() { Value |= QUAL_VOLATILE; }
    void setQualifiers(unsigned quals) {
        Value &= ~QUALS_MASK;
        Value |= (quals & QUALS_MASK);
    }
    void clearQualifiers() {
        Value &= ~QUALS_MASK;
    }

    // Helper functions
    bool isBuiltinType() const;
    bool isPointerType() const;
    bool isArrayType() const;
    bool isAliasType() const;
    bool isStructType() const;
    bool isFunctionType() const;
    bool isSubscriptable() const;
    bool isEnumType() const;
    bool isIntegerType() const;
    bool isArithmeticType() const;
    bool isScalarType() const;
    bool isIncompleteType() const;

    bool isConstant() const;    // NOTE: not is const!

    // for Debug/Diagnostic messages
    void DiagName(StringBuilder& buffer) const;
    void printName(StringBuilder& buffer) const;
    void print(StringBuilder& buffer) const;   // with ''
    void debugPrint(StringBuilder& buffer) const;   // no ''

    // Debug functions
    void dump() const;
#ifdef TYPE_DEBUG
    void fullDebug(StringBuilder& buffer, int indent) const;
#endif
private:
    void printQualifiers(StringBuilder& buffer) const;

    uintptr_t Value;
};


enum TypeClass {
    TC_BUILTIN = 0,
    TC_POINTER,
    TC_ARRAY,
    TC_UNRESOLVED,
    TC_ALIAS,
    TC_STRUCT,
    TC_ENUM,
    TC_FUNCTION,
    TC_MODULE,
};


class alignas(void*) Type {
protected:
    Type(TypeClass tc, QualType canon)
        : canonicalType(canon)
    {
        typeBits.typeClass = tc;
    }

    void* operator new(size_t bytes) noexcept {
        assert(0 && "Type cannot be allocated with regular 'new'");
        return 0;
    }
    void operator delete(void* data) {
        assert(0 && "Type cannot be released with regular 'delete'");
    }

    void printName(StringBuilder& buffer) const;
    void debugPrint(StringBuilder& buffer) const;
#ifdef TYPE_DEBUG
    void fullDebug(StringBuilder& buffer, int indent) const;
    void fullDebugImpl(StringBuilder& buffer, int indent) const;
#endif
    QualType getCanonicalType() const { return canonicalType; }
public:
    void* operator new(size_t bytes, const ASTContext& C, unsigned alignment = 8);

    TypeClass getTypeClass() const { return static_cast<TypeClass>(typeBits.typeClass); }
    bool hasCanonicalType() const { return canonicalType.isValid(); }
    void setCanonicalType(QualType qt) const;

    void DiagName(StringBuilder& buffer) const;
    void dump() const;

    bool isBuiltinType() const;
    bool isPointerType() const;
    bool isArrayType() const;
    bool isAliasType() const;
    bool isStructType() const;
    bool isFunctionType() const;
    bool isSubscriptable() const;

    static QualType Int8();
    static QualType Int16();
    static QualType Int32();
    static QualType Int64();
    static QualType UInt8();
    static QualType UInt16();
    static QualType UInt32();
    static QualType UInt64();
    static QualType Float32();
    static QualType Float64();
    static QualType Bool();
    static QualType Void();
protected:
    class TypeBitfields {
        friend class Type;

        unsigned typeClass : 8;
    };
    enum { NumTypeBits = 8 };

    class BuiltinTypeBits {
        friend class BuiltinType;
        unsigned : NumTypeBits;

        unsigned kind : 8;
    };

    class ArrayTypeBits {
        friend class ArrayType;
        unsigned : NumTypeBits;

        unsigned hasSize : 1;
        unsigned incremental : 1;
    };

    union {
        TypeBitfields typeBits;
        BuiltinTypeBits builtinTypeBits;
        ArrayTypeBits arrayTypeBits;
    };
private:
    friend class QualType;
    mutable QualType canonicalType; // can set during analysis/parsing

    Type(const Type&);
    Type& operator=(const Type&);
};


class BuiltinType : public Type {
public:
    enum Kind {
        Int8,
        Int16,
        Int32,
        Int64,
        UInt8,
        UInt16,
        UInt32,
        UInt64,
        Float32,
        Float64,
        Bool,
        Void,
    };

    BuiltinType(Kind k)
        : Type(TC_BUILTIN, QualType(this))
    {
        builtinTypeBits.kind = k;
    }
    static bool classof(const Type* T) { return T->getTypeClass() == TC_BUILTIN; }
    static BuiltinType* get(Kind k);

    Kind getKind() const { return static_cast<Kind>(builtinTypeBits.kind); }
    unsigned getWidth() const;
    unsigned getIntegerWidth() const;
    unsigned getAlignment() const {
        switch (getKind()) {
            case Int8:      return 1;
            case Int16:     return 2;
            case Int32:     return 4;
            case Int64:     return 8;
            case UInt8:     return 1;
            case UInt16:    return 2;
            case UInt32:    return 4;
            case UInt64:    return 8;
            case Float32:   return 4;
            case Float64:   return 8;
            case Bool:      return 1;
            case Void:      return 0;
        }
        return 0;       // to satisfy compiler
    }
    static const char* kind2name(Kind k);
    const char* getName() const;
    const char* getCName() const;
    bool isInteger() const;
    bool isArithmetic() const;
    bool isSignedInteger() const;
    bool isUnsignedInteger() const;
    bool isFloatingPoint() const;
    bool isVoid() const { return getKind() == Void; }
    bool isPromotableIntegerType() const {
        switch (getKind()) {
            case Int8:      return true;
            case Int16:     return true;
            case Int32:     return false;
            case Int64:     return false;
            case UInt8:     return true;
            case UInt16:    return true;
            case UInt32:    return false;
            case UInt64:    return false;
            case Float32:   return false;
            case Float64:   return false;
            case Bool:      return true;
            case Void:      return false;
        }
        return false;       // to satisfy compiler
    }

    void printName(StringBuilder& buffer) const;
    void debugPrint(StringBuilder& buffer) const;
#ifdef TYPE_DEBUG
    void fullDebugImpl(StringBuilder& buffer, int indent) const;
#endif
};


class PointerType : public Type {
public:
    PointerType(QualType Pointee)
        : Type(TC_POINTER, QualType())
        , PointeeType(Pointee)
    {}
    static bool classof(const Type* T) { return T->getTypeClass() == TC_POINTER; }

    QualType getPointeeType() const { return PointeeType; }

    void printName(StringBuilder& buffer) const;
    void debugPrint(StringBuilder& buffer) const;
#ifdef TYPE_DEBUG
    void fullDebugImpl(StringBuilder& buffer, int indent) const;
#endif
private:
    QualType PointeeType;
};


class ArrayType : public Type {
public:
    ArrayType(QualType et, Expr* size, bool isIncremental_)
        : Type(TC_ARRAY, QualType())
        , ElementType(et)
        , sizeExpr(size)
        , Size(32, 0, false)
    {
        arrayTypeBits.hasSize = 0;
        arrayTypeBits.incremental = isIncremental_;
    }
    static bool classof(const Type* T) { return T->getTypeClass() == TC_ARRAY; }

    QualType getElementType() const { return ElementType; }
    bool isIncremental() const { return arrayTypeBits.incremental; }
    Expr* getSizeExpr() const { return sizeExpr; }
    const llvm::APInt& getSize() const { return Size; }
    void setSize(const llvm::APInt& value);

    void printName(StringBuilder& buffer) const;
    void debugPrint(StringBuilder& buffer) const;
#ifdef TYPE_DEBUG
    void fullDebugImpl(StringBuilder& buffer, int indent) const;
#endif
private:
    QualType ElementType;
    Expr* sizeExpr;
    llvm::APInt Size;
};


// Represents symbols that refer to user type (eg 'Point')
class UnresolvedType : public Type {
public:
    UnresolvedType(IdentifierExpr* moduleName_, IdentifierExpr* typeName_)
        : Type(TC_UNRESOLVED, QualType())
        , moduleName(moduleName_)
        , typeName(typeName_)
    {}
    static bool classof(const Type* T) { return T->getTypeClass() == TC_UNRESOLVED; }

    IdentifierExpr* getModuleName() const { return moduleName; }
    IdentifierExpr* getTypeName() const { return typeName; }

    TypeDecl* getDecl() const;

    void printLiteral(StringBuilder& output) const;

    void printName(StringBuilder& buffer) const;
    void debugPrint(StringBuilder& buffer) const;
#ifdef TYPE_DEBUG
    void fullDebugImpl(StringBuilder& buffer, int indent) const;
#endif
private:
    IdentifierExpr* moduleName;
    IdentifierExpr* typeName;
};


// AliasType are used whenever 'type A B' is used. A is the AliasType,
// since we need a Type there.
class AliasType : public Type {
public:
    AliasType(AliasTypeDecl* d, QualType ref)
        : Type(TC_ALIAS, QualType())
        , decl(d)
        , refType(ref)
    {}
    static bool classof(const Type* T) { return T->getTypeClass() == TC_ALIAS; }

    AliasTypeDecl* getDecl() const { return decl; }
    QualType getRefType() const { return refType; }
    void updateRefType(QualType ref) { refType = ref; }

    void printName(StringBuilder& buffer) const;
    void debugPrint(StringBuilder& buffer) const;
#ifdef TYPE_DEBUG
    void fullDebugImpl(StringBuilder& buffer, int indent) const;
#endif
private:
    AliasTypeDecl* decl;
    QualType refType;
};


class StructType : public Type {
public:
    StructType()
        : Type(TC_STRUCT, QualType(this))
        , decl(0)
    {}
    static bool classof(const Type* T) { return T->getTypeClass() == TC_STRUCT; }

    void setDecl(StructTypeDecl* decl_) { decl = decl_; }
    StructTypeDecl* getDecl() const { return decl; }

    void printName(StringBuilder& buffer) const;
    void debugPrint(StringBuilder& buffer) const;
#ifdef TYPE_DEBUG
    void fullDebugImpl(StringBuilder& buffer, int indent) const;
#endif
private:
    StructTypeDecl* decl;
};


class EnumType : public Type {
public:
    EnumType()
        : Type(TC_ENUM, QualType())
        , decl(0)
    {}
    static bool classof(const Type* T) { return T->getTypeClass() == TC_ENUM; }

    void setDecl(EnumTypeDecl* decl_) { decl = decl_; }
    EnumTypeDecl* getDecl() const { return decl; }

    void printName(StringBuilder& buffer) const;
    void debugPrint(StringBuilder& buffer) const;
#ifdef TYPE_DEBUG
    void fullDebugImpl(StringBuilder& buffer, int indent) const;
#endif
private:
    EnumTypeDecl* decl;
};


class FunctionType : public Type {
public:
    FunctionType(FunctionDecl* F)
        : Type(TC_FUNCTION, QualType(this))
        , func(F)
    {}
    static bool classof(const Type* T) { return T->getTypeClass() == TC_FUNCTION; }
    FunctionDecl* getDecl() const { return func; }

    void printName(StringBuilder& buffer) const;
    void debugPrint(StringBuilder& buffer) const;
#ifdef TYPE_DEBUG
    void fullDebugImpl(StringBuilder& buffer, int indent) const;
#endif
    static bool sameProto(const FunctionType* lhs, const FunctionType* rhs);
private:
    FunctionDecl* func;
};


class ModuleType : public Type {
public:
    ModuleType(ImportDecl* decl_)
        : Type(TC_MODULE, QualType(this))
        , decl(decl_)
    {}
    static bool classof(const Type* T) { return T->getTypeClass() == TC_MODULE; }

    ImportDecl* getDecl() const { return decl; }
    const Module* getModule() const;

    void printName(StringBuilder& buffer) const;
    void debugPrint(StringBuilder& buffer) const;
#ifdef TYPE_DEBUG
    void fullDebugImpl(StringBuilder& buffer, int indent) const;
#endif
private:
    ImportDecl* decl;
};


template <class T> static inline bool isa(const Type* type) {
    return T::classof(type);
}

template <class T> static inline T* dyncast(Type* type) {
    if (isa<T>(type)) return static_cast<T*>(type);
    return 0;
}

template <class T> static inline const T* dyncast(const Type* type) {
    if (isa<T>(type)) return static_cast<const T*>(type);
    return 0;
}

template <class T> static inline T* cast(Type* type) {
    return static_cast<T*>(type);
}

template <class T> static inline const T* cast(const Type* type) {
    return static_cast<const T*>(type);
}



inline bool Type::isBuiltinType() const {
    assert(canonicalType.isValid());
    return isa<BuiltinType>(canonicalType);
}

inline bool Type::isPointerType() const {
    assert(canonicalType.isValid());
    return isa<PointerType>(canonicalType);
}

inline bool Type::isArrayType() const {
    assert(canonicalType.isValid());
    return isa<ArrayType>(canonicalType);
}

inline bool Type::isAliasType() const {
    assert(canonicalType.isValid());
    return isa<AliasType>(canonicalType);
}

inline bool Type::isStructType() const {
    assert(canonicalType.isValid());
    return isa<StructType>(canonicalType);
}

inline bool Type::isFunctionType() const {
    assert(canonicalType.isValid());
    return isa<FunctionType>(canonicalType);
}

inline bool Type::isSubscriptable() const {
    assert(canonicalType.isValid());
    return isa<PointerType>(canonicalType) || isa<ArrayType>(canonicalType);
}

}

#endif

