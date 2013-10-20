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

#ifndef DECL_H
#define DECL_H

#include <string>
#include <vector>
#include <assert.h>

#include <clang/Basic/SourceLocation.h>

#include "AST/OwningVector.h"
#include "AST/Type.h"

using clang::SourceLocation;

namespace llvm {
class Function;
}

namespace C2 {
class StringBuilder;
class Stmt;
class Expr;
class ArrayValueDecl;
class CompoundStmt;
class Package;

enum DeclKind {
    DECL_FUNC = 0,
    DECL_VAR,
    DECL_ENUMVALUE,
    DECL_ALIASTYPE,
    DECL_STRUCTTYPE,
    DECL_ENUMTYPE,
    DECL_FUNCTIONTYPE,
    DECL_ARRAYVALUE,
    DECL_USE
};

class Decl {
public:
    Decl(DeclKind k, const std::string& name_, SourceLocation loc_, bool is_public);
    virtual ~Decl();

    virtual void print(StringBuilder& buffer, unsigned indent) const = 0;

    const std::string& getName() const { return name; }
    SourceLocation getLocation() const { return loc; }

    DeclKind getKind() const { return static_cast<DeclKind>(DeclBits.dKind); }
    bool isPublic() const { return DeclBits.DeclIsPublic; }
    bool isUsed() const { return DeclBits.DeclIsUsed; }
    void setUsed() { DeclBits.DeclIsUsed = true; }

    void setPackage(const Package* pkg_) { pkg = pkg_; }
    const Package* getPackage() const { return pkg; }

    // for debugging
    void dump() const;
protected:
    const std::string name;
    SourceLocation loc;

    class DeclBitfields {
    public:
        unsigned dKind : 8;
        unsigned DeclIsPublic : 1;
        unsigned DeclIsUsed : 1;
        unsigned VarDeclHasLocalQualifier : 1;
        unsigned VarDeclIsParameter: 1;
        unsigned StructTypeIsStruct : 1;
        unsigned StructTypeIsGlobal : 1;
        unsigned FuncIsVariadic : 1;
        unsigned FuncHasDefaultArgs : 1;
        unsigned UseIsLocal : 1;
    };
    union {
        DeclBitfields DeclBits;
        unsigned BitsInit;      // to initialize all bits
    };
private:
    Package* pkg;

    Decl(const Decl&);
    Decl& operator= (const Decl&);
};


class VarDecl : public Decl {
public:
    VarDecl(const std::string& name_, SourceLocation loc_,
            QualType type_, Expr* initValue_, bool is_public = false);
    virtual ~VarDecl();
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_VAR;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;

    QualType getType() const { return type; }
    Expr* getInitValue() const { return initValue; }

    void setLocalQualifier() { DeclBits.VarDeclHasLocalQualifier = true; }
    bool hasLocalQualifier() const { return DeclBits.VarDeclHasLocalQualifier; }
    void setIsParameter() { DeclBits.VarDeclIsParameter = true; }
    bool isParameter() const { return DeclBits.VarDeclIsParameter; }

    // TODO move to GlobalVarDecl subclass
    typedef std::vector<ArrayValueDecl*> InitValues;
    typedef InitValues::const_iterator InitValuesConstIter;
    const InitValues& getIncrValues() const { return initValues; }
    void addInitValue(ArrayValueDecl* value);
private:
    QualType type;
    Expr* initValue;
    // TODO remove, since only for Incremental Arrays (subclass VarDecl -> GlobalVarDecl)
    InitValues initValues;
};


class FunctionDecl : public Decl {
public:
    FunctionDecl(const std::string& name_, SourceLocation loc_, bool is_public, QualType rtype_);
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

    QualType getReturnType() const { return rtype; }

    void setFunctionType(QualType qt) { functionType = qt; }
    QualType getType() const { return functionType; }

    // for codegen
    llvm::Function* getIRProto() const { return IRProto; }
    void setIRProto(llvm::Function* f) const { IRProto = f; }
private:
    QualType rtype;
    QualType functionType;

    typedef OwningVector<VarDecl> Args;
    Args args;
    CompoundStmt* body;
    mutable llvm::Function* IRProto;
};


class EnumConstantDecl : public Decl {
public:
    EnumConstantDecl(const std::string& name_, SourceLocation loc_, QualType type_, Expr* Init, bool is_public);
    virtual ~EnumConstantDecl();
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_ENUMVALUE;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;

    QualType getType() const { return type; }
    Expr* getInitValue() const { return InitVal; } // static value, NOT incremental values
    int getValue() const { return value; }
    void setValue(int value_) { value = value_; }
private:
    QualType type;
    Expr* InitVal;
    int value;      // set during analysis, TODO use APInt
};


class TypeDecl : public Decl {
protected:
    TypeDecl(DeclKind kind, const std::string& name_, SourceLocation loc_, QualType type_, bool is_public);
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

    QualType getType() const { return type; }
    void setType(QualType qt) { type = qt; }
protected:
    mutable QualType type;
};


class AliasTypeDecl : public TypeDecl {
public:
    AliasTypeDecl(const std::string& name_, SourceLocation loc_, QualType type_, bool is_public)
        : TypeDecl(DECL_ALIASTYPE, name_, loc_, type_, is_public)
    {}
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_ALIASTYPE;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;
};


class StructTypeDecl : public TypeDecl {
public:
    StructTypeDecl(const std::string& name_, SourceLocation loc_, QualType type_,
            bool is_struct, bool is_global, bool is_public);
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

    bool isStruct() const { return DeclBits.StructTypeIsStruct; }
    bool isGlobal() const { return DeclBits.StructTypeIsGlobal; }
private:
    typedef OwningVector<Decl> Members;
    Members members;
};

class EnumTypeDecl : public TypeDecl {
public:
    EnumTypeDecl(const std::string& name_, SourceLocation loc_,
            QualType implType_, QualType type_, bool is_public)
        : TypeDecl(DECL_ENUMTYPE, name_, loc_, type_, is_public)
        , implType(implType_)
    {}
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_ENUMTYPE;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;

    void addConstant(EnumConstantDecl* c) { constants.push_back(c); }
    unsigned numConstants() const { return constants.size(); }
    EnumConstantDecl* getConstant(unsigned index) const { return constants[index]; }

    QualType getImplType() const { return implType; }
private:
    typedef OwningVector<EnumConstantDecl> Constants;
    Constants constants;

    QualType implType;
};


class FunctionTypeDecl : public TypeDecl {
public:
    FunctionTypeDecl(FunctionDecl* func_);
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


class UseDecl : public Decl {
public:
    UseDecl(const std::string& name_, SourceLocation loc_, bool isLocal_, const char* alias_, SourceLocation aliasLoc_);
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_USE;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;

    const std::string& getAlias() const { return alias; }
    virtual clang::SourceLocation getAliasLocation() const { return aliasLoc; }
    bool isLocal() const { return DeclBits.UseIsLocal; }
private:
    std::string alias;
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
#ifdef CAST_DEBUG
#include <assert.h>
#endif

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

