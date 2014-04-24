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

#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/APInt.h>
#include <clang/Parse/ParseDiagnostic.h>
#include <clang/Sema/SemaDiagnostic.h>

#include "Analyser/FunctionAnalyser.h"
#include "Analyser/TypeChecker.h"
#include "Analyser/AnalyserUtils.h"
#include "Analyser/LiteralAnalyser.h"
#include "Analyser/constants.h"
#include "AST/Decl.h"
#include "AST/Expr.h"
#include "AST/Stmt.h"
#include "AST/Package.h"
#include "Utils/color.h"
#include "Utils/StringBuilder.h"

using namespace C2;
using namespace clang;

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
    case DECL_USE:
        break;
    }
    // TODO needed?
    expr->setCTC(CTC_NONE);
}


FunctionAnalyser::FunctionAnalyser(Scope& scope_,
                                   TypeChecker& typeRes_,
                                   TypeContext& tc,
                                   clang::DiagnosticsEngine& Diags_)
    : scope(scope_)
    , TC(typeRes_)
    , typeContext(tc)
    , EA(TC, Diags_)
    , Diags(Diags_)
    , errors(0)
    , CurrentFunction(0)
    , CurrentVarDecl(0)
    , constDiagID(0)
    , inConstExpr(false)
{
}

unsigned FunctionAnalyser::check(FunctionDecl* func) {
    CurrentFunction = func;
    errors = 0;
    scope.EnterScope(Scope::FnScope | Scope::DeclScope);

    checkFunction(func);

    scope.ExitScope();
    CurrentFunction = 0;
    return errors;
}

unsigned FunctionAnalyser::checkVarInit(VarDecl* V) {
    LOG_FUNC
    CurrentVarDecl = V;
    errors = 0;

    ConstModeSetter cms(*this, diag::err_init_element_not_constant);
    analyseInitExpr(V->getInitValue(), V->getType());
    // TODO if type is array, update type of VarDecl? (add size)

    CurrentVarDecl = 0;
    return errors;
}

unsigned FunctionAnalyser::checkArrayExpr(ArrayType* AT, Expr* E) {
    LOG_FUNC
    errors = 0;

    ConstModeSetter cms(*this, diag::err_init_element_not_constant);
    analyseArraySizeExpr(AT, E);

    return errors;
}

unsigned FunctionAnalyser::checkEnumValue(EnumConstantDecl* E, llvm::APSInt& nextValue) {
    LOG_FUNC;

    LiteralAnalyser LA(Diags);
    Expr* Init = E->getInitValue();
    if (Init) {
        QualType T = analyseExpr(Init, RHS);
        if (!T.isValid()) return 1;

        if (!Init->isConstant()) {
            Diags.Report(Init->getLocation(), diag::err_expr_not_ice) << 0 << Init->getSourceRange();
            return 1;
        }
        // TODO refactor duplicate code with analyseArraySizeExpr()
        QualType CT = T.getCanonicalType();
        if (!CT.isBuiltinType() || !cast<BuiltinType>(CT)->isInteger()) {
            Diags.Report(Init->getLocation(), diag::err_expr_not_ice) << 0 << Init->getSourceRange();
            return 1;
        }
        assert(Init->getCTC() == CTC_FULL);
        llvm::APSInt V = LA.checkLiterals(Init);
        nextValue = V;
    }
    E->setValue(nextValue);
    nextValue++;
    if (!LA.checkRange(E->getType(), Init, E->getLocation(), E->getValue())) return 1;

    return 0;
}

void FunctionAnalyser::checkFunction(FunctionDecl* func) {
    LOG_FUNC
    // add arguments to new scope
    for (unsigned i=0; i<func->numArgs(); i++) {
        VarDecl* arg = func->getArg(i);
        if (arg->getName() != "") {
            // check that argument names dont clash with globals
            if (!scope.checkScopedSymbol(arg)) {
                errors++;
                continue;
            }
            scope.addScopedSymbol(arg);
        }
    }
    if (errors) return;

    analyseCompoundStmt(func->getBody());
    if (errors) return;

    // check for return statement of return value is required
    QualType rtype = func->getReturnType();
    bool need_rvalue = (rtype.getTypePtr() != BuiltinType::get(BuiltinType::Void));
    if (need_rvalue) {
        CompoundStmt* compound = func->getBody();
        Stmt* lastStmt = compound->getLastStmt();
        if (!lastStmt || lastStmt->getKind() != STMT_RETURN) {
            Diags.Report(compound->getRight(), diag::warn_falloff_nonvoid_function);
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
        assert(0 && "not done here");
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
        // TODO
        break;
    case STMT_COMPOUND:
        if (!haveScope) scope.EnterScope(Scope::DeclScope);
        analyseCompoundStmt(S);
        if (!haveScope) scope.ExitScope();
        break;
    }
}

void FunctionAnalyser::analyseCompoundStmt(Stmt* stmt) {
    LOG_FUNC
    assert(CurrentFunction);
    CompoundStmt* compound = cast<CompoundStmt>(stmt);
    const StmtList& stmts = compound->getStmts();
    for (unsigned i=0; i<stmts.size(); i++) {
        analyseStmt(stmts[i]);
    }
}

void FunctionAnalyser::analyseIfStmt(Stmt* stmt) {
    LOG_FUNC
    IfStmt* I = cast<IfStmt>(stmt);
    Expr* cond = I->getCond();
    QualType Q1 = analyseExpr(cond, RHS);
    if (Q1.isValid()) {
        EA.check(Type::Bool(), cond);
    }
    scope.EnterScope(Scope::DeclScope);
    analyseStmt(I->getThen(), true);
    scope.ExitScope();

    Stmt* elseSt = I->getElse();
    if (elseSt) {
        scope.EnterScope(Scope::DeclScope);
        analyseStmt(elseSt, true);
        scope.ExitScope();
    }
}

void FunctionAnalyser::analyseWhileStmt(Stmt* stmt) {
    LOG_FUNC
    WhileStmt* W = cast<WhileStmt>(stmt);
    // TODO check cast to Bool, but Stmts don't have LType!
    analyseStmt(W->getCond());
    scope.EnterScope(Scope::BreakScope | Scope::ContinueScope | Scope::DeclScope | Scope::ControlScope);
    analyseStmt(W->getBody(), true);
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
    if (F->getCond()) analyseExpr(F->getCond(), RHS);
    if (F->getIncr()) analyseExpr(F->getIncr(), RHS);
    analyseStmt(F->getBody(), true);
    scope.ExitScope();
}

void FunctionAnalyser::analyseSwitchStmt(Stmt* stmt) {
    LOG_FUNC
    SwitchStmt* S = cast<SwitchStmt>(stmt);
    analyseExpr(S->getCond(), RHS);
    const StmtList& Cases = S->getCases();
    Stmt* defaultStmt = 0;
    scope.EnterScope(Scope::BreakScope | Scope::SwitchScope);
    for (unsigned i=0; i<Cases.size(); i++) {
        Stmt* C = Cases[i];
        switch (C->getKind()) {
        case STMT_CASE:
            analyseCaseStmt(C);
            break;
        case STMT_DEFAULT:
            if (defaultStmt) {
                Diags.Report(C->getLocation(), diag::err_multiple_default_labels_defined);
                Diags.Report(defaultStmt->getLocation(), diag::note_duplicate_case_prev);
            } else {
                defaultStmt = C;
            }
            analyseDefaultStmt(C);
            break;
        default:
            assert(0);
        }
    }
    scope.ExitScope();
}

void FunctionAnalyser::analyseBreakStmt(Stmt* stmt) {
    LOG_FUNC
    if (!scope.allowBreak()) {
        BreakStmt* B = cast<BreakStmt>(stmt);
        Diags.Report(B->getLocation(), diag::err_break_not_in_loop_or_switch);
    }
}

void FunctionAnalyser::analyseContinueStmt(Stmt* stmt) {
    LOG_FUNC
    if (!scope.allowContinue()) {
        ContinueStmt* C = cast<ContinueStmt>(stmt);
        Diags.Report(C->getLocation(), diag::err_continue_not_in_loop);
    }
}

void FunctionAnalyser::analyseLabelStmt(Stmt* S) {
    LOG_FUNC
    LabelStmt* L = cast<LabelStmt>(S);
    // TODO check label itself
    analyseStmt(L->getSubStmt());
}

void FunctionAnalyser::analyseCaseStmt(Stmt* stmt) {
    LOG_FUNC
    CaseStmt* C = cast<CaseStmt>(stmt);
    analyseExpr(C->getCond(), RHS);
    const StmtList& stmts = C->getStmts();
    for (unsigned i=0; i<stmts.size(); i++) {
        analyseStmt(stmts[i]);
    }
}

void FunctionAnalyser::analyseDefaultStmt(Stmt* stmt) {
    LOG_FUNC
    DefaultStmt* D = cast<DefaultStmt>(stmt);
    const StmtList& stmts = D->getStmts();
    for (unsigned i=0; i<stmts.size(); i++) {
        analyseStmt(stmts[i]);
    }
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
            Diags.Report(ret->getLocation(), diag::ext_return_has_expr) << CurrentFunction->getName() << 0
                << value->getSourceRange();
        } else {
            if (type.isValid()) {
                EA.check(rtype, value);
            }
        }
    } else {
        if (!no_rvalue) {
            Diags.Report(ret->getLocation(), diag::ext_return_missing_expr) << CurrentFunction->getName() << 0;
        }
    }
}

void FunctionAnalyser::analyseStmtExpr(Stmt* stmt) {
    LOG_FUNC
    Expr* expr = cast<Expr>(stmt);
    analyseExpr(expr, 0);
}

C2::QualType FunctionAnalyser::Decl2Type(Decl* decl) {
    LOG_FUNC
    assert(decl);
    switch (decl->getKind()) {
    case DECL_FUNC:
        {
            FunctionDecl* FD = cast<FunctionDecl>(decl);
            return FD->getType();
        }
    case DECL_VAR:
        {
            VarDecl* VD = cast<VarDecl>(decl);
            return resolveUserType(VD->getType());
        }
    case DECL_ENUMVALUE:
        {
            EnumConstantDecl* EC = cast<EnumConstantDecl>(decl);
            return EC->getType();
        }
    case DECL_ALIASTYPE:
    case DECL_STRUCTTYPE:
    case DECL_ENUMTYPE:
    case DECL_FUNCTIONTYPE:
        {
            TypeDecl* TD = cast<TypeDecl>(decl);
            return TD->getType();
        }
    case DECL_ARRAYVALUE:
    case DECL_USE:
        assert(0);
        break;
    }
    QualType qt;
    return qt;
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
            // return type: 'const char*'
            QualType Q = typeContext.getPointerType(Type::Int8());
            Q.addConst();
            if (!Q->hasCanonicalType()) Q->setCanonicalType(Q);
            expr->setType(Q);
            return Q;
        }
    case EXPR_NIL:
        {
            QualType Q = typeContext.getPointerType(Type::Void());
            if (!Q->hasCanonicalType()) Q->setCanonicalType(Q);
            expr->setType(Q);
            return Q;
        }
    case EXPR_CALL:
        return analyseCall(expr);
    case EXPR_IDENTIFIER:
        {
            IdentifierExpr* id = cast<IdentifierExpr>(expr);
            ScopeResult Res = analyseIdentifier(id);
            if (Res.getPackage()) {
                Diags.Report(id->getLocation(), diag::err_is_a_package) << id->getName();
                break;
            }
            Decl* D = Res.getDecl();
            if (!D) break;
            if (D == CurrentVarDecl) {
                Diags.Report(id->getLocation(), diag::err_var_self_init) << D->getName();
                return QualType();
            }
            if (side & LHS) checkDeclAssignment(D, expr);
            // TODO move this to analyseIdentifier?
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
            case DECL_ALIASTYPE:
            case DECL_STRUCTTYPE:
            case DECL_ENUMTYPE:
            case DECL_FUNCTIONTYPE:
                Diags.Report(id->getLocation(), diag::err_unexpected_typedef) << id->getName();
                expr->setConstant();
                break;
            case DECL_ARRAYVALUE:
            case DECL_USE:
                break;
            }
            if (side & RHS) D->setUsed();
            QualType T = Decl2Type(D);
            expr->setType(T);
            return T;
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
        return analyseBinaryOperator(expr, side);
    case EXPR_CONDOP:
        return analyseConditionalOperator(expr);
    case EXPR_UNARYOP:
        return analyseUnaryOperator(expr, side);
    case EXPR_BUILTIN:
        return analyseBuiltinExpr(expr);
    case EXPR_ARRAYSUBSCRIPT:
        return analyseArraySubscript(expr);
    case EXPR_MEMBER:
        return analyseMemberExpr(expr, side);
    case EXPR_PAREN:
        return analyseParenExpr(expr);
    }
    return QualType();
}

void FunctionAnalyser::analyseInitExpr(Expr* expr, QualType expectedType) {
    LOG_FUNC
    InitListExpr* ILE = dyncast<InitListExpr>(expr);
    const ArrayType* AT = dyncast<ArrayType>(expectedType.getCanonicalType());
    if (AT) {
        const QualType ET = AT->getElementType();
        bool isCharArray = (ET == Type::Int8() || ET == Type::UInt8());

        if (isCharArray) {
            if (!ILE && !isa<StringLiteral>(expr)) {
                Diags.Report(expr->getLocation(), diag::err_array_init_not_init_list) << 1;
                return;
            }
        } else {
            if (!ILE) {
                Diags.Report(expr->getLocation(), diag::err_array_init_not_init_list) << 0;
                return;
            }
        }
    }
    if (ILE) {
        analyseInitList(ILE, expectedType);
    } else {
        QualType Q = analyseExpr(expr, RHS);
        if (Q.isValid()) {
            if (inConstExpr && !expr->isConstant()) {
                assert(constDiagID);
                Diags.Report(expr->getLocation(), constDiagID) << expr->getSourceRange();
            } else {
                EA.check(expectedType, expr);
            }
            if (AT && AT->getSizeExpr()) {
                // it should be char array type already and expr is string literal
                assert(isa<StringLiteral>(expr));
                const StringLiteral* S = cast<StringLiteral>(expr);
                if (S->getByteLength() > AT->getSize().getZExtValue()) {
                    Diags.Report(S->getLocation(), diag::err_initializer_string_for_char_array_too_long) << S->getSourceRange();
                    return;
                }
            }
        }
    }
}

void FunctionAnalyser::analyseInitList(InitListExpr* expr, QualType Q) {
    LOG_FUNC
    ExprList& values = expr->getValues();
    if (Q.isArrayType()) {
        // TODO use helper function
        ArrayType* AT = cast<ArrayType>(Q.getCanonicalType().getTypePtr());
        QualType ET = AT->getElementType();
        // TODO check if size is specifier in type
        bool constant = true;
        for (unsigned i=0; i<values.size(); i++) {
            analyseInitExpr(values[i], ET);
            if (!values[i]->isConstant()) constant = false;
        }
        if (constant) expr->setConstant();
    } else if (Q.isStructType()) {
        // TODO use helper function
        StructType* TT = cast<StructType>(Q.getCanonicalType().getTypePtr());
        StructTypeDecl* STD = TT->getDecl();
        assert(STD->isStruct() && "TEMP only support structs for now");
        bool constant = true;
        for (unsigned i=0; i<values.size(); i++) {
            if (i >= STD->numMembers()) {
                // note: 0 for array, 2 for scalar, 3 for union, 4 for structs
                Diags.Report(values[STD->numMembers()]->getLocation(), diag::err_excess_initializers)
                    << 4;
                errors++;
                return;
            }
            VarDecl* VD = dyncast<VarDecl>(STD->getMember(i));
            assert(VD && "TEMP don't support sub-struct member inits");
            analyseInitExpr(values[i], VD->getType());
            if (!values[i]->isConstant()) constant = false;
        }
        if (constant) expr->setConstant();
    } else {
        // only allow 1
        switch (values.size()) {
        case 0:
            fprintf(stderr, "TODO ERROR: scalar initializer cannot be empty\n");
            errors++;
            break;
        case 1:
            // Q: allow initlist for single var?-> NO
            //see clang: cannot initialize variable of type %0 with initializer list">
            break;
        default:
            Diags.Report(values[1]->getLocation(), diag::err_excess_initializers) << 2;
            errors++;
            break;
        }
        return;
    }
}

void FunctionAnalyser::analyseSizeofExpr(Expr* expr) {
    switch (expr->getKind()) {
    case EXPR_INTEGER_LITERAL:
    case EXPR_FLOAT_LITERAL:
    case EXPR_BOOL_LITERAL:
    case EXPR_CHAR_LITERAL:
    case EXPR_STRING_LITERAL:
    case EXPR_NIL:
        // ok
        return;
    case EXPR_CALL:
        // not allowed
        assert(0 && "TODO good error");
        return;
    case EXPR_IDENTIFIER:
    {
        // TODO extract to function (duplicated code)
        IdentifierExpr* id = cast<IdentifierExpr>(expr);
        ScopeResult Res = analyseIdentifier(id);
        if (Res.getPackage()) {
            Diags.Report(id->getLocation(), diag::err_is_a_package) << id->getName();
            return;
        }
        Decl* D = Res.getDecl();
        if (!D) return;
        switch (D->getKind()) {
        case DECL_FUNC:
            // Q: allow?
        case DECL_VAR:
        case DECL_ENUMVALUE:
        case DECL_ALIASTYPE:
        case DECL_STRUCTTYPE:
        case DECL_ENUMTYPE:
        case DECL_FUNCTIONTYPE:
            // ok
            return;
        case DECL_ARRAYVALUE:
        case DECL_USE:
            assert(0 && "should not happen");
            break;
        }
        break;
    }
    case EXPR_INITLIST:
        assert(0 && "TODO good error");
        return;
    case EXPR_TYPE:
        // ok
        return;
    case EXPR_DECL:
        assert(0 && "should not happen");
        return;
    case EXPR_BINOP:
        // not allowed
        assert(0 && "TODO good error");
    case EXPR_CONDOP:
        assert(0 && "TODO allow?");
        return;
    case EXPR_UNARYOP:
        // some allowed (&)?
        // TODO
        return;
    case EXPR_BUILTIN:
        assert(0 && "should not happen");
        return;
    case EXPR_ARRAYSUBSCRIPT:
    case EXPR_MEMBER:
        // allowed
        assert(0 && "TODO");
        return;
    case EXPR_PAREN:
        analyseSizeofExpr(cast<ParenExpr>(expr)->getExpr());
        return;
    }

}

void FunctionAnalyser::analyseArraySizeExpr(ArrayType* AT, Expr* E) {
    LOG_FUNC
    QualType T = analyseExpr(E, RHS);
    if (!T.isValid()) return;

    if (inConstExpr && !E->isConstant()) {
        Diags.Report(E->getLocation(), diag::err_vla_decl_in_file_scope) << E->getSourceRange();
        errors++;
        return;
    }
    // check if type is integer
    QualType CT = T.getCanonicalType();
    if (!CT.isBuiltinType() || !cast<BuiltinType>(CT)->isInteger()) {
        StringBuilder buf;
        T.DiagName(buf);
        Diags.Report(E->getLocation(), diag::err_array_size_non_int) << buf << E->getSourceRange();
        errors++;
        return;
    }
    // check if negative
    if (E->getCTC() == CTC_FULL) {
        LiteralAnalyser LA(Diags);
        llvm::APSInt Result = LA.checkLiterals(E);
        if (Result.isSigned() && Result.isNegative()) {
            Diags.Report(E->getLocation(), diag::err_typecheck_negative_array_size) << E->getSourceRange();
            errors++;
        } else {
            AT->setSize(Result);
        }
    }
}

void FunctionAnalyser::analyseDeclExpr(Expr* expr) {
    LOG_FUNC
    DeclExpr* DE = cast<DeclExpr>(expr);
    VarDecl* decl = DE->getDecl();

    // check type and convert User types
    QualType type = decl->getType();
    unsigned errs = TC.checkType(type, false);
    errors += errs;
    if (!errs) {
        TC.resolveCanonicals(decl, type, true);
        ArrayType* AT = dyncast<ArrayType>(type.getTypePtr());
        if (AT) {
            Expr* sizeExpr = AT->getSizeExpr();
            if (sizeExpr) {
                analyseArraySizeExpr(AT, sizeExpr);
                if (sizeExpr->getCTC() != CTC_FULL && decl->getInitValue()) {
                    Diags.Report(decl->getLocation(), diag::err_vla_with_init_value) << decl->getInitValue()->getLocation();
                }
            } else {
                if (!decl->getInitValue()) {
                    Diags.Report(decl->getLocation(), diag::err_typecheck_incomplete_array_needs_initializer);
                }
            }
        }
    }

    // check name
    if (!scope.checkScopedSymbol(decl)) return;

    // check initial value
    Expr* initialValue = decl->getInitValue();
    if (initialValue && !errs) {
        CurrentVarDecl = decl;
        analyseInitExpr(initialValue, decl->getType());
        CurrentVarDecl = 0;
    }

    if (type.isConstQualified() && !initialValue) {
        Diags.Report(decl->getLocation(), diag::err_uninitialized_const_var) << decl->getName();
    }
    scope.addScopedSymbol(decl);
}

static ExprCTC combineCtc(ExprCTC left, ExprCTC right) {
    switch (left + right) {
    case 0:
        return CTC_NONE;
    case 1:
    case 2:
    case 3:
        return CTC_PARTIAL;
    case 4:
        return CTC_FULL;
    }
    assert(0 && "should not come here");
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

QualType FunctionAnalyser::analyseBinaryOperator(Expr* expr, unsigned side) {
    LOG_FUNC
    BinaryOperator* binop = cast<BinaryOperator>(expr);
    QualType TLeft, TRight;
    Expr* Left = binop->getLHS();
    Expr* Right = binop->getRHS();

    switch (binop->getOpcode()) {
    case BO_PtrMemD:
    case BO_PtrMemI:
        assert(0 && "unhandled binary operator type");
        break;
    case BO_Mul:
    case BO_Div:
    case BO_Rem:
    case BO_Add:
    case BO_Sub:
    case BO_Shl:
    case BO_Shr:
        // RHS, RHS
        TLeft = analyseExpr(Left, RHS);
        TRight = analyseExpr(Right, RHS);
        expr->setCTC(combineCtc(Left->getCTC(), Right->getCTC()));
        if (Left->isConstant() && Right->isConstant()) expr->setConstant();
        break;
    case BO_LE:
    case BO_LT:
    case BO_GE:
    case BO_GT:
    case BO_NE:
    case BO_EQ:
        // RHS, RHS
        TLeft = analyseExpr(Left, RHS);
        TRight = analyseExpr(Right, RHS);
        // NOTE: CTC is never full, because value is not interting, only type
        if (Left->isConstant() && Right->isConstant()) expr->setConstant();
        break;
    case BO_And:
    case BO_Xor:
    case BO_Or:
    case BO_LAnd:
    case BO_LOr:
        // TODO check if cast of SubExpr is ok
        // RHS, RHS
        TLeft = analyseExpr(Left, RHS);
        TRight = analyseExpr(Right, RHS);
        expr->setCTC(combineCtc(Left->getCTC(), Right->getCTC()));
        if (Left->isConstant() && Right->isConstant()) expr->setConstant();
        break;
    case BO_Assign:
        // LHS, RHS
        TLeft = analyseExpr(Left, side | LHS);
        TRight = analyseExpr(Right, RHS);
        break;
    case BO_MulAssign:
    case BO_DivAssign:
    case BO_RemAssign:
    case BO_AddAssign:
    case BO_SubAssign:
    case BO_ShlAssign:
    case BO_ShrAssign:
    case BO_AndAssign:
    case BO_XorAssign:
    case BO_OrAssign:
        // LHS|RHS, RHS
        TLeft = analyseExpr(Left, LHS | RHS);
        TRight = analyseExpr(Right, RHS);
        break;
    case BO_Comma:
        assert(0 && "unhandled binary operator type");
        break;
    }
    if (TLeft.isNull() || TRight.isNull()) return QualType();

    // determine Result type
    QualType Result;
    switch (binop->getOpcode()) {
    case BO_PtrMemD:
    case BO_PtrMemI:
        assert(0 && "unhandled binary operator type");
        break;
    case BO_Mul:
    case BO_Div:
    case BO_Rem:
    case BO_Add:
    case BO_Sub:
        // TODO return largest witdth of left/right (long*short -> long)
        // TODO apply UsualArithmeticConversions() to L + R
        // TEMP for now just return Right side
        // TEMP use UnaryConversion
        Result = TC.UsualUnaryConversions(Left);
        TC.UsualUnaryConversions(Right);
        break;
    case BO_Shl:
    case BO_Shr:
        Result = TLeft;
        break;
    case BO_LE:
    case BO_LT:
    case BO_GE:
    case BO_GT:
    case BO_NE:
    case BO_EQ:
        Result = Type::Bool();
        break;
    case BO_And:
    case BO_Xor:
    case BO_Or:
        Result = TLeft;
        break;
    case BO_LAnd:
    case BO_LOr:
        Result = Type::Bool();
        break;
    case BO_Assign:
    {
        assert(TRight.isValid());
        EA.check(TLeft, Right);
        checkAssignment(Left, TLeft);
        Result = TLeft;
        break;
    }
    case BO_MulAssign:
    case BO_DivAssign:
    case BO_RemAssign:
    case BO_AddAssign:
    case BO_SubAssign:
    case BO_ShlAssign:
    case BO_ShrAssign:
    case BO_AndAssign:
    case BO_XorAssign:
    case BO_OrAssign:
        checkAssignment(Left, TLeft);
        Result = TLeft;
        break;
    case BO_Comma:
        assert(0 && "unhandled binary operator type");
        break;
    }
    expr->setType(Result);
    return Result;
}

QualType FunctionAnalyser::analyseConditionalOperator(Expr* expr) {
    LOG_FUNC
    ConditionalOperator* condop = cast<ConditionalOperator>(expr);
    analyseExpr(condop->getCond(), RHS);
    // check if Condition can be casted to bool
    EA.check(Type::Bool(), condop->getCond());
    QualType TLeft = analyseExpr(condop->getLHS(), RHS);
    analyseExpr(condop->getRHS(), RHS);
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
            QualType Q = typeContext.getPointerType(LType);
            expr->setType(Q);
            expr->setConstant();
            TC.resolveCanonicals(0, Q, true);
            return Q;
        }
    case UO_Deref:
        LType = analyseExpr(SubExpr, side | RHS);
        if (LType.isNull()) return 0;
        if (!LType.isPointerType()) {
            char typeName[MAX_LEN_TYPENAME];
            StringBuilder buf(MAX_LEN_TYPENAME, typeName);
            LType.DiagName(buf);
            Diags.Report(unaryop->getOpLoc(), diag::err_typecheck_indirection_requires_pointer)
                << buf;
            return 0;
        } else {
            // TEMP use CanonicalType to avoid Unresolved types etc
            QualType Q = LType.getCanonicalType();
            const PointerType* P = cast<PointerType>(Q);
            return P->getPointeeType();
        }
        break;
    case UO_Plus:
    case UO_Minus:
    case UO_Not:
        LType = analyseExpr(SubExpr, side | RHS);
        unaryop->setCTC(SubExpr->getCTC());
        if (SubExpr->isConstant()) unaryop->setConstant();
        if (LType.isNull()) return 0;
        expr->setType(TC.UsualUnaryConversions(SubExpr));
        break;
    case UO_LNot:
        // TODO first cast expr to bool, then invert here, return type bool
        // TODO extract to function
        LType = analyseExpr(SubExpr, side | RHS);
        // TODO check conversion to bool here!!
        unaryop->setCTC(SubExpr->getCTC());
        // Also set type?
        if (LType.isNull()) return 0;
        //TC.UsualUnaryConversions(SubExpr);
        LType = Type::Bool();
        expr->setType(LType);
        break;
    default:
        assert(0 && "TODO");
        break;
    }
    return LType;
}

QualType FunctionAnalyser::analyseBuiltinExpr(Expr* expr) {
    LOG_FUNC
    BuiltinExpr* func = cast<BuiltinExpr>(expr);
    expr->setType(Type::UInt32());
    if (func->isSizeof()) { // sizeof()
        analyseSizeofExpr(func->getExpr());
    } else { // elemsof()
        // TODO proper checking
        analyseExpr(func->getExpr(), RHS);
        Expr* E = func->getExpr();
        IdentifierExpr* I = cast<IdentifierExpr>(E);
        Decl* D = I->getDecl();
        // should be VarDecl(for array/enum) or TypeDecl(array/enum)
        switch (D->getKind()) {
        case DECL_FUNC:
             fprintf(stderr, "TODO Function err\n");
            // ERROR
            break;
        case DECL_VAR:
            {
                VarDecl* VD = cast<VarDecl>(D);
                QualType Q = VD->getType();
                // TODO also allow elemsof for EnumType
                if (!Q.isArrayType()) {
                    StringBuilder msg;
                    Q.DiagName(msg);
                    Diags.Report(I->getLocation(), diag::err_invalid_elemsof_type)
                        << msg;
                }
                return Type::UInt32();
            }
        case DECL_ENUMVALUE:
            // ERROR
            break;
        case DECL_ALIASTYPE:
        case DECL_STRUCTTYPE:
        case DECL_ENUMTYPE:
        case DECL_FUNCTIONTYPE:
            assert(0 && "TODO");
            break;
        case DECL_ARRAYVALUE:
        case DECL_USE:
            assert(0);
            break;
        }
    }
    return Type::UInt32();
}

QualType FunctionAnalyser::analyseArraySubscript(Expr* expr) {
    LOG_FUNC
    ArraySubscriptExpr* sub = cast<ArraySubscriptExpr>(expr);
    QualType LType = analyseExpr(sub->getBase(), RHS);
    if (LType.isNull()) return 0;
    // TODO this should be done in analyseExpr()
    QualType LType2 = resolveUserType(LType);
    if (LType2.isNull()) return 0;
    if (!LType2.isSubscriptable()) {
        Diags.Report(expr->getLocation(), diag::err_typecheck_subscript);
        return 0;
    }
    analyseExpr(sub->getIndex(), RHS);
    QualType Result;
    if (isa<PointerType>(LType2)) {
        Result = cast<PointerType>(LType2)->getPointeeType();
    } else if (isa<ArrayType>(LType2)) {
        Result = cast<ArrayType>(LType2)->getElementType();
    }
    expr->setType(Result);
    return Result;
}

QualType FunctionAnalyser::analyseMemberExpr(Expr* expr, unsigned side) {
    LOG_FUNC
    MemberExpr* M = cast<MemberExpr>(expr);
    IdentifierExpr* member = M->getMember();

    bool isArrow = M->isArrow();
    // we dont know what we're looking at here, it could be:
    // pkg.type
    // pkg.var
    // pkg.func
    // var(Type=struct>.member
    // var[index].member
    // var->member
    Expr* base = M->getBase();
    if (base->getKind() == EXPR_IDENTIFIER) {
        IdentifierExpr* base_id = cast<IdentifierExpr>(base);
        ScopeResult SR = analyseIdentifier(base_id);
        Decl* SRD = SR.getDecl();
        if (SRD) {      // Struct.Member?
            M->setPkgPrefix(false);
            switch (SRD->getKind()) {
            case DECL_FUNC:
                fprintf(stderr, "error: member reference base 'type' is not a structure, union or package\n");
                return QualType();
            case DECL_ALIASTYPE:
                assert(0 && "TODO");
                break;
            case DECL_FUNCTIONTYPE:
            case DECL_ENUMTYPE:
                fprintf(stderr, "error: member reference base 'type' is not a structure, union or package\n");
                return QualType();
            case DECL_STRUCTTYPE:
                assert(0);  // will always be UnresolvedType
                break;
            case DECL_VAR:
                {
                    // TODO extract to function?
                    VarDecl* VD = cast<VarDecl>(SRD);
                    if (side & RHS) VD->setUsed();
                    QualType T = Decl2Type(VD);
                    assert(T.isValid());  // analyser should set

                    // for arrow it should be Ptr to StructType, otherwise StructType
                    if (isArrow) {
                        if (!T.isPointerType()) {
                            fprintf(stderr, "TODO using -> with non-pointer type\n");
                            char typeName[MAX_LEN_TYPENAME];
                            StringBuilder buf(MAX_LEN_TYPENAME, typeName);
                            T.DiagName(buf);
                            Diags.Report(M->getLocation(), diag::err_typecheck_member_reference_arrow)
                                << buf;
                            return QualType();
                        } else {
                            // deref
                            const PointerType* PT = cast<PointerType>(T);
                            T = resolveUserType(PT->getPointeeType());
                        }
                    } else {
                        if (T.isPointerType()) {
                            StringBuilder buf(MAX_LEN_TYPENAME);
                            T.DiagName(buf);
                            Diags.Report(M->getLocation(), diag::err_typecheck_member_reference_suggestion)
                                << buf << 0 << 1;
                            return QualType();
                        }
                    }
                    // Q: resolve UnresolvedTypes?

                    // check if struct/union type
                    if (!T->isStructType()) {
                        StringBuilder buf(MAX_LEN_TYPENAME);
                        T.DiagName(buf);
                        Diags.Report(M->getLocation(), diag::err_typecheck_member_reference_struct_union)
                            << buf << M->getSourceRange() << member->getLocation();
                        return QualType();
                    }
                    // TODO use return value?
                    analyseMember(T, member, side);
                    expr->setType(member->getType());
                    // TODO setCTC/setConstant but cleanup code
                }
                break;
            case DECL_ENUMVALUE:
                expr->setConstant();
                assert(0 && "TODO");
                break;
            case DECL_ARRAYVALUE:
            case DECL_USE:
                assert(0);
                break;
            }
        } else if (SR.getPackage()) {   // Pkg.Symbol
            M->setPkgPrefix(true);
            if (isArrow) {
                fprintf(stderr, "TODO ERROR: cannot use -> for package access\n");
                // continue checking
            }
            ScopeResult res = scope.findSymbolInPackage(member->getName(), member->getLocation(), SR.getPackage());
            Decl* D = res.getDecl();
            if (D) {
                QualType Q = Decl2Type(D);
                if (side & RHS) D->setUsed();
                member->setDecl(D);
                member->setType(Q);
                SetConstantFlags(D, member);
                expr->setCTC(member->getCTC());
                if (member->isConstant()) expr->setConstant();
                expr->setType(Q);
                return Q;
            }
            return QualType();
        } else  {
            return QualType();
        }
    } else {
        QualType LType = analyseExpr(base, RHS);
        if (LType.isNull()) return QualType();
        // TODO this should be done in analyseExpr()
        QualType LType2 = resolveUserType(LType);
        if (LType2.isNull()) return QualType();
        if (!LType2.isStructType()) {
            fprintf(stderr, "error: not a struct or union type\n");
            LType2->dump();
            return QualType();
        }
        return analyseMember(LType2, member, side);
    }
    return QualType();
}

QualType FunctionAnalyser::analyseMember(QualType T, IdentifierExpr* member, unsigned side) {
    LOG_FUNC
    const StructType* ST = cast<StructType>(T);
    const StructTypeDecl* S = ST->getDecl();
    Decl* match = S->find(member->getName());
    if (match) {
        member->setDecl(match);
        // NOT very nice, structs can have VarDecls or StructTypeDecls
        if (isa<VarDecl>(match)) {
            VarDecl* V = cast<VarDecl>(match);
            if (side & RHS) V->setUsed();
            member->setType(V->getType());
            return V->getType();
        }
        if (isa<StructTypeDecl>(match)) {
            return cast<StructTypeDecl>(match)->getType();
        }
        assert(0);
        return QualType();
    }
    char temp1[MAX_LEN_TYPENAME];
    StringBuilder buf1(MAX_LEN_TYPENAME, temp1);
    T->DiagName(buf1);

    char temp2[MAX_LEN_VARNAME];
    StringBuilder buf2(MAX_LEN_VARNAME, temp2);
    buf2 << '\'' << member->getName() << '\'';
    Diags.Report(member->getLocation(), diag::err_no_member) << temp2 << temp1;
    return QualType();
}

QualType FunctionAnalyser::analyseParenExpr(Expr* expr) {
    LOG_FUNC
    ParenExpr* P = cast<ParenExpr>(expr);
    QualType Q = analyseExpr(P->getExpr(), RHS);
    expr->setCTC(P->getExpr()->getCTC());
    expr->setType(Q);
    return Q;
}

QualType FunctionAnalyser::analyseCall(Expr* expr) {
    LOG_FUNC
    CallExpr* call = cast<CallExpr>(expr);
    // analyse function
    QualType LType = analyseExpr(call->getFn(), RHS);
    if (LType.isNull()) {
        fprintf(stderr, "CALL unknown function (already error)\n");
        call->getFn()->dump();
        return QualType();
    }
    // TODO this should be done in analyseExpr()
    QualType LType2 = resolveUserType(LType);
    if (LType2.isNull()) return QualType();
    if (!LType2.isFunctionType()) {
        char typeName[MAX_LEN_TYPENAME];
        StringBuilder buf(MAX_LEN_TYPENAME, typeName);
        LType2.DiagName(buf);
        Diags.Report(call->getLocation(), diag::err_typecheck_call_not_function) << typeName;
        return QualType();
    }

    const FunctionType* FT = cast<FunctionType>(LType2);
    FunctionDecl* func = FT->getDecl();
    func->setUsed();
    unsigned protoArgs = func->numArgs();
    unsigned callArgs = call->numArgs();
    unsigned minArgs = MIN(protoArgs, callArgs);
    for (unsigned i=0; i<minArgs; i++) {
        Expr* argGiven = call->getArg(i);
        QualType typeGiven = analyseExpr(argGiven, RHS);
        VarDecl* argFunc = func->getArg(i);
        QualType argType = argFunc->getType();
        if (typeGiven.isValid()) {
            assert(argType.isValid());
            EA.check(argType, argGiven);
        }
    }
    if (callArgs > protoArgs) {
        // more args given, check if function is variadic
        if (!func->isVariadic()) {
            Expr* arg = call->getArg(minArgs);
            unsigned msg = diag::err_typecheck_call_too_many_args;
            if (func->hasDefaultArgs()) msg = diag::err_typecheck_call_too_many_args_at_most;
            Diags.Report(arg->getLocation(), msg)
                << 0 << protoArgs << callArgs;
            return QualType();
        }
        for (unsigned i=minArgs; i<callArgs; i++) {
            Expr* argGiven = call->getArg(i);
            QualType typeGiven = analyseExpr(argGiven, RHS);
            // TODO use canonical
            if (typeGiven == Type::Void()) {
                fprintf(stderr, "ERROR: (TODO) passing 'void' to parameter of incompatible type '...'\n");
                //Diags.Report(argGiven->getLocation(), diag::err_typecheck_convert_incompatible)
                //    << "from" << "to" << 1;// << argGiven->getLocation();
            }
        }
    } else if (callArgs < protoArgs) {
        // less args given, check for default argument values
        for (unsigned i=minArgs; i<protoArgs; i++) {
            VarDecl* arg = func->getArg(i);
            if (!arg->getInitValue()) {
                if (func->hasDefaultArgs()) {
                    protoArgs = func->minArgs();
                    Diags.Report(arg->getLocation(), diag::err_typecheck_call_too_few_args_at_least)
                        << 0 << protoArgs << callArgs;
                } else {
                    Diags.Report(arg->getLocation(), diag::err_typecheck_call_too_few_args)
                        << 0 << protoArgs << callArgs;
                }
                return QualType();
            }
        }
    }
    call->setType(func->getReturnType());
    return func->getReturnType();
}

ScopeResult FunctionAnalyser::analyseIdentifier(IdentifierExpr* id) {
    LOG_FUNC
    ScopeResult res = scope.findSymbol(id->getName(), id->getLocation());
    Decl* D = res.getDecl();
    if (!res.isOK()) return res;

    if (D) {
        id->setDecl(D);
        SetConstantFlags(D, id);
    } else if (res.getPackage()) {
        // symbol is package
    } else {
        res.setOK(false);
        Diags.Report(id->getLocation(), diag::err_undeclared_var_use)
            << id->getName();
        ScopeResult res2 = scope.findSymbolInUsed(id->getName());
        Decl* D2 = res2.getDecl();
        if (D2) {
            assert(D2->getPackage());
            // Crashes if ambiguous
            Diags.Report(D->getLocation(), diag::note_function_suggestion)
                << AnalyserUtils::fullName(D2->getPackage()->getName(), id->getName());
        }
    }
    return res;
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
    case EXPR_TYPE:
        break;
    case EXPR_DECL:
        assert(0);  // can happen?
        return true;
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
        // ok
        return true;
    }
    // expr is not assignable
    // TODO test (also ternary)
    Diags.Report(expr->getLocation(), diag::err_typecheck_expression_not_modifiable_lvalue);
    return false;
}

void FunctionAnalyser::checkAssignment(Expr* assignee, QualType TLeft) {
    if (TLeft.isConstQualified()) {
        Diags.Report(assignee->getLocation(), diag::err_typecheck_assign_const) << assignee->getSourceRange();
    }
}

void FunctionAnalyser::checkDeclAssignment(Decl* decl, Expr* expr) {
    LOG_FUNC
    assert(decl);
    switch (decl->getKind()) {
    case DECL_FUNC:
        // error
        fprintf(stderr, "TODO cannot assign to non-variable\n");
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
        Diags.Report(expr->getLocation(), diag::err_typecheck_expression_not_modifiable_lvalue);
        break;
    case DECL_ARRAYVALUE:
        // error
        fprintf(stderr, "TODO cannot assign to non-variable\n");
        break;
    case DECL_USE:
        assert(0);
        break;
    }
}

C2::QualType FunctionAnalyser::resolveUserType(QualType T) {
    if (isa<UnresolvedType>(T)) {
        const UnresolvedType* U = cast<UnresolvedType>(T);
        TypeDecl* D = U->getDecl();
        assert(D);
        return D->getType();
    }
    return T;
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

