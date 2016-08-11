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

#ifndef AST_STMT_H
#define AST_STMT_H

#include <string>

#include <clang/Basic/SourceLocation.h>

#include "AST/OwningVector.h"

using clang::SourceLocation;

namespace C2 {

class StringBuilder;
class Expr;
class VarDecl;

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
    STMT_DECL,
};


class Stmt {
public:
    Stmt(StmtKind k);
    ~Stmt();
    StmtKind getKind() const { return static_cast<StmtKind>(stmtBits.sKind); }
    void print(StringBuilder& buffer, unsigned indent) const;
    void dump() const;
    SourceLocation getLocation() const;

protected:
    class StmtBitfields {
        friend class Stmt;

        unsigned sKind : 8;
    };
    enum { NumStmtBits = 8 };

    class ExprBitfields {
        friend class Expr;
        unsigned : NumStmtBits;

        unsigned eKind : 8;             // BBB TODO Merge with sKind
        unsigned ImpCast: 4;
        unsigned IsCTC: 2;          // CTC_FULL -> value matters, CTC_NONE -> type matters
        unsigned IsConstant : 1;    // const :"bla", test, 3. Not const: test(). Depends: a
    };
    enum { NumExprBits = 16 + NumStmtBits };      // TODO make 8 when eKind is merged

    class IdentifierExprBitfields {
        friend class IdentifierExpr;
        unsigned : NumExprBits;

        unsigned IsType : 1;
        unsigned IsStructFunction : 1;
    };

    class CallExprBitfields {
        friend class CallExpr;
        unsigned : NumExprBits;

        unsigned IsStructFunc : 1;
    };


    class IntegerLiteralBitfields {
        friend class IntegerLiteral;
        unsigned : NumExprBits;

        unsigned Radix: 2;
    };

    class BooleanLiteralBitfields {
        friend class BooleanLiteral;
        unsigned : NumExprBits;

        unsigned Value : 1;
    };

    class TypeExprBitfields {
        friend class TypeExpr;
        unsigned : NumExprBits;

        unsigned IsLocal : 1;
    };

    class BuiltinExprBitfields {
        friend class BuiltinExpr;
        unsigned : NumExprBits;

        unsigned IsSizeOf: 1;
    };

    class MemberExprBitfields {
        friend class MemberExpr;
        unsigned : NumExprBits;

        unsigned IsModPrefix: 1;
        unsigned IsStructFunction : 1;
        unsigned IsStaticStructFunction : 1;
    };

    class InitListExprBitfields {
        friend class InitListExpr;
        unsigned : NumExprBits;

        unsigned HasDesignators : 1;
    };

    class DesignatedInitExprBitfields {
        friend class DesignatedInitExpr;
        unsigned : NumExprBits;

        unsigned DesignatorKind : 1;
    };

    union {
        StmtBitfields stmtBits;
        ExprBitfields exprBits;
        IdentifierExprBitfields identifierExprBits;
        CallExprBitfields callExprBits;
        IntegerLiteralBitfields integerLiteralBits;
        BooleanLiteralBitfields booleanLiteralBits;
        TypeExprBitfields typeExprBits;
        BuiltinExprBitfields builtinExprBits;
        MemberExprBitfields memberExprBits;
        InitListExprBitfields initListExprBits;
        DesignatedInitExprBitfields designatedInitExprBits;
    };
private:
    Stmt(const Stmt&);
    Stmt& operator= (const Stmt&);
};


typedef OwningVector<Stmt> StmtList;

class ReturnStmt : public Stmt {
public:
    ReturnStmt(SourceLocation loc, Expr* value_);
    ~ReturnStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_RETURN;
    }

    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return RetLoc; }

    Expr* getExpr() const { return value; }
private:
    Expr* value;
    SourceLocation RetLoc;
};


class IfStmt : public Stmt {
public:
    IfStmt(SourceLocation ifLoc,
           Stmt* condition, Stmt* thenStmt,
           SourceLocation elseLoc, Stmt* elseStmt);
    ~IfStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_IF;
    }

    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return IfLoc; }

    VarDecl* getConditionVariable() const;

    Stmt* getCond() const { return SubExprs[COND]; }
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
    WhileStmt(SourceLocation Loc_, Stmt* Cond_, Stmt* Then_);
    ~WhileStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_WHILE;
    }

    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return Loc; }

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
    ~DoStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_DO;
    }

    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return Loc; }

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
    ~ForStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_FOR;
    }

    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return Loc; }

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
    SwitchStmt(SourceLocation Loc_, Stmt* Cond_, StmtList& Cases_);
    ~SwitchStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_SWITCH;
    }

    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return Loc; }

    Stmt* getCond() const { return Cond; }
    const StmtList& getCases() const { return Cases; }
private:
    SourceLocation Loc;
    Stmt* Cond;
    StmtList Cases;
};


class CaseStmt : public Stmt {
public:
    CaseStmt(SourceLocation Loc_, Expr* Cond_, StmtList& Stmts_);
    ~CaseStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_CASE;
    }

    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return Loc; }

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
    ~DefaultStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_DEFAULT;
    }

    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return Loc; }

    const StmtList& getStmts() const { return Stmts; }
private:
    SourceLocation Loc;
    StmtList Stmts;
};


class BreakStmt : public Stmt {
public:
    BreakStmt(SourceLocation Loc_);
    ~BreakStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_BREAK;
    }

    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return Loc; }
private:
    SourceLocation Loc;
};


class ContinueStmt : public Stmt {
public:
    ContinueStmt(SourceLocation Loc_);
    ~ContinueStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_CONTINUE;
    }

    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return Loc; }
private:
    SourceLocation Loc;
};


class LabelStmt : public Stmt {
public:
    LabelStmt(const char* name_, SourceLocation Loc_, Stmt* subStmt_);
    ~LabelStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_LABEL;
    }

    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return Loc; }
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
    ~GotoStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_GOTO;
    }

    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return GotoLoc; }
    const std::string& getName() const { return name; }
private:
    std::string name;
    SourceLocation GotoLoc;
    SourceLocation LabelLoc;
};


class CompoundStmt : public Stmt {
public:
    CompoundStmt(SourceLocation l, SourceLocation r, StmtList& stmts_);
    ~CompoundStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_COMPOUND;
    }

    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return Left; }

    const StmtList& getStmts() const { return Stmts; }
    Stmt* getLastStmt() const;
    SourceLocation getRight() const { return Right; }
private:
    SourceLocation Left;
    SourceLocation Right;
    StmtList Stmts;
};


class DeclStmt : public Stmt {
public:
    DeclStmt(VarDecl* decl_);
    ~DeclStmt();
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_DECL;
    }
    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const;

    const std::string& getName() const;
    VarDecl* getDecl() const { return decl; }
private:
    VarDecl* decl;
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

