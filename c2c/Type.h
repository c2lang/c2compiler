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

#define TYPE_CONST      (1<<1) 
#define TYPE_VOLATILE   (1<<2)
#define TYPE_LOCAL      (1<<3)

namespace llvm {
class Type;
}

namespace C2 {
class StringBuilder;
class StructMember;
class EnumValue;
class Argument;
class CodeGenContext;
class Expr;

class Type {
public:
    enum Kind {
        BUILTIN = 0,
        USER,       // used when parsing (underlying type is unknown yet)
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

    bool own() const { return kind != BUILTIN; }

    // generic
    Kind getKind() const { return kind; }
    void setRefTyp(Type* refType_) { refType = refType_; }

    // Builtin type
    void setBuiltinName(const char* name_, const char* cname_) {
        name = name_;
        cname = cname_;
    }

    // user type
    void setUserType(Expr* expr) { userType = expr; }

    // ARRAY
    void setArrayExpr(Expr* expr) { arrayExpr = expr; }

    // STRUCT/UNION
    void addStructMember(const char* name_, Type* type_);

    // ENUM
    void addEnumValue(const char* name_, int value_);

    // FUNC
    void setReturnType(Type* type);
    void addArgument(Type* type);

    // QUALIFIER
    void setQualifier(unsigned int flags);

    bool isCompatible(const Type& t2) const;

    void printFull(StringBuilder& buffer, int indent = 0) const;
    void printEffective(StringBuilder& buffer, int indent = 0) const;
    void print(int indent, StringBuilder& buffer) const;
    void dump() const;

    void generateC_PreName(StringBuilder& buffer) const;
    void generateC_PostName(StringBuilder& buffer) const;

    llvm::Type* convert(CodeGenContext& C);
private:
    // TODO remove printName
    void printName(StringBuilder& buffer) const;

    Kind kind;
    Type* refType;

    union {
        unsigned int initializer[2];

        // builtin
        struct {
            const char* name;
            const char* cname;
        };

        // user types
        Expr* userType;

        // struct | union specific
        StructMember* members;

        // enum
        EnumValue* enumValues;

        // func specific
        struct {
            Type* returnType;
            Argument* arguments;
        };

        // pointer
        // nothing needed

        // array specific
        Expr* arrayExpr;

        // qualifier specific
        unsigned int qualifiers;
    };
};

}

#endif

