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

Stmt::Stmt() {
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

void Stmt::dump() {
    StringBuilder buffer;
    print(0, buffer);
    fprintf(stderr, "%s\n", (const char*)buffer);
}


ReturnStmt::ReturnStmt(Expr* value_)
    : value(value_)
{}

ReturnStmt::~ReturnStmt() {
    delete value;
}

STMT_VISITOR_ACCEPT(ReturnStmt);

void ReturnStmt::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[return]\n";
    if (value) {
        value->print(indent + INDENT, buffer);
    }
}

void ReturnStmt::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "return";
    if (value) {
        buffer << ' ';
        value->generateC(0, buffer);
    }
    buffer << ";\n";
}


IfStmt::IfStmt(SourceLocation ifLoc,
               Expr* condition, Stmt* thenStmt,
               SourceLocation elseLoc, Stmt* elseStmt)
    : IfLoc(ifLoc)
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

STMT_VISITOR_ACCEPT(IfStmt);

void IfStmt::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[if]\n";
    SubExprs[COND]->print(indent + INDENT, buffer);
    if (SubExprs[THEN]) SubExprs[THEN]->print(indent + INDENT, buffer);
    if (SubExprs[ELSE]) SubExprs[ELSE]->print(indent + INDENT, buffer);
}

void IfStmt::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "if (";
    SubExprs[COND]->generateC(0, buffer);
    buffer << ")\n";
    SubExprs[THEN]->generateC(indent, buffer);
    if (SubExprs[ELSE]) {
        buffer.indent(indent);
        buffer << "else\n";
        SubExprs[ELSE]->generateC(indent, buffer);
    }
}


WhileStmt::WhileStmt(SourceLocation Loc_, Expr* Cond_, Stmt* Then_)
    : Loc(Loc_)
    , Cond(Cond_)
    , Then(Then_)
{}

WhileStmt::~WhileStmt() {
    delete Cond;
    delete Then;
}

STMT_VISITOR_ACCEPT(WhileStmt);

void WhileStmt::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[while]\n";
    Cond->print(indent + INDENT, buffer);
    Then->print(indent + INDENT, buffer);
}

void WhileStmt::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "while (";
    Cond->generateC(0, buffer);
    buffer << ")\n";
    Then->generateC(indent, buffer);
}


DoStmt::DoStmt(SourceLocation Loc_, Expr* Cond_, Stmt* Then_)
    : Loc(Loc_)
    , Cond(Cond_)
    , Then(Then_)
{}

DoStmt::~DoStmt() {
    delete Cond;
    delete Then;
}

STMT_VISITOR_ACCEPT(DoStmt);

void DoStmt::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[do]\n";
    Cond->print(indent + INDENT, buffer);
    Then->print(indent + INDENT, buffer);
}

void DoStmt::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "do\n";
    Then->generateC(indent, buffer);
    buffer.indent(indent);
    buffer << "while (";
    Cond->generateC(0, buffer);
    buffer << ");\n";
}


ForStmt::ForStmt(SourceLocation Loc_, Stmt* Init_, Expr* Cond_, Expr* Incr_, Stmt* Body_)
    : Loc(Loc_)
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

STMT_VISITOR_ACCEPT(ForStmt);

void ForStmt::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[for]\n";
    if (Init) Init->print(indent + INDENT, buffer);
    if (Cond) Cond->print(indent + INDENT, buffer);
    if (Incr) Incr->print(indent + INDENT, buffer);
    Body->print(indent + INDENT, buffer);
}

void ForStmt::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "for (";
    if (Init) {
        Init->generateC(0, buffer);
        buffer.strip('\n');
        buffer.strip(';');
    }
    buffer << ';';
    if (Cond) {
        buffer << ' ';
        Cond->generateC(0, buffer);
    }
    buffer << ';';
    if (Incr) {
        buffer << ' ';
        Incr->generateC(0, buffer);
    }
    buffer << ")\n";
    // TODO fix indentation
    Body->generateC(indent, buffer);
}


SwitchStmt::SwitchStmt(SourceLocation Loc_, Expr* Cond_, StmtList& Cases_)
    : Loc(Loc_)
    , Cond(Cond_)
    , Cases(Cases_)
{}

SwitchStmt::~SwitchStmt() {
    delete Cond;
}

STMT_VISITOR_ACCEPT(SwitchStmt);

void SwitchStmt::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[switch]\n";
    Cond->print(indent + INDENT, buffer);
    for (unsigned int i=0; i<Cases.size(); i++) {
        Cases[i]->print(indent + INDENT, buffer);
    }
}

void SwitchStmt::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "switch(";
    Cond->generateC(0, buffer);
    buffer << ") {\n";
    for (unsigned int i=0; i<Cases.size(); i++) {
        Cases[i]->generateC(indent + INDENT, buffer);
    }
    buffer.indent(indent);
    buffer << "}\n";
}


CaseStmt::CaseStmt(SourceLocation Loc_, Expr* Cond_, StmtList& Stmts_)
    : Loc(Loc_)
    , Cond(Cond_)
    , Stmts(Stmts_)
{}

CaseStmt::~CaseStmt() {
    delete Cond;
}

STMT_VISITOR_ACCEPT(CaseStmt);

void CaseStmt::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[case]\n";
    Cond->print(indent + INDENT, buffer);
    for (unsigned int i=0; i<Stmts.size(); i++) {
        Stmts[i]->print(indent + INDENT, buffer);
    }
}

void CaseStmt::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "case ";
    Cond->generateC(0, buffer);
    buffer << ":\n";
    for (unsigned int i=0; i<Stmts.size(); i++) {
        Stmts[i]->generateC(indent + INDENT, buffer);
    }
}


DefaultStmt::DefaultStmt(SourceLocation Loc_, StmtList& Stmts_)
    : Loc(Loc_)
    , Stmts(Stmts_)
{}

DefaultStmt::~DefaultStmt() {}

STMT_VISITOR_ACCEPT(DefaultStmt);

void DefaultStmt::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[default]\n";
    for (unsigned int i=0; i<Stmts.size(); i++) {
        Stmts[i]->print(indent + INDENT, buffer);
    }
}

void DefaultStmt::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "default:\n";
    for (unsigned int i=0; i<Stmts.size(); i++) {
        Stmts[i]->generateC(indent + INDENT, buffer);
    }
}


BreakStmt::BreakStmt(SourceLocation Loc_) : Loc(Loc_) {}

BreakStmt::~BreakStmt() {}

STMT_VISITOR_ACCEPT(BreakStmt);

void BreakStmt::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[break]\n";
}

void BreakStmt::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "break;\n";
}


ContinueStmt::ContinueStmt(SourceLocation Loc_) : Loc(Loc_) {}

ContinueStmt::~ContinueStmt() {}

STMT_VISITOR_ACCEPT(ContinueStmt);

void ContinueStmt::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[continue]\n";
}

void ContinueStmt::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "continue;\n";
}


LabelStmt::LabelStmt(const char* name_, SourceLocation Loc_, Stmt* subStmt_)
    : name(name_), Loc(Loc_), subStmt(subStmt_) {}

LabelStmt::~LabelStmt()
{
    delete subStmt;
}

STMT_VISITOR_ACCEPT(LabelStmt);

void LabelStmt::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[label]\n";
}

void LabelStmt::generateC(int indent, StringBuilder& buffer) {
    buffer << name << ":\n";
    subStmt->generateC(indent, buffer);
}


GotoStmt::GotoStmt(const char* name_, SourceLocation GotoLoc_, SourceLocation LabelLoc_)
    : name(name_), GotoLoc(GotoLoc_), LabelLoc(LabelLoc_) {}

GotoStmt::~GotoStmt() {}

STMT_VISITOR_ACCEPT(GotoStmt);

void GotoStmt::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[goto]\n";
}

void GotoStmt::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "goto " << name << ";\n";
}


CompoundStmt::CompoundStmt(SourceLocation l, SourceLocation r, StmtList& stmts_)
    : Left(l)
    , Right(r)
    , Stmts(stmts_)
{}

CompoundStmt::~CompoundStmt() {}

STMT_VISITOR_ACCEPT(CompoundStmt);

void CompoundStmt::print(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "[compound]\n";
    for (unsigned int i=0; i<Stmts.size(); i++) {
        Stmts[i]->print(indent + INDENT, buffer);
    }
}

void CompoundStmt::generateC(int indent, StringBuilder& buffer) {
    buffer.indent(indent);
    buffer << "{\n";
    for (unsigned int i=0; i<Stmts.size(); i++) {
        Stmts[i]->generateC(indent + INDENT, buffer);
    }
    buffer.indent(indent);
    buffer << "}\n";
}

