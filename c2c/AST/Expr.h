/* Copyright 2013-2016 Bas van den Berg
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
    EXPR_BINOP,
    EXPR_CONDOP,
    EXPR_UNARYOP,
    EXPR_BUILTIN,
    EXPR_ARRAYSUBSCRIPT,
    EXPR_MEMBER,
    EXPR_PAREN,
    EXPR_BITOFFSET,
    EXPR_CAST
};

enum ExprCTC {
    CTC_NONE = 0,
    CTC_PARTIAL,
    CTC_FULL,
};


class Expr : public Stmt {
public:
    Expr(ExprKind k, bool isConstant_);
    ~Expr();
    // from Stmt
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_EXPR;
    }
    void print(StringBuilder& buffer) const;
    void print(StringBuilder& buffer, unsigned indent) const;
    void printLiteral(StringBuilder& buffer) const;

    ExprKind getKind() const {
        return static_cast<ExprKind>(exprBits.eKind);
    }
    ExprCTC getCTC() const {
        return static_cast<ExprCTC>(exprBits.IsCTC);
    }
    void setCTC(ExprCTC ctc) { exprBits.IsCTC = ctc; }

    bool isConstant() const { return exprBits.IsConstant; }
    void setConstant() { exprBits.IsConstant = true; }

    clang::SourceRange getSourceRange() const {
        return clang::SourceRange(getLocStart(), getLocEnd());
    }
    SourceLocation getLocation() const;
    SourceLocation getLocStart() const;
    SourceLocation getLocEnd() const;
    QualType getType() const { return QT; }
    void setType(QualType t) { QT = t; }

    void setImpCast(BuiltinType::Kind k) { exprBits.ImpCast = k; }
    bool hasImpCast() const {
        return getImpCast() != BuiltinType::Void;
    }
    BuiltinType::Kind getImpCast() const {
        return static_cast<BuiltinType::Kind>(exprBits.ImpCast);
    }

private:
    QualType QT;

    Expr(const Expr&);
    Expr& operator= (const Expr&);
};


typedef std::vector<C2::Expr*> ExprList;

class IntegerLiteral : public Expr {
private:
    enum Radix {
        RADIX_2 = 0,
        RADIX_8,
        RADIX_10,
        RADIX_16
    };
public:
    IntegerLiteral(SourceLocation loc_, const llvm::APInt& V, unsigned radix = 10)
        : Expr(EXPR_INTEGER_LITERAL, true)
        , Value(V), loc(loc_)
    {

        setCTC(CTC_FULL);
        Radix r = RADIX_10;
        switch (radix) {
        case 2:
            r = RADIX_2;
            break;
        case 8:
            r = RADIX_8;
            break;
        case 10:
            r = RADIX_10;
            break;
        case 16:
            r = RADIX_16;
            break;
        default:
            assert(0 && "unsupported radix");
            break;
        }
        integerLiteralBits.Radix = r;
    }
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_INTEGER_LITERAL;
    }
    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return loc; }

    void printLiteral(StringBuilder& buffer) const;
    unsigned getRadix() const {
        Radix r = static_cast<IntegerLiteral::Radix>(integerLiteralBits.Radix);
        switch (r) {
        case RADIX_2:  return 2;
        case RADIX_8:  return 8;
        case RADIX_10: return 10;
        case RADIX_16: return 16;
        }
    }

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
    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return loc; }

    llvm::APFloat Value;
    clang::SourceLocation loc;
};


class BooleanLiteral : public Expr {
public:
    BooleanLiteral(SourceLocation loc_, bool val)
        : Expr(EXPR_BOOL_LITERAL, true)
        , loc(loc_)
    {
        booleanLiteralBits.Value = val;
        setCTC(CTC_FULL);
    }
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_BOOL_LITERAL;
    }
    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return loc; }
    bool getValue() const { return booleanLiteralBits.Value; }

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
    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return loc; }

    unsigned getValue() const { return value; }
    void printLiteral(StringBuilder& buffer) const;
private:
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
    void print(StringBuilder& buffer, unsigned indent) const;
    void printLiteral(StringBuilder& buffer) const;
    SourceLocation getLocation() const { return loc; }
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
    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return loc; }

    clang::SourceLocation loc;
};


// Represents a symbol reference (eg 'x' or 'counter')
class IdentifierExpr : public Expr {
public:
    IdentifierExpr(SourceLocation loc_, const std::string& name_)
        : Expr(EXPR_IDENTIFIER, false)
        , name(name_), loc(loc_), decl(0)
    {
        identifierExprBits.IsType = 0;
        identifierExprBits.IsStructFunction = 0;
    }
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_IDENTIFIER;
    }
    bool isType() const { return identifierExprBits.IsType; }
    void setIsType() { identifierExprBits.IsType = true; }

    void setIsStructFunction() { identifierExprBits.IsStructFunction = true; }
    bool isStructFunction() const { return identifierExprBits.IsStructFunction; }

    void print(StringBuilder& buffer, unsigned indent) const;
    void printLiteral(StringBuilder& buffer) const;
    SourceLocation getLocation() const { return loc; }

    const std::string& getName() const;
    void setDecl(Decl* decl_) {
        decl = decl_;
        name.clear();   // clear name to save memory
    }
    Decl* getDecl() const { return decl; }

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
        typeExprBits.IsLocal = 0;
        setType(QT_);
    }
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_TYPE;
    }
    void print(StringBuilder& buffer, unsigned indent) const;
    void setLocalQualifier() { typeExprBits.IsLocal = true; }
    bool hasLocalQualifier() const { return typeExprBits.IsLocal; }
};


class CallExpr : public Expr {
public:
    CallExpr(Expr* Fn_, SourceLocation rparenLoc_)
        : Expr(EXPR_CALL, false)
        , rparenLoc(rparenLoc_)
        , Fn(Fn_)
    {
        callExprBits.IsStructFunc = 0;
    }
    ~CallExpr();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_CALL;
    }
    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return Fn->getLocation(); }
    SourceLocation getLocStart() const { return Fn->getLocStart(); }
    SourceLocation getLocEnd() const { return rparenLoc; }

    void addArg(Expr* arg);

    Expr* getFn() const { return Fn; }
    Expr* getArg(unsigned i) const { return args[i]; }
    unsigned numArgs() const { return args.size(); }

    void setIsStructFunction() { callExprBits.IsStructFunc = true; }
    bool isStructFunction() const { return callExprBits.IsStructFunc; }
private:
    SourceLocation rparenLoc;
    Expr* Fn;
    typedef OwningVector<Expr> Args;
    Args args;
};


class InitListExpr : public Expr {
public:
    InitListExpr(SourceLocation lbraceLoc, SourceLocation rbraceLoc, ExprList& values_);
    ~InitListExpr();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_INITLIST;
    }
    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return leftBrace; }
    SourceLocation getLocStart() const { return leftBrace; }
    SourceLocation getLocEnd() const { return rightBrace; }

    const ExprList& getValues() const { return values; }
    void setDesignators() { initListExprBits.HasDesignators = true; }
    bool hasDesignators() const { return initListExprBits.HasDesignators; }

    // for incremental arrays
    void addExpr(Expr* E) { values.push_back(E); }
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
        designatedInitExprBits.DesignatorKind = ARRAY_DESIGNATOR;
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
        designatedInitExprBits.DesignatorKind = FIELD_DESIGNATOR;
    }
    ~DesignatedInitExpr();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_DESIGNATOR_INIT;
    }
    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return SquareOrNameLoc; }
    SourceLocation getLocStart() const { return SquareOrNameLoc; }
    SourceLocation getLocEnd() const { return initValue->getLocEnd(); }

    Expr* getInitValue() const { return initValue; }
    DesignatorKind getDesignatorKind() const {
        return static_cast<DesignatorKind>(designatedInitExprBits.DesignatorKind);
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


class BinaryOperator : public Expr {
public:
    typedef clang::BinaryOperatorKind Opcode;
    static const char* OpCode2str(clang::BinaryOperatorKind opc);

    BinaryOperator(Expr* lhs, Expr* rhs, Opcode opc, SourceLocation opLoc);
    ~BinaryOperator();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_BINOP;
    }
    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return opLoc; }
    SourceLocation getLocStart() const { return lhs->getLocStart(); }
    SourceLocation getLocEnd() const { return rhs->getLocEnd(); }

    Expr* getLHS() const { return lhs; }
    Expr* getRHS() const { return rhs; }
    Opcode getOpcode() const { return opc; }

    void printLiteral(StringBuilder& buffer) const;
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
    ~ConditionalOperator();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_CONDOP;
    }
    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return cond->getLocation(); }

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
    ~UnaryOperator();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_UNARYOP;
    }
    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return opLoc; }
    SourceLocation getLocStart() const {
        switch (opc) {
        case clang::UO_PostInc:
        case clang::UO_PostDec:
            return val->getLocStart();
        default:
            return opLoc;
        }
    }
    SourceLocation getLocEnd() const {
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
        builtinExprBits.IsSizeOf = isSizeof_;
        setCTC(CTC_FULL);
    }
    ~BuiltinExpr();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_BUILTIN;
    }
    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return Loc; }

    Expr* getExpr() const { return expr; }
    bool isSizeof() const { return builtinExprBits.IsSizeOf; }
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
    ~ArraySubscriptExpr();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_ARRAYSUBSCRIPT;
    }
    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return base->getLocation(); }
    SourceLocation getLocStart() const { return base->getLocStart(); }
    SourceLocation getLocEnd() const { return RLoc; }

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
    MemberExpr(Expr* Base_, IdentifierExpr* member_)
        : Expr(EXPR_MEMBER, false)
        , Base(Base_)
        , member(member_)
        , decl(0)
    {
        memberExprBits.IsModPrefix = 0;
        memberExprBits.IsStructFunction = 0;
        memberExprBits.IsStaticStructFunction = 0;
    }
    ~MemberExpr();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_MEMBER;
    }
    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return Base->getLocation(); }
    SourceLocation getLocStart() const { return Base->getLocStart(); }
    SourceLocation getLocEnd() const { return member->getLocEnd(); }

    Expr* getBase() const { return Base; }
    IdentifierExpr* getMember() const { return member; }

    Decl* getDecl() const { return decl; }
    void setDecl(Decl* D) { decl = D; }

    void setModulePrefix() { memberExprBits.IsModPrefix = true; }
    bool isModulePrefix() const { return memberExprBits.IsModPrefix; }
    void setIsStructFunction() { memberExprBits.IsStructFunction = true; }
    bool isStructFunction() const { return memberExprBits.IsStructFunction; }
    void setIsStaticStructFunction() { memberExprBits.IsStaticStructFunction = true; }
    bool isStaticStructFunction() const { return memberExprBits.IsStaticStructFunction; }

    // NOTE: uses static var
    void printLiteral(StringBuilder& buffer) const;
private:
    Expr* Base;
    IdentifierExpr* member;
    Decl* decl;
};


class ParenExpr : public Expr {
public:
    ParenExpr(SourceLocation l, SourceLocation r, Expr* val)
        : Expr(EXPR_PAREN, false)
        , L(l), R(r), Val(val)
    {}
    ~ParenExpr();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_PAREN;
    }
    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return L; }

    Expr* getExpr() const { return Val; }
    SourceLocation getLParen() const { return L; }
    SourceLocation getRParen() const { return R; }
    SourceLocation getLocStart() const { return L; }
    SourceLocation getLocEnd() const { return R; }
private:
    SourceLocation L, R;
    Expr* Val;
};


class BitOffsetExpr : public Expr {
public:
    BitOffsetExpr(Expr* lhs_, Expr* rhs_, SourceLocation colLoc_)
        : Expr(EXPR_BITOFFSET, false)
        , colLoc(colLoc_)
        , lhs(lhs_)
        , rhs(rhs_)
        , width(0)
    {}
    ~BitOffsetExpr();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_BITOFFSET;
    }
    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return colLoc; }
    SourceLocation getLocStart() const { return lhs->getLocStart(); }
    SourceLocation getLocEnd() const { return rhs->getLocEnd(); }

    Expr* getLHS() const { return lhs; }
    Expr* getRHS() const { return rhs; }
    unsigned char getWidth() const { return width; }
    void setWidth(unsigned char width_) { width = width_; }

    void printLiteral(StringBuilder& buffer) const;
private:
    SourceLocation colLoc;
    Expr* lhs;
    Expr* rhs;
    unsigned char width;    // only valid if constant
};


class ExplicitCastExpr : public Expr {
public:
    ExplicitCastExpr(SourceLocation loc, QualType type, Expr* expr_)
        : Expr(EXPR_CAST, false)
        , castLoc(loc)
        , destType(type)
        , inner(expr_)
    {}
    ~ExplicitCastExpr();
    static bool classof(const Expr* E) {
        return E->getKind() == EXPR_CAST;
    }
    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return castLoc; }
    SourceLocation getLocEnd() const { return inner->getLocEnd(); }

    QualType getDestType() const { return destType; }
    void setDestType(QualType Q) { destType = Q; }
    Expr* getInner() const { return inner; }
private:
    SourceLocation castLoc;
    QualType destType;
    Expr* inner;
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

