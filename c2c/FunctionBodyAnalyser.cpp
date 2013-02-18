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
            EnterScope();
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
            analyseCompound(func->getBody());
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

void FunctionBodyAnalyser::EnterScope() {
    curScope = new Scope(globalScope, curScope);
}

void FunctionBodyAnalyser::ExitScope() {
    Scope* parent = curScope->getParent();
    delete curScope;
    curScope = parent;
}

void FunctionBodyAnalyser::analyseCompound(Stmt* stmt) {
    CompoundStmt* compound = StmtCaster<CompoundStmt>::getType(stmt);
    assert(compound);
    const StmtList& stmts = compound->getStmts();
    for (unsigned int i=0; i<stmts.size(); i++) {
        Stmt* S = stmts[i];
        switch (S->stype()) {
        case STMT_RETURN:
            analyseReturnStmt(S);
            break;
        case STMT_EXPR:
            analyseStmtExpr(S);
            break;
        case STMT_IF:
        case STMT_WHILE:
        case STMT_DO:
        case STMT_FOR:
        case STMT_SWITCH:
        case STMT_CASE:
        case STMT_DEFAULT:
        case STMT_BREAK:
        case STMT_CONTINUE:
        case STMT_LABEL:
        case STMT_GOTO:
            break;
        case STMT_COMPOUND:
            EnterScope();
            analyseCompound(S);
            ExitScope();
            break;
        }
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
    case EXPR_CHARLITERAL:
    case EXPR_CALL:
    case EXPR_IDENTIFIER:
    case EXPR_INITLIST:
    case EXPR_TYPE:
        // dont handle here
        break;
    case EXPR_DECL:
        analyseDeclExpr(expr);
        break;
    case EXPR_BINOP:
    case EXPR_UNARYOP:
    case EXPR_SIZEOF:
    case EXPR_ARRAYSUBSCRIPT:
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

