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
#include "Type.h"

namespace C2 {

class StringBuilder;
class ExprVisitor;
class Package;
class Decl;

enum ExprType {
    EXPR_NUMBER=0,
    EXPR_STRING,
    EXPR_BOOL,
    EXPR_CHARLITERAL,
    EXPR_CALL,
    EXPR_IDENTIFIER,
    EXPR_INITLIST,
    EXPR_TYPE,
    EXPR_DECL,
    EXPR_BINOP,
    EXPR_CONDOP,
    EXPR_UNARYOP,
    EXPR_SIZEOF,
    EXPR_ARRAYSUBSCRIPT,
    EXPR_MEMBER,
    EXPR_PAREN,
};


class Expr : public Stmt {
public:
    Expr();
    virtual ~Expr();
    // from Stmt
    virtual StmtType stype() const { return STMT_EXPR; }
    virtual void acceptS(StmtVisitor& v) const;

    virtual ExprType etype() const = 0;
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
    virtual ExprType etype() const { return EXPR_NUMBER; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer) const;
    virtual SourceLocation getLocation() const { return loc; }

    double value;
    clang::SourceLocation loc;
};


class StringExpr : public Expr {
public:
    StringExpr(SourceLocation loc_, const std::string& val)
        : value(val), loc(loc_) {}
    virtual ExprType etype() const { return EXPR_STRING; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer) const;
    virtual SourceLocation getLocation() const { return loc; }

    std::string value;
    clang::SourceLocation loc;
};


class BoolLiteralExpr : public Expr {
public:
    BoolLiteralExpr(SourceLocation loc_, bool val)
        : value(val), loc(loc_) {}
    virtual ExprType etype() const { return EXPR_BOOL; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer) const;
    virtual SourceLocation getLocation() const { return loc; }

    bool value;
    clang::SourceLocation loc;
};


class CharLiteralExpr : public Expr {
public:
    CharLiteralExpr(SourceLocation loc_, unsigned val)
        : value(val), loc(loc_) {}
    virtual ExprType etype() const { return EXPR_CHARLITERAL; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer) const;
    virtual SourceLocation getLocation() const { return loc; }

    unsigned value;
    clang::SourceLocation loc;
};


class IdentifierExpr : public Expr {
public:
    IdentifierExpr(SourceLocation loc_, const std::string& name_)
        : name(name_), pkg(0), decl(0), loc(loc_) {}
    virtual ExprType etype() const { return EXPR_IDENTIFIER; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer) const;
    virtual SourceLocation getLocation() const { return loc; }

    const std::string& getName() const { return name; }
    void setPackage(const Package* pkg_) { pkg = pkg_; }
    const Package* getPackage() const { return pkg; }
    void setDecl(Decl* decl_) { decl = decl_; }
    Decl* getDecl() const { return decl; }
private:
    std::string name;
    const Package* pkg; // set during analysis
    Decl* decl;   // set during analysis
    clang::SourceLocation loc;
};


class TypeExpr : public Expr {
public:
    TypeExpr(QualType& QT_) : QT(QT_), isLocal(false) {}
    virtual ~TypeExpr();
    virtual ExprType etype() const { return EXPR_TYPE; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer) const;
    virtual SourceLocation getLocation() const {
        SourceLocation loc;
        return loc;
    }
    QualType& getType() { return QT; }
    void setType(QualType& QT_) { QT = QT_; }

    void setLocalQualifier() { isLocal = true; }
    bool hasLocalQualifier() const { return isLocal; }
private:
    QualType QT;
    bool isLocal;
};


class CallExpr : public Expr {
public:
    CallExpr(Expr* Fn_)
        : Fn(Fn_)
    {}
    virtual ~CallExpr();
    virtual ExprType etype() const { return EXPR_CALL; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer) const;
    virtual SourceLocation getLocation() const { return Fn->getLocation(); }

    void addArg(Expr* arg);

    Expr* getFn() const { return Fn; }
    Expr* getArg(unsigned int i) const { return args[i]; }
    unsigned int numArgs() const { return args.size(); }
private:
    // TODO add R/LParen
    Expr* Fn;
    typedef OwningVector<Expr> Args;
    Args args;
};


class InitListExpr : public Expr {
public:
    InitListExpr(SourceLocation lbraceLoc, SourceLocation rbraceLoc, ExprList& values_);
    virtual ~InitListExpr();
    virtual ExprType etype() const { return EXPR_INITLIST; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer) const;
    virtual SourceLocation getLocation() const { return leftBrace; }

    ExprList& getValues() { return values; }
private:
    SourceLocation leftBrace;
    SourceLocation rightBrace;
    ExprList values;
};


class DeclExpr : public Expr {
public:
    DeclExpr(const std::string& name_, SourceLocation& loc_,
            QualType type_, Expr* initValue_);
    virtual ~DeclExpr();
    virtual ExprType etype() const { return EXPR_DECL; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer) const;
    // used by VarDecls only to add pkgName
    virtual SourceLocation getLocation() const { return loc; }

    QualType getType() const { return type; }
    const std::string& getName() const { return name; }
    Expr* getInitValue() const { return initValue; }

    void setLocalQualifier() { localQualifier = true; }
    bool hasLocalQualifier() const { return localQualifier; }
private:
    std::string name;
    SourceLocation loc;
    QualType type;
    Type* canonicalType;
    Expr* initValue;
    bool localQualifier;
};


class BinaryOperator : public Expr {
public:
    typedef clang::BinaryOperatorKind Opcode;
    static const char* OpCode2str(clang::BinaryOperatorKind opc);

    BinaryOperator(Expr* lhs, Expr* rhs, Opcode opc, SourceLocation opLoc);
    virtual ~BinaryOperator();
    virtual ExprType etype() const { return EXPR_BINOP; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer) const;
    virtual SourceLocation getLocation() const { return lhs->getLocation(); }

    Expr* getLHS() const { return lhs; }
    Expr* getRHS() const { return rhs; }
    Opcode getOpcode() const { return opc; }
private:
    SourceLocation opLoc;
    Opcode opc;
    Expr* lhs;
    Expr* rhs;
};


class ConditionalOperator : public Expr {
public:
    ConditionalOperator(SourceLocation questionLoc, SourceLocation colonLoc,
                    Expr* cond_, Expr* lhs_, Expr* rhs_);
    virtual ~ConditionalOperator();
    virtual ExprType etype() const { return EXPR_CONDOP; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer) const;
    virtual SourceLocation getLocation() const { return cond->getLocation(); }

    Expr* getCond() const { return cond; }
    Expr* getLHS() const { return lhs; }
    Expr* getRHS() const { return rhs; }
private:
    SourceLocation QuestionLoc;
    SourceLocation ColonLoc;
    Expr* cond;
    Expr* lhs;
    Expr* rhs;
};


class UnaryOperator : public Expr {
public:
    typedef clang::UnaryOperatorKind Opcode;
    static const char* OpCode2str(clang::UnaryOperatorKind opc);

    UnaryOperator(SourceLocation opLoc_, Opcode opc, Expr* val_);
    virtual ~UnaryOperator();
    virtual ExprType etype() const { return EXPR_UNARYOP; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer) const;
    virtual SourceLocation getLocation() const { return opLoc; }

    Expr* getExpr() const { return val; }
    Opcode getOpcode() const { return opc; }
    SourceLocation getOpLoc() const { return opLoc; }
private:
    SourceLocation opLoc;
    Opcode opc;
    Expr* val;
};


class SizeofExpr : public Expr {
public:
    SizeofExpr(SourceLocation Loc, Expr* expr_);
    virtual ~SizeofExpr();
    virtual ExprType etype() const { return EXPR_SIZEOF; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer) const;
    virtual SourceLocation getLocation() const { return Loc; }

    Expr* getExpr() const { return expr; }
private:
    SourceLocation Loc;
    Expr* expr;
};


class ArraySubscriptExpr : public Expr {
public:
    ArraySubscriptExpr(SourceLocation RLoc_, Expr* Base_, Expr* Idx_);
    virtual ~ArraySubscriptExpr();
    virtual ExprType etype() const { return EXPR_ARRAYSUBSCRIPT; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer) const;
    virtual SourceLocation getLocation() const { return base->getLocation(); }

    Expr* getBase() const { return base; }
    Expr* getIndex() const { return idx; }
private:
    SourceLocation RLoc;
    Expr* base;
    Expr* idx;
};


class MemberExpr : public Expr {
public:
    MemberExpr(Expr* Base_, bool isArrow_, IdentifierExpr* Member_)
        : Base(Base_)
        , Member(Member_)
        , isArrow(isArrow_)
    {}
    virtual ~MemberExpr();
    virtual ExprType etype() const { return EXPR_MEMBER; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer) const;
    virtual SourceLocation getLocation() const { return Base->getLocation(); }

    Expr* getBase() const { return Base; }
    IdentifierExpr* getMember() const { return Member; }
    bool isArrowOp() const { return isArrow; }
    // NOTE: uses static var
    const char* getFullName() const;
private:
    Expr* Base;
    IdentifierExpr* Member;
    bool isArrow;
};


class ParenExpr : public Expr {
public:
    ParenExpr(SourceLocation l, SourceLocation r, Expr* val)
        : L(l), R(r), Val(val)
    {}
    virtual ~ParenExpr();
    virtual ExprType etype() const { return EXPR_PAREN; }
    virtual void acceptE(ExprVisitor& v);
    virtual void print(int indent, StringBuilder& buffer) const;
    virtual SourceLocation getLocation() const { return L; }

    Expr* getExpr() const { return Val; }
    clang::SourceRange getSourceRange() const { return clang::SourceRange(L, R); }
    SourceLocation getLParen() const { return L; }
    SourceLocation getRParen() const { return R; }
private:
    SourceLocation L, R;
    Expr* Val;
};


class ExprVisitor {
public:
    virtual ~ExprVisitor() {}
    virtual void visit(Expr&) { assert(0 && "unknown Expr type"); }    // add ExprClass below
    virtual void visit(NumberExpr&) {}
    virtual void visit(StringExpr&) {}
    virtual void visit(BoolLiteralExpr&) {}
    virtual void visit(CharLiteralExpr&) {}
    virtual void visit(CallExpr&) {}
    virtual void visit(IdentifierExpr&) {}
    virtual void visit(InitListExpr&) {}
    virtual void visit(TypeExpr&) {}
    virtual void visit(DeclExpr&) {}
    virtual void visit(BinaryOperator&) {}
    virtual void visit(ConditionalOperator&) {}
    virtual void visit(UnaryOperator&) {}
    virtual void visit(SizeofExpr&) {}
    virtual void visit(ArraySubscriptExpr&) {}
    virtual void visit(MemberExpr&) {}
    virtual void visit(ParenExpr&) {}
};

#define EXPR_VISITOR_ACCEPT(a) void a::acceptE(ExprVisitor& v) { v.visit(*this); }

template <class T> class ExprCaster : public ExprVisitor {
public:
    virtual void visit(T& node_) {
        node = &node_;
    }
    static T* getType(const Expr& node_) {
        // TEMP dirty temp cast
        ExprCaster<T> visitor((Expr&)node_);
        return visitor.node;
    }
    static T* getType(const Expr* node_) {
        // TEMP dirty temp cast
        ExprCaster<T> visitor((Expr&)*node_);
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

