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

#ifndef C2TYPE_H
#define C2TYPE_H

#include <string>
#include <vector>
#include "OwningVector.h"

#define QUAL_CONST      (0x1)
#define QUAL_VOLATILE   (0x2)
#define QUAL_RESTRICT   (0x4)

namespace C2 {
class StringBuilder;
class Argument;
class Expr;
class DeclExpr;
class TypeContext;
class Type;

typedef OwningVector<C2::DeclExpr> MemberList;

enum C2Type {
    TYPE_U8 = 0,
    TYPE_U16,
    TYPE_U32,
    TYPE_U64,
    TYPE_I8,
    TYPE_I16,
    TYPE_I32,
    TYPE_I64,
    TYPE_F32,
    TYPE_F64,
    TYPE_INT,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_VOID,
};


class QualType {
public:
    QualType()
        : type(0), qualifiers(0) {}
    QualType(Type* Ptr, unsigned Quals = 0)
        : type(Ptr), qualifiers(Quals) {}

    const Type* getTypePtr() const;
    const Type* getTypePtrOrNull() const { return type; }
    unsigned getQualifiers() const { return qualifiers; }

    const Type& operator*() const { return *getTypePtr(); }
    const Type* operator->() const { return getTypePtr(); }
    bool operator==(const QualType& rhs) {
        return (type == rhs.type) && (qualifiers = rhs.qualifiers);
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

    // Helper functions
    bool isPointerType() const;
    bool isUserType() const;
    bool isSubscriptable() const;
    bool isStructOrUnionType() const;
    bool isArrayType() const;
    bool isFuncType() const;
    bool isEnumType() const;

    // for error messages
    void DiagName(StringBuilder& buffer) const;

    // Debug functions
    enum RecursionType { RECURSE_NONE=0, RECURSE_ONCE, RECURSE_ALL };
    void print(int indent, StringBuilder& buffer, RecursionType recursive) const;
    void dump() const;
    //static void printCQualifier(StringBuilder& buffer, unsigned int flags);
private:
    Type* type;
    unsigned qualifiers;
};


class Type {
public:
    enum Kind {
        BUILTIN = 0,
        USER,       // used when parsing (underlying type is set in refType during analysis)
        STRUCT,
        UNION,
        ENUM,
        FUNC,       // abuses refType for its returntype (QualType cannot be in union)
        POINTER,    // has refType
        ARRAY,      // has refType
    };

    Type(Type::Kind kind_, QualType refType_ = 0);
    ~Type();

    // generic
    Kind getKind() const { return kind; }
    QualType getRefType() const { return refType; }
    void setRefType(QualType t);

    bool isBuiltinType() const { return kind == BUILTIN; }
    bool isUserType() const { return kind == USER; }
    bool isFuncType() const { return kind == FUNC; }
    bool isStructOrUnionType() const { return kind == STRUCT || kind == UNION; }
    bool isEnumType() const { return kind == ENUM; }
    bool isSubscriptable() const { return kind == ARRAY || kind == POINTER; }
    bool isPointerType() const { return kind == POINTER; }
    bool isArrayType() const { return kind == ARRAY; }

    unsigned getWidth() const;

    // for resolving canonical type
    //Type* getCanonical(TypeContext& context);

    // Builtin type
    void setBuiltinName(C2Type ct, const char* name_, const char* cname_, unsigned width_) {
        name = name_;
        cname = cname_;
        c2type = ct;
        width = width_;
    }
    C2Type getBuiltinType() const { return c2type; }
    const char* getCName() const { return cname; }

    // user type
    void setUserType(Expr* expr) { userType = expr; }
    Expr* getUserType() const { return userType; }

    // ARRAY
    void setArrayExpr(Expr* expr, bool ownExpr = true) {
        arrayExpr = expr;
        ownArrayExpr = ownExpr;
    }
    Expr* getArrayExpr() const { return arrayExpr; }

    // STRUCT/UNION/ENUM
    void setMembers(MemberList* members_);
    MemberList* getMembers() const;
    void setStructName(const char* name_) {
        sname = name_;
    }

    // FUNC
    void setReturnType(QualType type);
    QualType getReturnType() const { return refType; }
    void addArgument(QualType type);
    QualType getArgument(unsigned i) const;

    bool isCompatible(const Type& t2) const;

    void print(int indent, StringBuilder& buffer, QualType::RecursionType recursive) const;
    void DiagName(StringBuilder& buffer) const;
    void dump() const;

    // for analysis
    Expr* getBaseUserType() const;
private:
    friend class QualType;

    void printFull(StringBuilder& buffer, int indent = 0) const;
    void printEffective(StringBuilder& buffer, int indent = 0) const;
    void printName(StringBuilder& buffer) const;

    Kind kind;
    QualType refType;
    QualType CanonicalType;

    union {
        unsigned int initializer[4];    // TODO determine

        // builtin
        struct {
            const char* name;
            const char* cname;
            C2Type c2type;
            unsigned width;
        };

        // user types, can be IdentifierExpr or MemberExpr
        Expr* userType;

        // struct | union | enum specific
        struct {
            const char* sname;  // no ownership
            MemberList* members;
        };

        // func specific
        struct {
            Argument* arguments;
            const char* fname;   // can be 0 for function proto's.
        };

        // pointer
        // nothing needed

        // array specific
        struct {
            Expr* arrayExpr;
            bool ownArrayExpr;
        };
    };
};


class BuiltinType {
public:
    static C2::Type* get(C2Type t);
private:
    BuiltinType();
};


class TypeContext {
public:
    TypeContext();
    ~TypeContext();

    Type* getUser();
    QualType getPointer(QualType ref);
    Type* getStruct(bool isStruct);
    Type* getEnum();
    QualType getArray(QualType ref, Expr* sizeExpr, bool ownSize = true);
    Type* getFunction(QualType rtype);
private:
    typedef std::vector<Type*> Types;
    Types types;
};


}

#endif

