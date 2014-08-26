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

#ifndef AST_EXPR_H
#define AST_EXPR_H

#include <string>
#include <vector>

#include <llvm/ADT/APSInt.h>
#include <llvm/ADT/APFloat.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/AST/OperationKinds.h>

#include "AST/OwningVector.h"
#include "AST/Stmt.h"
#include "AST/Type.h"

namespace C2 {

class StringBuilder;
class Decl;
class VarDecl;

enum ExprKind {
    EXPR_INTEGER_LITERAL=0,
    EXPR_FLOAT_LITERAL,
    EXPR_BOOL_LITERAL,
    EXPR_CHAR_LITERAL,
    EXPR_STRING_LITERAL,
    EXPR_NIL,
    EXPR_IDENTIFIER,
    EXPR_TYPE,
    EXPR_CALL,
    EXPR_INITLIST,
    EXPR_DESIGNATOR_INIT,
    EXPR_DECL,
    EXPR_BINOP,
    EXPR_CONDOP,
    EXPR_UNARYOP,
    EXPR_BUILTIN,
    EXPR_ARRAYSUBSCRIPT,
    EXPR_MEMBER,
    EXPR_PAREN
};

enum ExprCTC {
    CTC_NONE = 0,
    CTC_PARTIAL,
    CTC_FULL,
};


class Expr : public Stmt {
public:
    Expr(ExprKind k, bool isConstant_);
    virtual ~Expr();
    // from Stmt
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_EXPR;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;

    ExprKind getKind() const {
        return static_cast<ExprKind>(StmtBits.eKind);
    }
    ExprCTC getCTC() const {
        return static_cast<ExprCTC>(StmtBits.ExprIsCTC);
    }
    void setCTC(ExprCTC ctc) { StmtBits.ExprIsCTC = ctc; }

    bool isConstant() const { return StmtBits.ExprIsConstant; }
    void setConstant() { StmtBits.ExprIsConstant = true; }

    clang::SourceRange getSourceRange() const {
        return clang::SourceRange(getLocStart(), getLocEnd());
    }
    virtual SourceLocation getLocStart() const {
        return getLocation();
    }
    virtual SourceLocation getLocEnd() const {
        return getLocation();
    }
    QualType getType() const { return QT; }
    void setType(QualType t) { QT = t; }

    void setImpCast(BuiltinType::Kind k) { StmtBits.ExprImpCast = k; }
    bool hasImpCast() const {
        return getImpCast() != BuiltinType::Void;
    }
    BuiltinType::Kind getImpCast() const {
        return static_cast<BuiltinType::Kind>(StmtBits.ExprImpCast);
    }

    virtual void printLiteral(StringBuilder& buffer) const {};
private:
    QualType QT;

    Expr(const Expr&);
    Expr& operator= (const Expr&);
};


typedef std::vector<C2::Expr*> ExprList;

class IntegerLiteral : public Expr {
public:
    IntegerLiteral(SourceLocation loc_, const llvm::APInt& V)
        : Expr(EXPR_INTEGER_LITERAL, true)
        , Value(V), loc(loc_)
    {
        setCTC(CTC_FULL);
    }
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_INTEGER_LITERAL;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return loc; }

    virtual void printLiteral(StringBuilder& buffer) const;

    llvm::APInt Value;
private:
    clang::SourceLocation loc;
};


class FloatingLiteral : public Expr {
public:
    FloatingLiteral(SourceLocation loc_, const llvm::APFloat& V)
        : Expr(EXPR_FLOAT_LITERAL, true)
        , Value(V), loc(loc_)
    {
        setCTC(CTC_FULL);
    }
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_FLOAT_LITERAL;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return loc; }

    llvm::APFloat Value;
    clang::SourceLocation loc;
};


class BooleanLiteral : public Expr {
public:
    BooleanLiteral(SourceLocation loc_, bool val)
        : Expr(EXPR_BOOL_LITERAL, true)
        , loc(loc_)
    {
        StmtBits.BoolLiteralValue = val;
        setCTC(CTC_FULL);
    }
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_BOOL_LITERAL;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return loc; }
    bool getValue() const { return StmtBits.BoolLiteralValue; }

    clang::SourceLocation loc;
};


class CharacterLiteral : public Expr {
public:
    CharacterLiteral(SourceLocation loc_, unsigned val)
        : Expr(EXPR_CHAR_LITERAL, true)
        , value(val), loc(loc_)
    {
        setCTC(CTC_FULL);
    }
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_CHAR_LITERAL;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return loc; }

    unsigned getValue() const { return value; }
    virtual void printLiteral(StringBuilder& buffer) const;
private:
    // TODO use StmtBits (need to use union then)
    unsigned value;
    clang::SourceLocation loc;
};


class StringLiteral : public Expr {
public:
    StringLiteral(SourceLocation loc_, const std::string& val)
        : Expr(EXPR_STRING_LITERAL, true)
        , value(val), loc(loc_)
    {}
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_STRING_LITERAL;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return loc; }
    int getByteLength() const { return value.size(); }

    std::string value;
    clang::SourceLocation loc;
};


class NilExpr : public Expr {
public:
    NilExpr(SourceLocation loc_)
        : Expr(EXPR_NIL, true)
        , loc(loc_)
    {}
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_NIL;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return loc; }

    clang::SourceLocation loc;
};


// Represents a symbol reference (eg 'x' or 'counter')
class IdentifierExpr : public Expr {
public:
    IdentifierExpr(SourceLocation loc_, const std::string& name_)
        : Expr(EXPR_IDENTIFIER, false)
        , name(name_), loc(loc_), decl(0) {}
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_IDENTIFIER;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return loc; }

    const std::string& getName() const;
    void setDecl(Decl* decl_) {
        decl = decl_;
        name.clear();   // clear name to save memory
    }
    Decl* getDecl() const { return decl; }

    virtual void printLiteral(StringBuilder& buffer) const;
private:
    std::string name;
    clang::SourceLocation loc;
    Decl* decl;   // set during analysis
};


class TypeExpr : public Expr {
public:
    TypeExpr(QualType QT_)
        : Expr(EXPR_TYPE, true)
    {
        setType(QT_);
    }
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_TYPE;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const {
        SourceLocation loc;
        return loc;
    }
    void setLocalQualifier() { StmtBits.TypeExprIsLocal = true; }
    bool hasLocalQualifier() const { return StmtBits.TypeExprIsLocal; }
};


class CallExpr : public Expr {
public:
    CallExpr(Expr* Fn_, SourceLocation r)
        : Expr(EXPR_CALL, false)
        , R(r)
        , Fn(Fn_)
    {}
    virtual ~CallExpr();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_CALL;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return Fn->getLocation(); }
    virtual SourceLocation getLocStart() const {
        return Fn->getLocStart();
    }
    virtual SourceLocation getLocEnd() const {
        return R;
    }

    void addArg(Expr* arg);

    Expr* getFn() const { return Fn; }
    Expr* getArg(unsigned i) const { return args[i]; }
    unsigned numArgs() const { return args.size(); }
private:
    SourceLocation R;
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
    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return leftBrace; }
    virtual SourceLocation getLocStart() const { return leftBrace; }
    virtual SourceLocation getLocEnd() const { return rightBrace; }

    const ExprList& getValues() const { return values; }
    void setDesignators() { StmtBits.InitListHasDesignators = true; }
    bool hasDesignators() const { return StmtBits.InitListHasDesignators; }
private:
    SourceLocation leftBrace;
    SourceLocation rightBrace;
    ExprList values;
};


class DesignatedInitExpr : public Expr {
public:
    typedef enum {
        ARRAY_DESIGNATOR,
        FIELD_DESIGNATOR,
    } DesignatorKind;
    DesignatedInitExpr(SourceLocation left, Expr* d, Expr* i)
        : Expr(EXPR_DESIGNATOR_INIT, false)
        , SquareOrNameLoc(left)
        , initValue(i)
        , designator(d)
        , index(64, false)
        //, member(0)
    {
        index = llvm::APInt(64, -1, true);
        StmtBits.DesignatorKind = ARRAY_DESIGNATOR;
    }
    DesignatedInitExpr(SourceLocation left, const char* name, Expr* i)
        : Expr(EXPR_DESIGNATOR_INIT, false)
        , SquareOrNameLoc(left)
        , initValue(i)
        , designator(0)
        , index(64, false)
        , field(name)
        //, member(0)
    {
        StmtBits.DesignatorKind = FIELD_DESIGNATOR;
    }
    virtual ~DesignatedInitExpr();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_DESIGNATOR_INIT;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocStart() const { return SquareOrNameLoc; }
    virtual SourceLocation getLocEnd() const { return initValue->getLocEnd(); }
    virtual SourceLocation getLocation() const { return SquareOrNameLoc; }

    Expr* getInitValue() const { return initValue; }
    DesignatorKind getDesignatorKind() const {
        return static_cast<DesignatorKind>(StmtBits.DesignatorKind);
    }
    // for Array designator
    Expr* getDesignator() const { return designator; }
    llvm::APSInt getIndex() const { return index; }
    void setIndex(llvm::APSInt i) { index = i; }
    // for Field designator
    const std::string& getField() const { return field; }
private:
    SourceLocation SquareOrNameLoc;
    Expr* initValue;

    // Array designator
    Expr* designator;
    llvm::APSInt index; // set during analysis
    // Field designator
    const std::string field;
};


class DeclExpr : public Expr {
public:
    DeclExpr(VarDecl* decl_);
    virtual ~DeclExpr();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_DECL;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;

    const std::string& getName() const;
    virtual SourceLocation getLocation() const;
    QualType getDeclType() const;
    Expr* getInitValue() const;
    bool hasLocalQualifier() const;
    VarDecl* getDecl() const { return decl; }
private:
    VarDecl* decl;
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
    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return opLoc; }
    virtual SourceLocation getLocStart() const { return lhs->getLocStart();  }
    virtual SourceLocation getLocEnd() const { return rhs->getLocEnd(); }

    Expr* getLHS() const { return lhs; }
    Expr* getRHS() const { return rhs; }
    Opcode getOpcode() const { return opc; }

    virtual void printLiteral(StringBuilder& buffer) const;
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
    virtual void print(StringBuilder& buffer, unsigned indent) const;
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

    UnaryOperator(SourceLocation opLoc_, Opcode opc_, Expr* val_)
        : Expr(EXPR_UNARYOP, false)
        , opLoc(opLoc_)
        , opc(opc_)
        , val(val_)
    {}
    virtual ~UnaryOperator();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_UNARYOP;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return opLoc; }
    virtual SourceLocation getLocStart() const {
        switch (opc) {
        case clang::UO_PostInc:
        case clang::UO_PostDec:
            return val->getLocStart();
        default:
            return opLoc;
        }
    }
    virtual SourceLocation getLocEnd() const {
        switch (opc) {
        case clang::UO_PostInc:
        case clang::UO_PostDec:
            return opLoc;
        default:
            return val->getLocEnd();
        }
    }

    Expr* getExpr() const { return val; }
    Opcode getOpcode() const { return opc; }
    SourceLocation getOpLoc() const { return opLoc; }
private:
    SourceLocation opLoc;
    Opcode opc;
    Expr* val;
};


// BuiltinExpr's are sizeof() and elemsof() expressions
class BuiltinExpr : public Expr {
public:
    BuiltinExpr(SourceLocation Loc_, Expr* expr_, bool isSizeof_)
        : Expr(EXPR_BUILTIN, true)
        , Loc(Loc_)
        , expr(expr_)
    {
        StmtBits.BuiltInIsSizeOf = isSizeof_;
        setCTC(CTC_FULL);
    }
    virtual ~BuiltinExpr();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_BUILTIN;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return Loc; }

    Expr* getExpr() const { return expr; }
    bool isSizeof() const { return StmtBits.BuiltInIsSizeOf; }
private:
    SourceLocation Loc;
    Expr* expr;
};


// Represents an array expression like a[10] or array[a*3]
class ArraySubscriptExpr : public Expr {
public:
    ArraySubscriptExpr(SourceLocation RLoc_, Expr* Base_, Expr* Idx_)
        : Expr(EXPR_ARRAYSUBSCRIPT, false)
        , RLoc(RLoc_)
        , base(Base_)
        , idx(Idx_)
    {}
    virtual ~ArraySubscriptExpr();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_ARRAYSUBSCRIPT;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return base->getLocation(); }
    virtual SourceLocation getLocStart() const { return base->getLocStart(); }
    virtual SourceLocation getLocEnd() const { return RLoc; }

    Expr* getBase() const { return base; }
    Expr* getIndex() const { return idx; }
private:
    SourceLocation RLoc;
    Expr* base;
    Expr* idx;
};


// Represents a symbol reference 'a.b'/'a.b.c', A can be a module,struct or other Member expr
class MemberExpr : public Expr {
public:
    MemberExpr(Expr* Base_, bool isArrow_, const std::string& member_, SourceLocation loc_)
        : Expr(EXPR_MEMBER, false)
        , Base(Base_)
        , member(member_)
        , loc(loc_)
        , decl(0)
    {
        StmtBits.MemberExprIsArrow = isArrow_;
    }
    virtual ~MemberExpr();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_MEMBER;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return Base->getLocation(); }
    virtual SourceLocation getLocStart() const { return Base->getLocStart(); }
    virtual SourceLocation getLocEnd() const { return loc; }

    Expr* getBase() const { return Base; }
    const std::string& getMemberName() const { return member; }
    SourceLocation getMemberLoc() const { return loc; }
    Decl* getDecl() const { return decl; }
    void setDecl(Decl* D) { decl = D; }

    bool isArrow() const { return StmtBits.MemberExprIsArrow; }
    void setModulePrefix(bool v) { StmtBits.MemberExprIsModPrefix = v; }
    bool isModulePrefix() const { return StmtBits.MemberExprIsModPrefix; }

    // NOTE: uses static var
    virtual void printLiteral(StringBuilder& buffer) const;
private:
    Expr* Base;
    const std::string member;
    SourceLocation loc;
    Decl* decl;
};


class ParenExpr : public Expr {
public:
    ParenExpr(SourceLocation l, SourceLocation r, Expr* val)
        : Expr(EXPR_PAREN, false)
        , L(l), R(r), Val(val)
    {}
    virtual ~ParenExpr();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_PAREN;
    }
    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return L; }

    Expr* getExpr() const { return Val; }
    SourceLocation getLParen() const { return L; }
    SourceLocation getRParen() const { return R; }
    virtual SourceLocation getLocStart() const { return L; }
    virtual SourceLocation getLocEnd() const { return R; }
private:
    SourceLocation L, R;
    Expr* Val;
};


template <class T> static inline bool isa(const Expr* E) {
    return T::classof(E);
}

template <class T> static inline T* dyncast(Expr* E) {
    if (isa<T>(E)) return static_cast<T*>(E);
    return 0;
}

template <class T> static inline const T* dyncast(const Expr* E) {
    if (isa<T>(E)) return static_cast<const T*>(E);
    return 0;
}

//#define CAST_DEBUG
#ifdef CAST_DEBUG
#include <assert.h>
#endif

template <class T> static inline T* cast(Expr* E) {
#ifdef CAST_DEBUG
    assert(isa<T>(E));
#endif
    return static_cast<T*>(E);
}

template <class T> static inline const T* cast(const Expr* E) {
#ifdef CAST_DEBUG
    assert(isa<T>(E));
#endif
    return static_cast<const T*>(E);
}

}

#endif

