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

#include <stdio.h>
#include <string.h>

#include "AST/Stmt.h"
#include "AST/Expr.h"
#include "Utils/StringBuilder.h"
#include "Utils/Utils.h"
#include "Utils/color.h"
#include "Utils/constants.h"

using namespace C2;
using namespace std;

//#define STMT_DEBUG
#ifdef STMT_DEBUG
static int creationCount;
static int deleteCount;
#endif

Stmt::Stmt(StmtKind k) : BitsInit(0) {
    StmtBits.sKind = k;
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
    print(buffer, 0);
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

void ReturnStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "ReturnStmt\n";
    if (value) {
        value->print(buffer, indent + INDENT);
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

void IfStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "IfStmt\n";
    SubExprs[COND]->print(buffer, indent + INDENT);
    if (SubExprs[THEN]) SubExprs[THEN]->print(buffer, indent + INDENT);
    if (SubExprs[ELSE]) SubExprs[ELSE]->print(buffer, indent + INDENT);
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

DoStmt::~DoStmt() {
    delete Cond;
    delete Then;
}

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

ForStmt::~ForStmt() {
    delete Body;
    delete Incr;
    delete Cond;
    delete Init;
}

void ForStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "ForStmt\n";
    if (Init) Init->print(buffer, indent + INDENT);
    if (Cond) Cond->print(buffer, indent + INDENT);
    if (Incr) Incr->print(buffer, indent + INDENT);
    Body->print(buffer, indent + INDENT);
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

void SwitchStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "SwitchStmt\n";
    Cond->print(buffer, indent + INDENT);
    for (unsigned i=0; i<Cases.size(); i++) {
        Cases[i]->print(buffer, indent + INDENT);
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

void CaseStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "CaseStmt\n";
    Cond->print(buffer, indent + INDENT);
    for (unsigned i=0; i<Stmts.size(); i++) {
        Stmts[i]->print(buffer, indent + INDENT);
    }
}


DefaultStmt::DefaultStmt(SourceLocation Loc_, StmtList& Stmts_)
    : Stmt(STMT_DEFAULT)
    , Loc(Loc_)
    , Stmts(Stmts_)
{}

DefaultStmt::~DefaultStmt() {}

void DefaultStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "DefaultStmt\n";
    for (unsigned i=0; i<Stmts.size(); i++) {
        Stmts[i]->print(buffer, indent + INDENT);
    }
}


BreakStmt::BreakStmt(SourceLocation Loc_)
    : Stmt(STMT_BREAK)
    , Loc(Loc_)
{}

BreakStmt::~BreakStmt() {}

void BreakStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "BreakStmt\n";
}


ContinueStmt::ContinueStmt(SourceLocation Loc_)
    : Stmt(STMT_BREAK)
    , Loc(Loc_)
{}

ContinueStmt::~ContinueStmt() {}

void ContinueStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "ContinueStmt\n";
}


LabelStmt::LabelStmt(const char* name_, SourceLocation Loc_, Stmt* subStmt_)
    : Stmt(STMT_LABEL)
    , name(name_), Loc(Loc_), subStmt(subStmt_)
{}

LabelStmt::~LabelStmt()
{
    delete subStmt;
}

void LabelStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "LabelStmt\n";
    subStmt->print(buffer, indent + INDENT);
}


GotoStmt::GotoStmt(const char* name_, SourceLocation GotoLoc_, SourceLocation LabelLoc_)
    : Stmt(STMT_GOTO)
    , name(name_), GotoLoc(GotoLoc_), LabelLoc(LabelLoc_)
{}

GotoStmt::~GotoStmt() {}

void GotoStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "GotoStmt\n";
}


CompoundStmt::CompoundStmt(SourceLocation l, SourceLocation r, StmtList& stmts_)
    : Stmt(STMT_COMPOUND)
    , Left(l)
    , Right(r)
    , Stmts(stmts_)
{}

CompoundStmt::~CompoundStmt() {}

void CompoundStmt::print(StringBuilder& buffer, unsigned indent) const {
    buffer.indent(indent);
    buffer.setColor(COL_STMT);
    buffer << "CompoundStmt\n";
    for (unsigned i=0; i<Stmts.size(); i++) {
        Stmts[i]->print(buffer, indent + INDENT);
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

