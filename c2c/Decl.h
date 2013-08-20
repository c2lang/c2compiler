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
#include "OwningVector.h"
#include "Type.h"

using clang::SourceLocation;

namespace llvm {
class Function;
}

namespace C2 {
class StringBuilder;
class Stmt;
class Expr;
class DeclExpr;
class DeclVisitor;
class ArrayValueDecl;
class CompoundStmt;

enum DeclKind {
    DECL_FUNC = 0,
    DECL_VAR,
    DECL_ENUMVALUE,
    DECL_TYPE,
    DECL_ARRAYVALUE,
    DECL_USE
};

class Decl {
public:
    Decl(DeclKind k, bool is_public);
    virtual ~Decl();

    virtual void print(StringBuilder& buffer) = 0;
    virtual const std::string& getName() const = 0;
    virtual clang::SourceLocation getLocation() const = 0;

    DeclKind getKind() const { return static_cast<DeclKind>(DeclBits.dKind); }
    bool isPublic() const { return DeclBits.DeclIsPublic; }

    // for debugging
    void dump();
protected:
    class DeclBitfields {
    public:
        unsigned dKind : 8;
        unsigned DeclIsPublic : 1;
        unsigned FuncIsVariadic : 1;
        unsigned UseIsLocal : 1;
    };
    union {
        DeclBitfields DeclBits;
        unsigned BitsInit;      // to initialize all bits
    };
private:
    Decl(const Decl&);
    Decl& operator= (const Decl&);
};


class FunctionDecl : public Decl {
public:
    FunctionDecl(const std::string& name_, SourceLocation loc_, bool is_public, QualType rtype_);
    virtual ~FunctionDecl();
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_FUNC;
    }
    virtual void print(StringBuilder& buffer);

    void setBody(CompoundStmt* body_) {
        assert(body == 0);
        body = body_;
    }
    CompoundStmt* getBody() const { return body; }
    DeclExpr* findArg(const std::string& name) const;
    DeclExpr* getArg(unsigned int i) const { return args[i]; }
    unsigned int numArgs() const { return args.size(); }
    void addArg(DeclExpr* arg);
    virtual const std::string& getName() const { return name; }
    virtual clang::SourceLocation getLocation() const { return loc; }
    QualType getReturnType() const { return rtype; }
    void setVariadic() { DeclBits.FuncIsVariadic = true; }
    bool isVariadic() const { return DeclBits.FuncIsVariadic; }

    void setFunctionType(QualType qt) { functionType = qt; }
    QualType getType() const { return functionType; }

    // for codegen
    llvm::Function* getIRProto() const { return IRProto; }
    void setIRProto(llvm::Function* f) { IRProto = f; }
private:
    // TODO remove
    friend class CodeGenFunction;

    std::string name;
    clang::SourceLocation loc;
    QualType rtype;
    QualType functionType;

    typedef OwningVector<DeclExpr> Args;
    Args args;
    CompoundStmt* body;
    llvm::Function* IRProto;
};


class VarDecl : public Decl {
public:
    VarDecl(DeclExpr* decl_, bool is_public, bool inExpr);
    virtual ~VarDecl();
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_VAR;
    }
    virtual void print(StringBuilder& buffer);

    virtual const std::string& getName() const;
    virtual clang::SourceLocation getLocation() const;
    QualType getType() const;

    Expr* getInitValue() const; // static value, NOT incremental values
    typedef std::vector<ArrayValueDecl*> InitValues;
    typedef InitValues::const_iterator InitValuesConstIter;
    const InitValues& getIncrValues() const { return initValues; }
    void addInitValue(ArrayValueDecl* value);
private:
    DeclExpr* decl;
    InitValues initValues;
};


class EnumConstantDecl : public Decl {
public:
    EnumConstantDecl(const std::string& name_, SourceLocation loc_, QualType type_, Expr* Init, bool is_public);
    virtual ~EnumConstantDecl();
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_ENUMVALUE;
    }
    virtual void print(StringBuilder& buffer);

    virtual const std::string& getName() const { return name; }
    virtual clang::SourceLocation getLocation() const { return loc; }
    QualType getType() const { return type; }
    Expr* getInitValue() const { return InitVal; } // static value, NOT incremental values
    int getValue() const { return value; }
private:
    std::string name;
    SourceLocation loc;
    QualType type;
    Expr* InitVal;
    int value;      // set during analysis, TODO use APInt
};


class TypeDecl : public Decl {
public:
    TypeDecl(const std::string& name_, SourceLocation loc_, QualType type_, bool is_public);
    virtual ~TypeDecl();
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_TYPE;
    }
    virtual void print(StringBuilder& buffer);

    virtual const std::string& getName() const { return name; }
    virtual clang::SourceLocation getLocation() const { return loc; }
    QualType& getType() { return type; }
private:
    std::string name;
    SourceLocation loc;
    QualType type;
};


class ArrayValueDecl : public Decl {
public:
    ArrayValueDecl(const std::string& name_, SourceLocation loc_, Expr* value_);
    virtual ~ArrayValueDecl();
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_ARRAYVALUE;
    }
    virtual void print(StringBuilder& buffer);

    virtual const std::string& getName() const { return name; }
    virtual clang::SourceLocation getLocation() const { return loc; }
    Expr* getExpr() const { return value; }
private:
    std::string name;
    SourceLocation loc;
    Expr* value;
};


class UseDecl : public Decl {
public:
    UseDecl(const std::string& name_, SourceLocation loc_, bool isLocal_, const char* alias_, SourceLocation aliasLoc_);
    static bool classof(const Decl* D) {
        return D->getKind() == DECL_USE;
    }
    virtual void print(StringBuilder& buffer);

    virtual const std::string& getName() const { return name; }
    const std::string& getAlias() const { return alias; }
    virtual clang::SourceLocation getLocation() const { return loc; }
    virtual clang::SourceLocation getAliasLocation() const { return aliasLoc; }
    bool isLocal() const { return DeclBits.UseIsLocal; }
private:
    std::string name;
    std::string alias;
    SourceLocation loc;
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

