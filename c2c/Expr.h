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

#ifndef EXPR_H
#define EXPR_H

#include <string>
#include <vector>

#include <clang/Basic/SourceLocation.h>
#include <clang/AST/OperationKinds.h>

#include "OwningVector.h"
#include "Stmt.h"

namespace C2 {

class StringBuilder;
class ExprVisitor;
class Type;

enum ExprType {
    EXPR_NUMBER=0,
    EXPR_STRING,
    EXPR_CHARLITERAL,
    EXPR_CALL,
    EXPR_IDENTIFIER,
    EXPR_INITLIST,
    EXPR_TYPE,
    EXPR_DECL,
    EXPR_BINOP,
    EXPR_UNARYOP,
    EXPR_SIZEOF,
    EXPR_ARRAYSUBSCRIPT,
    EXPR_MEMBER,
};


class Expr : public Stmt {
public:
    Expr();
    virtual ~Expr();
    // from Stmt
    virtual StmtType stype() { return STMT_EXPR; }
    virtual void acceptS(StmtVisitor& v);

    virtual ExprType ntype() = 0;
    virtual void acceptE(ExprVisitor& v) = 0;

    virtual clang::SourceRange getSourceRange() {
        return clang::SourceRange();
    }
    void setStatementFlag() { isStatement = true; }
    bool isStmt() const { return isStatement; }
private:
    bool isStatement;
    Expr(const Expr&);
    Expr& operator= (const Expr&);
};


typedef std::vector<C2::Expr*> ExprList;

class NumberExpr : public Expr {
public:
    NumberExpr(SourceLocation loc_, double val)
        : value(val), loc(loc_) {}
    virtual ExprType ntype() { return EXPR_NUMBER; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);

    double value;
    clang::SourceLocation loc;
};


class StringExpr : public Expr {
public:
    StringExpr(SourceLocation loc_, const std::string& val)
        : value(val), loc(loc_) {}
    virtual ExprType ntype() { return EXPR_STRING; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);

    std::string value;
    clang::SourceLocation loc;
};


class CharLiteralExpr : public Expr {
public:
    CharLiteralExpr(SourceLocation loc_, unsigned val)
        : value(val), loc(loc_) {}
    virtual ExprType ntype() { return EXPR_CHARLITERAL; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);

    unsigned value;
    clang::SourceLocation loc;
};


class IdentifierExpr : public Expr {
public:
    IdentifierExpr(SourceLocation ploc_, const std::string& pname_,
                   SourceLocation loc_, const std::string& name_)
        : pname(pname_), ploc(ploc_), name(name_), loc(loc_) {}
    virtual ExprType ntype() { return EXPR_IDENTIFIER; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);

    // NOTE uses static var
    const char* getName() const;
    SourceLocation getLocation() const { return loc; }

    std::string pname;
    clang::SourceLocation ploc;
    std::string name;
    clang::SourceLocation loc;
};


class TypeExpr : public Expr {
public:
    TypeExpr(Type* type_) : type(type_) {}
    virtual ~TypeExpr();
    virtual ExprType ntype() { return EXPR_TYPE; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);

    void addArray(Expr* sizeExpr);
    void addPointer();
    void addQualifier(unsigned int qualifier);
    Type* takeType() {
        Type* tmp = type;
        type = 0;
        return tmp;
    }
private:
    Type* type;
};


class CallExpr : public Expr {
public:
    CallExpr(IdentifierExpr* Fn_)
        : Fn(Fn_)
    {}
    virtual ~CallExpr();
    virtual ExprType ntype() { return EXPR_CALL; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);

    void addArg(Expr* arg);

    IdentifierExpr* getId() const { return Fn; }
    Expr* getArg(unsigned int i) const { return args[i]; }
    unsigned int numArgs() const { return args.size(); }
private:
    // TODO add R/LParen
    IdentifierExpr* Fn;
    typedef OwningVector<Expr> Args;
    Args args;
};


class InitListExpr : public Expr {
public:
    InitListExpr(SourceLocation lbraceLoc, SourceLocation rbraceLoc, ExprList& values_);
    virtual ~InitListExpr();
    virtual ExprType ntype() { return EXPR_INITLIST; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);
private:
    SourceLocation leftBrace;
    SourceLocation rightBrace;
    ExprList values;
};


class DeclExpr : public Expr {
public:
    DeclExpr(const std::string& name_, SourceLocation& loc_,
            Type* type_, Expr* initValue_);
    virtual ~DeclExpr();
    virtual ExprType ntype() { return EXPR_DECL; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    // used by VarDecls only to add pkgName
    virtual void generateC(StringBuilder& buffer, const std::string& pkgName);
    virtual llvm::Value* codeGen(CodeGenContext& context);

    Type* getType() const { return type; }
    const std::string& getName() const { return name; }
    SourceLocation getLocation() const { return loc; }
private:
    std::string name;
    SourceLocation loc;
    Type* type;
    Expr* initValue;
};


class BinOpExpr : public Expr {
public:
    typedef clang::BinaryOperatorKind Opcode;

    BinOpExpr(Expr* lhs, Expr* rhs, Opcode opc, SourceLocation opLoc);
    virtual ~BinOpExpr();
    virtual ExprType ntype() { return EXPR_BINOP; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);

    Expr* getLeft() const { return lhs; }
    Expr* getRight() const { return rhs; }
private:
    SourceLocation opLoc;
    Opcode opc;
    Expr* lhs;
    Expr* rhs;
};


class UnaryOpExpr : public Expr {
public:
    typedef clang::UnaryOperatorKind Opcode;

    UnaryOpExpr(SourceLocation opLoc_, Opcode opc, Expr* val_);
    virtual ~UnaryOpExpr();
    virtual ExprType ntype() { return EXPR_UNARYOP; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);

    Expr* getExpr() const { return val; }
private:
    SourceLocation opLoc;
    Opcode opc;
    Expr* val;
};


class SizeofExpr : public Expr {
public:
    SizeofExpr(SourceLocation Loc, Expr* expr_);
    virtual ~SizeofExpr();
    virtual ExprType ntype() { return EXPR_SIZEOF; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);

    Expr* getExpr() const { return expr; }
private:
    SourceLocation Loc;
    Expr* expr;
};


class ArraySubscriptExpr : public Expr {
public:
    ArraySubscriptExpr(SourceLocation RLoc_, Expr* Base_, Expr* Idx_);
    virtual ~ArraySubscriptExpr();
    virtual ExprType ntype() { return EXPR_ARRAYSUBSCRIPT; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);

    Expr* getBase() const { return base; }
    Expr* getIndex() const { return idx; }
private:
    SourceLocation RLoc;
    Expr* base;
    Expr* idx;
};


class MemberExpr : public Expr {
public:
    MemberExpr(Expr* Base_, bool isArrow_, Expr* Member_)
        : Base(Base_)
        , Member(Member_)
        , isArrow(isArrow_)
    {}
    virtual ~MemberExpr();
    virtual ExprType ntype() { return EXPR_MEMBER; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);
private:
    Expr* Base;
    Expr* Member;
    bool isArrow;
};


class ExprVisitor {
public:
    virtual ~ExprVisitor() {}
    virtual void visit(Expr&) { assert(0 && "unknown Expr type"); }    // add ExprClass below
    virtual void visit(NumberExpr&) {}
    virtual void visit(StringExpr&) {}
    virtual void visit(CharLiteralExpr&) {}
    virtual void visit(CallExpr&) {}
    virtual void visit(IdentifierExpr&) {}
    virtual void visit(InitListExpr&) {}
    virtual void visit(TypeExpr&) {}
    virtual void visit(DeclExpr&) {}
    virtual void visit(BinOpExpr&) {}
    virtual void visit(UnaryOpExpr&) {}
    virtual void visit(SizeofExpr&) {}
    virtual void visit(ArraySubscriptExpr&) {}
    virtual void visit(MemberExpr&) {}
};

#define EXPR_VISITOR_ACCEPT(a) void a::acceptE(ExprVisitor& v) { v.visit(*this); }

template <class T> class ExprCaster : public ExprVisitor {
public:
    virtual void visit(T& node_) {
        node = &node_;
    }
    static T* getType(Expr& node_) {
        ExprCaster<T> visitor(node_);
        return visitor.node;
    }
    static T* getType(Expr* node_) {
        ExprCaster<T> visitor(*node_);
        return visitor.node;
    }
private:
    ExprCaster(Expr& n) : node(0) {
        n.acceptE(*this);
    }
    T* node;
};

}

#endif

