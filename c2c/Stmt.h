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

#ifndef STMT_H
#define STMT_H

#include <string>
#include <vector>

#include <clang/Basic/SourceLocation.h>
#include "OwningVector.h"

using clang::SourceLocation;

namespace llvm {
class Value;
}

namespace C2 {

class StringBuilder;
class StmtVisitor;
class Expr;
class CodeGenContext;

enum StmtType {
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
    Stmt();
    virtual ~Stmt();
    virtual StmtType stype() const = 0;
    virtual void acceptS(StmtVisitor& v) = 0;
    virtual void print(int indent, StringBuilder& buffer) = 0;
    virtual void generateC(int indent, StringBuilder& buffer) = 0;
    virtual llvm::Value* codeGen(CodeGenContext& context) = 0;
    void dump();
private:
    Stmt(const Stmt&);
    Stmt& operator= (const Stmt&);
};


typedef OwningVector<Stmt> StmtList;

class ReturnStmt : public Stmt {
public:
    ReturnStmt(SourceLocation loc,Expr* value_);
    virtual ~ReturnStmt();
    virtual StmtType stype() const { return STMT_RETURN; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);

    Expr* getExpr() const { return value; }
    SourceLocation getLocation() const { return RetLoc; }
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
    virtual StmtType stype() const { return STMT_IF; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);

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
    WhileStmt(SourceLocation Loc_, Expr* Cond_, Stmt* Then_);
    virtual ~WhileStmt();
    virtual StmtType stype() const { return STMT_WHILE; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);

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
    virtual StmtType stype() const { return STMT_DO; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);

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
    virtual StmtType stype() const { return STMT_FOR; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);

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
    virtual StmtType stype() const { return STMT_SWITCH; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);

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
    virtual StmtType stype() const { return STMT_CASE; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);

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
    virtual ~DefaultStmt();
    virtual StmtType stype() const { return STMT_DEFAULT; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);

    SourceLocation getLocation() const { return Loc; }
    const StmtList& getStmts() const { return Stmts; }
private:
    SourceLocation Loc;
    StmtList Stmts;
};


class BreakStmt : public Stmt {
public:
    BreakStmt(SourceLocation Loc_);
    virtual ~BreakStmt();
    virtual StmtType stype() const { return STMT_BREAK; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);

    SourceLocation getLocation() const { return Loc; }
private:
    SourceLocation Loc;
};


class ContinueStmt : public Stmt {
public:
    ContinueStmt(SourceLocation Loc_);
    virtual ~ContinueStmt();
    virtual StmtType stype() const { return STMT_CONTINUE; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);

    SourceLocation getLocation() const { return Loc; }
private:
    SourceLocation Loc;
};


class LabelStmt : public Stmt {
public:
    LabelStmt(const char* name_, SourceLocation Loc_, Stmt* subStmt_);
    virtual ~LabelStmt();
    virtual StmtType stype() const { return STMT_LABEL; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);
    Stmt* getSubStmt() const { return subStmt; }
private:
    std::string name;
    SourceLocation Loc;
    Stmt* subStmt;
};


class GotoStmt : public Stmt {
public:
    GotoStmt(const char* name_, SourceLocation GotoLoc_, SourceLocation LabelLoc_);
    virtual ~GotoStmt();
    virtual StmtType stype() const { return STMT_GOTO; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);
private:
    std::string name;
    SourceLocation GotoLoc;
    SourceLocation LabelLoc;
};


class CompoundStmt : public Stmt {
public:
    CompoundStmt(SourceLocation l, SourceLocation r, StmtList& stmts_);
    virtual ~CompoundStmt();
    virtual StmtType stype() const { return STMT_COMPOUND; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);

    const StmtList& getStmts() const { return Stmts; }
    Stmt* getLastStmt() const;
    SourceLocation getRight() const { return Right; }
private:
    SourceLocation Left;
    SourceLocation Right;
    StmtList Stmts;
};


class StmtVisitor {
public:
    virtual ~StmtVisitor() {}
    virtual void visit(C2::Stmt&) { assert(0); }    // add subclass below
    virtual void visit(ReturnStmt&) {}
    virtual void visit(IfStmt&) {}
    virtual void visit(WhileStmt&) {}
    virtual void visit(DoStmt&) {}
    virtual void visit(ForStmt&) {}
    virtual void visit(SwitchStmt&) {}
    virtual void visit(CaseStmt&) {}
    virtual void visit(DefaultStmt&) {}
    virtual void visit(BreakStmt&) {}
    virtual void visit(ContinueStmt&) {}
    virtual void visit(LabelStmt&) {}
    virtual void visit(GotoStmt&) {}
    virtual void visit(CompoundStmt&) {}
    virtual void visit(Expr&) {}
};

#define STMT_VISITOR_ACCEPT(a) void a::acceptS(StmtVisitor& v) { v.visit(*this); }

template <class T> class StmtCaster : public StmtVisitor {
public:
    virtual void visit(T& node_) {
        node = &node_;
    }
    static T* getType(C2::Stmt& node_) {
        StmtCaster<T> visitor(node_);
        return visitor.node;
    }
    static T* getType(C2::Stmt* node_) {
        StmtCaster<T> visitor(*node_);
        return visitor.node;
    }
private:
    StmtCaster(C2::Stmt& n) : node(0) {
        n.acceptS(*this);
    }
    T* node;
};

}

#endif

