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

#include <stdio.h>

#include "AST/Stmt.h"
#include "AST/Expr.h"
#include "AST/Decl.h"
#include "AST/ASTContext.h"
#include "Utils/StringBuilder.h"
#include "Utils/color.h"
#include "Utils/UtilsConstants.h"

using namespace C2;
using namespace std;

Stmt::Stmt(StmtKind k) {
    stmtBits.sKind = k;
}

void* Stmt::operator new(size_t bytes, const C2::ASTContext& C, unsigned alignment) {
    return ::operator new(bytes, C, alignment);
}

void Stmt::print(StringBuilder& buffer, unsigned indent) const {
    switch (getKind()) {
    case STMT_RETURN:
        return cast<ReturnStmt>(this)->print(buffer, indent);
    case STMT_EXPR:
        return cast<Expr>(this)->print(buffer, indent);
    case STMT_IF:
        return cast<IfStmt>(this)->print(buffer, indent);
    case STMT_WHILE:
        return cast<WhileStmt>(this)->print(buffer, indent);
    case STMT_DO:
        return cast<DoStmt>(this)->print(buffer, indent);
    case STMT_FOR:
        return cast<ForStmt>(this)->print(buffer, indent);
    case STMT_SWITCH:
        return cast<SwitchStmt>(this)->print(buffer, indent);
    case STMT_CASE:
        return cast<CaseStmt>(this)->print(buffer, indent);
    case STMT_DEFAULT:
        return cast<DefaultStmt>(this)->print(buffer, indent);
    case STMT_BREAK:
        return cast<BreakStmt>(this)->print(buffer, indent);
    case STMT_CONTINUE:
        return cast<ContinueStmt>(this)->print(buffer, indent);
    case STMT_LABEL:
        return cast<LabelStmt>(this)->print(buffer, indent);
    case STMT_GOTO:
        return cast<GotoStmt>(this)->print(buffer, indent);
    case STMT_COMPOUND:
        return cast<CompoundStmt>(this)->print(buffer, indent);
    case STMT_DECL:
        return cast<DeclStmt>(this)->print(buffer, indent);
    }
}

SourceLocation Stmt::getLocation() const {
    switch (getKind()) {
    case STMT_RETURN:
        return cast<ReturnStmt>(this)->getLocation();
    case STMT_EXPR:
        return cast<Expr>(this)->getLocation();
    case STMT_IF:
        return cast<IfStmt>(this)->getLocation();
    case STMT_WHILE:
        return cast<WhileStmt>(this)->getLocation();
    case STMT_DO:
        return cast<DoStmt>(this)->getLocation();
    case STMT_FOR:
        return cast<ForStmt>(this)->getLocation();
    case STMT_SWITCH:
        return cast<SwitchStmt>(this)->getLocation();
    case STMT_CASE:
        return cast<CaseStmt>(this)->getLocation();
    case STMT_DEFAULT:
        return cast<DefaultStmt>(this)->getLocation();
    case STMT_BREAK:
        return cast<BreakStmt>(this)->getLocation();
    case STMT_CONTINUE:
        return cast<ContinueStmt>(this)->getLocation();
    case STMT_LABEL:
        return cast<LabelStmt>(this)->getLocation();
    case STMT_GOTO:
        return cast<GotoStmt>(this)->getLocation();
    case STMT_COMPOUND:
        return cast<CompoundStmt>(this)->getLocation();
    case STMT_DECL:
        return cast<DeclStmt>(this)->getLocation();
    }
}

void Stmt::dump() const {
    StringBuilder buffer;
    print(buffer, 0);
    fprintf(stderr, "%s\n", buffer.c_str());
}


ReturnStmt::ReturnStmt(SourceLocation loc, Expr* value_)
    : Stmt(STMT_RETURN)
    , RetLoc(loc)
    , value(value_)
{}

void ReturnStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "ReturnStmt\n";
    if (value) {
        value->print(buffer, indent + INDENT);
    }
}


IfStmt::IfStmt(SourceLocation ifLoc,
               Stmt* condition, Stmt* thenStmt,
               SourceLocation elseLoc, Stmt* elseStmt)
    : Stmt(STMT_IF)
    , IfLoc(ifLoc)
    , ElseLoc(elseLoc)
{
    SubExprs[VAR] = 0;  // unused?
    SubExprs[COND] = condition;
    SubExprs[THEN] = thenStmt;
    SubExprs[ELSE] = elseStmt;
}

void IfStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "IfStmt\n";
    SubExprs[COND]->print(buffer, indent + INDENT);
    if (SubExprs[THEN]) SubExprs[THEN]->print(buffer, indent + INDENT);
    if (SubExprs[ELSE]) SubExprs[ELSE]->print(buffer, indent + INDENT);
}

VarDecl* IfStmt::getConditionVariable() const {
    const Stmt* C = getCond();
    if (isa<DeclStmt>(C)) {
        return cast<DeclStmt>(C)->getDecl();
    } else {
        return NULL;
    }
}

WhileStmt::WhileStmt(SourceLocation Loc_, Stmt* Cond_, Stmt* Then_)
    : Stmt(STMT_WHILE)
    , Loc(Loc_)
    , Cond(Cond_)
    , Then(Then_)
{}

void WhileStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "WhileStmt\n";
    Cond->print(buffer, indent + INDENT);
    Then->print(buffer, indent + INDENT);
}


DoStmt::DoStmt(SourceLocation Loc_, Expr* Cond_, Stmt* Then_)
    : Stmt(STMT_DO)
    , Loc(Loc_)
    , Cond(Cond_)
    , Then(Then_)
{}

void DoStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "DoStmt\n";
    Cond->print(buffer, indent + INDENT);
    Then->print(buffer, indent + INDENT);
}


ForStmt::ForStmt(SourceLocation Loc_, Stmt* Init_, Expr* Cond_, Expr* Incr_, Stmt* Body_)
    : Stmt(STMT_FOR)
    , Loc(Loc_)
    , Init(Init_)
    , Cond(Cond_)
    , Incr(Incr_)
    , Body(Body_)
{}

void ForStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "ForStmt\n";
    if (Init) Init->print(buffer, indent + INDENT);
    if (Cond) Cond->print(buffer, indent + INDENT);
    if (Incr) Incr->print(buffer, indent + INDENT);
    Body->print(buffer, indent + INDENT);
}


SwitchStmt::SwitchStmt(SourceLocation Loc_, Stmt* Cond_, Stmt** cases_, unsigned numCases_)
    : Stmt(STMT_SWITCH)
    , Loc(Loc_)
    , Cond(Cond_)
    , cases(cases_)
{
    switchStmtBits.numCases = numCases_;
}

void SwitchStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "SwitchStmt\n";
    Cond->print(buffer, indent + INDENT);
    for (unsigned i=0; i<numCases(); i++) {
        cases[i]->print(buffer, indent + INDENT);
    }
}


CaseStmt::CaseStmt(SourceLocation Loc_, Expr* Cond_, Stmt** stmts_, unsigned numStmts_)
    : Stmt(STMT_CASE)
    , Loc(Loc_)
    , Cond(Cond_)
    , stmts(stmts_)
{
    caseStmtBits.numStmts = numStmts_;
}

void CaseStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "CaseStmt\n";
    Cond->print(buffer, indent + INDENT);
    for (unsigned i=0; i<numStmts(); i++) {
        stmts[i]->print(buffer, indent + INDENT);
    }
}


DefaultStmt::DefaultStmt(SourceLocation Loc_, Stmt** stmts_, unsigned numStmts_)
    : Stmt(STMT_DEFAULT)
    , Loc(Loc_)
    , stmts(stmts_)
{
    defaultStmtBits.numStmts = numStmts_;
}

void DefaultStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "DefaultStmt\n";
    for (unsigned i=0; i<numStmts(); i++) {
        stmts[i]->print(buffer, indent + INDENT);
    }
}


BreakStmt::BreakStmt(SourceLocation Loc_)
    : Stmt(STMT_BREAK)
    , Loc(Loc_)
{}

void BreakStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "BreakStmt\n";
}


ContinueStmt::ContinueStmt(SourceLocation Loc_)
    : Stmt(STMT_CONTINUE)
    , Loc(Loc_)
{}

void ContinueStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "ContinueStmt\n";
}


LabelStmt::LabelStmt(const char* name_, SourceLocation Loc_, Stmt* subStmt_)
    : Stmt(STMT_LABEL)
    , Loc(Loc_)
    , name(name_)
    , subStmt(subStmt_)
{}

void LabelStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "LabelStmt ";
    buffer.setColor(COL_VALUE);
    buffer << name << '\n';
    subStmt->print(buffer, indent + INDENT);
}


GotoStmt::GotoStmt(const char* name_, SourceLocation GotoLoc_, SourceLocation LabelLoc_)
    : Stmt(STMT_GOTO)
    , name(name_), GotoLoc(GotoLoc_), LabelLoc(LabelLoc_)
{}

void GotoStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "GotoStmt ";
    buffer.setColor(COL_VALUE);
    buffer << name << '\n';
}


CompoundStmt::CompoundStmt(SourceLocation l, SourceLocation r, Stmt** stmts_, unsigned numStmts_)
    : Stmt(STMT_COMPOUND)
    , Left(l)
    , Right(r)
    , stmts(stmts_)
{
    compoundStmtBits.numStmts = numStmts_;
}

void CompoundStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "CompoundStmt\n";
    for (unsigned i=0; i<numStmts(); i++) {
        stmts[i]->print(buffer, indent + INDENT);
    }
}

Stmt* CompoundStmt::getLastStmt() const {
    if (numStmts() == 0) return 0;

    Stmt* last = stmts[numStmts() -1];

    // TODO handle goto statement as last statement
    while (1) {
        switch (last->getKind()) {
        case STMT_LABEL:
            last = cast<LabelStmt>(last)->getSubStmt();
            break;
        case STMT_COMPOUND:
            return cast<CompoundStmt>(last)->getLastStmt();
        default:
            return last;
        }
    }
}


DeclStmt::DeclStmt(VarDecl* decl_)
    : Stmt(STMT_DECL)
    , decl(decl_)
{}

void DeclStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "DeclStmt\n";
    decl->print(buffer, indent + INDENT);
}

SourceLocation DeclStmt::getLocation() const {
    return decl->getLocation();
}

const char* DeclStmt::getName() const {
    return decl->getName();
}

