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

#ifndef AST_STMT_H
#define AST_STMT_H

#include <string>
#include <vector>

#include <clang/Basic/SourceLocation.h>

#include "AST/OwningVector.h"

using clang::SourceLocation;

namespace llvm {
class Value;
}

namespace C2 {

class StringBuilder;
class Expr;

enum StmtKind {
    STMT_RETURN = 0,
    STMT_EXPR,
    STMT_IF,
    STMT_WHILE,
    STMT_DO,
    STMT_FOR,
    STMT_SWITCH,
    STMT_CASE,
    STMT_DEFAULT,
    STMT_BREAK,
    STMT_CONTINUE,
    STMT_LABEL,
    STMT_GOTO,
    STMT_COMPOUND,
};


class Stmt {
public:
    Stmt(StmtKind k);
    virtual ~Stmt();
    StmtKind getKind() const { return static_cast<StmtKind>(StmtBits.sKind); }
    virtual void print(StringBuilder& buffer, unsigned indent) const = 0;
    void dump() const;
    virtual SourceLocation getLocation() const = 0;

protected:
    class StmtBitfields {
    public:
        unsigned sKind : 8;
        unsigned eKind : 8;
        unsigned ExprIsCTC: 2;          // CTC_FULL -> value matters, CTC_NONE -> type matters
        unsigned ExprIsConstant : 1;    // const :"bla", test, 3. Not const: test(). Depends: a
        unsigned ExprImpCast: 4;
        unsigned BoolLiteralValue : 1;
        unsigned TypeExprIsLocal : 1;
        unsigned BuiltInIsSizeOf: 1;
        unsigned MemberExprIsArrow: 1;
        unsigned MemberExprIsModPrefix: 1;
        unsigned InitListHasDesignators : 1;
        unsigned DesignatorKind : 1;
    };
    union {
        StmtBitfields StmtBits;
        unsigned BitsInit;      // to initialize all bits
    };
private:
    Stmt(const Stmt&);
    Stmt& operator= (const Stmt&);
};


typedef OwningVector<Stmt> StmtList;

class ReturnStmt : public Stmt {
public:
    ReturnStmt(SourceLocation loc,Expr* value_);
    virtual ~ReturnStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_RETURN;
    }

    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return RetLoc; }

    Expr* getExpr() const { return value; }
private:
    Expr* value;
    SourceLocation RetLoc;
};


class IfStmt : public Stmt {
public:
    IfStmt(SourceLocation ifLoc,
           Expr* condition, Stmt* thenStmt,
           SourceLocation elseLoc, Stmt* elseStmt);
    virtual ~IfStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_IF;
    }

    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return IfLoc; }

    Expr* getCond() const { return reinterpret_cast<Expr*>(SubExprs[COND]); }
    Stmt* getThen() const { return SubExprs[THEN]; }
    Stmt* getElse() const { return SubExprs[ELSE]; }
private:
    enum { VAR, COND, THEN, ELSE, END_EXPR };
    Stmt* SubExprs[END_EXPR];

    SourceLocation IfLoc;
    SourceLocation ElseLoc;
};


class WhileStmt : public Stmt {
public:
    WhileStmt(SourceLocation Loc_, Expr* Cond_, Stmt* Then_);
    virtual ~WhileStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_WHILE;
    }

    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return Loc; }

    Stmt* getCond() const { return Cond; }
    Stmt* getBody() const { return Then; }
private:
    SourceLocation Loc;
    Stmt* Cond;
    Stmt* Then;
};


class DoStmt : public Stmt {
public:
    DoStmt(SourceLocation Loc_, Expr* Cond_, Stmt* Then_);
    virtual ~DoStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_DO;
    }

    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return Loc; }

    Stmt* getCond() const { return Cond; }
    Stmt* getBody() const { return Then; }
private:
    SourceLocation Loc;
    Stmt* Cond;
    Stmt* Then;
};


class ForStmt : public Stmt {
public:
    ForStmt(SourceLocation Loc_, Stmt* Init_, Expr* Cond_, Expr* Incr_, Stmt* Body_);
    virtual ~ForStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_FOR;
    }

    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return Loc; }

    Stmt* getInit() const { return Init; }
    Expr* getCond() const { return Cond; }
    Expr* getIncr() const { return Incr; }
    Stmt* getBody() const { return Body; }
private:
    SourceLocation Loc;
    Stmt* Init;
    Expr* Cond;
    Expr* Incr;
    Stmt* Body;
};


class SwitchStmt : public Stmt {
public:
    SwitchStmt(SourceLocation Loc_, Expr* Cond_, StmtList& Cases_);
    virtual ~SwitchStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_SWITCH;
    }

    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return Loc; }

    Expr* getCond() const { return Cond; }
    const StmtList& getCases() const { return Cases; }
private:
    SourceLocation Loc;
    Expr* Cond;
    StmtList Cases;
};


class CaseStmt : public Stmt {
public:
    CaseStmt(SourceLocation Loc_, Expr* Cond_, StmtList& Stmts_);
    virtual ~CaseStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_CASE;
    }

    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return Loc; }

    Expr* getCond() const { return Cond; }
    const StmtList& getStmts() const { return Stmts; }
private:
    SourceLocation Loc;
    Expr* Cond;
    StmtList Stmts;
};


class DefaultStmt : public Stmt {
public:
    DefaultStmt(SourceLocation Loc_, StmtList& Stmts_);
    virtual ~DefaultStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_DEFAULT;
    }

    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return Loc; }

    const StmtList& getStmts() const { return Stmts; }
private:
    SourceLocation Loc;
    StmtList Stmts;
};


class BreakStmt : public Stmt {
public:
    BreakStmt(SourceLocation Loc_);
    virtual ~BreakStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_BREAK;
    }

    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return Loc; }
private:
    SourceLocation Loc;
};


class ContinueStmt : public Stmt {
public:
    ContinueStmt(SourceLocation Loc_);
    virtual ~ContinueStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_CONTINUE;
    }

    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return Loc; }
private:
    SourceLocation Loc;
};


class LabelStmt : public Stmt {
public:
    LabelStmt(const char* name_, SourceLocation Loc_, Stmt* subStmt_);
    virtual ~LabelStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_LABEL;
    }

    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return Loc; }
    Stmt* getSubStmt() const { return subStmt; }
    const std::string& getName() const { return name; }
private:
    std::string name;
    SourceLocation Loc;
    Stmt* subStmt;
};


class GotoStmt : public Stmt {
public:
    GotoStmt(const char* name_, SourceLocation GotoLoc_, SourceLocation LabelLoc_);
    virtual ~GotoStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_GOTO;
    }

    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return GotoLoc; }
    const std::string& getName() const { return name; }
private:
    std::string name;
    SourceLocation GotoLoc;
    SourceLocation LabelLoc;
};


class CompoundStmt : public Stmt {
public:
    CompoundStmt(SourceLocation l, SourceLocation r, StmtList& stmts_);
    virtual ~CompoundStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_COMPOUND;
    }

    virtual void print(StringBuilder& buffer, unsigned indent) const;
    virtual SourceLocation getLocation() const { return Left; }

    const StmtList& getStmts() const { return Stmts; }
    Stmt* getLastStmt() const;
    SourceLocation getRight() const { return Right; }
private:
    SourceLocation Left;
    SourceLocation Right;
    StmtList Stmts;
};


template <class T> static inline bool isa(const Stmt* S) {
    return T::classof(S);
}

template <class T> static inline T* dyncast(Stmt* S) {
    if (isa<T>(S)) return static_cast<T*>(S);
    return 0;
}

template <class T> static inline const T* dyncast(const Stmt* S) {
    if (isa<T>(S)) return static_cast<const T*>(S);
    return 0;
}

//#define CAST_DEBUG
#ifdef CAST_DEBUG
#include <assert.h>
#endif

template <class T> static inline T* cast(Stmt* S) {
#ifdef CAST_DEBUG
    assert(isa<T>(S));
#endif
    return static_cast<T*>(S);
}

template <class T> static inline const T* cast(const Stmt* S) {
#ifdef CAST_DEBUG
    assert(isa<T>(S));
#endif
    return static_cast<const T*>(S);
}

}

#endif

