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
#include "Utils.h"
#include "color.h"

//#define TYPE_DEBUG

using namespace C2;

namespace C2 {

class Argument {
public:
    Argument(Type* type_) : type(type_), next(0) {}
    ~Argument() {}
    Type* type;
    Argument* next;
};

}

static void printArray(StringBuilder& buffer, Expr* expr) {
    if (expr == 0) {
        buffer << "[]";
    } else {
        buffer << '[';
        expr->print(0, buffer);
        buffer << ']';
    }
}

void Type::printQualifier(StringBuilder& buffer, unsigned int flags) {
    if (flags & TYPE_LOCAL) buffer << "local ";
    if (flags & TYPE_VOLATILE) buffer << "volatile ";
    if (flags & TYPE_CONST) buffer << "const ";
}

void Type::printCQualifier(StringBuilder& buffer, unsigned int flags) {
    if (flags & TYPE_LOCAL) buffer << "static ";
    if (flags & TYPE_VOLATILE) buffer << "volatile ";
    if (flags & TYPE_CONST) buffer << "const ";
}

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
    case Type::QUALIFIER: return "qualifier";
    }
    return "UNKNOWN";
}
#endif

Type::Type(Type::Kind kind_, Type* refType_)
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
        assert(refType == 0);
        break;
    case POINTER:
    case ARRAY:
    case QUALIFIER:
        assert(refType);
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
        delete members;
        break;
    case FUNC:
        while (arguments) {
            Argument* next = arguments->next;
            delete arguments;
            arguments = next;
        }
        break;
    case POINTER:
        break;
    case ARRAY:
        if (ownArrayExpr) delete arrayExpr;
        break;
    case QUALIFIER:
        break;
    }
}

unsigned Type::getWidth() const {
    switch (kind) {
    case BUILTIN:
        return width;
    case USER:
        assert(0 && "Should not happen");
        break;
    case STRUCT:
    case UNION:
    case ENUM:
        assert(0 && "TODO");
        break;
    case FUNC:
        return 4;
    case POINTER:
        return 4;
    case ARRAY:
    case QUALIFIER:
        return refType->getWidth();
    }
    return 0;
}

bool Type::isConst() const {
    // TODO recursively?
    if (kind == QUALIFIER) return (qualifiers & TYPE_CONST);
    else return false;
}

void Type::setRefType(Type* t) {
    assert(kind == USER);
    refType = t;
}

Type* Type::getCanonical(TypeContext& context) {
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
    case QUALIFIER:
        // TODO
        break;
    }
    return this;
}

void Type::setMembers(MemberList* members_) {
    assert(kind == STRUCT || kind == UNION || kind == ENUM);
    assert(members == 0);
    members = members_;
}

MemberList* Type::getMembers() const {
    assert(kind == STRUCT || kind == UNION || kind == ENUM);
    return members;
}

void Type::setReturnType(Type* type) {
    assert(kind == FUNC);
    returnType = type;
}

void Type::addArgument(Type* type_) {
    assert(kind == FUNC);
    Argument* arg = new Argument(type_);
    if (arguments == 0) {
        arguments = arg;
    } else {
        Argument* last = arguments;
        while (last->next) last = last->next;
        last->next = arg;
    }
}

Type* Type::getArgument(unsigned i) const {
    Argument* arg = arguments;
    while (arg && i) {
        arg = arg->next;
        i--;
    }
    if (arg) return arg->type;
    else return 0;
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
    case QUALIFIER:
        // TODO
        return false;
    }
    return true;
}

void Type::setQualifier(unsigned int flags) {
    assert(kind == QUALIFIER);
    qualifiers = flags;
}

void Type::printFull(StringBuilder& buffer, int indent) const {
    switch (kind) {
    case USER:
        assert(0 && "TODO");
        break;
    case BUILTIN:
        buffer << name;
        break;
    case STRUCT:
        buffer.indent(indent);
        buffer << "struct " << " {\n";
        if (members) {
            for (unsigned i=0; i<members->size(); i++) {
                buffer.indent(2*(indent+1));
                DeclExpr* mem = (*members)[i];
                mem->getType()->printFull(buffer, indent+1);
                buffer << ' ' << mem->getName() << ";\n";
            }
        }
        buffer.indent(indent);
        buffer << '}';
        break;
    case UNION:
        buffer.indent(indent);
        buffer << "union " << " {\n";
        if (members) {
            for (unsigned i=0; i<members->size(); i++) {
                buffer.indent(2*(indent+1));
                DeclExpr* mem = (*members)[i];
                mem->getType()->printFull(buffer, indent+1);
                buffer << ' ' << mem->getName() << ";\n";
            }
        }
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
        assert(returnType);
        buffer.indent(indent);
        buffer << "func " << ' ';
        returnType->printName(buffer);
        buffer << '(';
        Argument* arg = arguments;
        while (arg) {
            arg->type->printName(buffer);
            if (arg->next != 0) buffer << ", ";
            arg = arg->next;
        }
        buffer << ')';
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
    case QUALIFIER:
        buffer.indent(indent);
        printQualifier(buffer, qualifiers);
        refType->printFull(buffer, 0);
        break;
    }
}

void Type::printEffective(StringBuilder& buffer, int indent) const {
    switch (kind) {
    case BUILTIN:
        assert(name);
        buffer.indent(indent);
        buffer << name;
        break;
    case USER:
        assert(0 && "TODO");
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
        buffer << "(struct)";
        break;
    case FUNC:
    {
        buffer.indent(indent);
        buffer << "(func)";
        returnType->printName(buffer);
        buffer << '(';
        Argument* arg = arguments;
        while (arg) {
            arg->type->printName(buffer);
            if (arg->next != 0) buffer << ", ";
            arg = arg->next;
        }
        buffer << ')';
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
    case QUALIFIER:
        buffer.indent(indent);
        printQualifier(buffer, qualifiers);
        refType->printEffective(buffer);
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
        assert(0);
        break;
    case USER:
        assert(refType);
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
    case QUALIFIER:
        printQualifier(buffer, qualifiers);
        refType->printName(buffer);
        break;
    }
}

void Type::print(int indent, StringBuilder& buffer, RecursionType recursive) const {
    buffer.indent(indent);
    buffer << "[type] ";
    switch (kind) {
    case BUILTIN:
        buffer << "(builtin) " << name << '\n';
        break;
    case USER:
        buffer << "(user)\n";
        assert(userType);
        userType->print(indent + INDENT, buffer);
        if (refType && recursive != RECURSE_NONE) {
            buffer.indent(indent + INDENT);
            buffer << ANSI_CYAN << "resolved to:" << ANSI_NORMAL << ' ';
            refType->printName(buffer);
            buffer << '\n';
        }
        break;
    case UNION:
        buffer << "(union)\n";
        if (members) {
            for (unsigned i=0; i<members->size(); i++) {
                buffer.indent(2*(indent+1));
                DeclExpr* mem = (*members)[i];
                mem->getType()->print(indent + INDENT, buffer, recursive);
            }
        }
        break;
    case ENUM:
        buffer << "(enum)\n";
        if (members && recursive != RECURSE_NONE) {
            for (unsigned i=0; i<members->size(); i++) {
                DeclExpr* mem = (*members)[i];
                mem->print(indent + INDENT, buffer);
            }
        }
        break;
    case STRUCT:
        buffer << "(struct)\n";
        if (members) {
            for (unsigned i=0; i<members->size(); i++) {
                DeclExpr* mem = (*members)[i];
                mem->getType()->print(indent + INDENT, buffer, recursive);
            }
        }
        break;
    case FUNC:
    {
        buffer << "(func)\n";
        buffer.indent(indent + INDENT);
        buffer << COL_ATTR << "returnType:" << ANSI_NORMAL << '\n';
        buffer.indent(indent + INDENT);
        returnType->printName(buffer);
        buffer << '\n';
        Argument* arg = arguments;
        if (arg) {
            buffer.indent(indent + INDENT);
            buffer << COL_ATTR << "args:" << ANSI_NORMAL << '\n';
        }
        while (arg) {
            buffer.indent(indent + INDENT);
            arg->type->printName(buffer);
            buffer << '\n';
            arg = arg->next;
        }
        break;
    }
    case POINTER:
        buffer << "(pointer)\n";
        refType->print(indent + INDENT, buffer, recursive);
        break;
    case ARRAY:
        buffer << "(array)\n";
        refType->print(indent + INDENT, buffer, recursive);
        if (arrayExpr) {
            buffer.indent(indent);
            buffer << COL_ATTR << "size:" << ANSI_NORMAL << '\n';
            arrayExpr->print(indent + INDENT, buffer);
        }
        break;
    case QUALIFIER:
        buffer << "(qualifier)\n";
        refType->print(indent + INDENT, buffer, recursive);
        break;
    }
}

void Type::DiagName(StringBuilder& buffer) const {
    buffer << '\'';
    printName(buffer);
    buffer << '\'';
}

void Type::dump() const {
    StringBuilder buffer;
    //printEffective(buffer, 0);
    print(0, buffer, RECURSE_ALL);
    fprintf(stderr, "[TYPE] %s\n", (const char*)buffer);
}

bool Type::hasBuiltinBase() const {
    switch (kind) {
    case BUILTIN:
        return true;
    case STRUCT:
    case UNION:
    case ENUM:
    case FUNC:
    case USER:
        return false;
    case POINTER:
    case ARRAY:
    case QUALIFIER:
        return refType->hasBuiltinBase();
    }
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
    case QUALIFIER:
        return refType->getBaseUserType();
    }
}


static C2::Type type_u8(Type::BUILTIN);
static C2::Type type_u16(Type::BUILTIN);
static C2::Type type_u32(Type::BUILTIN);
static C2::Type type_s8(Type::BUILTIN);
static C2::Type type_s16(Type::BUILTIN);
static C2::Type type_s32(Type::BUILTIN);
static C2::Type type_int(Type::BUILTIN);
static C2::Type type_char(Type::BUILTIN);
static C2::Type type_string(Type::BUILTIN);
static C2::Type type_f32(Type::BUILTIN);
static C2::Type type_f64(Type::BUILTIN);
static C2::Type type_bool(Type::BUILTIN);
static C2::Type type_void(Type::BUILTIN);

BuiltinType::BuiltinType() {
    type_u8.setBuiltinName(TYPE_U8, "u8", "unsigned char", 1);
    type_u16.setBuiltinName(TYPE_U16, "u16", "unsigned short", 2);
    type_u32.setBuiltinName(TYPE_U32,"u32", "unsigned int", 4);
    type_s8.setBuiltinName(TYPE_S8,"s8", "char", 1);
    type_s16.setBuiltinName(TYPE_S16,"s16", "short", 2);
    type_s32.setBuiltinName(TYPE_S32, "s32", "int", 4);
    type_int.setBuiltinName(TYPE_INT,"int", "int", 4);
    type_char.setBuiltinName(TYPE_CHAR, "char", "char", 1);
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
    case TYPE_S8:     return &type_s8;
    case TYPE_S16:    return &type_s16;
    case TYPE_S32:    return &type_s32;
    case TYPE_INT:    return &type_int;
    case TYPE_STRING: return &type_string;
    case TYPE_FLOAT:  return &type_f32;
    case TYPE_F32:    return &type_f32;
    case TYPE_F64:    return &type_f64;
    case TYPE_CHAR:   return &type_char;
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

Type* TypeContext::getPointer(Type* ref) {
    // just search all pointer types with refType = ref
    for (unsigned i=0; i<types.size(); i++) {
        Type* t = types[i];
        if (t->getKind() == Type::POINTER && t->getRefType() == ref) {
            return t;
        }
    }
    // create new
    Type* N = new Type(Type::POINTER, ref);
    types.push_back(N);
    return N;
}

Type* TypeContext::getStruct(bool isStruct) {
    Type* T = new Type(isStruct ? Type::STRUCT : Type::UNION);
    types.push_back(T);
    return T;
}

Type* TypeContext::getEnum() {
    Type* T = new Type(Type::ENUM);
    types.push_back(T);
    return T;
}

Type* TypeContext::getArray(Type* ref, Expr* sizeExpr, bool ownSize) {
    Type* T = new Type(Type::ARRAY, ref);
    T->setArrayExpr(sizeExpr, ownSize);
    types.push_back(T);
    return T;
}

Type* TypeContext::getQualifier(Type* ref, unsigned int qualifier) {
    // TODO lookup same qualifiers with same ref type
    Type* T = new Type(Type::QUALIFIER, ref);
    T->setQualifier(qualifier);
    types.push_back(T);
    return T;
}

Type* TypeContext::getFunction(Type* rtype) {
    Type* proto = new Type(Type::FUNC);
    proto->setReturnType(rtype);
    types.push_back(proto);
    return proto;
}

