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

#include "FunctionAnalyser.h"
#include "Decl.h"
#include "Expr.h"
#include "Type.h"
#include "Stmt.h"
#include "Package.h"
#include "Scope.h"
#include "color.h"
#include "Utils.h"
#include "StringBuilder.h"

using namespace C2;
using namespace clang;

FunctionAnalyser::FunctionAnalyser(FileScope& scope_,
                                           TypeContext& tc,
                                           clang::DiagnosticsEngine& Diags_)
    : globalScope(scope_)
    , typeContext(tc)
    , scopeIndex(0)
    , curScope(0)
    , Diags(Diags_)
    , errors(0)
{
    Scope* parent = 0;
    for (int i=0; i<MAX_SCOPE_DEPTH; i++) {
        scopes[i].InitOnce(globalScope, parent);
        parent = &scopes[i];
    }
}

FunctionAnalyser::~FunctionAnalyser() {
}

bool FunctionAnalyser::handle(Decl* decl) {
    switch (decl->dtype()) {
    case DECL_FUNC:
        {
            func = DeclCaster<FunctionDecl>::getType(decl);
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

            // check for return statement of return value is required
            Type* rtype = func->getReturnType();
            bool need_rvalue = (rtype != BuiltinType::get(TYPE_VOID));
            if (need_rvalue) {
                CompoundStmt* compound = StmtCaster<CompoundStmt>::getType(func->getBody());
                assert(compound);
                Stmt* lastStmt = compound->getLastStmt();
                if (!lastStmt || lastStmt->stype() != STMT_RETURN) {
                    Diags.Report(compound->getRight(), diag::warn_falloff_nonvoid_function);
                }
            }

            ExitScope();
            func = 0;
        }
        break;
    case DECL_VAR:
        {
            // Bit nasty to analyse Initialization values, but we need a lot of shared code
            VarDecl* VD = DeclCaster<VarDecl>::getType(decl);
            Type* T = VD->getType();
            if (T->isArrayType() && T->getArrayExpr()) {
                EnterScope(0);
                analyseInitExpr(T->getArrayExpr(),  BuiltinType::get(TYPE_INT));
                ExitScope();
            }
            Expr* Init = VD->getInitValue();
            if (Init) {
                EnterScope(0);
                analyseInitExpr(Init, VD->getCanonicalType());
                ExitScope();
            }
        }
        break;
    case DECL_TYPE:
    case DECL_ARRAYVALUE:
    case DECL_USE:
        // nothing to do
        break;
    }
    return false;
}

void FunctionAnalyser::EnterScope(unsigned int flags) {
    assert (scopeIndex < MAX_SCOPE_DEPTH && "out of scopes");
    scopes[scopeIndex].Init(flags);
    curScope = &scopes[scopeIndex];
    scopeIndex++;
}

void FunctionAnalyser::ExitScope() {
    scopeIndex--;
    Scope* parent = curScope->getParent();
    curScope = parent;
}

void FunctionAnalyser::analyseStmt(Stmt* S, bool haveScope) {
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

void FunctionAnalyser::analyseCompoundStmt(Stmt* stmt) {
    CompoundStmt* compound = StmtCaster<CompoundStmt>::getType(stmt);
    assert(compound);
    const StmtList& stmts = compound->getStmts();
    for (unsigned int i=0; i<stmts.size(); i++) {
        analyseStmt(stmts[i]);
    }
}

void FunctionAnalyser::analyseIfStmt(Stmt* stmt) {
    IfStmt* I = StmtCaster<IfStmt>::getType(stmt);
    assert(I);
    Expr* cond = I->getCond();
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

void FunctionAnalyser::analyseWhileStmt(Stmt* stmt) {
    WhileStmt* W = StmtCaster<WhileStmt>::getType(stmt);
    assert(W);
    analyseStmt(W->getCond());
    EnterScope(Scope::BreakScope | Scope::ContinueScope | Scope::DeclScope | Scope::ControlScope);
    analyseStmt(W->getBody(), true);
    ExitScope();

}

void FunctionAnalyser::analyseDoStmt(Stmt* stmt) {
    DoStmt* D = StmtCaster<DoStmt>::getType(stmt);
    assert(D);
    EnterScope(Scope::BreakScope | Scope::ContinueScope | Scope::DeclScope);
    analyseStmt(D->getBody());
    ExitScope();
    analyseStmt(D->getCond());
}

void FunctionAnalyser::analyseForStmt(Stmt* stmt) {
    ForStmt* F = StmtCaster<ForStmt>::getType(stmt);
    assert(F);
    EnterScope(Scope::BreakScope | Scope::ContinueScope | Scope::DeclScope | Scope::ControlScope);
    if (F->getInit()) analyseStmt(F->getInit());
    if (F->getCond()) analyseExpr(F->getCond());
    if (F->getIncr()) analyseExpr(F->getIncr());
    analyseStmt(F->getBody(), true);
    ExitScope();
}

void FunctionAnalyser::analyseSwitchStmt(Stmt* stmt) {
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

void FunctionAnalyser::analyseBreakStmt(Stmt* stmt) {
    if (!curScope->allowBreak()) {
        BreakStmt* B = StmtCaster<BreakStmt>::getType(stmt);
        assert(B);
        Diags.Report(B->getLocation(), diag::err_break_not_in_loop_or_switch);
    }
}

void FunctionAnalyser::analyseContinueStmt(Stmt* stmt) {
    if (!curScope->allowContinue()) {
        ContinueStmt* C = StmtCaster<ContinueStmt>::getType(stmt);
        assert(C);
        Diags.Report(C->getLocation(), diag::err_continue_not_in_loop);
    }
}

void FunctionAnalyser::analyseCaseStmt(Stmt* stmt) {
    CaseStmt* C = StmtCaster<CaseStmt>::getType(stmt);
    assert(C);
    analyseExpr(C->getCond());
    const StmtList& stmts = C->getStmts();
    for (unsigned int i=0; i<stmts.size(); i++) {
        analyseStmt(stmts[i]);
    }
}

void FunctionAnalyser::analyseDefaultStmt(Stmt* stmt) {
    DefaultStmt* D = StmtCaster<DefaultStmt>::getType(stmt);
    assert(D);
    const StmtList& stmts = D->getStmts();
    for (unsigned int i=0; i<stmts.size(); i++) {
        analyseStmt(stmts[i]);
    }
}

void FunctionAnalyser::analyseReturnStmt(Stmt* stmt) {
    ReturnStmt* ret = StmtCaster<ReturnStmt>::getType(stmt);
    assert(ret);
    Expr* value = ret->getExpr();
    Type* rtype = func->getReturnType();
    bool no_rvalue = (rtype == BuiltinType::get(TYPE_VOID));
    if (value) {
        Type* type = analyseExpr(value);
        if (no_rvalue) {
            Diags.Report(ret->getLocation(), diag::ext_return_has_expr) << func->getName() << 0;
            // TODO value->getSourceRange()
        } else {
            // TODO check if type and rtype are compatible
        }
    } else {
        if (!no_rvalue) {
            Diags.Report(ret->getLocation(), diag::ext_return_missing_expr) << func->getName() << 0;
        }
    }
}

void FunctionAnalyser::analyseStmtExpr(Stmt* stmt) {
    Expr* expr = StmtCaster<Expr>::getType(stmt);
    assert(expr);
    analyseExpr(expr);
}

C2::Type* FunctionAnalyser::Decl2Type(Decl* decl) {
    assert(decl);
    switch (decl->dtype()) {
    case DECL_FUNC:
        {
            FunctionDecl* FD = DeclCaster<FunctionDecl>::getType(decl);
            Type* canonical = FD->getCanonicalType();
            assert(canonical && "need function's canonical type");
            return canonical;
        }
        break;
    case DECL_VAR:
        {
            VarDecl* VD = DeclCaster<VarDecl>::getType(decl);
            Type* canonical = VD->getCanonicalType();
            assert(canonical && "need variable's canonical type");
            return canonical;
        }
        break;
    case DECL_TYPE:
        {
            TypeDecl* TD = DeclCaster<TypeDecl>::getType(decl);
            return TD->getType();
        }
        break;
    case DECL_ARRAYVALUE:
    case DECL_USE:
        assert(0);
        break;
    }
    return 0;
}

C2::Type* FunctionAnalyser::analyseExpr(Expr* expr) {
    switch (expr->etype()) {
    case EXPR_NUMBER:
        // TEMP for now always return type int
        return BuiltinType::get(TYPE_INT);
    case EXPR_STRING:
    case EXPR_BOOL:
    case EXPR_CHARLITERAL:
        // TODO return type
        break;
    case EXPR_CALL:
        return analyseCall(expr);
    case EXPR_IDENTIFIER:
        {
            ScopeResult Res = analyseIdentifier(expr);
            if (!Res.ok) return 0;
            if (!Res.decl) return 0;
            if (Res.pkg) {
                IdentifierExpr* id = ExprCaster<IdentifierExpr>::getType(expr);
                id->setPackage(Res.pkg);
            }
            // NOTE: expr should not be package name (handled above)
            return Decl2Type(Res.decl);
        }
        break;
    case EXPR_INITLIST:
    case EXPR_TYPE:
        // dont handle here
        break;
    case EXPR_DECL:
        analyseDeclExpr(expr);
        break;
    case EXPR_BINOP:
        return analyseBinaryOperator(expr);
    case EXPR_CONDOP:
        return analyseConditionalOperator(expr);
    case EXPR_UNARYOP:
        return analyseUnaryOperator(expr);
    case EXPR_SIZEOF:
        analyseSizeofExpr(expr);
        break;
    case EXPR_ARRAYSUBSCRIPT:
        return analyseArraySubscript(expr);
    case EXPR_MEMBER:
        return analyseMemberExpr(expr);
    case EXPR_PAREN:
        return analyseParenExpr(expr);
    }
    return 0;
}

void FunctionAnalyser::analyseInitExpr(Expr* expr, Type* canonical) {
    // TODO compare RHS type with canonical

    switch (expr->etype()) {
    case EXPR_NUMBER:
    case EXPR_STRING:
    case EXPR_BOOL:
    case EXPR_CHARLITERAL:
        // TODO check if compatible
        break;
    case EXPR_CALL:
        // TODO not allowed, give error
        assert(0 && "TODO ERROR");
        break;
    case EXPR_IDENTIFIER:
        {
            ScopeResult Res = analyseIdentifier(expr);
            if (!Res.ok) return;
            if (!Res.decl) return;
            if (Res.pkg) {
                IdentifierExpr* id = ExprCaster<IdentifierExpr>::getType(expr);
                id->setPackage(Res.pkg);
            }
        }
        break;
    case EXPR_INITLIST:
        analyseInitList(expr, canonical);
        break;
    case EXPR_TYPE:
        assert(0 && "??");
        break;
    case EXPR_DECL:
        assert(0 && "TODO ERROR");
        break;
    case EXPR_BINOP:
        analyseBinaryOperator(expr);
        break;
    case EXPR_CONDOP:
        analyseConditionalOperator(expr);
        break;
    case EXPR_UNARYOP:
        analyseUnaryOperator(expr);
        break;
    case EXPR_SIZEOF:
        analyseSizeofExpr(expr);
        break;
    case EXPR_ARRAYSUBSCRIPT:
        analyseArraySubscript(expr);
        break;
    case EXPR_MEMBER:
        // TODO dont allow struct.member, only pkg.constant
        analyseMemberExpr(expr);
        break;
    case EXPR_PAREN:
        analyseParenExpr(expr);
        break;
    }
}

void FunctionAnalyser::analyseInitList(Expr* expr, Type* type) {
    InitListExpr* I = ExprCaster<InitListExpr>::getType(expr);
    assert(I);

    switch (type->getKind()) {
    case Type::USER:
        assert(0 && "should not happen");
        break;
    case Type::STRUCT:
    case Type::UNION:
        {
            MemberList* members = type->getMembers();
            assert(members);
            // check array member type with each value in initlist
            ExprList& values = I->getValues();
            for (unsigned int i=0; i<values.size(); i++) {
                if (i >= members->size()) {
                    // TODO error: 'excess elements in array initializer'
                    return;
                }
                DeclExpr* member = (*members)[i];
                Type* mtype = member->getCanonicalType();
                assert(mtype);
                analyseInitExpr(values[i], mtype);
            }
        }
        break;
    case Type::ARRAY:
        {
            // check array member type with each value in initlist
            ExprList& values = I->getValues();
            for (unsigned int i=0; i<values.size(); i++) {
                analyseInitExpr(values[i], type->getRefType());
            }
        }
        break;
    case Type::QUALIFIER:
        // TODO
        assert(0 && "can happen?");
        break;
    default:
        {
            StringBuilder temp;
            type->DiagName(temp);
        Diags.Report(expr->getExprLoc(), diag::err_invalid_type_initializer_list) << temp;
        }
        break;
    }
}

void FunctionAnalyser::analyseDeclExpr(Expr* expr) {
    DeclExpr* decl = ExprCaster<DeclExpr>::getType(expr);
    assert(decl);

    // check type and convert User types
    Type* type = decl->getType();
    errors += globalScope.checkType(type, false);

    Type* canonicalType = type->getCanonical(typeContext);
    decl->setCanonicalType(canonicalType);

    // check name
    ScopeResult res = curScope->findSymbol(decl->getName());
    if (res.decl) {
        // TODO check other attributes?
        Diags.Report(decl->getLocation(), diag::err_redefinition)
            << decl->getName();
        Diags.Report(res.decl->getLocation(), diag::note_previous_definition);
        return;
    }
    // check initial value
    Expr* initialValue = decl->getInitValue();
    if (initialValue) {
        // TODO check initial value type
        analyseExpr(initialValue);
    }
    curScope->addDecl(new VarDecl(decl, false, true));
}

Type* FunctionAnalyser::analyseBinaryOperator(Expr* expr) {
    BinaryOperator* binop = ExprCaster<BinaryOperator>::getType(expr);
    assert(binop);
    Type* TLeft = analyseExpr(binop->getLHS());
    Type* TRight = analyseExpr(binop->getRHS());
    // assigning to 'A' from incompatible type 'B'
    // diag::err_typecheck_convert_incompatible

    // NOTE: only handle 'a = b' now, not other binary operations
    if (binop->getOpcode() == BO_Assign) {
        return checkAssignmentOperands(TLeft, TRight);
    }
    return 0;
}

Type* FunctionAnalyser::analyseConditionalOperator(Expr* expr) {
    ConditionalOperator* condop = ExprCaster<ConditionalOperator>::getType(expr);
    assert(condop);
    analyseExpr(condop->getCond());
    Type* TLeft = analyseExpr(condop->getLHS());
    analyseExpr(condop->getRHS());
    // TODO also check type of RHS
    return TLeft;
}

Type* FunctionAnalyser::analyseUnaryOperator(Expr* expr) {
    UnaryOperator* unaryop = ExprCaster<UnaryOperator>::getType(expr);
    assert(unaryop);
    Type* LType = analyseExpr(unaryop->getExpr());
    if (!LType) return 0;
    switch (unaryop->getOpcode()) {
    case UO_AddrOf:
        return typeContext.getPointer(LType);
    case UO_Deref:
        // TODO handle user types
        if (!LType->isPointerType()) {
            // TODO use function to get name
            StringBuilder buf;
            buf << '\'';
            LType->printEffective(buf, 0);
            buf << '\'';
            Diags.Report(unaryop->getOpLoc(), diag::err_typecheck_indirection_requires_pointer)
                << buf;
            return 0;
        }
        break;
    default:
        break;
    }
    return LType;
}

void FunctionAnalyser::analyseSizeofExpr(Expr* expr) {
    SizeofExpr* size = ExprCaster<SizeofExpr>::getType(expr);
    assert(size);
    // TODO can also be type
    analyseExpr(size->getExpr());
}

Type* FunctionAnalyser::analyseArraySubscript(Expr* expr) {
    ArraySubscriptExpr* sub = ExprCaster<ArraySubscriptExpr>::getType(expr);
    assert(sub);
    Type* LType = analyseExpr(sub->getBase());
    if (!LType) return 0;
    // TODO this should be done in analyseExpr()
    Type* LType2 = resolveUserType(LType);
    if (!LType2) return 0;
    if (!LType2->isSubscriptable()) {
        Diags.Report(expr->getExprLoc(), diag::err_typecheck_subscript);
        return 0;
    }
    analyseExpr(sub->getIndex());
    return LType2->getRefType();
}

Type* FunctionAnalyser::analyseMemberExpr(Expr* expr) {
    MemberExpr* M = ExprCaster<MemberExpr>::getType(expr);
    assert(M);
    IdentifierExpr* member = M->getMember();

    bool isArrow = M->isArrowOp();
    // Hmm we dont know what we're looking at here, can be:
    // pkg.type
    // pkg.var
    // pkg.func
    // var(Type=struct>.member
    // var[index].member
    // var->member
    // At least check if it exists for now
    Expr* base = M->getBase();
    if (base->etype() == EXPR_IDENTIFIER) {
        ScopeResult SR = analyseIdentifier(base);
        if (!SR.ok) return 0;
        if (SR.decl) {
            IdentifierExpr* base_id = ExprCaster<IdentifierExpr>::getType(base);
            switch (SR.decl->dtype()) {
            case DECL_FUNC:
            case DECL_TYPE:
                fprintf(stderr, "error: member reference base 'type' is not a structure, union or package\n");
                return 0;
            case DECL_VAR:
                {
                    // TODO extract to function?
                    VarDecl* VD = DeclCaster<VarDecl>::getType(SR.decl);
                    Type* T = VD->getType();
                    assert(T);  // analyser should set

                    if (isArrow) {
                        if (T->getKind() != Type::POINTER) {
                            fprintf(stderr, "TODO using -> with non-pointer type\n");
                            // continue analysing
                        } else {
                            // deref
                            T = T->getRefType();
                        }
                    } else {
                        if (T->getKind() == Type::POINTER) {
                            fprintf(stderr, "TODO using . with pointer type\n");
                            // just deref and continue for now
                            T = T->getRefType();
                        }
                    }
                    if (T->getKind() == Type::USER) {
                        T = T->getRefType();
                        assert(T && "analyser should set refType");
                    }
                    // check if struct/union type
                    // TODO do the lookup once during declaration. Just have pointer to real Type here.
                    // This cannot be a User type (but can be struct/union etc)
                    if (!T->isStructOrUnionType()) {
                        // TODO need loc of Op, for now take member
/*
                        Diags.Report(member->getLocation(), diag::err_typecheck_member_reference_struct_union)
                            << T->toString() << M->getSourceRange() << member->getLocation();
*/
                        fprintf(stderr, "error: type of symbol '%s' is not a struct or union\n",
                            base_id->getName().c_str());
                        return 0;
                    }
                    // find member in struct
                    MemberList* members = T->getMembers();
                    for (unsigned i=0; i<members->size(); i++) {
                        DeclExpr* de = (*members)[i];
                        if (de->getName() == member->getName()) { // found
                            return de->getType();
                        }
                    }
                    fprintf(stderr, "error: Type 'todo' has no member '%s'\n", member->getName().c_str());
                    return 0;
                }
                break;
            case DECL_ARRAYVALUE:
            case DECL_USE:
                assert(0);
                break;
            }
        } else if (SR.pkg) {
            if (isArrow) {
                fprintf(stderr, "TODO ERROR: cannot use -> for package access\n");
                // continue checking
            }
            // lookup member in package
            Decl* D = SR.pkg->findSymbol(member->getName());
            if (!D) {
                Diags.Report(member->getLocation(), diag::err_unknown_package_symbol)
                    << SR.pkg->getName() << member->getName();
                return 0;
            }
            if (SR.external && !D->isPublic()) {
                Diags.Report(member->getLocation(), diag::err_not_public)
                    << Utils::fullName(SR.pkg->getName(), D->getName());
                return 0;
            }
            member->setPackage(SR.pkg);
            return Decl2Type(D);
        }
    } else {
        Type* LType = analyseExpr(base);
        if (!LType) return 0;
        // TODO this should be done in analyseExpr()
        Type* LType2 = resolveUserType(LType);
        if (!LType2) return 0;
        if (!LType2->isStructOrUnionType()) {
            fprintf(stderr, "error: not a struct or union type\n");
            LType2->dump();
            return 0;
        }
        // TODO refactor, code below is copied from above
        // find member in struct
        MemberList* members = LType2->getMembers();
        for (unsigned i=0; i<members->size(); i++) {
            DeclExpr* de = (*members)[i];
            if (de->getName() == member->getName()) { // found
                return de->getType();
            }
        }
        fprintf(stderr, "error: Type 'todo' has no member '%s'\n", member->getName().c_str());
        return 0;
    }
    return 0;
}

Type* FunctionAnalyser::analyseParenExpr(Expr* expr) {
    ParenExpr* P = ExprCaster<ParenExpr>::getType(expr);
    assert(P);
    return analyseExpr(P->getExpr());
}

C2::Type* FunctionAnalyser::analyseCall(Expr* expr) {
    CallExpr* call = ExprCaster<CallExpr>::getType(expr);
    assert(call);
    // analyse function
    Type* LType = analyseExpr(call->getFn());
    if (!LType) {
        fprintf(stderr, "CALL unknown function (already error)\n");
        call->getFn()->dump();
        return 0;
    }
    // TODO this should be done in analyseExpr()
    Type* LType2 = resolveUserType(LType);
    if (!LType2) return 0;
    if (!LType2->isFuncType()) {
        fprintf(stderr, "error: NOT a function type TODO\n");
        LType->dump();
        return 0;
    }

    // TODO check if Ellipsoid otherwise compare num args with num params
    for (unsigned i=0; i<call->numArgs(); i++) {
        Expr* arg = call->getArg(i);
        Type* ArgGot = analyseExpr(arg);
        Type* ArgNeed = LType2->getArgument(i);
/*
        fprintf(stderr, "ARG %d:\n", i);
        fprintf(stderr, "  got: ");
        ArgGot->dump();
        fprintf(stderr, "  need: ");
        if (ArgNeed) ArgNeed->dump();
        else fprintf(stderr, "-\n");
*/
        // ..
        // TODO match number + types with proto
    }
    // return function's return type
    return LType2->getReturnType();
}

ScopeResult FunctionAnalyser::analyseIdentifier(Expr* expr) {
    IdentifierExpr* id = ExprCaster<IdentifierExpr>::getType(expr);
    assert(id);
    ScopeResult res = curScope->findSymbol(id->getName());
    if (res.decl) {
        if (res.ambiguous) {
            res.ok = false;
            fprintf(stderr, "TODO ambiguous variable\n");
            // TODO show alternatives
            return res;
        }
        if (!res.visible) {
            res.ok = false;
            Diags.Report(id->getLocation(), diag::err_not_public) << id->getName();
            return res;
        }
    } else {
        if (res.pkg) {
            // symbol is package
        } else {
            res.ok = false;
            // TODO search all packages?
            Diags.Report(id->getLocation(), diag::err_undeclared_var_use)
                << id->getName();
        }
    }
    return res;
}

C2::Type* FunctionAnalyser::checkAssignmentOperands(Type* left, Type* right) {

    // TEMP only check for (int) = (float) for now
    if (right == BuiltinType::get(TYPE_F32) &&
        left == BuiltinType::get(TYPE_U32))
    {
        // implicit conversion turns floating-point number into integer: 'float' to 'Number' (aka 'int')
/*
        SourceLocation loc;
        SourceRange r1;
        SourceRange r2;

        Diags.Report(loc, diag::warn_impcast_float_integer) << "A" << "B" << r1 << r2;
*/
        fprintf(stderr, "ERROR: implicit conversion of floating-point number to integer\n");
        return left;
    }
/*
    SourceLocation loc;
    Diags.Report(loc, diag::err_typecheck_convert_incompatible) << "left" << "right" << "action";
    fprintf(stderr, "LEFT\n");
    left->dump();
    fprintf(stderr, "RIGHT\n");
    right->dump();
*/
    return 0;
}

C2::Type* FunctionAnalyser::resolveUserType(Type* T) {
    if (T->isUserType()) {
        Type* t2 = T->getRefType();
        assert(t2);
        return t2;
    }
    return T;
}

