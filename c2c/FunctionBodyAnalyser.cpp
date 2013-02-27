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

#include <clang/Parse/ParseDiagnostic.h>
#include <clang/Sema/SemaDiagnostic.h>

#include "FunctionBodyAnalyser.h"
#include "Decl.h"
#include "Expr.h"
#include "Type.h"
#include "Stmt.h"
#include "Package.h"
#include "Scope.h"
#include "color.h"

using namespace C2;
using namespace clang;

FunctionBodyAnalyser::FunctionBodyAnalyser(GlobalScope& scope_, clang::DiagnosticsEngine& Diags_)
    : globalScope(scope_)
    , curScope(0)
    , Diags(Diags_)
    , errors(0)
{}

FunctionBodyAnalyser::~FunctionBodyAnalyser() {
}

bool FunctionBodyAnalyser::handle(Decl* decl) {
    switch (decl->dtype()) {
    case DECL_FUNC:
        {
            FunctionDecl* func = DeclCaster<FunctionDecl>::getType(decl);
            assert(func);
            EnterScope(Scope::FnScope | Scope::DeclScope);
            // add arguments to new scope
            // Note: duplicate argument names are already checked by Sema
            for (unsigned i=0; i<func->numArgs(); i++) {
                DeclExpr* de = func->getArg(i);
                // check that argument names dont clash with globals
                ScopeResult res = globalScope.findSymbol(de->getName());
                if (res.decl) {
                    // TODO check other attributes?
                    Diags.Report(de->getLocation(), diag::err_redefinition)
                        << de->getName();
                    Diags.Report(res.decl->getLocation(), diag::note_previous_definition);
                    continue;
                }

                // wrap in VarDecl
                // TODO MEMLEAK in VarDecl -> or throw away in ~Scope() ?
                curScope->addDecl(new VarDecl(de, false, true));
            }
            analyseCompoundStmt(func->getBody());
            ExitScope();
        }
        break;
    case DECL_VAR:
    case DECL_TYPE:
    case DECL_ARRAYVALUE:
    case DECL_USE:
        // nothing to do
        break;
    }
    return false;
}

void FunctionBodyAnalyser::EnterScope(unsigned int flags) {
    curScope = new Scope(globalScope, curScope, flags);
}

void FunctionBodyAnalyser::ExitScope() {
    Scope* parent = curScope->getParent();
    delete curScope;
    curScope = parent;
}

void FunctionBodyAnalyser::analyseStmt(Stmt* S, bool haveScope) {
    switch (S->stype()) {
    case STMT_RETURN:
        analyseReturnStmt(S);
        break;
    case STMT_EXPR:
        analyseStmtExpr(S);
        break;
    case STMT_IF:
        analyseIfStmt(S);
        break;
    case STMT_WHILE:
        analyseWhileStmt(S);
        break;
    case STMT_DO:
        analyseDoStmt(S);
        break;
    case STMT_FOR:
        analyseForStmt(S);
        break;
    case STMT_SWITCH:
        analyseSwitchStmt(S);
        break;
    case STMT_CASE:
    case STMT_DEFAULT:
        assert(0 && "not done here");
        break;
    case STMT_BREAK:
        analyseBreakStmt(S);
        break;
    case STMT_CONTINUE:
        analyseContinueStmt(S);
        break;
    case STMT_LABEL:
    case STMT_GOTO:
        break;
    case STMT_COMPOUND:
        if (!haveScope) EnterScope(Scope::DeclScope);
        analyseCompoundStmt(S);
        if (!haveScope) ExitScope();
        break;
    }
}

void FunctionBodyAnalyser::analyseCompoundStmt(Stmt* stmt) {
    CompoundStmt* compound = StmtCaster<CompoundStmt>::getType(stmt);
    assert(compound);
    const StmtList& stmts = compound->getStmts();
    for (unsigned int i=0; i<stmts.size(); i++) {
        analyseStmt(stmts[i]);
    }
}

void FunctionBodyAnalyser::analyseIfStmt(Stmt* stmt) {
    IfStmt* I = StmtCaster<IfStmt>::getType(stmt);
    assert(I);
    Stmt* condSt = I->getCond();
    Expr* cond = StmtCaster<Expr>::getType(condSt);
    assert(cond);
    analyseExpr(cond);
    EnterScope(Scope::DeclScope);
    analyseStmt(I->getThen(), true);
    ExitScope();

    Stmt* elseSt = I->getElse();
    if (elseSt) {
        EnterScope(Scope::DeclScope);
        analyseStmt(elseSt, true);
        ExitScope();
    }
}

void FunctionBodyAnalyser::analyseWhileStmt(Stmt* stmt) {
    WhileStmt* W = StmtCaster<WhileStmt>::getType(stmt);
    assert(W);
    analyseStmt(W->getCond());
    EnterScope(Scope::BreakScope | Scope::ContinueScope | Scope::DeclScope | Scope::ControlScope);
    analyseStmt(W->getBody(), true);
    ExitScope();

}

void FunctionBodyAnalyser::analyseDoStmt(Stmt* stmt) {
    DoStmt* D = StmtCaster<DoStmt>::getType(stmt);
    assert(D);
    EnterScope(Scope::BreakScope | Scope::ContinueScope | Scope::DeclScope);
    analyseStmt(D->getBody());
    ExitScope();
    analyseStmt(D->getCond());
}

void FunctionBodyAnalyser::analyseForStmt(Stmt* stmt) {
    ForStmt* F = StmtCaster<ForStmt>::getType(stmt);
    assert(F);
    EnterScope(Scope::BreakScope | Scope::ContinueScope | Scope::DeclScope | Scope::ControlScope);
    if (F->getInit()) analyseStmt(F->getInit());
    if (F->getCond()) analyseExpr(F->getCond());
    if (F->getIncr()) analyseExpr(F->getIncr());
    analyseStmt(F->getBody(), true);
    ExitScope();
}

void FunctionBodyAnalyser::analyseSwitchStmt(Stmt* stmt) {
    SwitchStmt* S = StmtCaster<SwitchStmt>::getType(stmt);
    assert(S);
    analyseExpr(S->getCond());
    const StmtList& Cases = S->getCases();
    Stmt* defaultStmt = 0;
    EnterScope(Scope::BreakScope | Scope::SwitchScope);
    for (unsigned i=0; i<Cases.size(); i++) {
        Stmt* C = Cases[i];
        switch (C->stype()) {
        case STMT_CASE:
            analyseCaseStmt(C);
            break;
        case STMT_DEFAULT:
            if (defaultStmt) {
                fprintf(stderr, "multiple defaults TODO\n");
                // TODO need location
                //diag::err_multiple_default_labels_defined
                //diag::note_duplicate_case_prev
            } else {
                defaultStmt = C;
            }
            analyseDefaultStmt(C);
            break;
        default:
            assert(0);
        }
    }
    ExitScope();
}

void FunctionBodyAnalyser::analyseBreakStmt(Stmt* stmt) {
    if (!curScope->allowBreak()) {
        BreakStmt* B = StmtCaster<BreakStmt>::getType(stmt);
        assert(B);
        Diags.Report(B->getLocation(), diag::err_break_not_in_loop_or_switch);
    }
}

void FunctionBodyAnalyser::analyseContinueStmt(Stmt* stmt) {
    if (!curScope->allowContinue()) {
        ContinueStmt* C = StmtCaster<ContinueStmt>::getType(stmt);
        assert(C);
        Diags.Report(C->getLocation(), diag::err_continue_not_in_loop);
    }
}

void FunctionBodyAnalyser::analyseCaseStmt(Stmt* stmt) {
    CaseStmt* C = StmtCaster<CaseStmt>::getType(stmt);
    assert(C);
    analyseExpr(C->getCond());
    const StmtList& stmts = C->getStmts();
    for (unsigned int i=0; i<stmts.size(); i++) {
        analyseStmt(stmts[i]);
    }
}

void FunctionBodyAnalyser::analyseDefaultStmt(Stmt* stmt) {
    DefaultStmt* D = StmtCaster<DefaultStmt>::getType(stmt);
    assert(D);
    const StmtList& stmts = D->getStmts();
    for (unsigned int i=0; i<stmts.size(); i++) {
        analyseStmt(stmts[i]);
    }
}

void FunctionBodyAnalyser::analyseReturnStmt(Stmt* stmt) {
    ReturnStmt* ret = StmtCaster<ReturnStmt>::getType(stmt);
    assert(ret);
    Expr* value = ret->getExpr();
    if (value) analyseExpr(value);
}

void FunctionBodyAnalyser::analyseStmtExpr(Stmt* stmt) {
    Expr* expr = StmtCaster<Expr>::getType(stmt);
    assert(expr);
    analyseExpr(expr);
}

void FunctionBodyAnalyser::analyseExpr(Expr* expr) {
    switch (expr->ntype()) {
    case EXPR_NUMBER:
    case EXPR_STRING:
    case EXPR_BOOL:
    case EXPR_CHARLITERAL:
        break;
    case EXPR_CALL:
        analyseCall(expr);
        break;
    case EXPR_IDENTIFIER:
        analyseIdentifier(expr);
        break;
    case EXPR_INITLIST:
    case EXPR_TYPE:
        // dont handle here
        break;
    case EXPR_DECL:
        analyseDeclExpr(expr);
        break;
    case EXPR_BINOP:
        analyseBinOpExpr(expr);
        break;
    case EXPR_UNARYOP:
        analyseUnaryOpExpr(expr);
        break;
    case EXPR_SIZEOF:
        analyseSizeofExpr(expr);
        break;
    case EXPR_ARRAYSUBSCRIPT:
        analyseArraySubscript(expr);
        break;
    case EXPR_MEMBER:
        // dont handle here
        break;
    }
}

void FunctionBodyAnalyser::analyseDeclExpr(Expr* expr) {
    DeclExpr* decl = ExprCaster<DeclExpr>::getType(expr);
    assert(decl);

    // check type
    Type* type = decl->getType();
    errors += globalScope.checkType(type, false);

    // check name
    // TODO pkg prefixes
    ScopeResult res = curScope->findSymbol(decl->getName());
    if (res.decl) {
        // TODO check other attributes?
        Diags.Report(decl->getLocation(), diag::err_redefinition)
            << decl->getName();
        Diags.Report(res.decl->getLocation(), diag::note_previous_definition);
        return;
    }
    curScope->addDecl(new VarDecl(decl, false, true));
}

void FunctionBodyAnalyser::analyseBinOpExpr(Expr* expr) {
    BinOpExpr* binop = ExprCaster<BinOpExpr>::getType(expr);
    assert(binop);
    analyseExpr(binop->getLeft());
    analyseExpr(binop->getRight());
}

void FunctionBodyAnalyser::analyseUnaryOpExpr(Expr* expr) {
    UnaryOpExpr* unaryop = ExprCaster<UnaryOpExpr>::getType(expr);
    assert(unaryop);
    analyseExpr(unaryop->getExpr());
}

void FunctionBodyAnalyser::analyseSizeofExpr(Expr* expr) {
    SizeofExpr* size = ExprCaster<SizeofExpr>::getType(expr);
    assert(size);
    // TODO can also be type
    analyseExpr(size->getExpr());
}

void FunctionBodyAnalyser::analyseArraySubscript(Expr* expr) {
    ArraySubscriptExpr* sub = ExprCaster<ArraySubscriptExpr>::getType(expr);
    assert(sub);
    analyseExpr(sub->getBase());
    analyseExpr(sub->getIndex());
}

void FunctionBodyAnalyser::analyseCall(Expr* expr) {
    CallExpr* call = ExprCaster<CallExpr>::getType(expr);
    assert(call);
    // analyse function name
    analyseIdentifier(call->getId());
    // analyse arguments
    for (unsigned i=0; i<call->numArgs(); i++) {
        Expr* arg = call->getArg(i);
        analyseExpr(arg);
    }
}

void FunctionBodyAnalyser::analyseIdentifier(Expr* expr) {
    IdentifierExpr* id = ExprCaster<IdentifierExpr>::getType(expr);
    assert(id);
    ScopeResult res = curScope->findSymbol(id->pname, id->name);
    if (id->pname != "" && !res.pkg) {
        // TODO try to fix error (search all packages -> global->fixPackage(id->pname)
        return;
    }
    if (res.decl) {
        if (res.ambiguous) {
            fprintf(stderr, "TODO ambiguous variable\n");
            // TODO show alternatives
            return;
        }
        if (!res.visible) {
            Diags.Report(id->getLocation(), diag::err_not_public) << id->getName();
            return;
        }
    } else {
        if (res.pkg) {
            Diags.Report(id->getLocation(), diag::err_unknown_package_symbol)
                << res.pkg->getName() << id->name;
        } else {
            Diags.Report(id->getLocation(), diag::err_undeclared_var_use)
                << id->getName();
        }
        return;

    }
    // TODO we dont know which type of symbol is allowed here, for now only allow vars
    switch (res.decl->dtype()) {
    case DECL_FUNC:
    case DECL_VAR:
        break;
    case DECL_TYPE:
    case DECL_ARRAYVALUE:
    case DECL_USE:
        fprintf(stderr, "TODO WRONG SYMBOL TYPE\n");
        return;
    }
    // ok
}

