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
#include "CodeGenerator.h"

using namespace C2;

namespace C2 {

struct StructMember {
    StructMember(const char* name_, Type* type_)
        : name(name_)
        , type(type_)
        , next(0)
    {}
    ~StructMember() {
        if (type->own()) delete type;
    }
    std::string name;
    Type* type;
    StructMember* next;
};

struct EnumValue {
    EnumValue(const char* name_, int value_)
        : name(name_)
        , value(value_)
        , next(0)
    {}
    std::string name;
    int value;
    EnumValue* next;
};

struct Argument {
    Argument(Type* type_) : type(type_), next(0) {}
    ~Argument() {
        if (type->own()) delete type;
    }
    Type* type;
    Argument* next;
};

}

static void printArray(StringBuilder& buffer, Expr* expr) {
    if (expr == 0) buffer << "[]";
    else buffer << '[';
    expr->print(0, buffer);
     buffer << ']';
}

static void printQualifier(StringBuilder& buffer, unsigned int flags) {
    if (flags & TYPE_LOCAL) buffer << "local ";
    if (flags & TYPE_VOLATILE) buffer << "volatile ";
    if (flags & TYPE_CONST) buffer << "const ";
}


Type::Type(Type::Kind kind_, Type* refType_)
    : kind(kind_)
    , refType(refType_)
{
    memset(initializer, 0, sizeof(initializer));

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
    if (refType) {
        assert(kind != BUILTIN);
        if (refType->own()) delete refType;
    }
    switch (kind) {
    case BUILTIN:
        break;
    case USER:
        delete userType;
        break;
    case STRUCT:
        while (members) {
            StructMember* next = members->next;
            delete members;
            members = next;
        }
        break;
    case UNION:
        break;
    case ENUM:
        while (enumValues) {
            EnumValue* next = enumValues->next;
            delete enumValues;
            enumValues = next;
        }
        break;
    case FUNC:
        if (returnType && returnType->own()) delete returnType;
        while (arguments) {
            Argument* next = arguments->next;
            delete arguments;
            arguments = next;
        }
        break;
    case POINTER:
        break;
    case ARRAY:
        delete arrayExpr;
        break;
    case QUALIFIER:
        break;
    }
}

void Type::addStructMember(const char* name_, Type* type_) {
    assert(kind == STRUCT || kind == UNION);
    StructMember* newm = new StructMember(name_, type_);
    if (members == 0) {
        members = newm;
    } else {
        StructMember* last = members;
        while (last->next) last = last->next;
        last->next = newm;
    }
}

void Type::addEnumValue(const char* name_, int value_) {
    assert(kind == ENUM);
    EnumValue* newv = new EnumValue(name_, value_);
    if (enumValues == 0) {
        enumValues = newv;
    } else {
        EnumValue* last = enumValues;
        while (last->next) last = last->next;
        last->next = newv;
    }
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
    {
        buffer.indent(indent);
        buffer << "struct " << " {\n";
        StructMember* mem = members;
        while (mem) {
            buffer.indent(2*(indent+1));
            mem->type->printFull(buffer, indent+1);
            buffer << ' ' << mem->name << ";\n";
            mem = mem->next;
        }
        buffer.indent(indent);
        buffer << '}';
        break;
    }
    case UNION:
    {
        buffer.indent(indent);
        buffer << "union " << " {\n";
        StructMember* mem = members;
        while (mem) {
            buffer.indent(2*(indent+1));
            mem->type->printName(buffer);
            buffer << ' ' << mem->name << ";\n";
            mem = mem->next;
        }
        buffer.indent(indent);
        buffer << '}';
        break;
    }
    case ENUM:
    { 
        buffer.indent(indent);
        buffer << "enum " << " {\n";
        EnumValue* val = enumValues;
        while (val) {
            buffer.indent(2*(indent+1));
            buffer << val->name << " = " << val->value << ",\n";
            val = val->next;
        }
        buffer.indent(indent);
        buffer << '}';
        break;
    }
    case FUNC:
    {
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
        return;
    case STRUCT:
    case UNION:
    case ENUM:
    case FUNC:
        assert(0);
        break;
    case USER:
        userType->generateC(0, buffer);
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

void Type::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[type] ";
    switch (kind) {
    case BUILTIN:
        buffer << "(builtin) " << name << '\n';
        break;
    case USER:
        buffer << "(user)\n";
        ASSERT_NOT_NULL(userType);
        userType->print(indent + INDENT, buffer);
        break;
    case UNION:
        buffer << "(union)\n";
        {
            StructMember* member = members;
            while (member) {
                member->type->print(indent + INDENT, buffer);
                member = member->next;
            }
        }
        break;
    case ENUM:
        buffer << "(enum)\n";
        break;
    case STRUCT:
        buffer << "(struct)\n";
        {
            StructMember* member = members;
            while (member) {
                member->type->print(indent + INDENT, buffer);
                member = member->next;
            }
        }
        break;
    case FUNC:
    {
        buffer << "(func)\n";
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
        buffer << "(pointer)\n";
        refType->print(indent + INDENT, buffer);
        break;
    case ARRAY:
        buffer << "(array)\n";
        refType->print(indent + INDENT, buffer);
        break;
    case QUALIFIER:
        buffer << "(qualifier)\n";
        refType->print(indent + INDENT, buffer);
        break;
    }
}

void Type::dump() const {
    StringBuilder buffer;
    //printEffective(buffer, 0);
    print(0, buffer);
    fprintf(stderr, "[TYPE] %s\n", (const char*)buffer);
}

void Type::generateC_PreName(StringBuilder& buffer) const {
    switch (kind) {
    case BUILTIN:
        assert(cname);
        buffer << cname;
        break;
    case STRUCT:
        buffer << "struct {\n";
        {
            StructMember* member = members;
            while (member) {
                member->type->generateC_PreName(buffer);
                buffer << ' ' << member->name;
                member->type->generateC_PostName(buffer);
                buffer << ";\n";
                member = member->next;
            }
        }
        buffer << "}";
        break;
    case UNION:
        buffer << "union {\n";
        {
            StructMember* member = members;
            while (member) {
                member->type->generateC_PreName(buffer);
                buffer << ' ' << member->name;
                member->type->generateC_PostName(buffer);
                buffer << ";\n";
                member = member->next;
            }
        }
        buffer << "}";
        break;
    case ENUM:
    case FUNC:
        assert(0 && "TODO");
        break;
    case USER:
        userType->generateC(0, buffer);
        break;
    case POINTER:
        refType->generateC_PreName(buffer);
        buffer << '*';
        break;
    case ARRAY:
        refType->generateC_PreName(buffer);
        break;
    case QUALIFIER:
        printQualifier(buffer, qualifiers);
        refType->generateC_PreName(buffer);
        break;
    }
}

void Type::generateC_PostName(StringBuilder& buffer) const {
    if (kind == ARRAY) {
        ASSERT_NOT_NULL(refType);
        refType->generateC_PostName(buffer);
        buffer << '[';
        if (arrayExpr) arrayExpr->generateC(0, buffer);
        buffer << ']';
    }
}

llvm::Type* Type::convert(CodeGenContext& C) {
    llvm::Type* tt = 0;
    switch (kind) {
    case BUILTIN:
        {
            // TEMP, use C2Type enum
            // TODO make u8/16/32 unsigned
            if (strcmp(name, "u8") == 0) return C.builder.getInt8Ty();
            if (strcmp(name, "u16") == 0) return C.builder.getInt16Ty();
            if (strcmp(name, "u32") == 0) return C.builder.getInt32Ty();
            if (strcmp(name, "int") == 0) return C.builder.getInt32Ty();
            if (strcmp(name, "char") == 0) return C.builder.getInt8Ty();
            if (strcmp(name, "float") == 0) return C.builder.getFloatTy();
            if (strcmp(name, "void") == 0) return C.builder.getVoidTy();
            if (strcmp(name, "s8") == 0) return C.builder.getInt8Ty();
            if (strcmp(name, "s16") == 0) return C.builder.getInt16Ty();
            if (strcmp(name, "s32") == 0) return C.builder.getInt32Ty();
            // TODO 'string' type
            assert(0 && "TODO");
        }
        break;
    case USER:
        assert(0 && "TODO");
        break;
    case STRUCT:
        assert(0 && "TODO");
        break;
    case UNION:
        assert(0 && "TODO");
        break;
    case ENUM:
        assert(0 && "TODO");
        break;
    case FUNC:
        assert(0 && "TODO");
        break;
    case POINTER:
        tt = refType->convert(C);
        return tt->getPointerTo();
    case ARRAY:
        // Hmm for function args, array are simply converted to pointers, do that for now
        // array: use type = ArrayType::get(elementType, numElements)
        tt = refType->convert(C);
        return tt->getPointerTo();
    case QUALIFIER:
        assert(0 && "TODO");
        break;
    }

    return 0;
}

