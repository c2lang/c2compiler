/* Copyright 2013-2022 Bas van den Berg
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
#include <assert.h>

#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/APInt.h>
#include "Clang/ParseDiagnostic.h"
#include "Clang/SemaDiagnostic.h"

#include "Analyser/FunctionAnalyser.h"
#include "Analyser/TypeResolver.h"
#include "Analyser/CTVAnalyser.h"
#include "Analyser/Scope.h"
#include "Analyser/AnalyserConstants.h"
#include "Analyser/AnalyserUtils.h"
#include "AST/Expr.h"
#include "Utils/color.h"
#include "Utils/TargetInfo.h"
#include "Utils/StringBuilder.h"

using namespace C2;
using namespace c2lang;

//#define ANALYSER_DEBUG

#ifdef ANALYSER_DEBUG

#include <iostream>
#define LOG_FUNC std::cerr << ANSI_MAGENTA << __func__ << "()" << ANSI_NORMAL << "\n";
#else
#define LOG_FUNC
#endif

#define MIN(a, b) ((a < b) ? a : b)

#define LHS 0x01
#define RHS 0x02

static bool isCharPtr(QualType t) {
    QualType Q = t.getCanonicalType();
    const PointerType* pt = dyncast<PointerType>(Q);
    if (!pt) return false;
    const QualType inner = pt->getPointeeType();
    return inner.isCharType();
}

FunctionAnalyser::FunctionAnalyser(Scope& scope_,
                                   TypeResolver& typeRes_,
                                   ASTContext& context_,
                                   c2lang::DiagnosticsEngine& Diags_,
                                   const TargetInfo& target_,
                                   bool isInterface_)
    : scope(scope_)
    , TR(typeRes_)
    , Context(context_)
    , EA(Diags_, target_, context_)
    , Diags(Diags_)
    , target(target_)
    , CurrentFunction(0)
    , CurrentVarDecl(0)
    , constDiagID(0)
    , inConstExpr(false)
    , usedPublicly(false)
    , isInterface(isInterface_)
    , fallthrough(0)
    , allowStaticMember(false)
{
    callStack.callDepth = 0;
}

void FunctionAnalyser::check(FunctionDecl* func) {
    CurrentFunction = func;
    scope.EnterScope(Scope::FnScope | Scope::DeclScope);

    checkFunction(func);

    scope.ExitScope();

    // check labels
    for (LabelsIter iter = labels.begin(); iter != labels.end(); ++iter) {
        LabelDecl* LD = *iter;
        if (LD->getStmt()) {    // have label part
            if (!LD->isUsed()) {
                Diag(LD->getLocation(), diag::warn_unused_label) << LD->DiagName();
            }
        } else {    // only have goto part
            Diag(LD->getLocation(), diag:: err_undeclared_label_use) << LD->DiagName();
        }
    }
    labels.clear();

    CurrentFunction = 0;
}

void FunctionAnalyser::checkFunction(FunctionDecl* func) {
    LOG_FUNC
    bool no_unused_params = func->hasAttribute(ATTR_UNUSED_PARAMS);
    if (isInterface) no_unused_params = true;
    // add arguments to new scope
    for (unsigned i=0; i<func->numArgs(); i++) {
        VarDecl* arg = func->getArg(i);

        if (no_unused_params) arg->setUsed();
        if (!arg->hasEmptyName()) {
            // check that argument names dont clash with globals
            if (!scope.checkScopedSymbol(arg)) {
                continue;
            }
            scope.addScopedSymbol(arg);
        }
    }
    if (scope.hasErrorOccurred()) return;

    CompoundStmt* body = func->getBody();
    if (body) {
        analyseCompoundStmt(body);
    }
    if (scope.hasErrorOccurred()) return;

    // check for return statement of return value is required
    QualType rtype = func->getReturnType();
    bool need_rvalue = (rtype.getTypePtr() != BuiltinType::get(BuiltinType::Void));
    if (need_rvalue && body) {
        Stmt* lastStmt = body->getLastStmt();
        if (!lastStmt || lastStmt->getKind() != STMT_RETURN) {
            Diag(body->getRight(), diag::warn_falloff_nonvoid_function);
        }
    }
}

bool FunctionAnalyser::analyseStmt(Stmt* S, bool haveScope) {
    LOG_FUNC
    switch (S->getKind()) {
    case STMT_RETURN:
        return analyseReturnStmt(S);
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
    case STMT_SSWITCH:
        analyseSSwitchStmt(S);
        break;
    case STMT_CASE:
    case STMT_DEFAULT:
        FATAL_ERROR("Unreachable"); // Done at other place.
        break;
    case STMT_BREAK:
        return analyseBreakStmt(S);
    case STMT_CONTINUE:
        return analyseContinueStmt(S);
    case STMT_FALLTHROUGH:
        return analyseFallthroughStmt(S);
    case STMT_LABEL:
        return analyseLabelStmt(S);
    case STMT_GOTO:
        analyseGotoStmt(S);
        break;
    case STMT_COMPOUND:
        if (!haveScope) scope.EnterScope(Scope::DeclScope);
        analyseCompoundStmt(S);
        if (!haveScope) scope.ExitScope();
        break;
    case STMT_DECL:
        analyseDeclStmt(S);
        break;
    case STMT_ASM:
        analyseAsmStmt(S);
        break;
    case STMT_ASSERT:
        analyseAssertStmt(S);
        break;
    }
    return true;
}

void FunctionAnalyser::analyseCompoundStmt(Stmt* stmt) {
    LOG_FUNC
    assert(CurrentFunction);
    CompoundStmt* compound = cast<CompoundStmt>(stmt);
    Stmt** stmts = compound->getStmts();
    for (unsigned i=0; i<compound->numStmts(); i++) {
        analyseStmt(stmts[i]);
        if (scope.hasErrorOccurred()) return;
    }
}

void FunctionAnalyser::analyseIfStmt(Stmt* stmt) {
    LOG_FUNC
    IfStmt* I = cast<IfStmt>(stmt);

    scope.EnterScope(Scope::DeclScope);
    analyseCondition(I->getCond());

    scope.EnterScope(Scope::DeclScope);
    analyseStmt(I->getThen(), true);
    scope.ExitScope();

    if (I->getElse()) {
        scope.EnterScope(Scope::DeclScope);
        analyseStmt(I->getElse(), true);
        scope.ExitScope();
    }
    scope.ExitScope();
}

void FunctionAnalyser::analyseWhileStmt(Stmt* stmt) {
    LOG_FUNC
    WhileStmt* W = cast<WhileStmt>(stmt);
    scope.EnterScope(Scope::DeclScope);
    analyseStmt(W->getCond());
    scope.EnterScope(Scope::BreakScope | Scope::ContinueScope | Scope::DeclScope | Scope::ControlScope);
    analyseStmt(W->getBody(), true);
    scope.ExitScope();
    scope.ExitScope();
}

void FunctionAnalyser::analyseDoStmt(Stmt* stmt) {
    LOG_FUNC
    DoStmt* D = cast<DoStmt>(stmt);
    scope.EnterScope(Scope::BreakScope | Scope::ContinueScope | Scope::DeclScope);
    analyseStmt(D->getBody());
    scope.ExitScope();
    analyseStmt(D->getCond());
}

void FunctionAnalyser::analyseForStmt(Stmt* stmt) {
    LOG_FUNC
    ForStmt* F = cast<ForStmt>(stmt);
    scope.EnterScope(Scope::BreakScope | Scope::ContinueScope | Scope::DeclScope | Scope::ControlScope);
    if (F->getInit()) analyseStmt(F->getInit());

    Expr** cond = F->getCond2();
    if (cond) {
        QualType CT = analyseExpr(cond, RHS, true);
        if (!CT.isScalarType()) {
            StringBuilder buf(64);
            CT.DiagName(buf);
            Diag(F->getCond()->getLocation(), diag::err_typecheck_statement_requires_scalar) << buf;
        }
    }

    Expr** incr = F->getIncr2();
    if (incr) analyseExpr(incr, RHS, true);
    analyseStmt(F->getBody(), true);
    scope.ExitScope();
}

void FunctionAnalyser::analyseSwitchStmt(Stmt* stmt) {
    LOG_FUNC
    SwitchStmt* S = cast<SwitchStmt>(stmt);

    scope.EnterScope(Scope::DeclScope);
    if (!analyseCondition(S->getCond())) return;

    QualType ct;
    Stmt* c = S->getCond();
    if (isa<DeclStmt>(c)) {
        DeclStmt* DS = cast<DeclStmt>(c);
        VarDecl* decl = DS->getDecl();
        ct = decl->getType();
    } else {
        Expr* e = dyncast<Expr>(c);
        assert(e);
        ct = e->getType();
    }
    if (isCharPtr(ct)) {
        Diags.Report(c->getLocation(), diag::err_switch_sswitch_suggest);
        return;
    }

    const EnumType* ET = dyncast<EnumType>(ct.getTypePtr());
    fallthrough = 0;
    unsigned numCases = S->numCases();
    Stmt** cases = S->getCases();
    Stmt* defaultStmt = 0;
    scope.EnterScope(Scope::BreakScope | Scope::SwitchScope);

    bool ok = true;
    for (unsigned i=0; i<numCases; i++) {
        Stmt* C = cases[i];
        switch (C->getKind()) {
        case STMT_CASE:
            ok &= analyseCaseStmt(C, ET);
            break;
        case STMT_DEFAULT:
            if (defaultStmt) {
                Diag(C->getLocation(), diag::err_multiple_default_labels_defined);
                //Diag(defaultStmt->getLocation(), diag::note_duplicate_case_prev);
            } else {
                fallthrough = 0;
                defaultStmt = C;
            }
            analyseDefaultStmt(C, true);
            if (i + 1 != numCases) {
                Diag(C->getLocation(), diag::err_case_default_not_last) << 0;
            }
            break;
        default:
            FATAL_ERROR("Unreachable");
        }
    }

    if (fallthrough) {
        Diag(fallthrough->getLocation(), diag::err_fallthrough_last_case);
    }

    fallthrough = 0;
    if (S->numCases() == 0) {
        Diag(S->getLocation(), diag::err_empty_switch);
    }
    if (ok && ET) checkEnumCases(S, ET);

    scope.ExitScope();
    scope.ExitScope();
}

void FunctionAnalyser::analyseSSwitchStmt(Stmt* stmt) {
    LOG_FUNC
    SSwitchStmt* S = cast<SSwitchStmt>(stmt);

    Expr** cond_ptr = S->getCond2();
    QualType Q = analyseExpr(cond_ptr, RHS, true);
    Expr* cond = *cond_ptr;
    if (!Q.isValid()) return;
    if (!isCharPtr(cond->getType())) {
        StringBuilder buf1(MAX_LEN_TYPENAME);
        cond->getType().DiagName(buf1);
        Diags.Report(cond->getLocation(), diag::err_illegal_type_conversion) << buf1 << "'i8*'";
        return;
    }

    scope.EnterScope(Scope::DeclScope);

    unsigned numCases = S->numCases();
    Stmt** cases = S->getCases();
    Stmt* defaultStmt = 0;
    scope.EnterScope(Scope::BreakScope | Scope::SwitchScope);
    for (unsigned i=0; i<numCases; i++) {
        Stmt* C = cases[i];
        switch (C->getKind()) {
        case STMT_CASE:
            analyseSSwitchCaseStmt(C);
            break;
        case STMT_DEFAULT:
            if (defaultStmt) {
                Diag(C->getLocation(), diag::err_sswitch_multiple_default_labels_defined);
                //Diag(defaultStmt->getLocation(), diag::note_duplicate_case_prev);
            } else {
                defaultStmt = C;
            }
            analyseDefaultStmt(C, false);
            if (i + 1 != numCases) {
                Diag(C->getLocation(), diag::err_case_default_not_last) << 1;
            }
            break;
        default:
            FATAL_ERROR("Unreachable");
        }
    }

    if (S->numCases() == 0) {
        Diag(S->getLocation(), diag::err_empty_sswitch);
    }
    // TODO check for duplicates (strings + nils)

    scope.ExitScope();
    scope.ExitScope();
}

bool FunctionAnalyser::analyseBreakStmt(Stmt* stmt) {
    LOG_FUNC
    if (!scope.allowBreak()) {
        BreakStmt* B = cast<BreakStmt>(stmt);
        Diag(B->getLocation(), diag::err_break_not_in_loop_or_switch);
        return false;
    }
    return true;
}

bool FunctionAnalyser::analyseContinueStmt(Stmt* stmt) {
    LOG_FUNC
    if (!scope.allowContinue()) {
        ContinueStmt* C = cast<ContinueStmt>(stmt);
        Diag(C->getLocation(), diag::err_continue_not_in_loop);
        return false;
    }
    return true;
}

bool FunctionAnalyser::analyseFallthroughStmt(Stmt* stmt) {
    LOG_FUNC
    FallthroughStmt* B = cast<FallthroughStmt>(stmt);
    if (fallthrough) return false;

    if (!scope.allowFallthrough()) {
        Diag(B->getLocation(), diag::err_fallthrough_not_in_switch);
        return false;
    }
    fallthrough = B;
    return true;
}

bool FunctionAnalyser::analyseLabelStmt(Stmt* S) {
    LOG_FUNC
    LabelStmt* L = cast<LabelStmt>(S);

    LabelDecl* LD = LookupOrCreateLabel(L->getName(), L->getLocation());
    if (LD->getStmt()) {
        Diag(L->getLocation(), diag::err_redefinition_of_label) <<  LD->DiagName();
        Diag(LD->getLocation(), diag::note_previous_definition);
        return false;
    } else {
        LD->setStmt(L);
        LD->setLocation(L->getLocation());
    }

    if (!analyseStmt(L->getSubStmt())) return false;
    // substmt cannot be declaration

    if (isa<DeclStmt>(L->getSubStmt())) {
        Diag(L->getSubStmt()->getLocation(), diag::err_decl_after_label);
        return false;
    }
    return true;
}

void FunctionAnalyser::analyseGotoStmt(Stmt* S) {
    LOG_FUNC
    GotoStmt* G = cast<GotoStmt>(S);
    IdentifierExpr* label = G->getLabel();
    LabelDecl* LD = LookupOrCreateLabel(label->getName(), label->getLocation());
    label->setDecl(LD, IdentifierExpr::REF_LABEL);
    LD->setUsed();
}

static bool isCaseTerminator(const Stmt* S) {
    switch (S->getKind()) {
    case STMT_RETURN:
        return true;
    case STMT_EXPR:
    {
        const Expr* E = cast<Expr>(S);
        const CallExpr* C = dyncast<CallExpr>(E);
        if (!C) return false;
        const FunctionType* FT = cast<FunctionType>(C->getFn()->getType().getTypePtr());
        const FunctionDecl* FD = FT->getDecl();
        if (FD->hasNoReturnAttr()) return true;
        if (!FD->hasAttributes()) return false;
        return false;
    }
    case STMT_BREAK:
    case STMT_CONTINUE:
    case STMT_FALLTHROUGH:
    case STMT_GOTO:
        return true;
    default:
        return false;
    }
}

bool FunctionAnalyser::analyseCaseStmt(Stmt* stmt, const EnumType* ET) {
    LOG_FUNC
    scope.EnterScope(Scope::DeclScope | Scope::SwitchScope);
    CaseStmt* C = cast<CaseStmt>(stmt);
    // TODO check return value?

    Expr* cond = C->getCond();
    bool ok = true;
    if (ET) {
        // only allow Identifier when switching on EnumType
        IdentifierExpr* id = dyncast<IdentifierExpr>(cond);
        if (id) {
            EnumTypeDecl* ETD = ET->getDecl();
            EnumConstantDecl* ECD = ETD->findConstant(id->getName());
            if (ECD) {
                id->setDecl(ECD, IdentifierExpr::REF_ENUM_CONSTANT);
                id->setType(ECD->getType().getCanonicalType());
                id->setCTC();
                id->setCTV(true);
                id->setIsRValue();
            } else {
                StringBuilder buf(MAX_LEN_TYPENAME);
                ETD->fullName(buf);
                Diag(id->getLocation(), diag::err_unknown_enum_constant)
                    << buf << id->getName();
                ok = false;
            }
        } else {
            if (isa<MemberExpr>(cond)) {
                Diag(cond->getLocation(), diag::err_prefixed_enum_constant);
            } else {
                Diag(cond->getLocation(), diag::err_not_enum_constant);
            }
            ok = false;
        }
    } else {
        analyseExpr(C->getCond2(), RHS, true);
    }

    Stmt** stmts = C->getStmts();
    fallthrough = 0;
    for (unsigned i=0; i<C->numStmts(); i++) {
        analyseStmt(stmts[i]);
    }
    const Stmt* last = 0;
    if (C->numStmts()) last = stmts[C->numStmts() -1];
    if (fallthrough && last != fallthrough) {
        Diag(fallthrough->getLocation(), diag::err_fallthrough_not_last);
    } else if (!last || !isCaseTerminator(last)) {
        Diag(last ? last->getLocation() : C->getLocation(), diag::err_switch_case_no_termination) << 0;
    }
    C->setHasDecls(scope.hasDecls());
    scope.ExitScope();
    return ok;
}

void FunctionAnalyser::analyseSSwitchCaseStmt(Stmt* stmt) {
    LOG_FUNC
    CaseStmt* C = cast<CaseStmt>(stmt);

    Expr* cond = C->getCond();
    switch (cond->getKind()) {
    case EXPR_STRING_LITERAL:
    case EXPR_NIL:
        break;
    default:
        Diag(cond->getLocation(), diag::err_sswitch_case_no_string);
        return;
    }

    scope.EnterScope(Scope::DeclScope);
    Stmt** stmts = C->getStmts();
    for (unsigned i=0; i<C->numStmts(); i++) {
        analyseStmt(stmts[i]);
    }
    C->setHasDecls(scope.hasDecls());
    scope.ExitScope();
}

void FunctionAnalyser::analyseDefaultStmt(Stmt* stmt, bool isSwitch) {
    LOG_FUNC
    scope.EnterScope(Scope::DeclScope);
    DefaultStmt* D = cast<DefaultStmt>(stmt);
    Stmt** stmts = D->getStmts();
    for (unsigned i=0; i<D->numStmts(); i++) {
        analyseStmt(stmts[i]);
    }
    if (isSwitch) {
        const Stmt* last = 0;
        if (D->numStmts()) last = stmts[D->numStmts() -1];
        if (!last || !isCaseTerminator(last)) {
            Diag(last ? last->getLocation() : D->getLocation(), diag::err_switch_case_no_termination) << 1;
        }
    }
    D->setHasDecls(scope.hasDecls());
    scope.ExitScope();
}

bool FunctionAnalyser::analyseReturnStmt(Stmt* stmt) {
    LOG_FUNC
    ReturnStmt* ret = cast<ReturnStmt>(stmt);
    Expr** value_ptr = ret->getExpr2();
    QualType rtype = CurrentFunction->getReturnType();
    bool no_rvalue = (rtype.getTypePtr() == Type::Void());
    if (value_ptr) {
        Expr* value = *value_ptr;
        QualType type = analyseExpr(value_ptr, RHS, true);
        if (no_rvalue) {
            Diag(ret->getLocation(), diag::ext_return_has_expr) << CurrentFunction->getName() << 0
                    << value->getSourceRange();
            return false;
        }
        if (type.isValid()) {
            EA.check(rtype, *value_ptr);
            if (EA.hasError()) return false;
        }
    } else {
        if (!no_rvalue) {
            Diag(ret->getLocation(), diag::ext_return_missing_expr) << CurrentFunction->getName() << 0;
            return false;
        }
    }
    return true;
}

void FunctionAnalyser::analyseDeclStmt(Stmt* stmt) {
    LOG_FUNC
    DeclStmt* DS = cast<DeclStmt>(stmt);
    VarDecl* decl = DS->getDecl();

    scope.setHasDecls();
    // TODO refactor haveError to return
    bool haveError = false;
    QualType Q = analyseType(decl->getType(), decl->getLocation());
    if (Q.isValid()) {
        decl->setType(Q);

        if (!isInterface) {
            if (!islower(decl->getName()[0])) {
                Diags.Report(decl->getLocation(), diag::err_upper_casing) << 1;
            }
        }

        if (Q.isArrayType()) {
            // TODO use Helper-function to get ArrayType (might be AliasType)
            ArrayType* AT = cast<ArrayType>(Q.getTypePtr());
            Expr* sizeExpr = AT->getSizeExpr();
            if (!sizeExpr && !decl->getInitValue()) {
                Diag(decl->getLocation(), diag::err_typecheck_incomplete_array_needs_initializer);
                haveError = true;
            }
        }
        TR.checkOpaqueType(decl->getLocation(), false, Q);

        if (!haveError && Q.isIncompleteType()) {
            StringBuilder name;
            Q.DiagName(name);
            Diag(decl->getLocation(), diag::err_typecheck_decl_incomplete_type) << name;
            haveError = true;
        }
    } else {
        haveError = true;
    }

    // check name
    if (!scope.checkScopedSymbol(decl)) return;

    // check initial value
    Expr** initialValue = decl->getInitValue2();
    if (initialValue && !haveError) {
        CurrentVarDecl = decl;
        analyseInitExpr(initialValue, decl->getType());
        CurrentVarDecl = 0;
    }

    if (Q.isConstQualified() && !initialValue) {
        Diag(decl->getLocation(), diag::err_uninitialized_const_var) << decl->getName();
    }
    scope.addScopedSymbol(decl);
}

void FunctionAnalyser::analyseAsmStmt(Stmt* stmt) {
    LOG_FUNC
    AsmStmt* A = cast<AsmStmt>(stmt);
    // outputs
    for (unsigned i=0; i<A->getNumOutputs(); ++i) {
        Expr** e = A->getOutputExpr2(i);
        analyseExpr(e, LHS, true);
    }
    // inputs
    for (unsigned i=0; i<A->getNumInputs(); ++i) {
        Expr** e = A->getInputExpr2(i);
        analyseExpr(e, RHS, true);
    }
    // clobbers
    for (unsigned i=0; i<A->getNumClobbers(); ++i) {
        StringLiteral** c = A->getClobber2(i);
        analyseExpr((Expr**)c, 0, true);
    }
}

void FunctionAnalyser::analyseAssertStmt(Stmt* stmt) {
    LOG_FUNC
    AssertStmt* A = cast<AssertStmt>(stmt);
    Expr** expr_ptr = A->getExpr2();
    QualType Q = analyseExpr(expr_ptr, RHS, true);
    if (!Q.isValid()) return;
    EA.check(Type::Bool(), A->getExpr());  // BB Not valid
}

bool FunctionAnalyser::analyseCondition(Stmt* stmt) {
    LOG_FUNC

    if (isa<DeclStmt>(stmt)) {
        analyseDeclStmt(stmt);
    } else {
        assert(isa<Expr>(stmt));
        Expr* E = cast<Expr>(stmt);
        // NOTE: cannot insert ImplicitCastExpr here!
        QualType Q1 = analyseExpr(&E, RHS, true);
        if (!Q1.isValid()) return false;
        EA.check(Type::Bool(), E);  // BB Not valid
    }
    return true;
}

void FunctionAnalyser::analyseStmtExpr(Stmt* stmt) {
    LOG_FUNC
    Expr* expr = cast<Expr>(stmt);
    // NOTE: cannot insert ImplicitCastExpr here!
    analyseExpr(&expr, 0, false);
}

C2::QualType FunctionAnalyser::analyseExpr(Expr** expr_ptr, unsigned side, bool need_rvalue) {
    LOG_FUNC
    QualType Result = analyseExprInner(expr_ptr, side);
    if (!Result.isValid()) return Result;
    Expr* expr = *expr_ptr;
    expr->setType(Result);

    if (need_rvalue && expr->isLValue()) {
        // TODO just Result?
        QualType Q = expr->getType();
        if (Q.isArrayType()) {
            QualType Qptr = AnalyserUtils::getPointerFromArray(Context, Q);
            insertImplicitCast(CK_ArrayToPointerDecay, expr_ptr, Qptr);
            return Qptr;
        } else {
            insertImplicitCast(CK_LValueToRValue, expr_ptr, Q);
        }
    }

    return Result;
}

C2::QualType inline FunctionAnalyser::analyseExprInner(Expr** expr_ptr, unsigned side) {
    LOG_FUNC

    Expr* expr = *expr_ptr;

    switch (expr->getKind()) {
    case EXPR_INTEGER_LITERAL:
        return EA.analyseIntegerLiteral(expr);
    case EXPR_FLOAT_LITERAL:
        // For now always return type float
        return Type::Float32();
    case EXPR_BOOL_LITERAL:
        return Type::Bool();
    case EXPR_CHAR_LITERAL:
        return Type::Int8();
    case EXPR_STRING_LITERAL:
    {
        StringLiteral* s = cast<StringLiteral>(expr);
        int len = s->getByteLength();
        QualType Q = Context.getArrayType(Type::Int8(), s->getByteLength());
        Q.addConst();
        QualType Ptr = Context.getPointerType(Type::Int8());
        Q->setCanonicalType(Ptr);
        return Q;
    }
    case EXPR_NIL:
    {
        QualType Q = Context.getPointerType(Type::Void());
        if (!Q->hasCanonicalType()) Q->setCanonicalType(Q);
        return Q;
    }
    case EXPR_CALL:
        return analyseCall(expr);
    case EXPR_IDENTIFIER:
        return analyseIdentifierExpr(expr_ptr, side);
    case EXPR_INITLIST:
    case EXPR_DESIGNATOR_INIT:
    case EXPR_TYPE:
        // dont handle here
        break;
    case EXPR_BINOP:
        return analyseBinaryOperator(expr, side);
    case EXPR_CONDOP:
        return analyseConditionalOperator(expr);
    case EXPR_UNARYOP:
        return analyseUnaryOperator(expr_ptr, side);
    case EXPR_BUILTIN:
        return analyseBuiltinExpr(expr);
    case EXPR_ARRAYSUBSCRIPT:
        return analyseArraySubscript(expr, side);
    case EXPR_MEMBER:
        return analyseMemberExpr(expr_ptr, side);
    case EXPR_PAREN:
        return analyseParenExpr(expr);
    case EXPR_BITOFFSET:
        FATAL_ERROR("Unreachable");
        break;
    case EXPR_EXPLICIT_CAST:
        return analyseExplicitCastExpr(expr);
    case EXPR_IMPLICIT_CAST:
        return analyseImplicitCastExpr(expr);
    }
    return QualType();
}

void FunctionAnalyser::analyseInitExpr(Expr** expr_ptr, QualType expectedType) {
    LOG_FUNC

    Expr* expr = *expr_ptr;
    InitListExpr* ILE = dyncast<InitListExpr>(expr);

    // FIXME: expectedType has no canonicalType yet!
    const ArrayType* AT = dyncast<ArrayType>(expectedType.getCanonicalType());
    if (AT) {
        const QualType ET = AT->getElementType().getCanonicalType();
        bool isCharArray = (ET == Type::Int8() || ET == Type::UInt8());

        if (isCharArray) {
            if (!ILE && !isa<StringLiteral>(expr)) {
                Diag(expr->getLocation(), diag::err_array_init_not_init_list) << 1;
                return;
            }
        } else {
            if (!ILE) {
                Diag(expr->getLocation(), diag::err_array_init_not_init_list) << 0;
                return;
            }
        }
    }

    if (ILE) {
        analyseInitList(ILE, expectedType);
    } else if (isa<DesignatedInitExpr>(expr)) {
        analyseDesignatorInitExpr(expr, expectedType);
    } else {
        QualType Q = analyseExpr(expr_ptr, RHS, true);
        expr = *expr_ptr; // Can have changed
        if (!Q.isValid()) return;

        if (inConstExpr && !expr->isCTC()) {
            assert(constDiagID);
            Diag(expr->getLocation(), constDiagID) << expr->getSourceRange();
        } else {
            EA.check(expectedType, expr);
        }
        if (AT && AT->getSizeExpr()) {
            // it should be char array type already and expr is string literal
            assert(isa<StringLiteral>(expr));
            const StringLiteral* S = cast<StringLiteral>(expr);
            if (S->getByteLength() > (int)AT->getSize().getZExtValue()) {
                Diag(S->getLocation(), diag::err_initializer_string_for_char_array_too_long) << S->getSourceRange();
                return;
            }
        }
    }
}

void FunctionAnalyser::analyseInitListArray(InitListExpr* expr, QualType Q, unsigned numValues, Expr** values) {
    LOG_FUNC
    bool haveDesignators = false;
    bool ok = true;
    // TODO use helper function
    ArrayType* AT = cast<ArrayType>(Q.getCanonicalType().getTypePtr());
    QualType ET = AT->getElementType();
    bool constant = true;
    for (unsigned i=0; i<numValues; i++) {
        analyseInitExpr(&values[i], ET);
        if (DesignatedInitExpr* D = dyncast<DesignatedInitExpr>(values[i])) {
            haveDesignators = true;
            if (D->getDesignatorKind() != DesignatedInitExpr::ARRAY_DESIGNATOR) {
                StringBuilder buf;
                Q.DiagName(buf);
                Diag(D->getLocation(), diag::err_field_designator_non_aggr) << 0 << buf;
                ok = false;
                continue;
            }
        }
        if (!values[i]->isCTC()) constant = false;
    }

    if (constant) expr->setCTC();
    if (haveDesignators) expr->setDesignators();
    // determine real array size
    llvm::APInt initSize(64, 0, false);
    if (haveDesignators && ok) {
        int64_t arraySize = -1;
        if (AT->getSizeExpr()) {    // size determined by expr
            arraySize = AT->getSize().getZExtValue();
        } //else, size determined by designators
        checkArrayDesignators(expr, &arraySize);
        initSize = arraySize;
    } else {
        if (AT->getSizeExpr()) {    // size determined by expr
            initSize = AT->getSize();
            uint64_t arraySize = AT->getSize().getZExtValue();
            if (numValues > arraySize) {
                int firstExceed = AT->getSize().getZExtValue();
                Diag(values[firstExceed]->getLocation(), diag::err_excess_initializers) << 0;
            }
        } else {    // size determined from #elems in initializer list
            initSize = numValues;
        }
    }
    AT->setSize(initSize);
    expr->setType(Q);
}

// We have an expression of the type .foo
// Return true to continue analysis, false to exit.
bool FunctionAnalyser::analyseFieldInDesignatedInitExpr(DesignatedInitExpr* D,
                                                        StructTypeDecl* STD,
                                                        QualType Q,
                                                        Fields &fields,
                                                        Expr* value,
                                                        bool &haveDesignators) {

    IdentifierExpr* field = cast<IdentifierExpr>(D->getField());
    // TODO can by member of anonymous sub-struct/union
    int memberIndex = STD->findIndex(field->getName());
    int anonUnionIndex = -1;
    StructTypeDecl *anonUnionDecl;
    // Check for matching a sub field of an anonymous union.
    if (memberIndex == -1) {
        // Let's get the anon field if available.
        memberIndex = STD->findIndex("");
        if (memberIndex > -1) {
            // Goto analysis of sub field.
            anonUnionDecl = dyncast<StructTypeDecl>(STD->getMember((unsigned)memberIndex));
            if (anonUnionDecl) {
                anonUnionIndex = anonUnionDecl->findIndex(field->getName());
            } else {
                memberIndex = -1;
            }
        }
    }

    // No match of direct value or anon union.
    if (memberIndex == -1) {
        StringBuilder fname(MAX_LEN_VARNAME);
        AnalyserUtils::quotedField(fname, field);
        StringBuilder tname(MAX_LEN_TYPENAME);
        Q.DiagName(tname);
        Diag(D->getLocation(), diag::err_field_designator_unknown) << fname << tname;
        return true;
    }


    Expr* existing = fields[memberIndex];
    if (existing) {
        StringBuilder fname(MAX_LEN_VARNAME);
        AnalyserUtils::quotedField(fname, field);
        Diag(D->getLocation(), diag::err_duplicate_field_init) << fname;
        Diag(existing->getLocation(), diag::note_previous_initializer) << 0 << 0;
        return true;
    }
    fields[memberIndex] = value;
    VarDecl* VD;
    if (anonUnionIndex > -1) {
        field->setDecl(anonUnionDecl, AnalyserUtils::globalDecl2RefKind(anonUnionDecl));
        VD = dyncast<VarDecl>(anonUnionDecl->getMember((unsigned)anonUnionIndex));
    } else {
        VD = dyncast<VarDecl>(STD->getMember((unsigned)memberIndex));
    }
    if (!VD) {
        TODO;
        assert(VD && "TEMP don't support sub-struct member inits");
    }
    field->setDecl(VD, AnalyserUtils::globalDecl2RefKind(VD));
    Expr** initValue = D->getInitValue2();
    analyseInitExpr(initValue, VD->getType());
    if (anonUnionIndex < 0 && isa<DesignatedInitExpr>(value) != haveDesignators) {
        Diag(value->getLocation(), diag::err_mixed_field_designator);
        return false;
    }
    return true;
}

void FunctionAnalyser::analyseInitListStruct(InitListExpr* expr, QualType Q, unsigned numValues, Expr** values) {
    LOG_FUNC
    bool haveDesignators = false;
    expr->setType(Q);
    // TODO use helper function
    StructType* TT = cast<StructType>(Q.getCanonicalType().getTypePtr());
    StructTypeDecl* STD = TT->getDecl();
    bool constant = true;
    // ether init whole struct with field designators, or don't use any (no mixing allowed)
    // NOTE: using different type for anonymous sub-union is allowed
    if (numValues != 0 && isa<DesignatedInitExpr>(values[0])) {
        haveDesignators = true;
    }

    // Check if this is a union, if so then we need designators when initializing.
    if (numValues > 0 && !haveDesignators && !STD->isStruct()) {
        Diag(values[0]->getLocation(), diag::err_field_designator_required_union);
        return;
    }

    // TODO cleanup this code (after unit-tests) Split into field-designator / non-designator init
    Fields fields;
    fields.resize(STD->numMembers());
    for (unsigned i = 0; i < numValues; i++) {
        if (i >= STD->numMembers()) {
            // note: 0 for array, 2 for scalar, 3 for union, 4 for structs
            Diag(values[STD->numMembers()]->getLocation(), diag::err_excess_initializers)
                << 4;
            return;
        }
        DesignatedInitExpr* D = dyncast<DesignatedInitExpr>(values[i]);
        if (!D) {
            VarDecl* VD = dyncast<VarDecl>(STD->getMember(i));
            if (!VD) {
                TODO;
            }
            analyseInitExpr(&values[i], VD->getType());
            if (!values[i]->isCTC()) constant = false;
            if (isa<DesignatedInitExpr>(values[i]) != haveDesignators) {
                Diag(values[i]->getLocation(), diag::err_mixed_field_designator);
                return;
            }
            continue;
        }
        switch (D->getDesignatorKind()) {
            case DesignatedInitExpr::ARRAY_DESIGNATOR:
            {
                StringBuilder buf;
                Q.DiagName(buf);
                Diag(D->getLocation(), diag::err_array_designator_non_array) << buf;
                return;
            }
            case DesignatedInitExpr::FIELD_DESIGNATOR:
                if (!analyseFieldInDesignatedInitExpr(D, STD, Q, fields, values[i], haveDesignators)) return;
                break;
        }
    }
    if (constant) expr->setCTC();
}

void FunctionAnalyser::analyseInitList(InitListExpr* expr, QualType Q) {
    LOG_FUNC
    Expr** values = expr->getValues();
    unsigned numValues = expr->numValues();
    if (Q.isArrayType()) {
        analyseInitListArray(expr, Q, numValues, values);
        return;
    }
    if (Q.isStructType()) {
        analyseInitListStruct(expr, Q, numValues, values);
        return;
    }
    // TODO always give error like case 1?
    // only allow 1
    switch (numValues) {
        case 0:
            fprintf(stderr, "TODO ERROR: scalar initializer cannot be empty\n");
            TODO;
            break;
        case 1:
        {
            StringBuilder buf;
            Q.DiagName(buf);
            Diag(expr->getLocation(), diag::err_invalid_initlist_init) << buf << expr->getSourceRange();
            break;
        }
        default:
            Diag(values[1]->getLocation(), diag::err_excess_initializers) << 2;
            break;
    }
    return;
}

void FunctionAnalyser::analyseDesignatorInitExpr(Expr* expr, QualType expectedType) {
    LOG_FUNC
    DesignatedInitExpr* D = cast<DesignatedInitExpr>(expr);
    if (D->getDesignatorKind() == DesignatedInitExpr::ARRAY_DESIGNATOR) {
        Expr** Desig_ptr = D->getDesignator2();
        Expr* Desig=  *Desig_ptr;
        QualType DT = analyseExpr(Desig_ptr, RHS, true);
        if (DT.isValid()) {
            if (!Desig->isCTC()) {
                assert(constDiagID);
                Diag(Desig->getLocation(), constDiagID) << Desig->getSourceRange();
            } else {
                if (!Desig->getType().isIntegerType()) {
                    Diag(Desig->getLocation(), diag::err_typecheck_subscript_not_integer) << Desig->getSourceRange();
                } else {
                    CTVAnalyser LA(Diags);
                    llvm::APSInt V = LA.checkLiterals(Desig);
                    if (V.isSigned() && V.isNegative()) {
                        Diag(Desig->getLocation(), diag::err_array_designator_negative) << V.toString(10) << Desig->getSourceRange();
                    } else {
                        D->setIndex(V);
                    }
                }
            }
        }
    } else {
        // cannot check here, need structtype
    }

    analyseInitExpr(D->getInitValue2(), expectedType);
    D->syncFlags(D->getInitValue());

    D->setType(expectedType);
}

QualType FunctionAnalyser::analyseSizeOfExpr(BuiltinExpr* B) {
    LOG_FUNC

    static constexpr unsigned FLOAT_SIZE_DEFAULT = 8;

    uint64_t width;
    allowStaticMember = true;

    Expr** expr_ptr = B->getExpr2();
    Expr* expr = *expr_ptr;

    switch (expr->getKind()) {
    case EXPR_FLOAT_LITERAL:
        width = FLOAT_SIZE_DEFAULT;
        break;
    case EXPR_INITLIST:
    case EXPR_DESIGNATOR_INIT:
        TODO; // Good error
        allowStaticMember = false;
        return QualType();
    case EXPR_TYPE:
    {
        // TODO can be public?
        QualType Q = analyseType(expr->getType(), expr->getLocation());
        if (!Q.isValid()) return QualType();
        // TODO here we want to use expr->getLocation()
        // But that does not properly find the right location!
        TR.checkOpaqueType(B->getLocation(), false, Q);
        TypeSize result = AnalyserUtils::sizeOfType(Q);
        width = result.size;
        break;
    }
    case EXPR_INTEGER_LITERAL:
    case EXPR_BOOL_LITERAL:
    case EXPR_CHAR_LITERAL:
    case EXPR_STRING_LITERAL:
    case EXPR_NIL:
    case EXPR_CALL:
    case EXPR_BINOP:
    case EXPR_CONDOP:
    case EXPR_UNARYOP:
    case EXPR_ARRAYSUBSCRIPT:
    case EXPR_MEMBER:
    case EXPR_IDENTIFIER:
    case EXPR_EXPLICIT_CAST: {
        QualType type = analyseExpr(expr_ptr, RHS, false);
        if (!type.isValid()) {
            allowStaticMember = false;
            return QualType();
        }
        TR.checkOpaqueType(expr->getLocation(), true, type);
        TypeSize result = AnalyserUtils::sizeOfType(type);
        width = result.size;
        break;
    }
    case EXPR_IMPLICIT_CAST:
        TODO;
        break;
    case EXPR_BUILTIN:
        FATAL_ERROR("Unreachable");
        allowStaticMember = false;
        return QualType();
    case EXPR_PAREN:
    case EXPR_BITOFFSET:
        FATAL_ERROR("Unreachable");
        allowStaticMember = false;
        return QualType();
    }
    B->setValue(llvm::APSInt::getUnsigned(width));
    allowStaticMember = false;
    return Type::UInt32();
}

QualType FunctionAnalyser::analyseElemsOfExpr(BuiltinExpr* B) {
    LOG_FUNC

    Expr** E_ptr = B->getExpr2();
    Expr* orig = B->getExpr();
    QualType T = analyseExpr(E_ptr, RHS, false);
    // use orig to skip ArrayToPointerDecay
    T = orig->getType();

    llvm::APSInt i(32, 1);
    const ArrayType* AT = dyncast<ArrayType>(T.getCanonicalType());
    if (AT) {
        i = AT->getSize();
        B->setValue(i);
        return Type::UInt32();
    }
    const EnumType* ET = dyncast<EnumType>(T);  // NOTE: dont use canonicalType!
    if (ET) {
        EnumTypeDecl* ETD = ET->getDecl();
        i = ETD->numConstants();
        B->setValue(i);
        return Type::UInt32();
    }
    Expr* E = *E_ptr;
    Diag(E->getLocation(), diag::err_elemsof_no_array) << E->getSourceRange();
    return QualType();
}

QualType FunctionAnalyser::analyseEnumMinMaxExpr(BuiltinExpr* B, bool isMin) {
    LOG_FUNC
    Expr** E_ptr = B->getExpr2();
    Expr* E = *E_ptr;
    // TODO support memberExpr (module.Type)
    assert(isa<IdentifierExpr>(E));
    IdentifierExpr* I = cast<IdentifierExpr>(E);
    Decl* decl = analyseIdentifier(I);
    if (!decl) return QualType();

    EnumTypeDecl* Enum = 0;
    decl->setUsed();

    // = Type::UInt32();
    switch (decl->getKind()) {
    case DECL_VAR:
        if (const EnumType* ET = dyncast<EnumType>(decl->getType())) {
            Enum = ET->getDecl();
        }
        break;
    case DECL_ENUMTYPE:
        Enum = cast<EnumTypeDecl>(decl);
        break;
    default:
        break;
    }

    if (!Enum) {
        Diag(E->getLocation(), diag::err_enum_minmax_no_enum) << (isMin ? 0 : 1) << E->getSourceRange();
        return QualType();
    }

    if (isMin) B->setValue(Enum->getMinValue());
    else B->setValue(Enum->getMaxValue());

    return decl->getType();
}

StructTypeDecl* FunctionAnalyser::builtinExprToStructTypeDecl(BuiltinExpr* B) {
    Expr* structExpr = B->getExpr();
    Decl* structDecl = 0;
    // TODO scope.checkAccess(std, structExpr->getLocation());

    IdentifierExpr* I = dyncast<IdentifierExpr>(structExpr);
    if (I) {
        structDecl = analyseIdentifier(I);
        if (!structDecl) return 0;
    } else {    // MemberExpr: module.StructType
        assert(isa<MemberExpr>(structExpr));
        MemberExpr* M = cast<MemberExpr>(structExpr);
        Expr* base = M->getBase();
        IdentifierExpr* B = dyncast<IdentifierExpr>(base);
        assert(B);  // Don't allow substructs as type
        Decl* D = analyseIdentifier(B);
        if (!D) return 0;

        ImportDecl* ID = dyncast<ImportDecl>(D);
        if (!ID) {
            Diag(B->getLocation(), diag::err_unknown_module) << B->getName() << B->getSourceRange();
            return 0;
        }
        M->setModulePrefix();
        I = M->getMember();
        structDecl = scope.findSymbolInModule(I->getName(), I->getLocation(), ID->getModule());
        if (!structDecl) return 0;

        M->setDecl(structDecl);
        M->setType(D->getType());
        AnalyserUtils::SetConstantFlags(structDecl, M);
        QualType Q = structDecl->getType();
        I->setType(Q);
        I->setDecl(structDecl, AnalyserUtils::globalDecl2RefKind(structDecl));
    }
    structDecl->setUsed();

    StructTypeDecl* std = dyncast<StructTypeDecl>(structDecl);
    if (!std) {
        int idx = (B->getBuiltinKind() == BuiltinExpr::BUILTIN_OFFSETOF) ? 0 : 1;
        Diag(I->getLocation(), diag::err_builtin_non_struct_union) << idx << I->getName();
        return 0;
    }

    if (currentModule() != std->getModule() && std->hasAttribute(ATTR_OPAQUE)) {
        Expr* structExpr = B->getExpr();
        Diag(structExpr->getLocation(), diag::err_deref_opaque) << std->isStruct() << std->DiagName() << structExpr->getSourceRange();
        return 0;
    }
    return std;
}

QualType FunctionAnalyser::analyseOffsetOf(BuiltinExpr* B) {
    LOG_FUNC

    StructTypeDecl* std = builtinExprToStructTypeDecl(B);
    if (!std) return QualType();

    uint64_t off = 0;
    EA.analyseOffsetOf(B, std, B->getMember(), &off);
    return Type::UInt32();
}

QualType FunctionAnalyser::analyseToContainer(BuiltinExpr* B) {
    LOG_FUNC

    // check struct type
    StructTypeDecl* std = builtinExprToStructTypeDecl(B);
    if (!std) return QualType();
    QualType ST = std->getType();

    // check member
    // TODO try re-using code with analyseMemberExpr() or analyseStructMember()
    Expr* memberExpr = B->getMember();
    IdentifierExpr* member = dyncast<IdentifierExpr>(memberExpr);
    if (!member) { TODO; } // TODO support sub-member (memberExpr)

    Decl* match = std->findMember(member->getName());
    if (!match) {
        return EA.outputStructDiagnostics(ST, member, diag::err_no_member);
    }
    member->setDecl(match, IdentifierExpr::REF_STRUCT_MEMBER);
    QualType MT = match->getType();
    member->setType(MT);

    // check ptr
    Expr** ptrExpr_ptr = B->getPointer2();
    Expr* ptrExpr = *ptrExpr_ptr;
    QualType ptrType = analyseExpr(ptrExpr_ptr, RHS, false);
    if (!ptrType.isPointerType()) {
        EA.error(ptrExpr, Context.getPointerType(member->getType()), ptrType);
        return QualType();
    }
    QualType PT = cast<PointerType>(ptrType)->getPointeeType();
    // TODO BB allow conversion from void*
    // TODO BB use ExprAnalyser to do and insert casts etc
    if (PT != MT) {
        EA.error(ptrExpr, Context.getPointerType(member->getType()), ptrType);
        return QualType();
    }

    // if ptr is const qualified, tehn so should resulting Type (const Type*)
    if (PT.isConstQualified()) ST.addConst();
    QualType Q = Context.getPointerType(ST);
    return Q;
}

QualType FunctionAnalyser::analyseType(QualType Q, SourceLocation loc) {
    // NOTE: almost same as FileAnalyser::analyseType()
    LOG_FUNC

    if (Q->hasCanonicalType()) return Q;    // should be ok already

    QualType resolved;
    Type* T = Q.getTypePtr();

    switch (T->getTypeClass()) {
    case TC_BUILTIN:
        FATAL_ERROR("should not come here");
        break;
    case TC_POINTER:
    {
        // Dont return new type if not needed
        const PointerType* P = cast<PointerType>(T);
        QualType t1 = P->getPointeeType();
        QualType Result = analyseType(t1, loc);
        if (!Result.isValid()) return QualType();
        if (t1 == Result) {
            resolved = Q;
            Q->setCanonicalType(Q);
        } else {
            resolved = Context.getPointerType(Result);
        }
        break;
    }
    case TC_ARRAY:
    {
        ArrayType* AT = cast<ArrayType>(T);
        if (AT->isIncremental()) {
            Diag(loc, diag::err_incremental_array_function_scope);
            return QualType();
        }
        QualType ET = analyseType(AT->getElementType(), loc);
        if (!ET.isValid()) return QualType();

        Expr* sizeExpr = AT->getSizeExpr();
        if (sizeExpr) {
            if (!analyseArraySizeExpr(AT)) return QualType();
        }

        if (ET == AT->getElementType()) {
            resolved = Q;
            Q->setCanonicalType(Q);
        } else {
            resolved = Context.getArrayType(ET, AT->getSizeExpr(), AT->isIncremental());
            resolved->setCanonicalType(resolved);
            if (sizeExpr) {
                ArrayType* RA = cast<ArrayType>(resolved.getTypePtr());
                RA->setSize(AT->getSize());
            }
        }

        // TODO qualifiers
        break;
    }
    case TC_REF:
        resolved = analyseRefType(Q);
        break;
    case TC_ALIAS:
        resolved = analyseType(cast<AliasType>(T)->getRefType(), loc);
        break;
    case TC_STRUCT:
    case TC_ENUM:
    case TC_FUNCTION:
    case TC_MODULE:
        FATAL_ERROR("should not come here");
        break;
    }

    // NOTE: CanonicalType is always resolved (ie will not point to another AliasType, but to its Canonicaltype)
    if (!Q->hasCanonicalType()) {
        if (!resolved.isValid()) return QualType();

        if (resolved.getCanonicalType().getTypePtrOrNull() == NULL) {
            resolved.dump();
            FATAL_ERROR("missing canonical on resolved");
        }

        Q->setCanonicalType(resolved.getCanonicalType());
    }

    return resolved;
}

QualType FunctionAnalyser::analyseRefType(QualType Q) {
    LOG_FUNC

    const Type* T = Q.getTypePtr();
    const RefType* RT = cast<RefType>(T);
    IdentifierExpr* moduleName = RT->getModuleName();
    IdentifierExpr* typeName = RT->getTypeName();
    SourceLocation tLoc = typeName->getLocation();
    const std::string& tName = typeName->getName();

    Decl* D = 0;
    if (moduleName) { // mod.type
        const std::string& mName = moduleName->getName();
        const Module* mod = scope.findUsedModule(mName, moduleName->getLocation(), false);
        if (!mod) return QualType();
        Decl* modDecl = scope.findSymbol(mName, moduleName->getLocation(), true, false);
        assert(modDecl);
        moduleName->setDecl(modDecl, IdentifierExpr::REF_MODULE);

        D =  scope.findSymbolInModule(tName, tLoc, mod);
    } else { // type
        D = scope.findSymbol(tName, tLoc, true, false);
    }
    if (!D) return QualType();
    TypeDecl* TD = dyncast<TypeDecl>(D);
    if (!TD) {
        StringBuilder name;
        RT->printLiteral(name);
        Diags.Report(tLoc, diag::err_not_a_typename) << name.c_str();
        return QualType();
    }

    D->setUsed();
    typeName->setDecl(TD, IdentifierExpr::REF_TYPE);

    QualType result = TD->getType();
    if (Q.isConstQualified()) result.addConst();
    if (Q.isVolatileQualified()) result.addVolatile();
    return result;
}

bool FunctionAnalyser::analyseArraySizeExpr(ArrayType* AT) {
    LOG_FUNC

    QualType T = analyseExpr(AT->getSizeExpr2(), RHS, false);
    if (!T.isValid()) return false;

    Expr* sizeExpr = AT->getSizeExpr();

    // check if type is integer
    QualType CT = T.getCanonicalType();
    if (!CT.isBuiltinType() || !cast<BuiltinType>(CT)->isInteger()) {
        StringBuilder buf;
        T.DiagName(buf, false);
        Diag(sizeExpr->getLocation(), diag::err_array_size_non_int) << buf << sizeExpr->getSourceRange();
        return false;
    }

    if (!sizeExpr->isCTV()) {
        Diag(sizeExpr->getLocation(), diag::err_array_size_non_const);
        return false;
    }

    // check if negative
    CTVAnalyser LA(Diags);
    llvm::APSInt value = LA.checkLiterals(sizeExpr);

    if (value == 0) {
        Diag(sizeExpr->getLocation(), diag::err_array_size_zero);
        return false;
    }

    if (value.isSigned() && value.isNegative()) {
        Diag(sizeExpr->getLocation(), diag::err_typecheck_negative_array_size) << value.toString(10) << sizeExpr->getSourceRange();
        return false;
    }
    AT->setSize(value);
    return true;
}

QualType FunctionAnalyser::analyseBinaryOperator(Expr* expr, unsigned side) {
    LOG_FUNC
    BinaryOperator* binop = cast<BinaryOperator>(expr);
    QualType TLeft, TRight;
    Expr** Left_ptr = binop->getLHS2();
    // TODO dont read Left/Right here, can change due to inserted casts
    Expr* Left = NULL;
    Expr** Right_ptr = binop->getRHS2();
    Expr* Right = NULL;

    switch (binop->getOpcode()) {
    case BINOP_Mul:
    case BINOP_Div:
    case BINOP_Rem:
    case BINOP_Add:
    case BINOP_Sub:
    case BINOP_Shl:
    case BINOP_Shr:
        // RHS, RHS
        TLeft = analyseExpr(Left_ptr, RHS, true);
        TRight = analyseExpr(Right_ptr, RHS, true);
        Left = binop->getLHS();
        Right = binop->getRHS();
        expr->combineFlags(Left, Right);
        break;
    case BINOP_LE:
    case BINOP_LT:
    case BINOP_GE:
    case BINOP_GT:
    case BINOP_NE:
    case BINOP_EQ:
        // RHS, RHS
        TLeft = analyseExpr(Left_ptr, RHS, true);
        TRight = analyseExpr(Right_ptr, RHS, true);
        // NOTE: CTC is never full, because value is not interesting, only type
        Left = binop->getLHS();
        Right = binop->getRHS();
        expr->combineFlags(Left, Right);
        break;
    case BINOP_And:
    case BINOP_Xor:
    case BINOP_Or:
    case BINOP_LAnd:
    case BINOP_LOr:
        // TODO check if cast of SubExpr is ok
        // RHS, RHS
        TLeft = analyseExpr(Left_ptr, RHS, true);
        TRight = analyseExpr(Right_ptr, RHS, true);
        Left = binop->getLHS();
        Right = binop->getRHS();
        expr->combineFlags(Left, Right);
        break;
    case BINOP_Assign:
        // LHS, RHS
        TLeft = analyseExpr(Left_ptr, side | LHS, false);
        TRight = analyseExpr(Right_ptr, RHS, true);
        break;
    case BINOP_MulAssign:
    case BINOP_DivAssign:
    case BINOP_RemAssign:
    case BINOP_AddAssign:
    case BINOP_SubAssign:
    case BINOP_ShlAssign:
    case BINOP_ShrAssign:
    case BINOP_AndAssign:
    case BINOP_XorAssign:
    case BINOP_OrAssign:
        // LHS|RHS, RHS
        TLeft = analyseExpr(Left_ptr, LHS | RHS, false);
        TRight = analyseExpr(Right_ptr, RHS, true);
        break;
    case BINOP_Comma:
        FATAL_ERROR("unhandled binary operator type");
        break;
    }

    // TODO remove this check?
    if (TLeft.isNull() || TRight.isNull()) return QualType();

    Left = binop->getLHS();
    Right = binop->getRHS();

    switch (binop->getOpcode()) {
    case BINOP_Assign:
    {
        assert(TRight.isValid());
        checkAssignment(Left, TLeft, "left operand of assignment", binop->getLocation());
        if (scope.hasErrorOccurred()) return QualType();
        // special case for a[4:2] = b
        if (AnalyserUtils::isConstantBitOffset(Left) && Right->isCTC()) {
            CTVAnalyser LA(Diags);
            LA.checkBitOffset(Left, Right);
        } else {
            EA.check(TLeft, *Right_ptr);
            if (scope.hasErrorOccurred()) return QualType();
        }
        break;
    }
    case BINOP_MulAssign:
    case BINOP_DivAssign:
    case BINOP_RemAssign:
    case BINOP_AddAssign:
    case BINOP_SubAssign:
    case BINOP_ShlAssign:
    case BINOP_ShrAssign:
    case BINOP_AndAssign:
    case BINOP_XorAssign:
    case BINOP_OrAssign:
        checkAssignment(Left, TLeft, "left operand of assignment", binop->getLocation());
        break;
    default:
        break;
    }

    return EA.getBinOpType(binop);
}

QualType FunctionAnalyser::checkCondionalOperatorTypes(QualType TLeft, QualType TRight) {
    if (TLeft.isNull() || TRight.isNull()) return QualType();

    bool lptr = TLeft.isPointerType();
    bool rptr = TRight.isPointerType();
    if (lptr || rptr) {
        if (!lptr || !rptr || !EA.arePointersCompatible(TLeft, TRight)) return QualType();
        return TLeft;
    }
    bool lb = TLeft.isBuiltinType();
    bool rb = TRight.isBuiltinType();
    if (lb || rb) {
        if (!rb || ! lb) return QualType();
        return EA.LargestType(TLeft, TRight);
    }
    return TLeft;
}

QualType FunctionAnalyser::analyseConditionalOperator(Expr* expr) {
    LOG_FUNC
    ConditionalOperator* C = cast<ConditionalOperator>(expr);
    QualType TCond = analyseExpr(C->getCond2(), RHS, true);
    if (!TCond.isValid()) return QualType();

    // check if Condition can be casted to bool
    EA.check(Type::Bool(), C->getCond());
    QualType TLeft = analyseExpr(C->getLHS2(), RHS, true);
    QualType TRight = analyseExpr(C->getRHS2(), RHS, true);
    if (!TLeft.isValid() || !TRight.isValid()) return QualType();

    if (C->getCond()->isCTV() && C->getLHS()->isCTV() && C->getRHS()->isCTV()) {
        expr->setCTV(true);
    }

    QualType Res = checkCondionalOperatorTypes(TLeft, TRight);
    if (!Res.isValid()) {
        EA.Diag2(C->getLHS()->getLocation(), diag::err_typecheck_invalid_ternary_operands, TLeft, TRight) << C->getRHS()->getLocation();
    }
    return Res;
}

QualType FunctionAnalyser::analyseUnaryOperator(Expr** expr_ptr, unsigned side) {
    LOG_FUNC
    Expr* expr = *expr_ptr;
    UnaryOperator* unaryop = cast<UnaryOperator>(expr);
    Expr** SubExpr_ptr = unaryop->getExpr2();
    bool need_rvalue = true;

    switch (unaryop->getOpcode()) {
    case UO_PostInc:
    case UO_PostDec:
    case UO_PreInc:
    case UO_PreDec:
    case UO_AddrOf:
        need_rvalue = false; // these need lvalue
        side |= LHS; // these need lvalue
        break;
    case UO_Deref:
    case UO_Minus:
    case UO_Not:
    case UO_LNot:
        side |= RHS;
        break;
    }

    QualType LType = analyseExpr(SubExpr_ptr, side, need_rvalue);
    if (!LType.isValid()) return LType;
    SubExpr_ptr = unaryop->getExpr2();  // re-read in case casts were inserted
    Expr* SubExpr = *SubExpr_ptr; // TODO re-use expr?

    if (LType.isNull()) return QualType();
    if (LType.isVoidType()) {
        Diag(unaryop->getOpLoc(), diag::err_typecheck_unary_expr) << "'void'" << SubExpr->getSourceRange();
        return QualType();
    }

    // TODO cleanup, always break and do common stuff at end
    switch (unaryop->getOpcode()) {
    case UO_PostInc:
        if (!checkAssignment(SubExpr, LType, "increment operand", unaryop->getLocation())) return QualType();
        break;
    case UO_PostDec:
        if (!checkAssignment(SubExpr, LType, "decrement operand", unaryop->getLocation())) return QualType();
        break;
    case UO_PreInc:
        if (!checkAssignment(SubExpr, LType, "increment operand", unaryop->getLocation())) return QualType();
        break;
    case UO_PreDec:
        if (!checkAssignment(SubExpr, LType, "decrement operand", unaryop->getLocation())) return QualType();
        break;
    case UO_AddrOf:
        if (!checkAddressOfOperand(SubExpr)) return QualType();
        LType = Context.getPointerType(LType.getCanonicalType());
        expr->setCTC();
        break;
    case UO_Deref:
        if (!LType.isPointerType()) {
            char typeName[MAX_LEN_TYPENAME];
            StringBuilder buf(MAX_LEN_TYPENAME, typeName);
            LType.DiagName(buf);
            Diag(unaryop->getOpLoc(), diag::err_typecheck_indirection_requires_pointer) << buf;
            return QualType();
        } else {
            // TEMP use CanonicalType to avoid Unresolved types etc
            QualType Q = LType.getCanonicalType();
            const PointerType* P = cast<PointerType>(Q);
            return P->getPointeeType();
        }
        break;
    case UO_Minus:
        LType = AnalyserUtils::getMinusType(LType);
        if (!LType.isValid()) {
            char typeName[MAX_LEN_TYPENAME];
            StringBuilder buf(MAX_LEN_TYPENAME, typeName);
            SubExpr->getType().DiagName(buf);
            Diag(unaryop->getOpLoc(), diag::err_typecheck_unary_expr) << buf << SubExpr->getSourceRange();
            return QualType();
        }
        unaryop->syncFlags(SubExpr);
        break;
    case UO_Not:
        unaryop->syncFlags(SubExpr);
        LType = AnalyserUtils::UsualUnaryConversions(SubExpr);
        break;
    case UO_LNot:
        // TODO first cast expr to bool, then invert here, return type bool
        // TODO extract to function
        // TODO check conversion to bool here!!
        unaryop->setCTV(SubExpr->isCTV());
        // Also set type?
        //AnalyserUtils::UsualUnaryConversions(SubExpr);
        LType = Type::Bool();
        break;
    }
    return LType;
}

bool FunctionAnalyser::checkAddressOfOperand(Expr* expr) {
    expr = AnalyserUtils::getInnerExprAddressOf(expr);
    IdentifierExpr* I = dyncast<IdentifierExpr>(expr);
    if (!I) {
        StringBuilder buf(MAX_LEN_TYPENAME);
        expr->getType().DiagName(buf);
        Diag(expr->getLocation(), diag::err_typecheck_invalid_lvalue_addrof) << buf;
        return false;
    }
    IdentifierExpr::RefKind ref = I->getRefType();
    switch (ref) {
        case IdentifierExpr::REF_UNRESOLVED:
            FATAL_ERROR("should not come here");
            return false;
        case IdentifierExpr::REF_MODULE:
            Diag(expr->getLocation(), diag::err_typecheck_invalid_addrof) << "a module";
            return false;
        case IdentifierExpr::REF_FUNC:
            // NOTE: C2 does not allow address of function like C
            Diag(expr->getLocation(), diag::err_typecheck_invalid_addrof) << "a function";
            return false;
        case IdentifierExpr::REF_TYPE:
            Diag(expr->getLocation(), diag::err_typecheck_invalid_addrof) << "a type";
            return false;
        case IdentifierExpr::REF_VAR:
            return true;
        case IdentifierExpr::REF_ENUM_CONSTANT:
            Diag(expr->getLocation(), diag::err_typecheck_invalid_addrof) << "an enum constant";
            return false;
        case IdentifierExpr::REF_STRUCT_MEMBER:
            // TODO not if on type! (only on instance)
            // ok
            return true;
        case IdentifierExpr::REF_STRUCT_FUNC:
            Diag(expr->getLocation(), diag::err_typecheck_invalid_addrof) << "a function";
            return false;
        case IdentifierExpr::REF_LABEL:
            Diag(expr->getLocation(), diag::err_typecheck_invalid_addrof) << "a label";
            return false;
    }

    // NOTE: C also allow funcions, C2 not? (no just use function itself)
    // actually it should be an IdentifierExpr pointing to a VarDecl?
    // Must be IdentifierExpr that points to VarDecl?
    return true;
}

QualType FunctionAnalyser::analyseBuiltinExpr(Expr* expr) {
    LOG_FUNC
    BuiltinExpr* B = cast<BuiltinExpr>(expr);
    switch (B->getBuiltinKind()) {
    case BuiltinExpr::BUILTIN_SIZEOF:
        return analyseSizeOfExpr(B);
    case BuiltinExpr::BUILTIN_ELEMSOF:
        return analyseElemsOfExpr(B);
    case BuiltinExpr::BUILTIN_ENUM_MIN:
        return analyseEnumMinMaxExpr(B, true);
    case BuiltinExpr::BUILTIN_ENUM_MAX:
        return analyseEnumMinMaxExpr(B, false);
    case BuiltinExpr::BUILTIN_OFFSETOF:
        return analyseOffsetOf(B);
    case BuiltinExpr::BUILTIN_TO_CONTAINER:
        return analyseToContainer(B);
    }
}

QualType FunctionAnalyser::analyseArraySubscript(Expr* expr, unsigned side) {
    LOG_FUNC
    ArraySubscriptExpr* sub = cast<ArraySubscriptExpr>(expr);
    QualType LType = analyseExpr(sub->getBase2(), RHS, true);
    if (LType.isNull()) return 0;

    if (BitOffsetExpr* BO = dyncast<BitOffsetExpr>(sub->getIndex())) {
        if (side & LHS) {
            Diag(BO->getLocation(), diag::err_bitoffset_lhs) << expr->getSourceRange();
            return 0;
        }
        QualType T = analyseBitOffsetExpr(sub->getIndex(), LType, sub->getLocation());
        expr->combineFlags(sub->getBase(), sub->getIndex());
        // dont analyse partial BitOffset expressions per part
        return T;
    }

    // TODO check return type??
    analyseExpr(sub->getIndex2(), RHS, true);

    // Deference alias types
    if (isa<AliasType>(LType)) {
        LType = cast<AliasType>(LType)->getRefType();
    }

    if (!LType.isPointerType()) {
        Diag(expr->getLocation(), diag::err_typecheck_subscript);
        return 0;
    }

    QualType Result;
    if (isa<PointerType>(LType)) {
        Result = cast<PointerType>(LType)->getPointeeType();
    } else if (isa<ArrayType>(LType)) {
        Result = cast<ArrayType>(LType)->getElementType();
    }
    assert(Result.isValid());
    return Result;
}

QualType FunctionAnalyser::analyseMemberExpr(Expr** expr_ptr, unsigned side) {
    LOG_FUNC
    Expr* expr = *expr_ptr;
    MemberExpr* M = cast<MemberExpr>(expr);
    IdentifierExpr* member = M->getMember();

    // we dont know what we're looking at here, it could be:
    // mod.Type
    // mod.var
    // mod.func
    // struct-var.member
    // struct-var.struct_function
    // var[index].member
    // var->member (=> error)
    // Type.struct_function
    // Enum.Constant (eg. Color.Red)

    // NOTE: since we dont know which one, we can't insert LValueToRValue casts in analyseExpr(RHS)
    // Also optionally insert CK_ArrayToPointerDecay cast here

    QualType LType = analyseExpr(M->getBase2(), RHS, false);
    if (!LType.isValid()) return LType;

    const ModuleType* MT = dyncast<ModuleType>(LType);
    // module.X
    if (MT) {
        ImportDecl* ID = MT->getDecl();
        ID->setUsed();
        M->setModulePrefix();
        MT = cast<ModuleType>(LType.getTypePtr());
        Decl* D = scope.findSymbolInModule(member->getName(), member->getLocation(), MT->getModule());
        if (!D) return QualType();

        if (side & RHS) D->setUsed();
        M->setDecl(D);
        AnalyserUtils::SetConstantFlags(D, M);
        QualType Q = D->getType();
        M->setType(Q);
        member->setType(Q);
        member->setDecl(D, AnalyserUtils::globalDecl2RefKind(D));

        if (Q.isArrayType()) {
            QualType Qptr = AnalyserUtils::getPointerFromArray(Context, Q);
            insertImplicitCast(CK_ArrayToPointerDecay, expr_ptr, Qptr);
            return Qptr;
        }
        if (Q.isFunctionType()) {
            M->setIsRValue();
            member->setIsRValue();
            insertImplicitCast(CK_FunctionToPointerDecay, expr_ptr, Q);
        }

        if (M->isCTV()) M->setIsRValue(); // points to const number

        return Q;
    // Enum.Constant
    } else if (isa<EnumType>(LType)) {
        EnumType* ET = cast<EnumType>(LType.getTypePtr());
        EnumTypeDecl* ETD = ET->getDecl();
        EnumConstantDecl* ECD = ETD->findConstant(member->getName());
        if (!ECD) {
            StringBuilder buf(MAX_LEN_TYPENAME);
            ETD->fullName(buf);
            Diag(member->getLocation(), diag::err_unknown_enum_constant)
                << buf << member->getName();
            return QualType();
        }
        QualType Q = ETD->getType();
        M->setDecl(ECD);
        M->setIsEnumConstant();
        M->setIsRValue();
        AnalyserUtils::SetConstantFlags(ECD, M);
        member->setCTV(true);
        member->setCTC();
        member->setDecl(ECD, AnalyserUtils::globalDecl2RefKind(ECD));
        member->setIsRValue();
        return Q;
    } else {
        // dereference pointer
        if (LType.isPointerType()) {
            // TODO use function to get elementPtr (could be alias type)
            const PointerType* PT = cast<PointerType>(LType.getTypePtr());
            LType = PT->getPointeeType();
        }

        QualType S = AnalyserUtils::getStructType(LType);
        if (!S.isValid()) {
            StringBuilder buf(MAX_LEN_TYPENAME);
            LType.DiagName(buf);
            Diag(M->getLocation(), diag::err_typecheck_member_reference_struct_union)
                    << buf << M->getSourceRange() << member->getLocation();
            return QualType();
        }

        return analyseStructMember(S, expr_ptr, side, AnalyserUtils::exprIsType(M->getBase()));
    }
    return QualType();
}


Expr* FunctionAnalyser::insertImplicitCast(CastKind ck, Expr** inner_ptr, QualType Q) {
    Expr* inner = *inner_ptr;
    ImplicitCastExpr* ic = new (Context) ImplicitCastExpr(inner->getLocation(), ck, inner);
    ic->setType(Q);
    *inner_ptr = ic;
    return ic;
}

QualType FunctionAnalyser::analyseStaticStructMember(QualType T, Expr** expr_ptr, const StructTypeDecl* S, unsigned side)
{
    LOG_FUNC
    Expr* expr = *expr_ptr;
    MemberExpr* M = cast<MemberExpr>(expr);
    IdentifierExpr* member = M->getMember();

    Decl *field = S->findMember(member->getName());
    FunctionDecl* sfunc = S->findFunction(member->getName());

    Decl* d = field ? field : sfunc;
    if (!d) {
        return EA.outputStructDiagnostics(T, member, diag::err_no_member_struct_func);
    }

    scope.checkAccess(d, member->getLocation());
    if (side & RHS) d->setUsed();
    QualType Q = d->getType();
    M->setDecl(d);
    M->setType(Q);
    if (field) member->setDecl(d, IdentifierExpr::REF_STRUCT_MEMBER);
    else {
        member->setDecl(d, IdentifierExpr::REF_STRUCT_FUNC);
        M->setIsStructFunction();
        if (sfunc->isStaticStructFunc()) M->setIsStaticStructFunction();
    }
    member->setType(Q);
    AnalyserUtils::SetConstantFlags(d, M);

    if (!allowStaticMember) {
        if (sfunc) {
            // TBD: allow non-static use of struct functions?
            //if (!sfunc->isStaticStructFunc()) return EA.outputStructDiagnostics(T, member, diag::err_invalid_use_nonstatic_struct_func);
        } else {
            return EA.outputStructDiagnostics(T, member, diag::err_static_use_nonstatic_member);
        }
    }

    if (Q.isFunctionType()) {
        M->setIsRValue();
        M->setCTC();
        member->setIsRValue();
        insertImplicitCast(CK_FunctionToPointerDecay, expr_ptr, Q);
    }

    return Q;
}

// T is the type of the struct
// M the whole expression
// isStatic is true for Foo.myFunction(...)
QualType FunctionAnalyser::analyseStructMember(QualType T, Expr** expr_ptr, unsigned side, bool isStatic) {
    LOG_FUNC
    Expr* expr = *expr_ptr;
    MemberExpr* M = cast<MemberExpr>(expr);
    assert(M && "Expression missing");
    const StructType* ST = cast<StructType>(T);
    const StructTypeDecl* S = ST->getDecl();

    if (isStatic) return analyseStaticStructMember(T, expr_ptr, S, side);

    IdentifierExpr* member = M->getMember();
    Decl* match = S->find(member->getName());

    if (!match) {
        return EA.outputStructDiagnostics(T, member, diag::err_no_member_struct_func);
    }

    IdentifierExpr::RefKind ref = IdentifierExpr::REF_STRUCT_MEMBER;
    FunctionDecl* func = dyncast<FunctionDecl>(match);

    if (func) {
        scope.checkAccess(func, member->getLocation());

        if (func->isStaticStructFunc()) {
            Diag(member->getLocation(), diag::err_static_struct_func_notype);// << func->DiagName();
            return QualType();
        }
        M->setIsStructFunction();
        ref = IdentifierExpr::REF_STRUCT_FUNC;
        M->setIsRValue();
        member->setIsRValue();
        QualType Q = func->getType();
        M->setType(Q);
        M->setCTC();
        insertImplicitCast(CK_FunctionToPointerDecay, expr_ptr, Q);

        // Is this a simple member access? If so
        // disallow this (we don't have closures!)
        if (callStack.callDepth == 0) {
            Diag(member->getLocation(), diag::err_non_static_struct_func_usage)
                << M->getBase()->getSourceRange();
            return QualType();
        }
        callStack.setStructFunction(M->getBase());
    } else {
        // NOTE: access of struct-function is not a dereference
        if (currentModule() != S->getModule() && S->hasAttribute(ATTR_OPAQUE)) {
            Diag(M->getLocation(), diag::err_deref_opaque) << S->isStruct() << S->DiagName();
            return QualType();
        }
    }

    if (side & RHS) match->setUsed();
    M->setDecl(match);
    member->setDecl(match, ref);
    member->setType(match->getType());
    QualType result = match->getType();
    if (T.isConstQualified()) result.addConst();

    return result;
}

QualType FunctionAnalyser::analyseParenExpr(Expr* expr) {
    LOG_FUNC
    ParenExpr* P = cast<ParenExpr>(expr);
    Expr** inner_ptr = P->getExpr2();
    Expr* inner = *inner_ptr;
    QualType Q = analyseExpr(inner_ptr, RHS, false);

    expr->syncFlags(inner);
    expr->syncLValue(inner);
    // TODO sync lvalue status
    return Q;
}

// return whether Result is valid
bool FunctionAnalyser::analyseBitOffsetIndex(Expr** expr_ptr, llvm::APSInt* Result, BuiltinType* BaseType) {
    LOG_FUNC
    QualType T = analyseExpr(expr_ptr, RHS, true);
    if (!T.isValid()) return false;

    Expr* expr = *expr_ptr;

    if (!T.getCanonicalType().isIntegerType()) {
        StringBuilder buf;
        T.DiagName(buf, false);
        Diag(expr->getLocation(), diag::err_bitoffset_index_non_int)
                << buf << expr->getSourceRange();
        return false;
    }
    if (!expr->isCTV()) return false;

    CTVAnalyser LA(Diags);
    llvm::APSInt Val = LA.checkLiterals(expr);
    if (Val.isSigned() && Val.isNegative()) {
        Diag(expr->getLocation(), diag::err_bitoffset_index_negative)
                << Val.toString(10) << expr->getSourceRange();
        return false;
    }
    if (Val.ugt(BaseType->getWidth()-1)) {
        Diag(expr->getLocation(), diag::err_bitoffset_index_too_large)
                << Val.toString(10) << BaseType->getName() << expr->getSourceRange();
        return false;
    }
    *Result = Val;
    return true;
}

QualType FunctionAnalyser::analyseBitOffsetExpr(Expr* expr, QualType BaseType, SourceLocation base) {
    LOG_FUNC
    BitOffsetExpr* B = cast<BitOffsetExpr>(expr);

    // check if base type is unsigned integer
    QualType CT = BaseType.getCanonicalType();
    BuiltinType* BI = dyncast<BuiltinType>(CT.getTypePtr());
    if (!BI || !BI->isInteger() || BI->isSignedInteger()) {
        Diag(base, diag::err_bitoffset_not_unsigned);
        return QualType();
    }

    llvm::APSInt VL;
    bool VLvalid = analyseBitOffsetIndex(B->getLHS2(), &VL, BI);
    llvm::APSInt VR;
    bool VRvalid = analyseBitOffsetIndex(B->getRHS2(), &VR, BI);

    QualType Result;
    if (VLvalid && VRvalid) {
        if (VR > VL) {
            Diag(B->getLocation(), diag::err_bitoffset_invalid_order)
                    << B->getLHS()->getSourceRange() << B->getRHS()->getSourceRange();
            return QualType();
        } else {
            llvm::APInt width = VL - VR;
            ++width;

            if (width.ult(8+1)) {
                Result = Type::UInt8();
            } else if (width.ult(16+1)) {
                Result = Type::UInt16();
            } else if (width.ult(32+1)) {
                Result = Type::UInt32();
            } else if (width.ult(64+1)) {
                Result = Type::UInt64();
            } else {
                FATAL_ERROR("Unreachable");
            }
            B->setWidth((unsigned char)width.getSExtValue());
        }
    } else {
        Result = BaseType;
    }

    B->combineFlags(B->getLHS(), B->getRHS());
    return Result;
}

QualType FunctionAnalyser::analyseExplicitCastExpr(Expr* expr) {
    LOG_FUNC
    ExplicitCastExpr* E = cast<ExplicitCastExpr>(expr);

    QualType outerType = analyseType(E->getDestType(), expr->getLocation());

    QualType innerType = analyseExpr(E->getInner2(), RHS, true);
    if (innerType.isValid()) {
        E->setCTV(E->getInner()->isCTV());

        if (outerType.isValid()) {
            if (!EA.checkExplicitCast(E, outerType, innerType))
                return QualType();
        }
    }

    return outerType;
}

QualType FunctionAnalyser::analyseImplicitCastExpr(Expr* expr) {
    LOG_FUNC
    ImplicitCastExpr* E = cast<ImplicitCastExpr>(expr);

    // TODO check castkind

    QualType innerType = analyseExpr(E->getInner2(), RHS, true);
    return innerType;
}

QualType FunctionAnalyser::analyseCall(Expr* expr) {
    LOG_FUNC
    CallExpr* call = cast<CallExpr>(expr);
    // analyse function

    // First check that we haven't exceeded the
    // max depth (should be reeeally hard to do)
    if (callStack.reachedMax()) {
        // Bas, we probably want diagnostics here
        // instead of a fatal error...
        // TODO
        FATAL_ERROR("Reached max indirection depth");
    }
    // Push a null struct function on the callstack.
    callStack.push();
    QualType LType = analyseExpr(call->getFn2(), RHS, true);
    // Pop the null struct function off the callstack
    Expr* structFunction = callStack.pop();
    if (LType.isNull()) return QualType();      // already handled

    if (!LType.isFunctionType()) {
        char typeName[MAX_LEN_TYPENAME];
        StringBuilder buf(MAX_LEN_TYPENAME, typeName);
        LType.DiagName(buf);
        Diag(call->getLocation(), diag::err_typecheck_call_not_function) << typeName;
        return QualType();
    }

    const FunctionType* FT = cast<FunctionType>(LType);
    FunctionDecl* func = FT->getDecl();
    func->setUsed();
    if (structFunction) call->setIsStructFunction();
    if (!checkCallArgs(func, call, structFunction)) return QualType();
    return func->getReturnType();
}

QualType FunctionAnalyser::analyseIdentifierExpr(Expr** expr_ptr, unsigned side) {
    LOG_FUNC
    Expr* expr = *expr_ptr;
    IdentifierExpr* id = cast<IdentifierExpr>(expr);
    Decl* D = analyseIdentifier(id);
    if (!D) return QualType();
    if (D == CurrentVarDecl) {
        Diag(id->getLocation(), diag::err_var_self_init) << D->getName();
        return QualType();
    }

    AnalyserUtils::SetConstantFlags(D, id);

    switch (D->getKind()) {
    case DECL_FUNC:
        insertImplicitCast(CK_FunctionToPointerDecay, expr_ptr, D->getType());
        break;
    case DECL_VAR:
    case DECL_ENUMVALUE:
    case DECL_STRUCTTYPE:
    case DECL_ENUMTYPE:
        break;
    case DECL_ALIASTYPE:
    case DECL_FUNCTIONTYPE:
        Diag(id->getLocation(), diag::err_unexpected_typedef) << id->getName();
        expr->setCTC();
        expr->setIsRValue();
        break;
    case DECL_ARRAYVALUE:
        break;
    case DECL_IMPORT:
        expr->setCTC();
        expr->setIsRValue();
        break;
    case DECL_LABEL:
    case DECL_STATIC_ASSERT:
        FATAL_ERROR("Unreachable");
        break;
    }
    if (side & RHS) {
        D->setUsed();
        if (usedPublicly) D->setUsedPublic();
    }

    if (expr->isCTV()) expr->setIsRValue();

    //QualType T = D->getType();
    return (*expr_ptr)->getType();
}

bool FunctionAnalyser::checkCallArgs(FunctionDecl* func, CallExpr* call, Expr* structFunction) {
    LOG_FUNC
    unsigned funcArgs = func->numArgs();
    unsigned callArgs = call->numArgs();

    unsigned funcIndex = 0;
    unsigned callIndex = 0;

    const bool isStructFunction = (structFunction != nullptr);
    unsigned diagIndex = 0;
    if (isStructFunction) {
        if (AnalyserUtils::exprIsType(structFunction)) {
            diagIndex = 5;
        }
        else {
            diagIndex = 4;
            funcArgs--;
            funcIndex = 1;
        }
    }
    unsigned minArgs = MIN(funcArgs, callArgs);

    for (unsigned i = 0; i < minArgs; i++) {
        Expr** callArg = call->getArg2(callIndex);
        QualType callArgType = analyseExpr(callArg, RHS, true);
        // BB HUH no return on error?!? (maybe after checking all args)
        if (callArgType.isValid()) {
            VarDecl* funcArg = func->getArg(funcIndex);
            QualType funcArgType = funcArg->getType();
            assert(funcArgType.isValid());
            EA.check(funcArgType, *callArg);
        }
        callIndex++;
        funcIndex++;
    }
    if (callArgs > funcArgs) {
        // more args given, check if function is variadic
        if (!func->isVariadic()) {
            Expr* arg = call->getArg(callIndex);
            unsigned msg = diag::err_typecheck_call_too_many_args;
            if (func->hasDefaultArgs()) msg = diag::err_typecheck_call_too_many_args_at_most;
            Diag(arg->getLocation(), msg) << diagIndex << funcArgs << callArgs;
            return false;
        }
        for (unsigned i = minArgs; i < callArgs; i++) {
            Expr** callArg = call->getArg2(callIndex);
            QualType callArgType = analyseExpr(callArg, RHS, true);
            // TODO use canonical
            if (callArgType == Type::Void()) {
                fprintf(stderr, "ERROR: (TODO) passing 'void' to parameter of incompatible type '...'\n");
                TODO;
                //Diag(callArg->getLocation(), diag::err_typecheck_convert_incompatible)
                //    << "from" << "to" << 1;// << callArg->getLocation();
            }
            callIndex++;
        }
    }
    else if (callArgs < funcArgs) {
        // less args given, check for default argument values
        for (unsigned i = minArgs; i < funcArgs; i++) {
            VarDecl* arg = func->getArg(funcIndex++);
            if (!arg->getInitValue()) {
                unsigned msg = diag::err_typecheck_call_too_few_args;
                if (func->hasDefaultArgs()) {
                    funcArgs = func->minArgs();
                    if (isStructFunction) funcArgs--;
                    msg = diag::err_typecheck_call_too_few_args_at_least;
                }
                Diag(call->getLocEnd(), msg) << diagIndex << funcArgs << callArgs;
                return false;
            }
        }
    }
    return true;
}

Decl* FunctionAnalyser::analyseIdentifier(IdentifierExpr* id) {
    LOG_FUNC
    Decl* D = scope.findSymbol(id->getName(), id->getLocation(), false, false);
    if (D) {
        id->setDecl(D, AnalyserUtils::globalDecl2RefKind(D));
        id->setType(D->getType());
        AnalyserUtils::SetConstantFlags(D, id);

        if (usedPublicly && !D->isPublic()) {
            Diag(id->getLocation(), diag::err_non_public_constant) << D->DiagName();
        }
    }
    return D;
}

LabelDecl* FunctionAnalyser::LookupOrCreateLabel(const char* name, SourceLocation loc) {
    LOG_FUNC
    LabelDecl* LD = 0;
    for (LabelsIter iter = labels.begin(); iter != labels.end(); ++iter) {
        if (strcmp((*iter)->getName(), name) == 0) {
            LD = *iter;
            break;
        }
    }
    if (!LD) {
        LD = new (Context) LabelDecl(name, loc);
        // add to functionScope
        labels.push_back(LD);
    }
    return LD;
}

bool FunctionAnalyser::checkAssignment(Expr* assignee, QualType TLeft, const char* diag_msg, SourceLocation loc) {
    if (!assignee->isLValue()) {
        Diag(loc, diag::err_expected_lvalue) << diag_msg << assignee->getSourceRange();
        return false;
    }

    if (TLeft.isConstQualified()) {
        Diag(assignee->getLocation(), diag::err_typecheck_assign_const) << 5 << assignee->getSourceRange();
        return false;
    }

    if (TLeft.isArrayType()) {
        ArrayType* AT = cast<ArrayType>(TLeft.getTypePtr());
        if (AT->isIncremental()) {
            Diag(assignee->getLocation(), diag::err_incremental_array_value_function_scope);
            return false;
        } else {
            StringBuilder buf1(MAX_LEN_TYPENAME);
            TLeft.DiagName(buf1);
            Diag(assignee->getLocation(), diag::err_typecheck_array_not_modifiable_lvalue) << buf1;
            return false;
        }
    }
    return true;
}

void FunctionAnalyser::checkDeclAssignment(Decl* decl, Expr* expr) {
    LOG_FUNC
    assert(decl);
    switch (decl->getKind()) {
    case DECL_FUNC:
        break;
    case DECL_VAR:
        //VarDecl* VD = cast<VarDecl>(decl);
        //checkAssignment(expr, VD->getType());
        return;
    case DECL_ENUMVALUE:
    case DECL_ALIASTYPE:
    case DECL_STRUCTTYPE:
    case DECL_ENUMTYPE:
    case DECL_FUNCTIONTYPE:
    case DECL_ARRAYVALUE:
    case DECL_IMPORT:
    case DECL_LABEL:
    case DECL_STATIC_ASSERT:
        break;
    }
    Diag(expr->getLocation(), diag::err_typecheck_expression_not_modifiable_lvalue);
}

void FunctionAnalyser::checkArrayDesignators(InitListExpr* expr, int64_t* size) {
    LOG_FUNC
    typedef std::vector<Expr*> Indexes;
    Indexes indexes;
    Expr** values = expr->getValues();
    unsigned numValues = expr->numValues();
    indexes.resize(numValues);
    int maxIndex = 0;
    int currentIndex = -1;
    for (unsigned i=0; i<numValues; i++) {
        Expr* E = values[i];
        if (DesignatedInitExpr* D = dyncast<DesignatedInitExpr>(E)) {
            currentIndex = D->getIndex().getSExtValue();
            if (currentIndex == -1) return; // some designators are invalid
            if (*size != -1 && currentIndex >= *size) {
                Diag(E->getLocation(), diag::err_array_designator_too_large) << D->getIndex().toString(10) << (int)*size;
                return;
            }
        } else {
            currentIndex++;
        }
        if (*size != -1 && currentIndex >= *size) {
            Diag(E->getLocation(), diag::err_excess_initializers) << 0;
            return;
        }
        if (currentIndex >= (int)indexes.size()) {
            indexes.resize(currentIndex + 1);
        }
        Expr* existing = indexes[currentIndex];
        if (existing) {
            Diag(E->getLocation(), diag::err_duplicate_array_index_init) << E->getSourceRange();
            Diag(existing->getLocation(), diag::note_previous_initializer) << 0 << 0 << E->getSourceRange();
        } else {
            indexes[currentIndex] = E;
        }
        if (currentIndex > maxIndex) maxIndex = currentIndex;
    }
    if (*size == -1) *size = maxIndex + 1;
    //for (unsigned i=0; i<indexes.size(); i++) printf("[%d] = %p\n", i, indexes[i]);
}

void FunctionAnalyser::checkEnumCases(const SwitchStmt* SS, const EnumType* ET) const {
    const EnumTypeDecl* ETD = ET->getDecl();
    char enumHandled[ETD->numConstants()];
    memset(enumHandled, 0, sizeof(enumHandled));
    Stmt* TheDefaultStmt = 0;
    Stmt** cases = SS->getCases();
    for (unsigned i=0; i<SS->numCases(); i++) {
        if (isa<DefaultStmt>(cases[i])) {
            TheDefaultStmt = cases[i];
            continue;
        }
        const CaseStmt* CS = cast<CaseStmt>(cases[i]);
        const Expr* cond = CS->getCond();

        if (const IdentifierExpr* id = dyncast<IdentifierExpr>(cond)) {
            const EnumConstantDecl* ECD = dyncast<EnumConstantDecl>(id->getDecl());
            if (ECD) {
                int index = ETD->getIndex(ECD);
                if (index != -1) {
                    if (enumHandled[index]) {
                        Diag(cond->getLocation(), diag::err_duplicate_case) << ECD->getName() << cond->getSourceRange();
                    }
                    enumHandled[index] = true;
                    continue;
                }
            }
        }
        // TODO dont allow this anymore? (only Identifier)
        if (const MemberExpr* M = dyncast<MemberExpr>(cond)) {
            if (M->isEnumConstant()) {
                // check if it refers to right enumdecl
                // TODO: can M->getDecl() be NULL?
                const EnumConstantDecl* ECD = dyncast<EnumConstantDecl>(M->getDecl());
                if (ECD) {
                    int index = ETD->getIndex(ECD);
                    if (index != -1) {
                        if (enumHandled[index]) {
                            Diag(cond->getLocation(), diag::err_duplicate_case) << ECD->getName() << cond->getSourceRange();
                        }
                        enumHandled[index] = true;
                        continue;
                    }
                }

            }
        }
        // TODO BB cannot happen anymore, remove
        StringBuilder buf(64);
        QualType Q((Type*)ET, 0);
        Q.DiagName(buf);
        Diag(cond->getLocation(), diag::warn_not_in_enum) << buf << cond->getSourceRange();
    }

    SmallVector<std::string ,8> UnhandledNames;
    for (unsigned i=0; i<ETD->numConstants(); ++i) {
        if (!enumHandled[i]) UnhandledNames.push_back(ETD->getConstant(i)->getName());
    }

    if (TheDefaultStmt) {
        if (UnhandledNames.empty()) {
            Diag(TheDefaultStmt->getLocation(), diag::warn_unreachable_default);
        }
        return;
    }

    // Produce a nice diagnostic if multiple values aren't handled.
    if (!UnhandledNames.empty()) {
        DiagnosticBuilder DB = Diag(SS->getCond()->getLocation(),
                                    diag::warn_missing_case)
                               << (int)UnhandledNames.size();
        for (size_t I=0, E = std::min(UnhandledNames.size(), (size_t)3); I != E; ++I) {
            DB << UnhandledNames[I];
        }
    }
}

QualType FunctionAnalyser::getConditionType(const Stmt* C) const {
    if (const Expr* E = dyncast<Expr>(C)) {
        return E->getType();
    }
    if (const DeclStmt* D = dyncast<DeclStmt>(C)) {
        return D->getDecl()->getType();
    }
    FATAL_ERROR("Invalid Condition");
    return QualType();
}

DiagnosticBuilder FunctionAnalyser::Diag(SourceLocation Loc, unsigned DiagID) const {
    return Diags.Report(Loc, DiagID);
}

void FunctionAnalyser::pushMode(unsigned DiagID) {
    LOG_FUNC
    inConstExpr = true;
    constDiagID = DiagID;
}

void FunctionAnalyser::popMode() {
    LOG_FUNC
    inConstExpr = false;
    constDiagID = 0;
}

const Module* FunctionAnalyser::currentModule() const {
    const Module* mod = 0;
    if (CurrentFunction) mod = CurrentFunction->getModule();
    if (!mod && CurrentVarDecl) mod = CurrentVarDecl->getModule();
    assert(mod);
    return mod;
}

