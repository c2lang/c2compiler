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

using clang::SourceLocation;

namespace llvm {
class Function;
}

namespace C2 {
class Type;
class StringBuilder;
class Stmt;
class Expr;
class DeclExpr;
class DeclVisitor;
class ArrayValueDecl;
class CompoundStmt;

enum DeclType {
    DECL_FUNC = 0,
    DECL_VAR,
    DECL_ENUMVALUE,
    DECL_TYPE,
    DECL_ARRAYVALUE,
    DECL_USE
};

class Decl {
public:
    Decl(bool is_public_);
    virtual ~Decl();
    virtual DeclType dtype() const = 0;
    virtual void acceptD(DeclVisitor& v) = 0;
    virtual void print(StringBuilder& buffer) = 0;

    virtual const std::string& getName() const = 0;
    virtual clang::SourceLocation getLocation() const = 0;

    bool isPublic() const { return is_public; }
    static bool isSymbol(DeclType d);

    // for debugging
    void dump();
protected:
    bool is_public;
private:
    Decl(const Decl&);
    Decl& operator= (const Decl&);
};


class FunctionDecl : public Decl {
public:
    FunctionDecl(const std::string& name_, SourceLocation loc_, bool is_public_, Type* rtype_);
    virtual ~FunctionDecl();
    virtual DeclType dtype() const { return DECL_FUNC; }
    virtual void acceptD(DeclVisitor& v);
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
    Type* getReturnType() const { return rtype; }
    void setVariadic() { m_isVariadic = true; }
    bool isVariadic() const { return m_isVariadic; }

    Type* getCanonicalType() const { return canonicalType; }
    void setCanonicalType(Type* t) { canonicalType = t; }

    // for codegen
    llvm::Function* getIRProto() const { return IRProto; }
    void setIRProto(llvm::Function* f) { IRProto = f; }
private:
    friend class CodeGenFunction;

    std::string name;
    clang::SourceLocation loc;
    Type* rtype;

    typedef OwningVector<DeclExpr> Args;
    Args args;
    CompoundStmt* body;
    bool m_isVariadic;
    // TODO EllipsisLoc
    Type* canonicalType;
    llvm::Function* IRProto;
};


class VarDecl : public Decl {
public:
    VarDecl(DeclExpr* decl_, bool is_public, bool inExpr);
    virtual ~VarDecl();
    virtual DeclType dtype() const { return DECL_VAR; }
    virtual void acceptD(DeclVisitor& v);
    virtual void print(StringBuilder& buffer);

    virtual const std::string& getName() const;
    virtual clang::SourceLocation getLocation() const;
    Type* getType() const;
    Type* getCanonicalType() const;
    void setCanonicalType(Type* t);

    Expr* getInitValue() const; // static value, NOT incremental values
    typedef std::vector<ArrayValueDecl*> InitValues;
    typedef InitValues::const_iterator InitValuesConstIter;
    const InitValues& getIncrValues() const { return initValues; }
    void addInitValue(ArrayValueDecl* value);
private:
    DeclExpr* decl;
    unsigned int flags;    // inExpr;
    InitValues initValues;
};


class EnumConstantDecl : public Decl {
public:
    EnumConstantDecl(DeclExpr* decl_);
    virtual ~EnumConstantDecl();
    virtual DeclType dtype() const { return DECL_ENUMVALUE; }
    virtual void acceptD(DeclVisitor& v);
    virtual void print(StringBuilder& buffer);

    virtual const std::string& getName() const;
    virtual clang::SourceLocation getLocation() const;
    Type* getType() const;
    Type* getCanonicalType() const;
    void setCanonicalType(Type* t);

    Expr* getInitValue() const; // static value, NOT incremental values
private:
    DeclExpr* decl;
};


class TypeDecl : public Decl {
public:
    TypeDecl(const std::string& name_, SourceLocation loc_, Type* type_, bool is_public_);
    virtual ~TypeDecl();
    virtual DeclType dtype() const { return DECL_TYPE; }
    virtual void acceptD(DeclVisitor& v);
    virtual void print(StringBuilder& buffer);

    virtual const std::string& getName() const { return name; }
    virtual clang::SourceLocation getLocation() const { return loc; }
    Type* getType() const { return type; }
private:
    std::string name;
    SourceLocation loc;
    Type* type;
};


class ArrayValueDecl : public Decl {
public:
    ArrayValueDecl(const std::string& name_, SourceLocation loc_, Expr* value_);
    virtual ~ArrayValueDecl();
    virtual DeclType dtype() const { return DECL_ARRAYVALUE; }
    virtual void acceptD(DeclVisitor& v);
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
    virtual DeclType dtype() const { return DECL_USE; }
    virtual void acceptD(DeclVisitor& v);
    virtual void print(StringBuilder& buffer);

    virtual const std::string& getName() const { return name; }
    const std::string& getAlias() const { return alias; }
    virtual clang::SourceLocation getLocation() const { return loc; }
    virtual clang::SourceLocation getAliasLocation() const { return aliasLoc; }
    bool isLocal() const { return is_local; }
private:
    std::string name;
    std::string alias;
    SourceLocation loc;
    SourceLocation aliasLoc;
    bool is_local;
};


class DeclVisitor {
public:
    virtual ~DeclVisitor() {}
    virtual void visit(C2::Decl&) { assert(0); }    // add subclass below
    virtual void visit(FunctionDecl&) {}
    virtual void visit(VarDecl&) {}
    virtual void visit(EnumConstantDecl&) {}
    virtual void visit(TypeDecl&) {}
    virtual void visit(ArrayValueDecl&) {}
    virtual void visit(UseDecl&) {}
    // add more sub-classes here
};

#define DECL_VISITOR_ACCEPT(a) void a::acceptD(DeclVisitor& v) { v.visit(*this); }

template <class T> class DeclCaster : public DeclVisitor {
public:
    virtual void visit(T& node_) {
        node = &node_;
    }
    static T* getType(C2::Decl& node_) {
        DeclCaster<T> visitor(node_);
        return visitor.node;
    }
    static T* getType(C2::Decl* node_) {
        DeclCaster<T> visitor(*node_);
        return visitor.node;
    }
private:
    DeclCaster(C2::Decl& n) : node(0) {
        n.acceptD(*this);
    }
    T* node;
};

}

#endif

