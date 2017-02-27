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

#include "AST/AST.h"
#include "AST/Decl.h"
#include "AST/Stmt.h"
#include "AST/Expr.h"
#include "Refactor/RefFinder.h"

using namespace C2;

unsigned RefFinder::find() {
    // TODO search more places (types, inits, etc)

    // search Function bodies
    for (unsigned i=0; i<ast.numFunctions(); i++) {
        FunctionDecl* F = ast.getFunction(i);
        searchCompoundStmt(F->getBody());
    }

    return locs.size();
}

void RefFinder::searchStmt(const Stmt* S) {
    assert(S);
    switch (S->getKind()) {
    case STMT_RETURN:
    {
        const ReturnStmt* R = cast<ReturnStmt>(S);
        if (R->getExpr()) searchExpr(R->getExpr());
        break;
    }
    case STMT_EXPR:
        searchExpr(cast<Expr>(S));
        break;
    case STMT_IF:
    {
        const IfStmt* I = cast<IfStmt>(S);
        searchStmt(I->getCond());
        searchStmt(I->getThen());
        if (I->getElse()) searchStmt(I->getElse());
        break;
    }
    case STMT_WHILE:
    {
        const WhileStmt* W = cast<WhileStmt>(S);
        searchStmt(W->getCond());
        searchStmt(W->getBody());
        break;
    }
    case STMT_DO:
    {
        const DoStmt* D = cast<DoStmt>(S);
        searchStmt(D->getCond());
        searchStmt(D->getBody());
        break;
    }
    case STMT_FOR:
    {
        const ForStmt* F = cast<ForStmt>(S);
        if (F->getInit()) searchStmt(F->getInit());
        if (F->getCond()) searchExpr(F->getCond());
        if (F->getIncr()) searchExpr(F->getIncr());
        searchStmt(F->getBody());
        break;
    }
    case STMT_SWITCH:
    {
        const SwitchStmt* SW = cast<SwitchStmt>(S);
        searchStmt(SW->getCond());
        Stmt** Cases = SW->getCases();
        for (unsigned i=0; i<SW->numCases(); i++) {
            searchStmt(Cases[i]);
        }
        break;
    }
    case STMT_CASE:
    {
        const CaseStmt* C = cast<CaseStmt>(S);
        searchExpr(C->getCond());
        Stmt** stmts = C->getStmts();
        for (unsigned i=0; i<C->numStmts(); i++) {
            searchStmt(stmts[i]);
        }
        break;
    }
    case STMT_DEFAULT:
    {
        const DefaultStmt* D = cast<DefaultStmt>(S);
        Stmt** stmts = D->getStmts();
        for (unsigned i=0; i<D->numStmts(); i++) {
            searchStmt(stmts[i]);
        }
        break;
    }
    case STMT_BREAK:
    case STMT_CONTINUE:
    case STMT_LABEL:
    case STMT_GOTO:
        break;
    case STMT_COMPOUND:
        searchCompoundStmt(cast<CompoundStmt>(S));
        break;
    case STMT_DECL:
        // TODO
        //searchVarDecl(cast<DeclStmt>(S)->getDecl());
        break;
    }
}

void RefFinder::searchCompoundStmt(const CompoundStmt* C) {
    Stmt** stmts = C->getStmts();
    for (unsigned i=0; i<C->numStmts(); i++) {
        searchStmt(stmts[i]);
    }
}

void RefFinder::searchExpr(const Expr* E) {
    assert(E);
    switch (E->getKind()) {
    case EXPR_INTEGER_LITERAL:
    case EXPR_FLOAT_LITERAL:
    case EXPR_BOOL_LITERAL:
    case EXPR_CHAR_LITERAL:
    case EXPR_STRING_LITERAL:
    case EXPR_NIL:
        break;
    case EXPR_IDENTIFIER:
    {
        const IdentifierExpr* I = cast<IdentifierExpr>(E);
        if (I->getDecl() == decl) {
            addFileLocation(I->getLocation());
        }
        break;
    }
    case EXPR_TYPE:
        // only in sizeof(int), so no need to search here
        break;
    case EXPR_CALL:
    {
        const CallExpr* C = cast<CallExpr>(E);
        searchExpr(C->getFn());
        for (unsigned i=0; i<C->numArgs(); i++) {
            searchExpr(C->getArg(i));
        }
        break;
    }
    case EXPR_INITLIST:
    {
        const InitListExpr* I = cast<InitListExpr>(E);
        Expr** values = I->getValues();
        for (unsigned i=0; i<I->numValues(); i++) {
            searchExpr(values[i]);
        }
        break;
    }
    case EXPR_DESIGNATOR_INIT:
        break;
    case EXPR_BINOP:
    {
        const BinaryOperator* B = cast<BinaryOperator>(E);
        searchExpr(B->getLHS());
        searchExpr(B->getRHS());
        break;
    }
    case EXPR_CONDOP:
    {
        const ConditionalOperator* C = cast<ConditionalOperator>(E);
        searchExpr(C->getCond());
        searchExpr(C->getLHS());
        searchExpr(C->getRHS());
        break;
    }
    case EXPR_UNARYOP:
        searchExpr(cast<UnaryOperator>(E)->getExpr());
        break;
    case EXPR_BUILTIN:
        searchExpr(cast<BuiltinExpr>(E)->getExpr());
        break;
    case EXPR_ARRAYSUBSCRIPT:
    {
        const ArraySubscriptExpr* A = cast<ArraySubscriptExpr>(E);
        searchExpr(A->getBase());
        searchExpr(A->getIndex());
        break;
    }
    case EXPR_MEMBER:
    {
        const MemberExpr* M = cast<MemberExpr>(E);
        const IdentifierExpr* rhs = M->getMember();
        if (M->getDecl() == decl) {
            addFileLocation(rhs->getLocation());
        }
        break;
    }
    case EXPR_PAREN:
        searchExpr(cast<ParenExpr>(E)->getExpr());
        break;
    case EXPR_BITOFFSET:
    {
        const BitOffsetExpr* B = cast<BitOffsetExpr>(E);
        searchExpr(B->getLHS());
        searchExpr(B->getRHS());
        break;
    }
    break;
    case EXPR_CAST:
        assert(0 && "TODO");
        break;
    }
}

void RefFinder::addFileLocation(clang::SourceLocation loc) {
    if (!loc.isFileID()) return;         // skip macro expansions etc
    locs.push_back(loc);
}

