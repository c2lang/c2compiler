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

#ifndef AST_DECL_H
#define AST_DECL_H

#include <string>
#include <vector>
#include <assert.h>

#include <llvm/ADT/APSInt.h>
#include <clang/Basic/SourceLocation.h>

#include "AST/OwningVector.h"
#include "AST/Type.h"

using clang::SourceLocation;

namespace llvm {
class Function;
class Value;
}

namespace C2 {
class StringBuilder;
class Stmt;
class Expr;
class ArrayValueDecl;
class CompoundStmt;
class Module;

enum DeclKind {
    DECL_FUNC = 0,
    DECL_VAR,
    DECL_ENUMVALUE,
    DECL_ALIASTYPE,
    DECL_STRUCTTYPE,
    DECL_ENUMTYPE,
    DECL_FUNCTIONTYPE,
    DECL_ARRAYVALUE,
    DECL_IMPORT
};

class Decl {
public:
    Decl(DeclKind k, const std::string& name_, SourceLocation loc_,
         QualType type_, bool is_public, unsigned file_id);
    virtual ~Decl();

    virtual void print(StringBuilder& buffer, unsigned indent) const = 0;

    const std::string& getName() const { return name; }
    SourceLocation getLocation() const { return loc; }

    DeclKind getKind() const { return static_cast<DeclKind>(DeclBits.dKind); }
    bool isPublic() const { return DeclBits.DeclIsPublic; }
    bool isUsed() const { return DeclBits.DeclIsUsed; }
    bool isUsedPublic() const { return DeclBits.DeclIsUsedPublic; }
    void setUsed() { DeclBits.DeclIsUsed = true; }
    void setUsedPublic() { DeclBits.DeclIsUsedPublic = true; }

    void setModule(const Module* mod_) { mod = mod_; }
    const Module* getModule() const { return mod; }
    unsigned getFileID() const { return DeclBits.DeclFileID; }

    QualType getType() const { return type; }
    void setType(QualType t) { type = t; }

    // for debugging
    void dump() const;
protected:
    const std::string name;
    SourceLocation loc;
    QualType type;

    class DeclBitfields {
    public:
        unsigned dKind : 8;
        unsigned DeclFileID : 10;   // 10 bits for now
        unsigned DeclIsPublic : 1;
        unsigned DeclIsUsed : 1;
        unsigned DeclIsUsedPublic : 1;
        unsigned varDeclKind: 2;
        unsigned VarDeclHasLocalQualifier : 1;
        unsigned StructTypeIsStruct : 1;
        unsigned StructTypeIsGlobal : 1;
        unsigned FuncIsVariadic : 1;
        unsigned FuncHasDefaultArgs : 1;
        unsigned ImportIsLocal : 1;
    };
    union {
        DeclBitfields DeclBits;
        unsigned BitsInit;      // to initialize all bits
    };
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
    VarDecl(VarDeclKind k_, const std::string& name_, SourceLocation loc_,
            QualType type_, Expr* initValue_, bool is_public, unsigned file_id);
    virtual ~VarDecl();
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_VAR;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;

    Expr* getInitValue() const { return initValue; }

    void setLocalQualifier() { DeclBits.VarDeclHasLocalQualifier = true; }
    bool hasLocalQualifier() const { return DeclBits.VarDeclHasLocalQualifier; }
    bool isParameter() const { return getVarKind() == VARDECL_PARAM; }
    bool isGlobal() const { return getVarKind() == VARDECL_GLOBAL; }
    VarDeclKind getVarKind() const { return static_cast<VarDeclKind>(DeclBits.varDeclKind); }

#if 0
    // TODO move to ArrayVarDecl subclass
    typedef std::vector<ArrayValueDecl*> InitValues;
    typedef InitValues::const_iterator InitValuesConstIter;
    const InitValues& getIncrValues() const { return initValues; }
    void addInitValue(ArrayValueDecl* value);
#endif

    // for codegen
    llvm::Value* getIRValue() const { return IRValue; }
    void setIRValue(llvm::Value* v) const { IRValue = v; }
private:
    Expr* initValue;
    // TODO remove, since only for Incremental Arrays (subclass VarDecl -> GlobalVarDecl)
    //InitValues initValues;
    mutable llvm::Value* IRValue;
};


class FunctionDecl : public Decl {
public:
    FunctionDecl(const std::string& name_, SourceLocation loc_,
                 bool is_public, unsigned file_id, QualType rtype_);
    virtual ~FunctionDecl();
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_FUNC;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;

    void setBody(CompoundStmt* body_) {
        assert(body == 0);
        body = body_;
    }
    CompoundStmt* getBody() const { return body; }

    // args
    void addArg(VarDecl* arg) { args.push_back(arg); }
    VarDecl* findArg(const std::string& name) const;
    VarDecl* getArg(unsigned i) const { return args[i]; }
    unsigned numArgs() const { return args.size(); }
    unsigned minArgs() const;
    void setVariadic() { DeclBits.FuncIsVariadic = true; }
    bool isVariadic() const { return DeclBits.FuncIsVariadic; }
    void setDefaultArgs() { DeclBits.FuncHasDefaultArgs = true; }
    bool hasDefaultArgs() const { return DeclBits.FuncHasDefaultArgs; }

    // return type
    QualType getReturnType() const { return rtype; }
    void updateReturnType(QualType rt) { rtype = rt; }

    // for codegen
    llvm::Function* getIRProto() const { return IRProto; }
    void setIRProto(llvm::Function* f) const { IRProto = f; }
private:
    QualType rtype;

    typedef OwningVector<VarDecl> Args;
    Args args;
    CompoundStmt* body;
    mutable llvm::Function* IRProto;
};


class EnumConstantDecl : public Decl {
public:
    EnumConstantDecl(const std::string& name_, SourceLocation loc_, QualType type_, Expr* Init,
                     bool is_public, unsigned file_id);
    virtual ~EnumConstantDecl();
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_ENUMVALUE;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;

    Expr* getInitValue() const { return InitVal; } // static value, NOT incremental values
    llvm::APSInt getValue() const { return Val; }
    void setValue(llvm::APSInt v) { Val = v; }
private:
    Expr* InitVal;
    llvm::APSInt Val;
};


class TypeDecl : public Decl {
protected:
    TypeDecl(DeclKind kind, const std::string& name_, SourceLocation loc_,
             QualType type_, bool is_public, unsigned file_id);
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
    AliasTypeDecl(const std::string& name_, SourceLocation loc_, QualType type_, bool is_public, unsigned file_id)
        : TypeDecl(DECL_ALIASTYPE, name_, loc_, type_, is_public, file_id)
        , refType(type_)
    {}
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_ALIASTYPE;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;
    QualType getRefType() const { return refType; }
private:
    QualType refType;
};


class StructTypeDecl : public TypeDecl {
public:
    StructTypeDecl(const std::string& name_, SourceLocation loc_, QualType type_,
            bool is_struct, bool is_global, bool is_public, unsigned file_id);
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_STRUCTTYPE;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;

    void addMember(Decl* D);
    unsigned numMembers() const { return members.size(); }
    Decl* getMember(unsigned index) const { return members[index]; }
    Decl* find(const std::string& name_) const {
        for (unsigned i=0; i<members.size(); i++) {
            Decl* D = members[i];
            if (D->getName() == name_) return D;
        }
        return 0;
    }
    int findIndex(const std::string& name_) const {
        for (unsigned i=0; i<members.size(); i++) {
            Decl* D = members[i];
            if (D->getName() == name_) return i;
        }
        return -1;
    }

    bool isStruct() const { return DeclBits.StructTypeIsStruct; }
    bool isGlobal() const { return DeclBits.StructTypeIsGlobal; }
private:
    typedef OwningVector<Decl> Members;
    Members members;
};

class EnumTypeDecl : public TypeDecl {
public:
    EnumTypeDecl(const std::string& name_, SourceLocation loc_,
            QualType implType_, QualType type_, bool is_public, unsigned file_id)
        : TypeDecl(DECL_ENUMTYPE, name_, loc_, type_, is_public, file_id)
        , implType(implType_)
    {}
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_ENUMTYPE;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;

    void addConstant(EnumConstantDecl* c) { constants.push_back(c); }
    unsigned numConstants() const { return constants.size(); }
    EnumConstantDecl* getConstant(unsigned index) const { return constants[index]; }

private:
    typedef OwningVector<EnumConstantDecl> Constants;
    Constants constants;
    // TODO use
    QualType implType;
};


class FunctionTypeDecl : public TypeDecl {
public:
    FunctionTypeDecl(FunctionDecl* func_, unsigned file_id);
    virtual ~FunctionTypeDecl();
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_FUNCTIONTYPE;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;

    FunctionDecl* getDecl() const { return func; }
private:
    FunctionDecl* func;
};


class ArrayValueDecl : public Decl {
public:
    ArrayValueDecl(const std::string& name_, SourceLocation loc_, Expr* value_);
    virtual ~ArrayValueDecl();
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_ARRAYVALUE;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;

    Expr* getExpr() const { return value; }
private:
    Expr* value;
};


class ImportDecl : public Decl {
public:
    ImportDecl(const std::string& name_, SourceLocation loc_, bool isLocal_,
            const std::string& modName_, SourceLocation aliasLoc_);
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_IMPORT;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;

    const std::string& getModuleName() const { return modName; }
    virtual clang::SourceLocation getAliasLocation() const { return aliasLoc; }
    bool isLocal() const { return DeclBits.ImportIsLocal; }
private:
    std::string modName;
    SourceLocation aliasLoc;
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

