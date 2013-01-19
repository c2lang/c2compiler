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

#include <clang/Basic/SourceLocation.h>

using clang::SourceLocation;

namespace llvm {
class Value;
}

namespace C2 {
class Type;
class StringBuilder;
class Stmt;
class Expr;
class DeclExpr;
class DeclVisitor;
class CodeGenContext;

enum DeclType {
    DECL_FUNC = 0,
    DECL_VAR,
    DECL_TYPE
    //DECL_USING,
};

typedef std::vector<C2::Expr*> ExprList;

class Decl {
public:
    Decl();
    virtual ~Decl();
    virtual DeclType dtype() = 0;
    virtual void acceptD(DeclVisitor& v) = 0;
    virtual void print(StringBuilder& buffer) = 0;
    virtual void generateC(StringBuilder& buffer) = 0;
    virtual llvm::Value* codeGen(CodeGenContext& context) = 0;

    virtual const std::string& getName() const = 0;
    virtual clang::SourceLocation getLocation() const = 0;
private:
    Decl(const Decl&);
    Decl& operator= (const Decl&);
};


class FunctionDecl : public Decl {
public:
    FunctionDecl(const std::string& name_, SourceLocation loc_, bool is_public_, Type* rtype_);
    virtual ~FunctionDecl();
    virtual DeclType dtype() { return DECL_FUNC; }
    virtual void acceptD(DeclVisitor& v);
    virtual void print(StringBuilder& buffer);
    virtual void generateC(StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);

    void setBody(Stmt* body_) {
        // TODO assert body is null
        body = body_;
    }
    ExprList& getArgs() { return args; }
    virtual const std::string& getName() const { return name; }
    virtual clang::SourceLocation getLocation() const { return loc; }
private:
    std::string name;
    clang::SourceLocation loc;
    bool is_public;
    Type* rtype;

    ExprList args;
    Stmt* body;
};


class VarDecl : public Decl {
public:
    VarDecl(DeclExpr* decl_, bool is_public, bool inExpr);
    virtual ~VarDecl();
    virtual DeclType dtype() { return DECL_VAR; }
    virtual void acceptD(DeclVisitor& v);
    virtual void print(StringBuilder& buffer);
    virtual void generateC(StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);

    bool isPublic() const;
    bool isInExpr() const;
    virtual const std::string& getName() const;
    virtual clang::SourceLocation getLocation() const;
private:
    DeclExpr* decl;
    unsigned int flags;    // is_public and inExpr;
};


class TypeDecl : public Decl {
public:
    TypeDecl(const std::string& name_, SourceLocation loc_, Type* type_, bool is_public_);
    virtual ~TypeDecl();
    virtual DeclType dtype() { return DECL_TYPE; }
    virtual void acceptD(DeclVisitor& v);
    virtual void print(StringBuilder& buffer);
    virtual void generateC(StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);

    virtual const std::string& getName() const { return name; }
    virtual clang::SourceLocation getLocation() const { return loc; }
private:
    std::string name;
    SourceLocation loc;
    Type* type;
    bool is_public;
};


class DeclVisitor {
public:
    virtual ~DeclVisitor() {}
    virtual void visit(C2::Decl&) { assert(0); }    // add subclass below
    virtual void visit(FunctionDecl&) {}
    virtual void visit(VarDecl&) {}
    virtual void visit(TypeDecl&) {}
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

