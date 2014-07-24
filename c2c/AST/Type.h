/* Copyright 2013,2014 Bas van den Berg
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
#include <vector>
#include <string>

#include <llvm/ADT/APInt.h>
#include <clang/Basic/SourceLocation.h>

using clang::SourceLocation;

#define QUAL_CONST      (0x1)
#define QUAL_VOLATILE   (0x2)
#define QUALS_MASK (0x3)

//#define TYPE_DEBUG

namespace C2 {

class StringBuilder;
class Type;
class Expr;
class EnumConstantDecl;
class StructTypeDecl;
class EnumTypeDecl;
class FunctionDecl;
class AliasTypeDecl;
class TypeDecl;
class ImportDecl;
class Module;


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
    bool isIntegerType() const;

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

    //Type* type;
    //unsigned qualifiers;
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
    TC_PACKAGE,
};


class Type {
protected:
    Type(TypeClass tc, QualType canon)
        : typeClass(tc)
        , canonicalType(canon)
    {}
    virtual void printName(StringBuilder& buffer) const = 0;
    virtual void debugPrint(StringBuilder& buffer) const = 0;
#ifdef TYPE_DEBUG
    virtual void fullDebug(StringBuilder& buffer, int indent) const = 0;
#endif
    QualType getCanonicalType() const { return canonicalType; }
public:
    virtual ~Type() {}

    TypeClass getTypeClass() const { return typeClass; }
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
    virtual void debugPrint(StringBuilder& buffer) const;
#ifdef TYPE_DEBUG
    virtual void fullDebug(StringBuilder& buffer, int indent) const;
#endif
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
    virtual void debugPrint(StringBuilder& buffer) const;
#ifdef TYPE_DEBUG
    virtual void fullDebug(StringBuilder& buffer, int indent) const;
#endif
private:
    QualType PointeeType;
};


class ArrayType : public Type {
public:
    ArrayType(QualType et, Expr* size, bool ownSize_)
        : Type(TC_ARRAY, QualType())
        , ElementType(et)
        , sizeExpr(size)
        , Size(32, 0, false)
        , hasSize(false)
        , ownSizeExpr(ownSize_)
    {}
    virtual ~ArrayType();
    static bool classof(const Type* T) { return T->getTypeClass() == TC_ARRAY; }

    QualType getElementType() const { return ElementType; }
    Expr* getSizeExpr() const { return sizeExpr; }
    const llvm::APInt& getSize() const { return Size; }
    void setSize(const llvm::APInt& value);
protected:
    virtual void printName(StringBuilder& buffer) const;
    virtual void debugPrint(StringBuilder& buffer) const;
#ifdef TYPE_DEBUG
    virtual void fullDebug(StringBuilder& buffer, int indent) const;
#endif
private:
    QualType ElementType;
    Expr* sizeExpr;
    llvm::APInt Size;
    bool hasSize;
    bool ownSizeExpr;
};


// Represents symbols that refer to user type (eg 'Point')
class UnresolvedType : public Type {
public:
    UnresolvedType(SourceLocation ploc_, const std::string& pname_,
                   SourceLocation tloc_, const std::string& tname_)
        : Type(TC_UNRESOLVED, QualType())
        , pname(pname_)
        , tname(tname_)
        , ploc(ploc_)
        , tloc(tloc_)
        , decl(0)
    {}
    virtual ~UnresolvedType() {}
    static bool classof(const Type* T) { return T->getTypeClass() == TC_UNRESOLVED; }

    const std::string& getPName() const { return pname; }
    const std::string& getTName() const { return tname; }
    SourceLocation getPLoc() const { return ploc; }
    SourceLocation getTLoc() const { return tloc; }

    void setDecl(TypeDecl* t) const { decl = t; }
    TypeDecl* getDecl() const { return decl; }
    void printLiteral(StringBuilder& output) const;
protected:
    virtual void printName(StringBuilder& buffer) const;
    virtual void debugPrint(StringBuilder& buffer) const;
#ifdef TYPE_DEBUG
    virtual void fullDebug(StringBuilder& buffer, int indent) const;
#endif
private:
    std::string pname;
    std::string tname;
    SourceLocation ploc;
    SourceLocation tloc;
    mutable TypeDecl* decl;
};


// AliasType are used whenever 'type A B' is used. B is the AliasType,
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
protected:
    virtual void printName(StringBuilder& buffer) const;
    virtual void debugPrint(StringBuilder& buffer) const;
#ifdef TYPE_DEBUG
    virtual void fullDebug(StringBuilder& buffer, int indent) const;
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
protected:
    virtual void printName(StringBuilder& buffer) const;
    virtual void debugPrint(StringBuilder& buffer) const;
#ifdef TYPE_DEBUG
    virtual void fullDebug(StringBuilder& buffer, int indent) const;
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
protected:
    virtual void printName(StringBuilder& buffer) const;
    virtual void debugPrint(StringBuilder& buffer) const;
#ifdef TYPE_DEBUG
    virtual void fullDebug(StringBuilder& buffer, int indent) const;
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
protected:
    virtual void printName(StringBuilder& buffer) const;
    virtual void debugPrint(StringBuilder& buffer) const;
#ifdef TYPE_DEBUG
    virtual void fullDebug(StringBuilder& buffer, int indent) const;
#endif
private:
    FunctionDecl* func;
};


class ModuleType : public Type {
public:
    ModuleType(ImportDecl* decl_)
        : Type(TC_PACKAGE, QualType(this))
        , decl(decl_)
    {}
    static bool classof(const Type* T) { return T->getTypeClass() == TC_PACKAGE; }

    ImportDecl* getDecl() const { return decl; }
    const Module* getModule() const;
protected:
    virtual void printName(StringBuilder& buffer) const;
    virtual void debugPrint(StringBuilder& buffer) const;
#ifdef TYPE_DEBUG
    virtual void fullDebug(StringBuilder& buffer, int indent) const;
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

class TypeContext {
public:
    TypeContext();
    ~TypeContext();

    QualType getPointerType(QualType ref);
    QualType getArrayType(QualType element, Expr* size, bool ownSize);
    QualType getUnresolvedType(SourceLocation ploc, const std::string& pname,
                               SourceLocation tloc, const std::string& tname);
    QualType getAliasType(AliasTypeDecl* A, QualType ref);
    QualType getStructType();
    QualType getEnumType();
    QualType getFunctionType(FunctionDecl* F);
    QualType getModuleType(ImportDecl* D);
private:
    QualType add(Type* T);

    typedef std::vector<Type*> Types;
    Types types;
};

}

#endif

