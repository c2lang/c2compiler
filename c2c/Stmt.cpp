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

#include <stdio.h>
#include <string.h>
#include "Stmt.h"
#include "Expr.h"
#include "StringBuilder.h"
#include "Utils.h"

using namespace C2;
using namespace std;

//#define STMT_DEBUG
#ifdef STMT_DEBUG
static int creationCount;
static int deleteCount;
#endif

Stmt::Stmt(StmtKind k) : kind(k) {
#ifdef STMT_DEBUG
    creationCount++;
    fprintf(stderr, "[STMT] create %p  created %d deleted %d\n", this, creationCount, deleteCount);
#endif
}

Stmt::~Stmt() {
#ifdef STMT_DEBUG
    deleteCount++;
    fprintf(stderr, "[STMT] delete %p  created %d deleted %d\n", this, creationCount, deleteCount);
#endif
}

void Stmt::dump() const {
    StringBuilder buffer;
    print(0, buffer);
    fprintf(stderr, "%s\n", (const char*)buffer);
}


ReturnStmt::ReturnStmt(SourceLocation loc, Expr* value_)
    : Stmt(STMT_RETURN)
    , value(value_)
    , RetLoc(loc)
{}

ReturnStmt::~ReturnStmt() {
    delete value;
}

void ReturnStmt::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[return]\n";
    if (value) {
        value->print(indent + INDENT, buffer);
    }
}


IfStmt::IfStmt(SourceLocation ifLoc,
               Expr* condition, Stmt* thenStmt,
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

IfStmt::~IfStmt() {
    delete SubExprs[VAR];
    delete SubExprs[COND];
    delete SubExprs[THEN];
    delete SubExprs[ELSE];
}

void IfStmt::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[if]\n";
    SubExprs[COND]->print(indent + INDENT, buffer);
    if (SubExprs[THEN]) SubExprs[THEN]->print(indent + INDENT, buffer);
    if (SubExprs[ELSE]) SubExprs[ELSE]->print(indent + INDENT, buffer);
}


WhileStmt::WhileStmt(SourceLocation Loc_, Expr* Cond_, Stmt* Then_)
    : Stmt(STMT_WHILE)
    , Loc(Loc_)
    , Cond(Cond_)
    , Then(Then_)
{}

WhileStmt::~WhileStmt() {
    delete Cond;
    delete Then;
}

void WhileStmt::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[while]\n";
    Cond->print(indent + INDENT, buffer);
    Then->print(indent + INDENT, buffer);
}


DoStmt::DoStmt(SourceLocation Loc_, Expr* Cond_, Stmt* Then_)
    : Stmt(STMT_DO)
    , Loc(Loc_)
    , Cond(Cond_)
    , Then(Then_)
{}

DoStmt::~DoStmt() {
    delete Cond;
    delete Then;
}

void DoStmt::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[do]\n";
    Cond->print(indent + INDENT, buffer);
    Then->print(indent + INDENT, buffer);
}


ForStmt::ForStmt(SourceLocation Loc_, Stmt* Init_, Expr* Cond_, Expr* Incr_, Stmt* Body_)
    : Stmt(STMT_FOR)
    , Loc(Loc_)
    , Init(Init_)
    , Cond(Cond_)
    , Incr(Incr_)
    , Body(Body_)
{}

ForStmt::~ForStmt() {
    delete Body;
    delete Incr;
    delete Cond;
    delete Init;
}

void ForStmt::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[for]\n";
    if (Init) Init->print(indent + INDENT, buffer);
    if (Cond) Cond->print(indent + INDENT, buffer);
    if (Incr) Incr->print(indent + INDENT, buffer);
    Body->print(indent + INDENT, buffer);
}


SwitchStmt::SwitchStmt(SourceLocation Loc_, Expr* Cond_, StmtList& Cases_)
    : Stmt(STMT_SWITCH)
    , Loc(Loc_)
    , Cond(Cond_)
    , Cases(Cases_)
{}

SwitchStmt::~SwitchStmt() {
    delete Cond;
}

void SwitchStmt::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[switch]\n";
    Cond->print(indent + INDENT, buffer);
    for (unsigned int i=0; i<Cases.size(); i++) {
        Cases[i]->print(indent + INDENT, buffer);
    }
}


CaseStmt::CaseStmt(SourceLocation Loc_, Expr* Cond_, StmtList& Stmts_)
    : Stmt(STMT_CASE)
    , Loc(Loc_)
    , Cond(Cond_)
    , Stmts(Stmts_)
{}

CaseStmt::~CaseStmt() {
    delete Cond;
}

void CaseStmt::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[case]\n";
    Cond->print(indent + INDENT, buffer);
    for (unsigned int i=0; i<Stmts.size(); i++) {
        Stmts[i]->print(indent + INDENT, buffer);
    }
}


DefaultStmt::DefaultStmt(SourceLocation Loc_, StmtList& Stmts_)
    : Stmt(STMT_DEFAULT)
    , Loc(Loc_)
    , Stmts(Stmts_)
{}

DefaultStmt::~DefaultStmt() {}

void DefaultStmt::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[default]\n";
    for (unsigned int i=0; i<Stmts.size(); i++) {
        Stmts[i]->print(indent + INDENT, buffer);
    }
}


BreakStmt::BreakStmt(SourceLocation Loc_)
    : Stmt(STMT_BREAK)
    , Loc(Loc_)
{}

BreakStmt::~BreakStmt() {}

void BreakStmt::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[break]\n";
}


ContinueStmt::ContinueStmt(SourceLocation Loc_)
    : Stmt(STMT_BREAK)
    , Loc(Loc_)
{}

ContinueStmt::~ContinueStmt() {}

void ContinueStmt::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[continue]\n";
}


LabelStmt::LabelStmt(const char* name_, SourceLocation Loc_, Stmt* subStmt_)
    : Stmt(STMT_LABEL)
    , name(name_), Loc(Loc_), subStmt(subStmt_)
{}

LabelStmt::~LabelStmt()
{
    delete subStmt;
}

void LabelStmt::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[label]\n";
    subStmt->print(indent + INDENT, buffer);
}


GotoStmt::GotoStmt(const char* name_, SourceLocation GotoLoc_, SourceLocation LabelLoc_)
    : Stmt(STMT_GOTO)
    , name(name_), GotoLoc(GotoLoc_), LabelLoc(LabelLoc_)
{}

GotoStmt::~GotoStmt() {}

void GotoStmt::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[goto]\n";
}


CompoundStmt::CompoundStmt(SourceLocation l, SourceLocation r, StmtList& stmts_)
    : Stmt(STMT_COMPOUND)
    , Left(l)
    , Right(r)
    , Stmts(stmts_)
{}

CompoundStmt::~CompoundStmt() {}

void CompoundStmt::print(int indent, StringBuilder& buffer) const {
    buffer.indent(indent);
    buffer << "[compound]\n";
    for (unsigned int i=0; i<Stmts.size(); i++) {
        Stmts[i]->print(indent + INDENT, buffer);
    }
}

Stmt* CompoundStmt::getLastStmt() const {
    if (Stmts.size() == 0) return 0;
    else {
        // NOTE: if last is compound, get last from that one
        // NOTE: if last is label, get label.subStmt
        // TODO handle goto statement as last statement
        Stmt* last = Stmts[Stmts.size() -1];
        switch (last->getKind()) {
        case STMT_LABEL:
        {
            LabelStmt* label = cast<LabelStmt>(last);
            // TODO handle compound substatements
            return label->getSubStmt();
        }
        case STMT_COMPOUND:
        {
            CompoundStmt* compound = cast<CompoundStmt>(last);
            return compound->getLastStmt();
        }
        default:
            return last;
        }
    }
}

