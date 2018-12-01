/* Copyright 2013-2018 Bas van den Berg
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
#include "Analyser/LiteralAnalyser.h"
#include "Analyser/Scope.h"
#include "Analyser/AnalyserConstants.h"
#include "AST/Expr.h"
#include "Utils/color.h"
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

static void SetConstantFlags(Decl* D, Expr* expr) {
    switch (D->getKind()) {
    case DECL_FUNC:
        expr->setConstant();
        break;
    case DECL_VAR:
    {
        VarDecl* VD = cast<VarDecl>(D);
        QualType T = VD->getType();
        if (T.isConstQualified()) {
            Expr* Init = VD->getInitValue();
            if (Init) {
                // Copy CTC status of Init Expr
                expr->setCTC(Init->getCTC());
            }
            expr->setConstant();
            return;
        }
        break;
    }
    case DECL_ENUMVALUE:
        expr->setCTC(CTC_FULL);
        expr->setConstant();
        return;
    case DECL_ALIASTYPE:
    case DECL_STRUCTTYPE:
    case DECL_ENUMTYPE:
    case DECL_FUNCTIONTYPE:
        expr->setConstant();
        break;
    case DECL_ARRAYVALUE:
    case DECL_IMPORT:
    case DECL_LABEL:
        break;
    }
    // TODO needed?
    expr->setCTC(CTC_NONE);
}


FunctionAnalyser::FunctionAnalyser(Scope& scope_,
                                   TypeResolver& typeRes_,
                                   ASTContext& context_,
                                   c2lang::DiagnosticsEngine& Diags_,
                                   bool isInterface_)
    : scope(scope_)
    , TR(typeRes_)
    , Context(context_)
    , EA(Diags_)
    , Diags(Diags_)
    , CurrentFunction(0)
    , CurrentVarDecl(0)
    , constDiagID(0)
    , inConstExpr(false)
    , usedPublicly(false)
    , isInterface(isInterface_)
{
    callStack.callDepth = 0;
}

void FunctionAnalyser::check(FunctionDecl* func) {
    // check argument inits
    for (unsigned i=0; i<func->numArgs(); i++) {
        VarDecl* Arg = func->getArg(i);
        if (Arg->getInitValue()) {
            checkVarInit(Arg);
        }
    }

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

void FunctionAnalyser::checkVarInit(VarDecl* V) {
    LOG_FUNC
    CurrentVarDecl = V;

    ConstModeSetter cms(*this, diag::err_init_element_not_constant);
    usedPublicly = (V->isPublic() && (!V->isGlobal() || V->getType().isConstQualified()));
    analyseInitExpr(V->getInitValue(), V->getType());
    usedPublicly = false;
    // TODO if type is array, update type of VarDecl? (add size)
    CurrentVarDecl = 0;
}

void FunctionAnalyser::checkArraySizeExpr(VarDecl* V) {
    LOG_FUNC
    CurrentVarDecl = V;

    ConstModeSetter cms(*this, diag::err_init_element_not_constant);
    usedPublicly = V->isPublic();
    analyseArrayType(V, V->getType());
    usedPublicly = false;

    CurrentVarDecl = 0;
}

unsigned FunctionAnalyser::checkEnumValue(EnumConstantDecl* E, llvm::APSInt& nextValue) {
    LOG_FUNC;

    LiteralAnalyser LA(Diags);
    Expr* Init = E->getInitValue();
    if (Init) {
        usedPublicly = E->isPublic();
        QualType T = analyseExpr(Init, RHS);
        usedPublicly = false;
        if (!T.isValid()) return 1;

        if (!Init->isConstant()) {
            Diag(Init->getLocation(), diag::err_expr_not_ice) << 0 << Init->getSourceRange();
            return 1;
        }
        // TODO refactor duplicate code with analyseArraySizeExpr()
        QualType CT = T.getCanonicalType();
        if (!CT.isBuiltinType() || !cast<BuiltinType>(CT)->isInteger()) {
            Diag(Init->getLocation(), diag::err_expr_not_ice) << 0 << Init->getSourceRange();
            return 1;
        }
        assert(Init->getCTC() == CTC_FULL);
        llvm::APSInt V = LA.checkLiterals(Init);
        nextValue = V;
    }
    E->setValue(nextValue);
    ++nextValue;
    if (!LA.checkRange(E->getType(), Init, E->getLocation(), E->getValue())) return 1;

    return 0;
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

void FunctionAnalyser::analyseStmt(Stmt* S, bool haveScope) {
    LOG_FUNC
    switch (S->getKind()) {
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
        FATAL_ERROR("Unreachable"); // Done at other place.
        break;
    case STMT_BREAK:
        analyseBreakStmt(S);
        break;
    case STMT_CONTINUE:
        analyseContinueStmt(S);
        break;
    case STMT_LABEL:
        analyseLabelStmt(S);
        break;
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
    }
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
    if (F->getCond()) {
        QualType CT = analyseExpr(F->getCond(), RHS);
        if (!CT.isScalarType()) {
            StringBuilder buf(64);
            CT.DiagName(buf);
            Diag(F->getCond()->getLocation(), diag::err_typecheck_statement_requires_scalar) << buf;
        }
    }
    if (F->getIncr()) analyseExpr(F->getIncr(), RHS);
    analyseStmt(F->getBody(), true);
    scope.ExitScope();
}

void FunctionAnalyser::analyseSwitchStmt(Stmt* stmt) {
    LOG_FUNC
    SwitchStmt* S = cast<SwitchStmt>(stmt);

    scope.EnterScope(Scope::DeclScope);
    if (!analyseCondition(S->getCond())) return;

    unsigned numCases = S->numCases();
    Stmt** cases = S->getCases();
    Stmt* defaultStmt = 0;
    scope.EnterScope(Scope::BreakScope | Scope::SwitchScope);
    for (unsigned i=0; i<numCases; i++) {
        Stmt* C = cases[i];
        switch (C->getKind()) {
        case STMT_CASE:
            analyseCaseStmt(C);
            break;
        case STMT_DEFAULT:
            if (defaultStmt) {
                Diag(C->getLocation(), diag::err_multiple_default_labels_defined);
                Diag(defaultStmt->getLocation(), diag::note_duplicate_case_prev);
            } else {
                defaultStmt = C;
            }
            analyseDefaultStmt(C);
            break;
        default:
            FATAL_ERROR("Unreachable");
        }
    }

    QualType QT = getConditionType(S->getCond());
    if (const EnumType* ET = dyncast<EnumType>(QT)) {
        checkEnumCases(S, ET);
    }

    scope.ExitScope();
    scope.ExitScope();
}

void FunctionAnalyser::analyseBreakStmt(Stmt* stmt) {
    LOG_FUNC
    if (!scope.allowBreak()) {
        BreakStmt* B = cast<BreakStmt>(stmt);
        Diag(B->getLocation(), diag::err_break_not_in_loop_or_switch);
    }
}

void FunctionAnalyser::analyseContinueStmt(Stmt* stmt) {
    LOG_FUNC
    if (!scope.allowContinue()) {
        ContinueStmt* C = cast<ContinueStmt>(stmt);
        Diag(C->getLocation(), diag::err_continue_not_in_loop);
    }
}

void FunctionAnalyser::analyseLabelStmt(Stmt* S) {
    LOG_FUNC
    LabelStmt* L = cast<LabelStmt>(S);

    LabelDecl* LD = LookupOrCreateLabel(L->getName(), L->getLocation());
    if (LD->getStmt()) {
        Diag(L->getLocation(), diag::err_redefinition_of_label) <<  LD->DiagName();
        Diag(LD->getLocation(), diag::note_previous_definition);
    } else {
        LD->setStmt(L);
        LD->setLocation(L->getLocation());
    }

    analyseStmt(L->getSubStmt());
    // substmt cannot be declaration

    if (isa<DeclStmt>(L->getSubStmt())) {
        Diag(L->getSubStmt()->getLocation(), diag::err_decl_after_label);
    }
}

void FunctionAnalyser::analyseGotoStmt(Stmt* S) {
    LOG_FUNC
    GotoStmt* G = cast<GotoStmt>(S);
    IdentifierExpr* label = G->getLabel();
    LabelDecl* LD = LookupOrCreateLabel(label->getName(), label->getLocation());
    label->setDecl(LD, IdentifierExpr::REF_LABEL);
    LD->setUsed();
}

void FunctionAnalyser::analyseCaseStmt(Stmt* stmt) {
    LOG_FUNC
    scope.EnterScope(Scope::DeclScope);
    CaseStmt* C = cast<CaseStmt>(stmt);
    analyseExpr(C->getCond(), RHS);
    Stmt** stmts = C->getStmts();
    for (unsigned i=0; i<C->numStmts(); i++) {
        analyseStmt(stmts[i]);
    }
    C->setHasDecls(scope.hasDecls());
    scope.ExitScope();
}

void FunctionAnalyser::analyseDefaultStmt(Stmt* stmt) {
    LOG_FUNC
    scope.EnterScope(Scope::DeclScope);
    DefaultStmt* D = cast<DefaultStmt>(stmt);
    Stmt** stmts = D->getStmts();
    for (unsigned i=0; i<D->numStmts(); i++) {
        analyseStmt(stmts[i]);
    }
    D->setHasDecls(scope.hasDecls());
    scope.ExitScope();
}

void FunctionAnalyser::analyseReturnStmt(Stmt* stmt) {
    LOG_FUNC
    ReturnStmt* ret = cast<ReturnStmt>(stmt);
    Expr* value = ret->getExpr();
    QualType rtype = CurrentFunction->getReturnType();
    bool no_rvalue = (rtype.getTypePtr() == Type::Void());
    if (value) {
        QualType type = analyseExpr(value, RHS);
        if (no_rvalue) {
            Diag(ret->getLocation(), diag::ext_return_has_expr) << CurrentFunction->getName() << 0
                    << value->getSourceRange();
        } else {
            if (type.isValid()) {
                EA.check(rtype, value);
            }
        }
    } else {
        if (!no_rvalue) {
            Diag(ret->getLocation(), diag::ext_return_missing_expr) << CurrentFunction->getName() << 0;
        }
    }
}

void FunctionAnalyser::analyseDeclStmt(Stmt* stmt) {
    LOG_FUNC
    DeclStmt* DS = cast<DeclStmt>(stmt);
    VarDecl* decl = DS->getDecl();

    scope.setHasDecls();
    bool haveError = false;
    QualType Q = TR.resolveType(decl->getType(), decl->isPublic());
    if (Q.isValid()) {
        decl->setType(Q);

        if (!isInterface) {
            if (Q.isConstant()) {
                if (!isupper(decl->getName()[0])) {
                    Diags.Report(decl->getLocation(), diag::err_const_casing);
                }
            } else {
                if (!islower(decl->getName()[0])) {
                    Diags.Report(decl->getLocation(), diag::err_var_casing);
                }
            }
        }

        if (Q.isArrayType()) {
            // TODO use Helper-function to get ArrayType (might be AliasType)
            ArrayType* AT = cast<ArrayType>(Q.getTypePtr());
            Expr* sizeExpr = AT->getSizeExpr();
            if (sizeExpr) {
                analyseArraySizeExpr(AT);
                if (sizeExpr->getCTC() != CTC_FULL && decl->getInitValue()) {
                    Diag(decl->getLocation(), diag::err_vla_with_init_value) << decl->getInitValue()->getLocation();
                    haveError = true;
                }
            } else {
                if (AT->isIncremental()) {
                    Diag(decl->getLocation(), diag::err_incremental_array_function_scope);
                    haveError = true;
                } else if (!decl->getInitValue()) {
                    Diag(decl->getLocation(), diag::err_typecheck_incomplete_array_needs_initializer);
                    haveError = true;
                }
            }
        }
        TR.checkOpaqueType(decl->getLocation(), false, Q);

        if (!haveError) {
            if (!TR.requireCompleteType(decl->getLocation(), Q, diag::err_typecheck_decl_incomplete_type)) {
                haveError = true;
            }
        }
    } else {
        haveError = true;
    }

    // check name
    if (!scope.checkScopedSymbol(decl)) return;

    // check initial value
    Expr* initialValue = decl->getInitValue();
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
        Expr* e = A->getOutputExpr(i);
        analyseExpr(e, LHS);
    }
    // inputs
    for (unsigned i=0; i<A->getNumInputs(); ++i) {
        Expr* e = A->getInputExpr(i);
        analyseExpr(e, RHS);
    }
    // clobbers
    for (unsigned i=0; i<A->getNumClobbers(); ++i) {
        StringLiteral* c = A->getClobber(i);
        analyseExpr(c, 0);
    }
}

bool FunctionAnalyser::analyseCondition(Stmt* stmt) {
    LOG_FUNC

    if (isa<DeclStmt>(stmt)) {
        analyseDeclStmt(stmt);
    } else {
        assert(isa<Expr>(stmt));
        Expr* E = cast<Expr>(stmt);
        QualType Q1 = analyseExpr(E, RHS);
        if (!Q1.isValid()) return false;
        EA.check(Type::Bool(), E);
    }
    return true;
}

void FunctionAnalyser::analyseStmtExpr(Stmt* stmt) {
    LOG_FUNC
    Expr* expr = cast<Expr>(stmt);
    analyseExpr(expr, 0);
}

C2::QualType FunctionAnalyser::analyseExpr(Expr* expr, unsigned side) {
    LOG_FUNC
    if (side & LHS) {
        if (!checkAssignee(expr)) return QualType();
    }

    switch (expr->getKind()) {
    case EXPR_INTEGER_LITERAL:
        return analyseIntegerLiteral(expr);
    case EXPR_FLOAT_LITERAL:
        // For now always return type float
        expr->setType(Type::Float32());
        return Type::Float32();
    case EXPR_BOOL_LITERAL:
        expr->setType(Type::Bool());
        return Type::Bool();
    case EXPR_CHAR_LITERAL:
        expr->setType(Type::Int8());
        return Type::Int8();
    case EXPR_STRING_LITERAL:
    {
        // return type: 'const i8*'
        QualType Q = Context.getPointerType(Type::Int8());
        Q.addConst();
        if (!Q->hasCanonicalType()) Q->setCanonicalType(Q);
        expr->setType(Q);
        return Q;
    }
    case EXPR_NIL:
    {
        QualType Q = Context.getPointerType(Type::Void());
        if (!Q->hasCanonicalType()) Q->setCanonicalType(Q);
        expr->setType(Q);
        return Q;
    }
    case EXPR_CALL:
        return analyseCall(expr);
    case EXPR_IDENTIFIER:
    {
        IdentifierExpr* id = cast<IdentifierExpr>(expr);
        Decl* D = analyseIdentifier(id);
        if (!D) break;
        if (D == CurrentVarDecl) {
            Diag(id->getLocation(), diag::err_var_self_init) << D->getName();
            return QualType();
        }
        if (side & LHS) checkDeclAssignment(D, expr);
        switch (D->getKind()) {
        case DECL_FUNC:
            expr->setConstant();
            break;
        case DECL_VAR:
            break;
        case DECL_ENUMVALUE:
            expr->setCTC(CTC_FULL);
            expr->setConstant();
            break;
        case DECL_STRUCTTYPE:
            // Type.func is allowed as init
            // TODO handle, mark that Type was specified, not just return StructType
            break;
        case DECL_ALIASTYPE:
        case DECL_FUNCTIONTYPE:
            Diag(id->getLocation(), diag::err_unexpected_typedef) << id->getName();
            expr->setConstant();
            break;
        case DECL_ENUMTYPE:
            expr->setConstant();
            break;
        case DECL_ARRAYVALUE:
            break;
        case DECL_IMPORT:
            expr->setConstant();
            break;
        case DECL_LABEL:
            TODO;
            break;
        }
        if (side & RHS) {
            D->setUsed();
            if (usedPublicly) D->setUsedPublic();
        }
        QualType T = D->getType();
        expr->setType(T);
        return T;
    }
    break;
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
        return analyseUnaryOperator(expr, side);
    case EXPR_BUILTIN:
        return analyseBuiltinExpr(expr);
    case EXPR_ARRAYSUBSCRIPT:
        return analyseArraySubscript(expr, side);
    case EXPR_MEMBER:
        return analyseMemberExpr(expr, side);
    case EXPR_PAREN:
        return analyseParenExpr(expr);
    case EXPR_BITOFFSET:
        FATAL_ERROR("Unreachable");
        break;
    case EXPR_CAST:
        return analyseExplicitCastExpr(expr);
    }
    return QualType();
}

void FunctionAnalyser::analyseInitExpr(Expr* expr, QualType expectedType) {
    LOG_FUNC
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
        QualType Q = analyseExpr(expr, RHS);
        if (Q.isValid()) {
            if (inConstExpr && !expr->isConstant()) {
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
}

static IdentifierExpr::RefType globalDecl2RefType(const Decl* D) {
    switch (D->getKind()) {
    case DECL_FUNC:         return IdentifierExpr::REF_FUNC;
    case DECL_VAR:          return IdentifierExpr::REF_VAR;
    case DECL_ENUMVALUE:    return IdentifierExpr::REF_ENUM_CONSTANT;
    case DECL_ALIASTYPE:
    case DECL_STRUCTTYPE:
    case DECL_ENUMTYPE:
    case DECL_FUNCTIONTYPE:
                            return IdentifierExpr::REF_TYPE;
    case DECL_ARRAYVALUE:
                            return IdentifierExpr::REF_VAR;
    case DECL_IMPORT:
                            return IdentifierExpr::REF_MODULE;
    case DECL_LABEL:        return IdentifierExpr::REF_LABEL;
    }
}

void FunctionAnalyser::analyseInitListArray(InitListExpr* expr, QualType Q, unsigned numValues, Expr** values) {
    LOG_FUNC
    bool haveDesignators = false;
    bool haveErrors = false;
    // TODO use helper function
    ArrayType* AT = cast<ArrayType>(Q.getCanonicalType().getTypePtr());
    QualType ET = AT->getElementType();
    bool constant = true;
    for (unsigned i=0; i<numValues; i++) {
        analyseInitExpr(values[i], ET);
        if (DesignatedInitExpr* D = dyncast<DesignatedInitExpr>(values[i])) {
            haveDesignators = true;
            if (D->getDesignatorKind() != DesignatedInitExpr::ARRAY_DESIGNATOR) {
                StringBuilder buf;
                Q.DiagName(buf);
                Diag(D->getLocation(), diag::err_field_designator_non_aggr) << 0 << buf;
                haveErrors = true;
                continue;
            }
        }
        if (!values[i]->isConstant()) constant = false;
    }
    if (constant) expr->setConstant();
    if (haveDesignators) expr->setDesignators();
    // determine real array size
    llvm::APInt initSize(64, 0, false);
    if (haveDesignators && !haveErrors) {
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

static inline StringBuilder &quotedField(StringBuilder &builder, IdentifierExpr *field) {
    return builder << '\'' << field->getName() << '\'';
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
        quotedField(fname, field);
        StringBuilder tname(MAX_LEN_TYPENAME);
        Q.DiagName(tname);
        Diag(D->getLocation(), diag::err_field_designator_unknown) << fname << tname;
        return true;
    }


    Expr* existing = fields[memberIndex];
    if (existing) {
        StringBuilder fname(MAX_LEN_VARNAME);
        quotedField(fname, field);
        Diag(D->getLocation(), diag::err_duplicate_field_init) << fname;
        Diag(existing->getLocation(), diag::note_previous_initializer) << 0 << 0;
        return true;
    }
    fields[memberIndex] = value;
    VarDecl* VD;
    if (anonUnionIndex > -1) {
        field->setDecl(anonUnionDecl, globalDecl2RefType(anonUnionDecl));
        VD = dyncast<VarDecl>(anonUnionDecl->getMember((unsigned)anonUnionIndex));
    } else {
        VD = dyncast<VarDecl>(STD->getMember((unsigned)memberIndex));
    }
    if (!VD) {
        TODO;
        assert(VD && "TEMP don't support sub-struct member inits");
    }
    field->setDecl(VD, globalDecl2RefType(VD));
    analyseInitExpr(D->getInitValue(), VD->getType());
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
    assert(STD->isStruct() && "TEMP only support structs for now");
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
            analyseInitExpr(values[i], VD->getType());
            if (!values[i]->isConstant()) constant = false;
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
    if (constant) expr->setConstant();
}

void FunctionAnalyser::analyseInitList(InitListExpr* expr, QualType Q) {
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
        Expr* Desig = D->getDesignator();
        QualType DT = analyseExpr(Desig, RHS);
        if (DT.isValid()) {
            if (!Desig->isConstant()) {
                assert(constDiagID);
                Diag(Desig->getLocation(), constDiagID) << Desig->getSourceRange();
            } else {
                if (!Desig->getType().isIntegerType()) {
                    Diag(Desig->getLocation(), diag::err_typecheck_subscript_not_integer) << Desig->getSourceRange();
                } else {
                    LiteralAnalyser LA(Diags);
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

    analyseInitExpr(D->getInitValue(), expectedType);
    if (D->getInitValue()->isConstant()) D->setConstant();

    D->setType(expectedType);
}

static uint64_t sizeOfType(QualType type) {

    // TODO not a constant.
    constexpr unsigned POINTER_SIZE = 8;

    if (type.isNull()) return 0;

    type = type.getCanonicalType();
    switch (type->getTypeClass()) {
    case TC_UNRESOLVED:
    case TC_ALIAS:
        FATAL_ERROR("Should be resolved");
        return 1;
    case TC_BUILTIN:
        return (cast<BuiltinType>(type.getTypePtr())->getWidth() + 7) / 8;
    case TC_POINTER:
        return POINTER_SIZE;
    case TC_ARRAY:
    {
        ArrayType *arrayType = cast<ArrayType>(type.getTypePtr());
        return sizeOfType(arrayType->getElementType()) * arrayType->getSize().getZExtValue();
    }
    case TC_STRUCT:
        // TODO
        return 0;
    case TC_ENUM:
        // TODO
        return 0;
    case TC_FUNCTION:
        return POINTER_SIZE;
    case TC_MODULE:
        FATAL_ERROR("Cannot occur here");
    }
}
bool FunctionAnalyser::analyseSizeOfExpr(BuiltinExpr* B) {
    LOG_FUNC

    // Consider 128 for certain architectures. TODO
    // Also, elsewhere this is assumed to be 4!
    static constexpr unsigned FLOAT_SIZE_DEFAULT = 8;

    uint64_t width;

    Expr* expr = B->getExpr();

    switch (expr->getKind()) {
    case EXPR_FLOAT_LITERAL:
        width = FLOAT_SIZE_DEFAULT;
        break;
    case EXPR_INITLIST:
    case EXPR_DESIGNATOR_INIT:
        TODO; // Good error
        return false;
    case EXPR_TYPE:
    {
        // TODO can be public?
        QualType Q = TR.resolveType(expr->getType(), false);
        if (Q.isValid()) expr->setType(Q);
        // TODO here we want to use expr->getLocation()
        // But that does not properly find the right location!
        TR.checkOpaqueType(B->getLocation(), false, Q);
        expr->setType(Q);
        width = sizeOfType(Q);
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
    case EXPR_CAST: {
        QualType type = analyseExpr(expr, RHS);
        if (!type.isValid()) return false;
        TR.checkOpaqueType(expr->getLocation(), true, type);
        width = sizeOfType(type);
        break;
    }
    case EXPR_BUILTIN:
        FATAL_ERROR("Unreachable");
        return false;
    case EXPR_PAREN:
    case EXPR_BITOFFSET:
        FATAL_ERROR("Unreachable");
        return false;
}
    B->setValue(llvm::APSInt::getUnsigned(width));
    return true;
}

QualType FunctionAnalyser::analyseElemsOfExpr(BuiltinExpr* B) {
    LOG_FUNC

    Expr* E = B->getExpr();
    QualType T = analyseExpr(E, RHS);
    const ArrayType* AT = dyncast<ArrayType>(T.getCanonicalType());
    if (!AT) {
        Diag(E->getLocation(), diag::err_elemsof_no_array) << E->getSourceRange();
        return QualType();
    }
    B->setValue(llvm::APSInt(AT->getSize()));
    return Type::UInt32();
}

QualType FunctionAnalyser::analyseEnumMinMaxExpr(BuiltinExpr* B, bool isMin) {
    LOG_FUNC
    Expr* E = B->getExpr();
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

// sets ArrayType sizes recursively if sizeExpr is constant
// NOTE: doesn't check any initExpr itself
void FunctionAnalyser::analyseArrayType(VarDecl* V, QualType T) {
    LOG_FUNC
    // TODO V not needed? (or move more diags here from checkVarInits)
    if (!T.isArrayType()) return;

    switch (T->getTypeClass()) {
    case TC_BUILTIN:
        FATAL_ERROR("Unreachable");
        break;
    case TC_POINTER:
        analyseArrayType(V, cast<PointerType>(T)->getPointeeType());
        break;
    case TC_ARRAY:
    {
        ArrayType* AT = cast<ArrayType>(T.getTypePtr());
        analyseArraySizeExpr(AT);
        analyseArrayType(V, AT->getElementType());
        break;
    }
    case TC_UNRESOLVED:
        FATAL_ERROR("Unreachable");
        break;
    case TC_ALIAS:
        analyseArrayType(V, cast<AliasType>(T)->getRefType());
        break;
    case TC_STRUCT:
    case TC_ENUM:
    case TC_FUNCTION:
        FATAL_ERROR("Unreachable");
        break;
    case TC_MODULE:
        TODO;
        break;
    }
}

void FunctionAnalyser::analyseArraySizeExpr(ArrayType* AT) {
    LOG_FUNC
    Expr* E = AT->getSizeExpr();
    if (!E) return;

    QualType T = analyseExpr(E, RHS);
    if (!T.isValid()) return;

    if (inConstExpr && !E->isConstant()) {
        Diag(E->getLocation(), diag::err_vla_decl_in_file_scope) << E->getSourceRange();
        return;
    }
    // check if type is integer
    QualType CT = T.getCanonicalType();
    if (!CT.isBuiltinType() || !cast<BuiltinType>(CT)->isInteger()) {
        StringBuilder buf;
        T.DiagName(buf);
        Diag(E->getLocation(), diag::err_array_size_non_int) << buf << E->getSourceRange();
        return;
    }
    // check if negative
    if (E->getCTC() == CTC_FULL) {
        LiteralAnalyser LA(Diags);
        llvm::APSInt Result = LA.checkLiterals(E);
        if (Result.isSigned() && Result.isNegative()) {
            Diag(E->getLocation(), diag::err_typecheck_negative_array_size) << E->getSourceRange();
        } else {
            AT->setSize(Result);
        }
    }
}

static ExprCTC combineCtc(Expr* Result, const Expr* L, const Expr* R) {
    const ExprCTC left =  L->getCTC();
    const ExprCTC right = R->getCTC();
    switch (left + right) {
    case 0:
        Result->setCTC(CTC_NONE);
        return CTC_NONE;
    case 1:
    case 2:
    case 3:
        Result->setCTC(CTC_PARTIAL);
        return CTC_PARTIAL;
    case 4:
        Result->setCTC(CTC_FULL);
        return CTC_FULL;
    }
    FATAL_ERROR("Unreachable");
    return CTC_NONE;
}

QualType FunctionAnalyser::analyseIntegerLiteral(Expr* expr) {
    IntegerLiteral* I = cast<IntegerLiteral>(expr);
    // Fit smallest Type: int32 > uint32 > int64 > uint64
    // TODO unsigned types

    // TEMP for now assume signed
    // Q: we can determine size, but don't know if we need signed/unsigned
    //unsigned numbits = I->Value.getMinSignedBits();  // signed
    unsigned numbits = I->Value.getActiveBits();   // unsigned
    //if (numbits <= 8) return Type::Int8();
    //if (numbits <= 16) return Type::Int16();
    expr->setType(Type::Int32());
    if (numbits <= 32) return Type::Int32();
    expr->setType(Type::Int64());
    return Type::Int64();
}

static bool isConstantBitOffset(const Expr* E) {
    if (const ArraySubscriptExpr* A = dyncast<ArraySubscriptExpr>(E)) {
        if (const BitOffsetExpr* B = dyncast<BitOffsetExpr>(A->getIndex())) {
            return B->isConstant();
        }
    }
    return false;
}

QualType FunctionAnalyser::analyseBinaryOperator(Expr* expr, unsigned side) {
    LOG_FUNC
    BinaryOperator* binop = cast<BinaryOperator>(expr);
    QualType TLeft, TRight;
    Expr* Left = binop->getLHS();
    Expr* Right = binop->getRHS();

    switch (binop->getOpcode()) {
    case BINOP_Mul:
    case BINOP_Div:
    case BINOP_Rem:
    case BINOP_Add:
    case BINOP_Sub:
    case BINOP_Shl:
    case BINOP_Shr:
        // RHS, RHS
        TLeft = analyseExpr(Left, RHS);
        TRight = analyseExpr(Right, RHS);
        combineCtc(expr, Left, Right);
        if (Left->isConstant() && Right->isConstant()) expr->setConstant();
        break;
    case BINOP_LE:
    case BINOP_LT:
    case BINOP_GE:
    case BINOP_GT:
    case BINOP_NE:
    case BINOP_EQ:
        // RHS, RHS
        TLeft = analyseExpr(Left, RHS);
        TRight = analyseExpr(Right, RHS);
        // NOTE: CTC is never full, because value is not interting, only type
        if (Left->isConstant() && Right->isConstant()) expr->setConstant();
        break;
    case BINOP_And:
    case BINOP_Xor:
    case BINOP_Or:
    case BINOP_LAnd:
    case BINOP_LOr:
        // TODO check if cast of SubExpr is ok
        // RHS, RHS
        TLeft = analyseExpr(Left, RHS);
        TRight = analyseExpr(Right, RHS);
        combineCtc(expr, Left, Right);
        if (Left->isConstant() && Right->isConstant()) expr->setConstant();
        break;
    case BINOP_Assign:
        // LHS, RHS
        TLeft = analyseExpr(Left, side | LHS);
        TRight = analyseExpr(Right, RHS);
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
        TLeft = analyseExpr(Left, LHS | RHS);
        TRight = analyseExpr(Right, RHS);
        break;
    case BINOP_Comma:
        FATAL_ERROR("unhandled binary operator type");
        break;
    }
    if (TLeft.isNull() || TRight.isNull()) return QualType();

    // determine Result type
    QualType Result;
    switch (binop->getOpcode()) {
    case BINOP_Mul:
    case BINOP_Div:
    case BINOP_Rem:
    case BINOP_Add:
    case BINOP_Sub:
        // TODO return largest witdth of left/right (long*short -> long)
        // TODO apply UsualArithmeticConversions() to L + R
        // TEMP for now just return Right side
        // TEMP use UnaryConversion
        Result = UsualUnaryConversions(Left);
        UsualUnaryConversions(Right);
        break;
    case BINOP_Shl:
    case BINOP_Shr:
        Result = TLeft;
        break;
    case BINOP_LE:
    case BINOP_LT:
    case BINOP_GE:
    case BINOP_GT:
    case BINOP_NE:
    case BINOP_EQ:
        Result = Type::Bool();
        break;
    case BINOP_And:
    case BINOP_Xor:
    case BINOP_Or:
        Result = TLeft;
        break;
    case BINOP_LAnd:
    case BINOP_LOr:
        Result = Type::Bool();
        break;
    case BINOP_Assign:
    {
        assert(TRight.isValid());
        // special case for a[4:2] = b
        if (isConstantBitOffset(Left) && Right->isConstant()) {
            LiteralAnalyser LA(Diags);
            LA.checkBitOffset(Left, Right);
        } else {
            EA.check(TLeft, Right);
        }
        checkAssignment(Left, TLeft);
        Result = TLeft;
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
        checkAssignment(Left, TLeft);
        Result = TLeft;
        break;
    case BINOP_Comma:
        FATAL_ERROR("unhandled binary operator type");
        break;
    }
    expr->setType(Result);
    return Result;
}

QualType FunctionAnalyser::analyseConditionalOperator(Expr* expr) {
    LOG_FUNC
    ConditionalOperator* condop = cast<ConditionalOperator>(expr);
    QualType TCond = analyseExpr(condop->getCond(), RHS);
    if (!TCond.isValid()) return QualType();

    // check if Condition can be casted to bool
    EA.check(Type::Bool(), condop->getCond());
    QualType TLeft = analyseExpr(condop->getLHS(), RHS);

    QualType TRight = analyseExpr(condop->getRHS(), RHS);
    if (!TRight.isValid()) return QualType();

    expr->setCTC(CTC_PARTIAL);  // always set to Partial for ExprTypeAnalyser
    return TLeft;
}

QualType FunctionAnalyser::analyseUnaryOperator(Expr* expr, unsigned side) {
    LOG_FUNC
    UnaryOperator* unaryop = cast<UnaryOperator>(expr);
    Expr* SubExpr = unaryop->getExpr();
    QualType LType;
    // TODO cleanup, always break and do common stuff at end
    switch (unaryop->getOpcode()) {
    case UO_PostInc:
    case UO_PostDec:
    case UO_PreInc:
    case UO_PreDec:
        LType = analyseExpr(SubExpr, side | LHS);
        if (LType.isNull()) return 0;
        checkAssignment(SubExpr, LType);
        expr->setType(LType);
        break;
    case UO_AddrOf:
    {
        LType = analyseExpr(SubExpr, side | RHS);
        if (LType.isNull()) return 0;
        QualType Q = Context.getPointerType(LType);
        expr->setType(Q);
        expr->setConstant();
        // TODO use return type
        TR.resolveCanonicals(0, Q, true);
        return Q;
    }
    case UO_Deref:
        LType = analyseExpr(SubExpr, side | RHS);
        if (LType.isNull()) return 0;
        if (!LType.isPointerType()) {
            char typeName[MAX_LEN_TYPENAME];
            StringBuilder buf(MAX_LEN_TYPENAME, typeName);
            LType.DiagName(buf);
            Diag(unaryop->getOpLoc(), diag::err_typecheck_indirection_requires_pointer)
                    << buf;
            return 0;
        } else {
            // TEMP use CanonicalType to avoid Unresolved types etc
            QualType Q = LType.getCanonicalType();
            const PointerType* P = cast<PointerType>(Q);
            expr->setType(P->getPointeeType());
            return P->getPointeeType();
        }
        break;
    case UO_Minus:
    case UO_Not:
        LType = analyseExpr(SubExpr, side | RHS);
        unaryop->setCTC(SubExpr->getCTC());
        if (SubExpr->isConstant()) unaryop->setConstant();
        if (LType.isNull()) return 0;
        expr->setType(UsualUnaryConversions(SubExpr));
        break;
    case UO_LNot:
        // TODO first cast expr to bool, then invert here, return type bool
        // TODO extract to function
        LType = analyseExpr(SubExpr, side | RHS);
        // TODO check conversion to bool here!!
        unaryop->setCTC(SubExpr->getCTC());
        // Also set type?
        if (LType.isNull()) return 0;
        //UsualUnaryConversions(SubExpr);
        LType = Type::Bool();
        expr->setType(LType);
        break;
    }
    return LType;
}

QualType FunctionAnalyser::analyseBuiltinExpr(Expr* expr) {
    LOG_FUNC
    BuiltinExpr* B = cast<BuiltinExpr>(expr);
    expr->setType(Type::UInt32());
    switch (B->getBuiltinKind()) {
    case BuiltinExpr::BUILTIN_SIZEOF:
        if (!analyseSizeOfExpr(B)) return QualType();
        break;
    case BuiltinExpr::BUILTIN_ELEMSOF:
        return analyseElemsOfExpr(B);
    case BuiltinExpr::BUILTIN_ENUM_MIN:
        return analyseEnumMinMaxExpr(B, true);
    case BuiltinExpr::BUILTIN_ENUM_MAX:
        return analyseEnumMinMaxExpr(B, false);
    }

    return Type::UInt32();
}

QualType FunctionAnalyser::analyseArraySubscript(Expr* expr, unsigned side) {
    LOG_FUNC
    ArraySubscriptExpr* sub = cast<ArraySubscriptExpr>(expr);
    QualType LType = analyseExpr(sub->getBase(), RHS);
    if (LType.isNull()) return 0;

    if (BitOffsetExpr* BO = dyncast<BitOffsetExpr>(sub->getIndex())) {
        if (side & LHS) {
            Diag(BO->getLocation(), diag::err_bitoffset_lhs) << expr->getSourceRange();
            return 0;
        }
        QualType T = analyseBitOffsetExpr(sub->getIndex(), LType, sub->getLocation());
        expr->setType(T);
        combineCtc(expr, sub->getBase(), sub->getIndex());
        // dont analyse partial BitOffset expressions per part
        if (expr->getCTC() == CTC_PARTIAL) expr->setCTC(CTC_NONE);
        return T;
    }

    analyseExpr(sub->getIndex(), RHS);

    if (!LType.isSubscriptable()) {
        Diag(expr->getLocation(), diag::err_typecheck_subscript);
        return 0;
    }

    QualType Result;
    if (isa<PointerType>(LType)) {
        Result = cast<PointerType>(LType)->getPointeeType();
    } else if (isa<ArrayType>(LType)) {
        Result = cast<ArrayType>(LType)->getElementType();
    }
    expr->setType(Result);
    return Result;
}

QualType FunctionAnalyser::analyseMemberExpr(Expr* expr, unsigned side) {
    LOG_FUNC
    MemberExpr* M = cast<MemberExpr>(expr);
    IdentifierExpr* member = M->getMember();

    // we dont know what we're looking at here, it could be:
    // mod.Type
    // mod.var
    // mod.func
    // var<Type=struct>.member
    // var<Type=struct>.struct_function
    // var[index].member
    // var->member (= error)
    // Type.struct_function
    // Enum.Constant (eg. Color.Red)

    QualType LType = analyseExpr(M->getBase(), RHS);
    if (!LType.isValid()) return QualType();

    if (isa<ModuleType>(LType)) {
        M->setModulePrefix();
        ModuleType* MT = cast<ModuleType>(LType.getTypePtr());
        Decl* D = scope.findSymbolInModule(member->getName(), member->getLocation(), MT->getModule());
        if (D) {
            if (side & RHS) D->setUsed();
            M->setDecl(D);
            SetConstantFlags(D, M);
            QualType Q = D->getType();
            expr->setType(Q);
            member->setType(Q);
            member->setDecl(D, globalDecl2RefType(D));
            return Q;
        }
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
        M->setType(Q);
        M->setIsEnumConstant();
        SetConstantFlags(ECD, M);
        member->setCTC(CTC_FULL);
        member->setConstant();
        member->setType(Q);
        member->setDecl(ECD, globalDecl2RefType(ECD));
        return Q;
    } else {
        // dereference pointer
        if (LType.isPointerType()) {
            // TODO use function to get elementPtr (could be alias type)
            const PointerType* PT = cast<PointerType>(LType.getTypePtr());
            LType = PT->getPointeeType();
        }
        QualType S = getStructType(LType);
        if (!S.isValid()) {
            StringBuilder buf(MAX_LEN_TYPENAME);
            LType.DiagName(buf);
            Diag(M->getLocation(), diag::err_typecheck_member_reference_struct_union)
                    << buf << M->getSourceRange() << member->getLocation();
            return QualType();
        }
        return analyseStructMember(S, M, side, exprIsType(M->getBase()));
    }
    return QualType();
}



QualType FunctionAnalyser::analyseStaticStructFunction(QualType T, MemberExpr* M, const StructTypeDecl* S, unsigned side)
{
    IdentifierExpr* member = M->getMember();

    // Let's return the field if we find it.
    Decl *field = S->findMember(member->getName());
    if (field) {

        // TODO maybe check opaque?
        //if (CurrentFunction->getModule() != S->getModule() && S->hasAttribute(ATTR_OPAQUE)) {
        //    Diag(S->getLocation(), diag::err_deref_opaque) << S->isStruct() << S->DiagName();
        //   return QualType();
        //}

        field->setUsed();

        // Setup from function
        M->setDecl(field);
        M->setType(field->getType());

        // TODO should we set something like this?
        // M->setIsStaticStructFunction();

        member->setDecl(field, IdentifierExpr::REF_STRUCT_MEMBER);
        member->setType(field->getType());

        return field->getType();
    }

    // Analyzed as static & struct function.
     M->setIsStructFunction();
     M->setIsStaticStructFunction();

    // This is the right hand side.
    FunctionDecl* match = S->findFunction(member->getName());
    if (!match) {
        outputStructDiagnostics(T, member, diag::err_no_struct_func);
        return QualType();
    }

    // NOTE: static struct-functions are allowed outside call expr
    scope.checkAccess(match, member->getLocation());

    // The struct function can either be used as the pointer to
    // a function or a member call. Consequently simply
    // skip the call stack here.
    if (callStack.callDepth > 0) {
        callStack.setStructFunction(M->getBase());
    }

    // If right hand side of an expression, mark as used.
    if (side & RHS) match->setUsed();

    // Setup from function
    M->setDecl(match);
    M->setType(match->getType());

    // Set the member part as the function call.
    member->setDecl(match, IdentifierExpr::REF_STRUCT_FUNC);
    member->setType(match->getType());
    SetConstantFlags(match, M);
    return match->getType();
}

void FunctionAnalyser::outputStructDiagnostics(QualType T, IdentifierExpr* member, unsigned msg)
{
    char temp1[MAX_LEN_TYPENAME];
    StringBuilder buf1(MAX_LEN_TYPENAME, temp1);
    T->DiagName(buf1);
    char temp2[MAX_LEN_VARNAME];
    StringBuilder buf2(MAX_LEN_VARNAME, temp2);
    buf2 << '\'' << member->getName() << '\'';
    Diag(member->getLocation(), msg) << temp2 << temp1;
}

// T is the type of the struct
// M the whole expression
// isStatic is true for Foo.myFunction(...)
QualType FunctionAnalyser::analyseStructMember(QualType T, MemberExpr* M, unsigned side, bool isStatic) {
    LOG_FUNC
    assert(M && "Expression missing");
    const StructType* ST = cast<StructType>(T);
    const StructTypeDecl* S = ST->getDecl();

    if (isStatic) return analyseStaticStructFunction(T, M, S, side);

    assert(CurrentFunction);

    IdentifierExpr* member = M->getMember();
    Decl* match = S->find(member->getName());

    if (!match) {
        outputStructDiagnostics(T, member, diag::err_no_member_struct_func);
        return QualType();
    }

    IdentifierExpr::RefType ref = IdentifierExpr::REF_STRUCT_MEMBER;
    FunctionDecl* func = dyncast<FunctionDecl>(match);

    if (func) {
        scope.checkAccess(func, member->getLocation());

        if (!checkStructTypeArg(T, func)) {
            Diag(member->getLocation(), diag::err_static_struct_func_notype);// << func->DiagName();
            return QualType();
        }
        M->setIsStructFunction();
        ref = IdentifierExpr::REF_STRUCT_FUNC;

        // Is this a simple member access? If so
        // disallow this (we don't have closures!)
        if (callStack.callDepth == 0) {
            Diag(member->getLocation(), diag::err_non_static_struct_func_usage)
                << M->getBase()->getSourceRange();
            return QualType();
        }
        callStack.setStructFunction(M->getBase());
    }
    else {
        // NOTE: access of struct-function is not a dereference
        if (CurrentFunction->getModule() != S->getModule() && S->hasAttribute(ATTR_OPAQUE)) {
            Diag(M->getLocation(), diag::err_deref_opaque) << S->isStruct() << S->DiagName();
            return QualType();
        }
    }

    if (side & RHS) match->setUsed();
    M->setDecl(match);
    M->setType(match->getType());
    member->setDecl(match, ref);
    member->setType(match->getType());
    return match->getType();
}


bool FunctionAnalyser::checkStructTypeArg(QualType T,  FunctionDecl* func) const {
    if (func->numArgs() == 0) return false;

    VarDecl* functionArg = func->getArg(0);
    Decl* functionArgDecl = getStructDecl(functionArg->getType());

    // might be done more effictien by not creating PointerType first
    QualType callArgType = Context.getPointerType(T);
    Decl* callArgDecl = getStructDecl(callArgType);
    return (functionArgDecl == callArgDecl);
}

Decl* FunctionAnalyser::getStructDecl(QualType T) const {
    // try to convert QualType(Struct*) -> StructDecl
    const PointerType* pt = dyncast<PointerType>(T);
    if (!pt) return 0;

    const StructType* st = dyncast<StructType>(pt->getPointeeType());
    if (!st) return 0;

    return st->getDecl();
}

bool FunctionAnalyser::exprIsType(const Expr* E) const {
    // can be IdentifierExpr or MemberExpr
    switch (E->getKind()) {
    case EXPR_IDENTIFIER:
    {
        const IdentifierExpr* I = cast<IdentifierExpr>(E);
        return I->isType();
    }
    case EXPR_MEMBER:
    {
        const MemberExpr* M = cast<MemberExpr>(E);
        return M->getMember()->isType();
    }
    default:
        break;
    }
    return false;
}

QualType FunctionAnalyser::analyseParenExpr(Expr* expr) {
    LOG_FUNC
    ParenExpr* P = cast<ParenExpr>(expr);
    QualType Q = analyseExpr(P->getExpr(), RHS);

    expr->setCTC(P->getExpr()->getCTC());
    if (P->getExpr()->isConstant()) expr->setConstant();
    expr->setType(Q);
    return Q;
}

// return whether Result is valid
bool FunctionAnalyser::analyseBitOffsetIndex(Expr* expr, llvm::APSInt* Result, BuiltinType* BaseType) {
    LOG_FUNC
    QualType T = analyseExpr(expr, RHS);
    if (!T.isValid()) return false;

    if (!T.getCanonicalType().isIntegerType()) {
        StringBuilder buf;
        T.DiagName(buf);
        Diag(expr->getLocation(), diag::err_bitoffset_index_non_int)
                << buf << expr->getSourceRange();
        return false;
    }
    if (!expr->isConstant()) return false;

    LiteralAnalyser LA(Diags);
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
    bool VLvalid = analyseBitOffsetIndex(B->getLHS(), &VL, BI);
    llvm::APSInt VR;
    bool VRvalid = analyseBitOffsetIndex(B->getRHS(), &VR, BI);

    if (VLvalid && VRvalid) {
        if (VR > VL) {
            Diag(B->getLocation(), diag::err_bitoffset_invalid_order)
                    << B->getLHS()->getSourceRange() << B->getRHS()->getSourceRange();
            return QualType();
        } else {
            llvm::APInt width = VL - VR;
            ++width;

            QualType T;
            if (width.ult(8+1)) {
                T = Type::UInt8();
            } else if (width.ult(16+1)) {
                T = Type::UInt16();
            } else if (width.ult(32+1)) {
                T = Type::UInt32();
            } else if (width.ult(64+1)) {
                T = Type::UInt64();
            } else {
                FATAL_ERROR("Unreachable");
            }
            B->setType(T);
            B->setWidth((unsigned char)width.getSExtValue());
        }
    } else {
        B->setType(BaseType);
    }

    if (B->getLHS()->isConstant() && B->getRHS()->isConstant()) B->setConstant();
    combineCtc(B, B->getLHS(), B->getRHS());

    return B->getType();
}

QualType FunctionAnalyser::analyseExplicitCastExpr(Expr* expr) {
    LOG_FUNC
    ExplicitCastExpr* E = cast<ExplicitCastExpr>(expr);

    QualType outerType = TR.resolveType(E->getDestType(), false);
    if (outerType.isValid()) {
        E->setType(outerType);
        E->setDestType(outerType);
    }

    QualType innerType = analyseExpr(E->getInner(), RHS);
    if (innerType.isValid()) {
        E->setCTC(E->getInner()->getCTC());

        if (outerType.isValid()) {
            if (!EA.checkExplicitCast(E, outerType, innerType))
                return QualType();
        }
    }

    return outerType.isValid() ? outerType : QualType();
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
    QualType LType = analyseExpr(call->getFn(), RHS);
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
    call->setType(func->getReturnType());
    if (structFunction) call->setIsStructFunction();
    if (!checkCallArgs(func, call, structFunction)) return QualType();
    return func->getReturnType();
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
        if (exprIsType(structFunction)) {
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
        Expr* callArg = call->getArg(callIndex);
        QualType callArgType = analyseExpr(callArg, RHS);
        if (callArgType.isValid()) {
            VarDecl* funcArg = func->getArg(funcIndex);
            QualType funcArgType = funcArg->getType();
            assert(funcArgType.isValid());
            EA.check(funcArgType, callArg);
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
            Expr* callArg = call->getArg(callIndex);
            QualType callArgType = analyseExpr(callArg, RHS);
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
        id->setDecl(D, globalDecl2RefType(D));
        id->setType(D->getType());
        SetConstantFlags(D, id);
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

bool FunctionAnalyser::checkAssignee(Expr* expr) const {
    switch (expr->getKind()) {
    case EXPR_INTEGER_LITERAL:
    case EXPR_FLOAT_LITERAL:
    case EXPR_BOOL_LITERAL:
    case EXPR_CHAR_LITERAL:
    case EXPR_STRING_LITERAL:
    case EXPR_NIL:
        break;
    case EXPR_CALL:
    case EXPR_IDENTIFIER:
        // ok
        return true;
    case EXPR_INITLIST:
    case EXPR_DESIGNATOR_INIT:
    case EXPR_TYPE:
        break;
    case EXPR_BINOP:
        // ok
        return true;
    case EXPR_CONDOP:
        break;
    case EXPR_UNARYOP:
        // sometimes... (&)
        return true;
    case EXPR_BUILTIN:
        break;
    case EXPR_ARRAYSUBSCRIPT:
    case EXPR_MEMBER:
    case EXPR_PAREN:
    case EXPR_BITOFFSET:
        // ok
        return true;
    case EXPR_CAST:
        TODO;
        break;
    }
    // expr is not assignable
    // TODO test (also ternary)
    Diag(expr->getLocation(), diag::err_typecheck_expression_not_modifiable_lvalue);
    return false;
}

void FunctionAnalyser::checkAssignment(Expr* assignee, QualType TLeft) {
    if (TLeft.isArrayType()) {
        ArrayType* AT = cast<ArrayType>(TLeft.getTypePtr());
        if (AT->isIncremental()) {
            Diag(assignee->getLocation(), diag::err_incremental_array_value_function_scope);
        }
    }
    if (TLeft.isConstQualified()) {
        Diag(assignee->getLocation(), diag::err_typecheck_assign_const) << 5 << assignee->getSourceRange();
    }
}

void FunctionAnalyser::checkDeclAssignment(Decl* decl, Expr* expr) {
    LOG_FUNC
    assert(decl);
    switch (decl->getKind()) {
    case DECL_FUNC:
        // error
        fprintf(stderr, "TODO cannot assign to non-variable\n");
        TODO;
        break;
    case DECL_VAR:
    {
        //VarDecl* VD = cast<VarDecl>(decl);
        //checkAssignment(expr, VD->getType());
        break;
    }
    case DECL_ENUMVALUE:
    case DECL_ALIASTYPE:
    case DECL_STRUCTTYPE:
    case DECL_ENUMTYPE:
    case DECL_FUNCTIONTYPE:
        Diag(expr->getLocation(), diag::err_typecheck_expression_not_modifiable_lvalue);
        break;
    case DECL_ARRAYVALUE:
        // error
        fprintf(stderr, "TODO cannot assign to non-variable\n");
        TODO;
        break;
    case DECL_IMPORT:
    case DECL_LABEL:
        Diag(expr->getLocation(), diag::err_typecheck_expression_not_modifiable_lvalue);
        break;
    }
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
        // TODO add test cases!!
        // only allow enum states as cases if switchin on enum (MemberExpr -> EnumConstantDecl)
        const Expr* cond = CS->getCond();
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
        StringBuilder buf(64);
        ET->DiagName(buf);
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

QualType FunctionAnalyser::getStructType(QualType Q) const {
    // Q: use CanonicalType to get rid of AliasTypes?

    Type* T = Q.getTypePtr();
    switch (T->getTypeClass()) {
    case TC_BUILTIN:
        return QualType();
    case TC_POINTER:
    {
        // Could be pointer to structtype
        PointerType* PT = cast<PointerType>(T);
        return getStructType(PT->getPointeeType());
    }
    case TC_ARRAY:
        return QualType();
    case TC_UNRESOLVED:
        FATAL_ERROR("Unreachable");
        return QualType();
    case TC_ALIAS:
    {
        AliasType* AT = cast<AliasType>(T);
        return getStructType(AT->getRefType());
    }
    case TC_STRUCT:
        return Q;
    case TC_ENUM:
    case TC_FUNCTION:
    case TC_MODULE:
        return QualType();
    }
    FATAL_ERROR("Unreachable");
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

// Convert smaller types to int, others remain the same
// This function should only be called if Expr's type is ok for unary operator
QualType FunctionAnalyser::UsualUnaryConversions(Expr* expr) const {
    const Type* canon = expr->getType().getCanonicalType();

    if (const BuiltinType* BI = cast<BuiltinType>(canon)) {
        if (BI->isPromotableIntegerType()) {
            // TODO keep flags (const, etc)?
            expr->setImpCast(BuiltinType::Int32);
            return Type::Int32();
        }
    }

    if (canon->isPointerType()) {
        // TODO use TargetInfo to check if 32-bit
        return Type::UInt64();
    }

    return expr->getType();
}

