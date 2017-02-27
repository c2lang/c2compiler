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

#ifndef AST_DECL_H
#define AST_DECL_H

#include <string>
#include <assert.h>

#include <llvm/ADT/APSInt.h>
#include <clang/Basic/SourceLocation.h>

#include "AST/Type.h"
#include "AST/Attr.h"

using clang::SourceLocation;

namespace llvm {
class Function;
class Value;
}

namespace C2 {
class StringBuilder;
class Expr;
class CompoundStmt;
class LabelStmt;
class Module;
class ASTContext;

enum DeclKind {
    DECL_FUNC = 0,
    DECL_VAR,
    DECL_ENUMVALUE,
    DECL_ALIASTYPE,
    DECL_STRUCTTYPE,
    DECL_ENUMTYPE,
    DECL_FUNCTIONTYPE,
    DECL_ARRAYVALUE,
    DECL_IMPORT,
    DECL_LABEL
};

class alignas(void*) Decl {
public:
    Decl(DeclKind k, const char* name_, SourceLocation loc_,
         QualType type_, bool is_public);

    void* operator new(size_t bytes, const ASTContext& C, unsigned alignment = 8);

    void print(StringBuilder& buffer, unsigned indent) const;
    void printAttributes(StringBuilder& buffer, unsigned indent) const;

    const char* getName() const { return name; }
    bool hasEmptyName() const { return name[0] == 0; }
    void fullName(StringBuilder& output) const;
    std::string DiagName() const;
    SourceLocation getLocation() const { return loc; }

    DeclKind getKind() const { return static_cast<DeclKind>(declBits.dKind); }
    bool isExported() const { return declBits.IsExported; }
    void setExported() { declBits.IsExported = true; }
    bool isPublic() const { return declBits.IsPublic; }
    void setPublic(bool isPublic) { declBits.IsPublic = isPublic; }
    bool isUsed() const { return declBits.IsUsed; }
    void setUsed() { declBits.IsUsed = true; }
    bool isUsedPublic() const { return declBits.IsUsedPublic; }
    void setUsedPublic() { declBits.IsUsedPublic = true; }
    void setHasAttributes() { declBits.HasAttributes = true; }
    bool hasAttributes() const { return declBits.HasAttributes; }
    bool hasAttribute(AttrKind kind) const;
    const AttrList& getAttributes() const;

    void setModule(const Module* mod_) { mod = mod_; }
    const Module* getModule() const { return mod; }

    QualType getType() const { return type; }
    void setType(QualType t) { type = t; }

    // for debugging
    void dump() const;
protected:
    void* operator new(size_t bytes) noexcept {
        assert(0 && "Decl cannot be allocated with regular 'new'");
        return 0;
    }
    void operator delete(void* data) {
        assert(0 && "Decl cannot be released with regular 'delete'");
    }

    void printPublic(StringBuilder& buffer) const;

    class DeclBitfields {
        friend class Decl;

        unsigned dKind : 4;
        unsigned IsExported: 1;
        unsigned IsPublic : 1;
        unsigned IsUsed : 1;
        unsigned IsUsedPublic : 1;
        unsigned HasAttributes : 1;
    };
    enum { NumDeclBits = 16 };

    class VarDeclBits {
        friend class VarDecl;
        unsigned : NumDeclBits;

        unsigned Kind: 2;
        unsigned HasLocalQualifier : 1;
    };

    class StructTypeDeclBits {
        friend class StructTypeDecl;
        unsigned : NumDeclBits;

        unsigned numMembers : 8;
        unsigned numStructFunctions : 6;
        unsigned IsStruct : 1;
        unsigned IsGlobal : 1;
    };

    class EnumTypeDeclBits {
        friend class EnumTypeDecl;
        unsigned : NumDeclBits;

        unsigned numConstants : 15;
        unsigned incremental : 1;
    };

    class FunctionDeclBits {
        friend class FunctionDecl;
        unsigned : NumDeclBits;

        unsigned structFuncNameOffset : 8;
        unsigned numArgs : 6;
        unsigned IsVariadic : 1;
        unsigned HasDefaultArgs : 1;
    };

    class ImportDeclBits {
        friend class ImportDecl;
        unsigned : NumDeclBits;

        unsigned IsLocal : 1;
    };

    union {
        DeclBitfields declBits;
        VarDeclBits varDeclBits;
        StructTypeDeclBits structTypeDeclBits;
        EnumTypeDeclBits enumTypeDeclBits;
        FunctionDeclBits functionDeclBits;
        ImportDeclBits importDeclBits;
    };
    SourceLocation loc;
    QualType type;
    const char* name;
private:
    const Module* mod;

    Decl(const Decl&);
    Decl& operator= (const Decl&);
};


enum VarDeclKind {
    VARDECL_GLOBAL = 0,
    VARDECL_LOCAL,
    VARDECL_PARAM,
    VARDECL_MEMBER
};

class VarDecl : public Decl {
public:
    VarDecl(VarDeclKind k_, const char* name_, SourceLocation loc_,
            QualType type_, Expr* initValue_, bool is_public);
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_VAR;
    }
    void print(StringBuilder& buffer, unsigned indent) const;

    Expr* getInitValue() const { return initValue; }
    void setInitValue(Expr* v) {
        assert(initValue == 0);
        initValue = v;
    }

    void setLocalQualifier() { varDeclBits.HasLocalQualifier = true; }
    bool hasLocalQualifier() const { return varDeclBits.HasLocalQualifier; }
    bool isParameter() const { return getVarKind() == VARDECL_PARAM; }
    bool isGlobal() const { return getVarKind() == VARDECL_GLOBAL; }
    VarDeclKind getVarKind() const { return static_cast<VarDeclKind>(varDeclBits.Kind); }
    QualType getOrigType() const { return origType; }

    // for codegen
    llvm::Value* getIRValue() const { return IRValue; }
    void setIRValue(llvm::Value* v) const { IRValue = v; }
private:
    QualType origType;
    Expr* initValue;
    mutable llvm::Value* IRValue;
};


class FunctionDecl : public Decl {
public:
    FunctionDecl(const char* name_, SourceLocation loc_,
                 bool is_public, QualType rtype_);
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_FUNC;
    }
    void print(StringBuilder& buffer, unsigned indent) const;

    void setBody(CompoundStmt* body_) {
        assert(body == 0);
        body = body_;
    }
    CompoundStmt* getBody() const { return body; }

    void setStructFuncNameOffset(unsigned offset) { functionDeclBits.structFuncNameOffset = offset; }
    bool matchesStructFuncName(const char* name_) const {
        return strcmp(name + functionDeclBits.structFuncNameOffset, name_) == 0;
    }

    // args
    void setArgs(VarDecl** args_, unsigned numArgs_) {
        assert(args == 0);
        args = args_;
        functionDeclBits.numArgs = numArgs_;
    }
    VarDecl* getArg(unsigned i) const { return args[i]; }
    unsigned numArgs() const { return functionDeclBits.numArgs; }
    unsigned minArgs() const;
    void setVariadic() { functionDeclBits.IsVariadic = true; }
    bool isVariadic() const { return functionDeclBits.IsVariadic; }
    void setDefaultArgs() { functionDeclBits.HasDefaultArgs = true; }
    bool hasDefaultArgs() const { return functionDeclBits.HasDefaultArgs; }

    // return type
    QualType getReturnType() const { return rtype; }
    QualType getOrigReturnType() const { return origRType; }
    void setReturnType(QualType rt) { rtype = rt; }

    static bool sameProto(const FunctionDecl* lhs, const FunctionDecl* rhs);

    // for codegen
    llvm::Function* getIRProto() const { return IRProto; }
    void setIRProto(llvm::Function* f) const { IRProto = f; }
private:
    QualType rtype;
    QualType origRType;

    VarDecl** args;
    CompoundStmt* body;
    mutable llvm::Function* IRProto;
};


class EnumConstantDecl : public Decl {
public:
    EnumConstantDecl(const char* name_, SourceLocation loc_, QualType type_, Expr* Init,
                     bool is_public);
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_ENUMVALUE;
    }
    void print(StringBuilder& buffer, unsigned indent) const;

    Expr* getInitValue() const { return InitVal; } // static value, NOT incremental values
    llvm::APSInt getValue() const { return Val; }
    void setValue(llvm::APSInt v) { Val = v; }
private:
    Expr* InitVal;
    llvm::APSInt Val;
};


class TypeDecl : public Decl {
protected:
    TypeDecl(DeclKind kind, const char* name_, SourceLocation loc_,
             QualType type_, bool is_public);
public:
    static bool classof(const Decl* D) {
        switch (D->getKind()) {
        case DECL_ALIASTYPE:
        case DECL_STRUCTTYPE:
        case DECL_ENUMTYPE:
        case DECL_FUNCTIONTYPE:
            return true;
        default:
            return false;
        }
    }
};


class AliasTypeDecl : public TypeDecl {
public:
    AliasTypeDecl(const char* name_, SourceLocation loc_, QualType type_, bool is_public)
        : TypeDecl(DECL_ALIASTYPE, name_, loc_, type_, is_public)
        , refType(type_)
    {}
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_ALIASTYPE;
    }
    void print(StringBuilder& buffer, unsigned indent) const;
    QualType getRefType() const { return refType; }
private:
    QualType refType;
};


class StructTypeDecl : public TypeDecl {
public:
    StructTypeDecl(const char* name_, SourceLocation loc_, QualType type_,
            bool is_struct, bool is_global, bool is_public);
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_STRUCTTYPE;
    }
    void print(StringBuilder& buffer, unsigned indent) const;

    void setMembers(Decl** members_, unsigned numMembers_) {
        assert(members == 0);
        members = members_;
        structTypeDeclBits.numMembers = numMembers_;
    }
    void setStructFuncs(FunctionDecl** funcs, unsigned numFuncs) {
        assert(structFunctions == 0);
        structFunctions = funcs;
        structTypeDeclBits.numStructFunctions = numFuncs;
    }

    unsigned numMembers() const { return structTypeDeclBits.numMembers; }
    unsigned numStructFunctions() const { return structTypeDeclBits.numStructFunctions; }
    Decl* getMember(unsigned index) const { return members[index]; }
    Decl* find(const char* name_) const;
    Decl* findFunction(const char* name_) const;
    int findIndex(const char*  name_) const;
    void setOpaqueMembers();

    bool isStruct() const { return structTypeDeclBits.IsStruct; }
    bool isGlobal() const { return structTypeDeclBits.IsGlobal; }
private:
    Decl** members;
    FunctionDecl** structFunctions;
};

class EnumTypeDecl : public TypeDecl {
public:
    EnumTypeDecl(const char* name_, SourceLocation loc_,
            QualType implType_, QualType type_, bool is_incr, bool is_public)
        : TypeDecl(DECL_ENUMTYPE, name_, loc_, type_, is_public)
        , constants(0)
        , implType(implType_)
    {
        enumTypeDeclBits.incremental = is_incr;
        enumTypeDeclBits.numConstants = 0;
    }
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_ENUMTYPE;
    }
    void print(StringBuilder& buffer, unsigned indent) const;

    void setConstants(EnumConstantDecl** constants_, unsigned numConstants_) {
        assert(constants == 0);
        constants = constants_;
        enumTypeDeclBits.numConstants = numConstants_;
    }
    unsigned numConstants() const { return enumTypeDeclBits.numConstants; }
    EnumConstantDecl* getConstant(unsigned i) const { return constants[i]; }

    bool isIncremental() const { return enumTypeDeclBits.incremental; }

    int getIndex(const EnumConstantDecl* c) const;
    bool hasConstantValue(llvm::APSInt Val) const;
    QualType getImplType() const { return implType; }

    llvm::APSInt getMinValue() const;
    llvm::APSInt getMaxValue() const;
private:
    EnumConstantDecl** constants;
    QualType implType;
};


class FunctionTypeDecl : public TypeDecl {
public:
    FunctionTypeDecl(FunctionDecl* func_);
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_FUNCTIONTYPE;
    }
    void print(StringBuilder& buffer, unsigned indent) const;

    FunctionDecl* getDecl() const { return func; }
private:
    FunctionDecl* func;
};


class ArrayValueDecl : public Decl {
public:
    ArrayValueDecl(const char* name_, SourceLocation loc_, Expr* value_);
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_ARRAYVALUE;
    }
    void print(StringBuilder& buffer, unsigned indent) const;

    Expr* getExpr() const { return value; }
private:
    Expr* value;
};


class ImportDecl : public Decl {
public:
    ImportDecl(const char* name_, SourceLocation loc_, bool isLocal_,
            const char* modName_, SourceLocation aliasLoc_);
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_IMPORT;
    }
    void print(StringBuilder& buffer, unsigned indent) const;

    const char* getModuleName() const { return modName; }
    bool hasAlias() const {return aliasLoc.isValid(); }
    bool isLocal() const { return importDeclBits.IsLocal; }
private:
    const char* modName;
    SourceLocation aliasLoc;
};


class LabelDecl : public Decl {
public:
    LabelDecl(const char* name_, SourceLocation loc_);
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_LABEL;
    }
    void print(StringBuilder& buffer, unsigned indent) const;

    LabelStmt* getStmt() const { return TheStmt; }
    void setStmt(LabelStmt* S) { TheStmt = S; }
    void setLocation(SourceLocation loc_) { loc = loc_; }
private:
    LabelStmt* TheStmt;
};


template <class T> static inline bool isa(const Decl* D) {
    return T::classof(D);
}

template <class T> static inline T* dyncast(Decl* D) {
    if (isa<T>(D)) return static_cast<T*>(D);
    return 0;
}

template <class T> static inline const T* dyncast(const Decl* D) {
    if (isa<T>(D)) return static_cast<const T*>(D);
    return 0;
}

//#define CAST_DEBUG

template <class T> static inline T* cast(Decl* D) {
#ifdef CAST_DEBUG
    assert(isa<T>(D));
#endif
    return static_cast<T*>(D);
}

template <class T> static inline const T* cast(const Decl* D) {
#ifdef CAST_DEBUG
    assert(isa<T>(D));
#endif
    return static_cast<const T*>(D);
}

}

#endif

