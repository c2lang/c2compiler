/* Copyright 2013-2017 Bas van den Berg
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

#include <vector>

#include <clang/Basic/SourceLocation.h>


using clang::SourceLocation;

namespace C2 {

class StringBuilder;
class Expr;
class VarDecl;
class ASTContext;

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


class alignas(void*) Stmt {
public:
    Stmt(StmtKind k);
    StmtKind getKind() const { return static_cast<StmtKind>(stmtBits.sKind); }
    void print(StringBuilder& buffer, unsigned indent) const;
    void dump() const;
    SourceLocation getLocation() const;

protected:
    // See Clang comments in include/clang/AST/Stmt.h about operator new/delete
    void* operator new(size_t bytes) noexcept {
        assert(0 && "Stmt cannot be allocated with regular 'new'");
        return 0;
    }
    void operator delete(void* data) {
        assert(0 && "Stmt cannot be released with regular 'delete'");
    }

public:
    void* operator new(size_t bytes, const ASTContext& C, unsigned alignment = 8);
    // placement operator, for sub-class specific allocators
    void* operator new(size_t bytes, void* mem) noexcept { return mem; }

    void operator delete(void*, const ASTContext& C, unsigned) noexcept {}
    void operator delete(void*, const ASTContext* C, unsigned) noexcept {}
    void operator delete(void*, size_t) noexcept {}
    void operator delete(void*, void*) noexcept {}

protected:
    class StmtBitfields {
        friend class Stmt;

        unsigned sKind : 8;
    };
    enum { NumStmtBits = 8 };

    class SwitchStmtBitfields {
        friend class SwitchStmt;
        unsigned : NumStmtBits;

        unsigned numCases : 32 - NumStmtBits;
    };

    class CaseStmtBitfields {
        friend class CaseStmt;
        unsigned : NumStmtBits;

        unsigned numStmts : 32 - NumStmtBits;
    };

    class DefaultStmtBitfields {
        friend class DefaultStmt;
        unsigned : NumStmtBits;

        unsigned numStmts : 32 - NumStmtBits;
    };

    class CompoundStmtBitfields {
        friend class CompoundStmt;
        unsigned : NumStmtBits;

        unsigned numStmts : 32 - NumStmtBits;
    };

    class ExprBitfields {
        friend class Expr;
        unsigned : NumStmtBits;

        unsigned eKind : 8;
        unsigned ImpCast: 4;
        unsigned IsCTC: 2;          // CTC_FULL -> value matters, CTC_NONE -> type matters
        unsigned IsConstant : 1;    // const :"bla", test, 3. Not const: test(). Depends: a
    };
    enum { NumExprBits = 16 + NumStmtBits };

    class IdentifierExprBitfields {
        friend class IdentifierExpr;
        unsigned : NumExprBits;

        unsigned IsType : 1;
        unsigned IsStructFunction : 1;
        unsigned haveDecl : 1;
    };

    class CallExprBitfields {
        friend class CallExpr;
        unsigned : NumExprBits;

        unsigned IsStructFunc : 1;
        unsigned numArgs : 4;
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

    class BinaryOperatorBitfields {
        friend class BinaryOperator;
        unsigned : NumExprBits;

        unsigned opcode : 6;
    };

    class UnaryOperatorBitfields {
        friend class UnaryOperator;
        unsigned : NumExprBits;

        unsigned opcode : 5;
    };

    class BuiltinExprBitfields {
        friend class BuiltinExpr;
        unsigned : NumExprBits;

        unsigned builtinKind: 2;
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

    class BitOffsetExprBitfields {
        friend class BitOffsetExpr;
        unsigned : NumExprBits;

        unsigned width : 8;
    };

    union {
        StmtBitfields stmtBits;
        SwitchStmtBitfields switchStmtBits;
        CaseStmtBitfields caseStmtBits;
        DefaultStmtBitfields defaultStmtBits;
        CompoundStmtBitfields compoundStmtBits;

        ExprBitfields exprBits;
        IdentifierExprBitfields identifierExprBits;
        CallExprBitfields callExprBits;
        IntegerLiteralBitfields integerLiteralBits;
        BooleanLiteralBitfields booleanLiteralBits;
        TypeExprBitfields typeExprBits;
        BinaryOperatorBitfields binaryOperatorBits;
        UnaryOperatorBitfields unaryOperatorBits;
        BuiltinExprBitfields builtinExprBits;
        MemberExprBitfields memberExprBits;
        InitListExprBitfields initListExprBits;
        DesignatedInitExprBitfields designatedInitExprBits;
        BitOffsetExprBitfields bitOffsetExprBits;
    };
private:
    Stmt(const Stmt&);
    Stmt& operator= (const Stmt&);
};


typedef std::vector<Stmt*> StmtList;

class ReturnStmt : public Stmt {
public:
    ReturnStmt(SourceLocation loc, Expr* value_);
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_RETURN;
    }

    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return RetLoc; }

    Expr* getExpr() const { return value; }
private:
    SourceLocation RetLoc;
    Expr* value;
};


class IfStmt : public Stmt {
public:
    IfStmt(SourceLocation ifLoc,
           Stmt* condition, Stmt* thenStmt,
           SourceLocation elseLoc, Stmt* elseStmt);
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
    SwitchStmt(SourceLocation Loc_, Stmt* Cond_, Stmt** cases_, unsigned numCases_);
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_SWITCH;
    }

    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return Loc; }

    Stmt* getCond() const { return Cond; }
    unsigned numCases() const { return switchStmtBits.numCases; }
    Stmt** getCases() const { return cases; }
private:
    SourceLocation Loc;
    Stmt* Cond;
    Stmt** cases;
};


class CaseStmt : public Stmt {
public:
    CaseStmt(SourceLocation Loc_, Expr* Cond_, Stmt** stmts_, unsigned numStmts_);
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_CASE;
    }

    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return Loc; }

    Expr* getCond() const { return Cond; }
    unsigned numStmts() const { return caseStmtBits.numStmts; }
    Stmt** getStmts() const { return stmts; }
private:
    SourceLocation Loc;
    Expr* Cond;
    Stmt** stmts;
};


class DefaultStmt : public Stmt {
public:
    DefaultStmt(SourceLocation Loc_, Stmt** stmts_, unsigned numStmts_);
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_DEFAULT;
    }

    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return Loc; }

    unsigned numStmts() const { return defaultStmtBits.numStmts; }
    Stmt** getStmts() const { return stmts; }
private:
    SourceLocation Loc;
    Stmt** stmts;
};


class BreakStmt : public Stmt {
public:
    BreakStmt(SourceLocation Loc_);
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
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_LABEL;
    }

    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return Loc; }
    Stmt* getSubStmt() const { return subStmt; }
    const char* getName() const { return name; }
private:
    SourceLocation Loc;
    const char* name;
    Stmt* subStmt;
};


class GotoStmt : public Stmt {
public:
    GotoStmt(const char* name_, SourceLocation GotoLoc_, SourceLocation LabelLoc_);
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_GOTO;
    }

    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return GotoLoc; }
    const char* getName() const { return name; }
private:
    const char* name;
    SourceLocation GotoLoc;
    SourceLocation LabelLoc;
};


class CompoundStmt : public Stmt {
public:
    CompoundStmt(SourceLocation l, SourceLocation r, Stmt** stmts_, unsigned numStmts_);
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_COMPOUND;
    }

    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const { return Left; }

    unsigned numStmts() const { return compoundStmtBits.numStmts; }
    Stmt** getStmts() const { return stmts; }

    Stmt* getLastStmt() const;
    SourceLocation getRight() const { return Right; }
private:
    SourceLocation Left;
    SourceLocation Right;
    Stmt** stmts;
};


class DeclStmt : public Stmt {
public:
    DeclStmt(VarDecl* decl_);
    static bool classof(const Stmt* S) {
        return S->getKind() == STMT_DECL;
    }
    void print(StringBuilder& buffer, unsigned indent) const;
    SourceLocation getLocation() const;

    const char* getName() const;
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

