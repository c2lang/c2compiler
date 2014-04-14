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

#ifndef TYPE_H
#define TYPE_H

#include <assert.h>
#include <vector>
#include <string>

// NOTE: a canonical type will never have Unresolved types in it (fully resolved).

// TODO: Use TypeBits bitfield

// LATER: Merge Ptr and Qualtype bits? (so QualType is 4 bytes)

// IDEA: match UnresolvedType on name in TypeContext? (same name re-uses UnresolvedType?)
//      Number n1;
//      Number n2; -> same UnresolvedType

#define QUAL_CONST      (0x1)
#define QUAL_VOLATILE   (0x2)
#define QUAL_RESTRICT   (0x4)

namespace C2 {

class StringBuilder;
class Type;
class Expr;
class EnumConstantDecl;
class StructTypeDecl;
class EnumTypeDecl;
class FunctionDecl;
class TypeDecl;


class QualType {
public:
    QualType()
        : type(0), qualifiers(0) {}
    QualType(Type* Ptr, unsigned Quals = 0)
        : type(Ptr), qualifiers(Quals) {}

    Type* getTypePtr() const;
    const Type* getTypePtrOrNull() const { return type; }
    unsigned getQualifiers() const { return qualifiers; }

    const Type& operator*() const { return *getTypePtr(); }
    const Type* operator->() const { return getTypePtr(); }
    operator const Type*() const { return getTypePtr(); }
    bool operator==(const QualType& rhs) {
        return (type == rhs.type) && (qualifiers == rhs.qualifiers);
    }
    bool isNull() const { return type == NULL; }
    bool isValid() const { return type != NULL; }
    bool isConstQualified() const { return (qualifiers & QUAL_CONST); }
    bool isVolatileQualified() const { return (qualifiers & QUAL_VOLATILE); }
    bool isRestrictQualified() const { return (qualifiers & QUAL_RESTRICT); }
    bool hasQualifiers() const { return qualifiers != 0; }
    QualType getCanonicalType() const;

    void addConst() { qualifiers |= QUAL_CONST; }
    void addVolatile() { qualifiers |= QUAL_VOLATILE; }
    void addRestrict() { qualifiers |= QUAL_RESTRICT; }
    void setQualifiers(unsigned quals) { qualifiers = quals; }
    void clearQualifiers() { qualifiers = 0; }

    // Helper functions
    bool isBuiltinType() const;
    bool isPointerType() const;
    bool isArrayType() const;
    bool isAliasType() const;
    bool isStructType() const;
    bool isFunctionType() const;
    bool isSubscriptable() const;

    // for Debug/Diagnostic messages
    void DiagName(StringBuilder& buffer) const;
    void printName(StringBuilder& buffer) const;
    void debugPrint(StringBuilder& buffer, unsigned indent) const;

    // Debug functions
    void dump() const;
private:
    void printQualifiers(StringBuilder& buffer, unsigned indent) const;

    Type* type;
    unsigned qualifiers;
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
};


class Type {
protected:
    Type(TypeClass tc, QualType canon)
        : typeClass(tc)
        , canonicalType(canon)
    {}
    virtual void printName(StringBuilder& buffer) const = 0;
    virtual void debugPrint(StringBuilder& buffer, unsigned indent) const = 0;
public:
    virtual ~Type() {}

    TypeClass getTypeClass() const { return typeClass; }
    bool hasCanonicalType() const { return canonicalType.isValid(); }
    QualType getCanonicalType() const { return canonicalType; }
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
private:
    friend class QualType;

    TypeClass typeClass;
    mutable QualType canonicalType; // can set during analysis/parsing

    Type(const Type&);
    void operator=(const Type&);
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
        , kind(k)
    {}
    static bool classof(const Type* T) { return T->getTypeClass() == TC_BUILTIN; }
    static BuiltinType* get(Kind k);

    Kind getKind() const { return kind; }
    unsigned getWidth() const;
    unsigned getIntegerWidth() const;
    unsigned getAlignment() const {
        switch (kind) {
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
    bool isSignedInteger() const;
    bool isUnsignedInteger() const;
    bool isFloatingPoint() const;
    bool isVoid() const { return kind == Void; }
    bool isPromotableIntegerType() const {
        switch (kind) {
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

protected:
    virtual void printName(StringBuilder& buffer) const;
    virtual void debugPrint(StringBuilder& buffer, unsigned indent) const;
private:
    Kind kind;
};


class PointerType : public Type {
public:
    PointerType(QualType Pointee)
        : Type(TC_POINTER, QualType())
        , PointeeType(Pointee)
    {}
    static bool classof(const Type* T) { return T->getTypeClass() == TC_POINTER; }

    QualType getPointeeType() const { return PointeeType; }

protected:
    virtual void printName(StringBuilder& buffer) const;
    virtual void debugPrint(StringBuilder& buffer, unsigned indent) const;
private:
    QualType PointeeType;
};


class ArrayType : public Type {
public:
    ArrayType(QualType et, Expr* size, bool ownSize_)
        : Type(TC_ARRAY, QualType())
        , ElementType(et)
        , sizeExpr(size)
        , ownSize(ownSize_)
    {}
    virtual ~ArrayType();
    static bool classof(const Type* T) { return T->getTypeClass() == TC_ARRAY; }

    QualType getElementType() const { return ElementType; }
    Expr* getSizeExpr() const { return sizeExpr; }

protected:
    virtual void printName(StringBuilder& buffer) const;
    virtual void debugPrint(StringBuilder& buffer, unsigned indent) const;
private:
    QualType ElementType;
    Expr* sizeExpr;
    bool ownSize;
};


class UnresolvedType : public Type {
public:
    UnresolvedType(Expr* E)
        : Type(TC_UNRESOLVED, QualType())
        , expr(E)
        , decl(0)
    {}
    virtual ~UnresolvedType();
    static bool classof(const Type* T) { return T->getTypeClass() == TC_UNRESOLVED; }

    Expr* getExpr() const { return expr; }
    void setDecl(TypeDecl* t) const { decl = t; }
    TypeDecl* getDecl() const { return decl; }
protected:
    virtual void printName(StringBuilder& buffer) const;
    virtual void debugPrint(StringBuilder& buffer, unsigned indent) const;
private:
    Expr* expr;         // can be IdentifierExpr (type) or MemberExpr (pkg.type)
    mutable TypeDecl* decl;
};


// AliasType are used whenever 'type A B' is used. B is the AliasType,
// since we need a Type there.
// UnresolvedTypeDecl? -> have string
// TODO need SourceLocation -> so put in Decl
class AliasType : public Type {
public:
    AliasType(QualType refType_, const std::string& name_)
        : Type(TC_ALIAS, QualType())
        , refType(refType_)
        , name(name_)
    {}
    static bool classof(const Type* T) { return T->getTypeClass() == TC_ALIAS; }

    QualType getRefType() const { return refType; }
    const std::string&  getName() const { return name; }

protected:
    virtual void printName(StringBuilder& buffer) const;
    virtual void debugPrint(StringBuilder& buffer, unsigned indent) const;
private:
    QualType refType;
    std::string name;
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
protected:
    virtual void printName(StringBuilder& buffer) const;
    virtual void debugPrint(StringBuilder& buffer, unsigned indent) const;
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
protected:
    virtual void printName(StringBuilder& buffer) const;
    virtual void debugPrint(StringBuilder& buffer, unsigned indent) const;
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
protected:
    virtual void printName(StringBuilder& buffer) const;
    virtual void debugPrint(StringBuilder& buffer, unsigned indent) const;
private:
    FunctionDecl* func;
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

class TypeContext {
public:
    TypeContext();
    ~TypeContext();

    QualType getPointerType(QualType ref);
    QualType getArrayType(QualType element, Expr* size, bool ownSize);
    QualType getUnresolvedType(Expr* E);
    QualType getAliasType(QualType refType, const std::string& name);
    QualType getStructType();
    QualType getEnumType();
    QualType getFunctionType(FunctionDecl* F);
private:
    QualType add(Type* T);

    typedef std::vector<Type*> Types;
    Types types;
};

}

#endif

