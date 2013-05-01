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

#define TYPE_CONST      (1<<1) 
#define TYPE_VOLATILE   (1<<2)
#define TYPE_LOCAL      (1<<3)

namespace llvm {
class Type;
}

namespace C2 {
class StringBuilder;
class EnumValue;
class Argument;
class CodeGenContext;
class Expr;
class DeclExpr;

typedef OwningVector<C2::DeclExpr> MemberList;

class Type {
public:
    enum Kind {
        BUILTIN = 0,
        USER,       // used when parsing (underlying type is set in refType during analysis)
        STRUCT,
        UNION,
        ENUM,
        FUNC,
        POINTER,    // has refType
        ARRAY,      // has refType
        QUALIFIER   // has refType
    };

    Type(Type::Kind kind_, Type* refType_ = 0);
    ~Type();

    // generic
    Kind getKind() const { return kind; }
    Type* getRefType() const { return refType; }
    void setRefType(Type* t);
    bool isUserType() const { return kind == USER; }
    bool isFuncType() const { return kind == FUNC; }
    bool isStructOrUnionType() const { return kind == STRUCT || kind == UNION; }
    bool isSubscriptable() const { return kind == ARRAY || kind == POINTER; }
    bool isPointerType() const { return kind == POINTER; }

    // Builtin type
    void setBuiltinName(const char* name_, const char* cname_) {
        name = name_;
        cname = cname_;
    }

    // user type
    void setUserType(Expr* expr) { userType = expr; }
    Expr* getUserType() const { return userType; }

    // ARRAY
    void setArrayExpr(Expr* expr) { arrayExpr = expr; }

    // STRUCT/UNION
    void setMembers(MemberList& members_);
    MemberList* getMembers() const;

    // ENUM
    void addEnumValue(const char* name_, int value_);

    // FUNC
    void setReturnType(Type* type);
    void addArgument(Type* type);
    Type* getReturnType() const { return returnType; }

    // QUALIFIER
    void setQualifier(unsigned int flags);

    bool isCompatible(const Type& t2) const;

    void printFull(StringBuilder& buffer, int indent = 0) const;
    void printEffective(StringBuilder& buffer, int indent = 0) const;
    void print(int indent, StringBuilder& buffer, bool recursive=true) const;
    void dump() const;

    void generateC_PreName(StringBuilder& buffer) const;
    void generateC_PostName(StringBuilder& buffer) const;

    // for analysis
    bool hasBuiltinBase() const;
    Expr* getBaseUserType() const;

    llvm::Type* convert(CodeGenContext& C);
private:
    // TODO remove printName
    void printName(StringBuilder& buffer) const;

    Kind kind;
    Type* refType;

    union {
        unsigned int initializer[4];    // TODO determine

        // builtin
        struct {
            const char* name;
            const char* cname;
        };

        // user types, can be IdentifierExpr or MemberExpr
        Expr* userType;

        // struct | union specific
        struct {
            MemberList* members;
            const char* sname; // no ownership?
        };

        // enum
        struct {
            EnumValue* enumValues;
            const char* ename; // no ownership?
        };

        // func specific
        struct {
            Type* returnType;
            Argument* arguments;
            const char* fname;   // can be 0 for function proto's.
        };

        // pointer
        // nothing needed

        // array specific
        Expr* arrayExpr;

        // qualifier specific
        unsigned int qualifiers;
    };
};


enum C2Type {
    TYPE_U8 = 0,
    TYPE_U16,
    TYPE_U32,
    TYPE_S8,
    TYPE_S16,
    TYPE_S32,
    TYPE_INT,
    TYPE_STRING,
    TYPE_FLOAT,
    TYPE_F32,
    TYPE_F64,
    TYPE_CHAR,
    TYPE_BOOL,
    TYPE_VOID,
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
    Type* getPointer(Type* ref);
    Type* getStruct(bool isStruct);
    Type* getArray(Type* ref, Expr* sizeExpr);
    Type* getQualifier(Type* ref, unsigned int qualifier);
private:
    typedef std::vector<Type*> Types;
    Types types;
};


}

#endif

