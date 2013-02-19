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

