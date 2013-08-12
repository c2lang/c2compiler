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
#include <llvm/ADT/APSInt.h>
#include <llvm/ADT/APFloat.h>

#include "OwningVector.h"
#include "Stmt.h"
#include "Type.h"

namespace C2 {

class StringBuilder;
class ExprVisitor;
class Package;
class Decl;

enum ExprKind {
    EXPR_INTEGER_LITERAL=0,
    EXPR_STRING,
    EXPR_BOOL,
    EXPR_CHARLITERAL,
    EXPR_FLOAT_LITERAL,
    EXPR_CALL,
    EXPR_IDENTIFIER,
    EXPR_INITLIST,
    EXPR_TYPE,
    EXPR_DECL,
    EXPR_BINOP,
    EXPR_CONDOP,
    EXPR_UNARYOP,
    EXPR_BUILTIN,
    EXPR_ARRAYSUBSCRIPT,
    EXPR_MEMBER,
    EXPR_PAREN,
};


class Expr : public Stmt {
public:
    Expr(ExprKind k);
    virtual ~Expr();
    // from Stmt
    virtual StmtType stype() const { return STMT_EXPR; }
    virtual void acceptS(StmtVisitor& v) const;

    ExprKind getKind() const { return kind; }

    virtual clang::SourceRange getSourceRange() {
        return clang::SourceRange();
    }

    void setStatementFlag() { isStatement = true; }
    bool isStmt() const { return isStatement; }
private:
    bool isStatement;
    ExprKind kind;

    Expr(const Expr&);
    Expr& operator= (const Expr&);
};


typedef std::vector<C2::Expr*> ExprList;

class IntegerLiteral : public Expr {
public:
    IntegerLiteral(SourceLocation loc_, const llvm::APInt& V)
        : Expr(EXPR_INTEGER_LITERAL)
        , Value(V), loc(loc_) {}
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_INTEGER_LITERAL;
    }
    virtual void print(int indent, StringBuilder& buffer) const;
    virtual SourceLocation getLocation() const { return loc; }

    llvm::APInt Value;
    clang::SourceLocation loc;
};


class FloatingLiteral : public Expr {
public:
    FloatingLiteral(SourceLocation loc_, const llvm::APFloat& V)
        : Expr(EXPR_FLOAT_LITERAL)
        , Value(V), loc(loc_) {}
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_FLOAT_LITERAL;
    }
    virtual void print(int indent, StringBuilder& buffer) const;
    virtual SourceLocation getLocation() const { return loc; }

    llvm::APFloat Value;
    clang::SourceLocation loc;
};


class StringExpr : public Expr {
public:
    StringExpr(SourceLocation loc_, const std::string& val)
        : Expr(EXPR_STRING)
        , value(val), loc(loc_) {}
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_STRING;
    }
    virtual void print(int indent, StringBuilder& buffer) const;
    virtual SourceLocation getLocation() const { return loc; }

    std::string value;
    clang::SourceLocation loc;
};


class BoolLiteralExpr : public Expr {
public:
    BoolLiteralExpr(SourceLocation loc_, bool val)
        : Expr(EXPR_BOOL)
        , value(val), loc(loc_) {}
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_BOOL;
    }
    virtual void print(int indent, StringBuilder& buffer) const;
    virtual SourceLocation getLocation() const { return loc; }

    bool value;
    clang::SourceLocation loc;
};


class CharLiteralExpr : public Expr {
public:
    CharLiteralExpr(SourceLocation loc_, unsigned val)
        : Expr(EXPR_CHARLITERAL)
        , value(val), loc(loc_) {}
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_CHARLITERAL;
    }
    virtual void print(int indent, StringBuilder& buffer) const;
    virtual SourceLocation getLocation() const { return loc; }

    unsigned value;
    clang::SourceLocation loc;
};


class IdentifierExpr : public Expr {
public:
    IdentifierExpr(SourceLocation loc_, const std::string& name_)
        : Expr(EXPR_IDENTIFIER)
        , name(name_), pkg(0), decl(0), loc(loc_) {}
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_IDENTIFIER;
    }
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
    TypeExpr(QualType& QT_)
        : Expr(EXPR_TYPE)
        , QT(QT_)
        , isLocal(false) {}
    virtual ~TypeExpr();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_TYPE;
    }
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
        : Expr(EXPR_CALL)
        , Fn(Fn_)
    {}
    virtual ~CallExpr();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_CALL;
    }
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
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_INITLIST;
    }
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
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_DECL;
    }
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
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_BINOP;
    }
    virtual void print(int indent, StringBuilder& buffer) const;
    virtual SourceLocation getLocation() const { return opLoc; }

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
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_CONDOP;
    }
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
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_UNARYOP;
    }
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


class BuiltinExpr : public Expr {
public:
    BuiltinExpr(SourceLocation Loc_, Expr* expr_, bool isSizeof_)
        : Expr(EXPR_BUILTIN)
        , Loc(Loc_)
        , expr(expr_)
        , isSizeof(isSizeof_)
    {}
    virtual ~BuiltinExpr();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_BUILTIN;
    }
    virtual void print(int indent, StringBuilder& buffer) const;
    virtual SourceLocation getLocation() const { return Loc; }

    Expr* getExpr() const { return expr; }
    bool isSizeFunc() const { return isSizeof; }
private:
    SourceLocation Loc;
    Expr* expr;
    bool isSizeof;
};


class ArraySubscriptExpr : public Expr {
public:
    ArraySubscriptExpr(SourceLocation RLoc_, Expr* Base_, Expr* Idx_);
    virtual ~ArraySubscriptExpr();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_ARRAYSUBSCRIPT;
    }
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
        : Expr(EXPR_MEMBER)
        , Base(Base_)
        , Member(Member_)
        , isArrow(isArrow_)
    {}
    virtual ~MemberExpr();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_MEMBER;
    }
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
        : Expr(EXPR_PAREN)
        , L(l), R(r), Val(val)
    {}
    virtual ~ParenExpr();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_PAREN;
    }
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


template <class T> static inline bool isa(const Expr* E) {
    return T::classof(E);
}

template <class T> static inline T* cast(Expr* E) {
    if (isa<T>(E)) return static_cast<T*>(E);
    return 0;
}

template <class T> static inline const T* cast(const Expr* E) {
    if (isa<T>(E)) return static_cast<const T*>(E);
    return 0;
}

}

#endif

