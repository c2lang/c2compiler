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

#include "Analyser/FunctionAnalyser.h"
#include "Analyser/TypeChecker.h"
#include "Analyser/AnalyserUtils.h"
#include "AST/Decl.h"
#include "AST/Expr.h"
#include "AST/Stmt.h"
#include "AST/Package.h"
#include "AST/constants.h"
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

static ExprCTC decl2ctc(Decl* D) {
    switch (D->getKind()) {
    case DECL_FUNC:
        break;
    case DECL_VAR:
        {
            VarDecl* VD = cast<VarDecl>(D);
            if (VD->getType().isConstQualified()) return CTC_FULL;
            break;
        }
    case DECL_ENUMVALUE:
        return CTC_FULL;
    case DECL_ALIASTYPE:
    case DECL_STRUCTTYPE:
    case DECL_ENUMTYPE:
    case DECL_FUNCTIONTYPE:
    case DECL_ARRAYVALUE:
    case DECL_USE:
        break;
    }
    return CTC_NONE;
}


FunctionAnalyser::FunctionAnalyser(Scope& scope_,
                                   TypeChecker& typeRes_,
                                   TypeContext& tc,
                                   clang::DiagnosticsEngine& Diags_)
    : scope(scope_)
    , typeResolver(typeRes_)
    , typeContext(tc)
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

    CurrentVarDecl = 0;
    return errors;
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
    case STMT_GOTO:
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
        typeResolver.checkCompatible(Type::Bool(), Q1, cond->getLocation(), TypeChecker::CONV_CONV);
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
            Diags.Report(ret->getLocation(), diag::ext_return_has_expr) << CurrentFunction->getName() << 0;
            // TODO value->getSourceRange()
        } else {
            if (type.isValid()) {
                typeResolver.checkCompatible(rtype, type, value->getLocation(), TypeChecker::CONV_CONV);
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
        return Type::Float32();
    case EXPR_BOOL_LITERAL:
        return Type::Bool();
    case EXPR_CHAR_LITERAL:
        return Type::Int8();
    case EXPR_STRING_LITERAL:
        {
            // return type: 'const char*'
            QualType Q = typeContext.getPointerType(Type::Int8());
            Q.addConst();
            if (!Q->hasCanonicalType()) Q->setCanonicalType(Q);
            return Q;
        }
    case EXPR_NIL:
        {
            QualType Q = typeContext.getPointerType(Type::Void());
            if (!Q->hasCanonicalType()) Q->setCanonicalType(Q);
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
            // NOTE: expr should not be package name (handled above)
            // TODO LHS: check if VarDecl
            if (side & LHS) checkDeclAssignment(D, expr);
            if (side & RHS) D->setUsed();
            return Decl2Type(D);
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

    switch (expr->getKind()) {
    case EXPR_INTEGER_LITERAL:
    case EXPR_FLOAT_LITERAL:
    case EXPR_BOOL_LITERAL:
    case EXPR_CHAR_LITERAL:
    case EXPR_STRING_LITERAL:
    case EXPR_NIL:
        // TODO check if compatible
        break;
    case EXPR_CALL:
        // TODO check return type (if void -> size of array has non-integer type 'void')
        //assert(constDiagID);
        //Diags.Report(expr->getLocation(), constDiagID);
        // TEMP assume compile-time constant (CTC)
        Diags.Report(expr->getLocation(), diag::err_init_element_not_constant);
        return;
    case EXPR_IDENTIFIER:
        {
            IdentifierExpr* id = cast<IdentifierExpr>(expr);
            ScopeResult Res = analyseIdentifier(id);
            if (Res.getPackage()) {
                Diags.Report(id->getLocation(), diag::err_is_a_package) << id->getName();
                return;
            }
            Decl* D = Res.getDecl();
            if (!D) return;
            if (D == CurrentVarDecl) {
                Diags.Report(id->getLocation(), diag::err_var_self_init) << D->getName();
                return;
            }
            D->setUsed();       // always set used, even if causing error below
            switch (D->getKind()) {
            case DECL_FUNC:
                {
                    // TODO check if void*
                    FunctionDecl* FD = cast<FunctionDecl>(D);
                    typeResolver.checkCompatible(expectedType, FD->getType(), id->getLocation(), TypeChecker::CONV_INIT);
                }
                break;
            case DECL_VAR:
                {
                    VarDecl* VD = cast<VarDecl>(D);
                    if (inConstExpr) {
                        QualType T = VD->getType();
                        if (!T.isConstQualified()) {
                            assert(constDiagID);
                            Diags.Report(expr->getLocation(), constDiagID);
                            return;
                        }
                    }
                }
                break;
            case DECL_ENUMVALUE:
                // TODO check type compatibility
                break;
            case DECL_ALIASTYPE:
            case DECL_STRUCTTYPE:
            case DECL_ENUMTYPE:
            case DECL_FUNCTIONTYPE:
                Diags.Report(id->getLocation(), diag::err_type_in_initializer);
                break;
            case DECL_ARRAYVALUE:
            case DECL_USE:
                assert(0 && "shouldn't come here");
                break;
            }
        }
        break;
    case EXPR_INITLIST:
        assert(CurrentVarDecl);
        analyseInitList(cast<InitListExpr>(expr), expectedType);
        break;
    case EXPR_TYPE:
        assert(0 && "??");
        break;
    case EXPR_DECL:
        assert(0 && "TODO ERROR?");
        break;
    case EXPR_BINOP:
        analyseBinaryOperator(expr, RHS);
        break;
    case EXPR_CONDOP:
        analyseConditionalOperator(expr);
        break;
    case EXPR_UNARYOP:
        analyseUnaryOperator(expr, RHS);
        break;
    case EXPR_BUILTIN:
        analyseBuiltinExpr(expr);
        break;
    case EXPR_ARRAYSUBSCRIPT:
        analyseArraySubscript(expr);
        break;
    case EXPR_MEMBER:
        // TODO dont allow struct.member, only pkg.constant
        analyseMemberExpr(expr, RHS);
        break;
    case EXPR_PAREN:
        analyseParenExpr(expr);
        break;
    }
}

void FunctionAnalyser::analyseInitList(InitListExpr* expr, QualType expectedType) {
    LOG_FUNC

    // TODO for now don't support nested initLists, need stack of CurrentDecl for that
    assert(CurrentVarDecl);
    QualType Q = CurrentVarDecl->getType();
    ExprList& values = expr->getValues();
    if (Q.isArrayType()) {
        // TODO use helper function
        ArrayType* AT = cast<ArrayType>(Q->getCanonicalType().getTypePtr());
        QualType ET = AT->getElementType();
        // TODO check if size is specifier in type
        for (unsigned i=0; i<values.size(); i++) {
            analyseInitExpr(values[i], ET);
        }
    } else if (Q.isStructType()) {
        // TODO use helper function
        StructType* TT = cast<StructType>(Q->getCanonicalType().getTypePtr());
        StructTypeDecl* STD = TT->getDecl();
        assert(STD->isStruct() && "TEMP only support structs for now");
        for (unsigned i=0; i<values.size(); i++) {
            if (i >= STD->numMembers()) {
                // note: 0 for array, 2 for scalar, 3 for union, 4 for structs
                Diags.Report(values[STD->numMembers()]->getLocation(), diag::err_excess_initializers)
                    << 4;
                errors++;
                return;
            }
            // NOTE: doesn't fit for sub-struct members! (need Decl in interface)
            // TODO: add VD to CurrentVarDecl stack?
            VarDecl* VD = dyncast<VarDecl>(STD->getMember(i));
            assert(VD && "TEMP don't support sub-struct member inits");
            //checkInitValue(VD, values[i], VD->getType());
            analyseInitExpr(values[i], VD->getType());
        }
    } else {
        // only allow 1
        switch (values.size()) {
        case 0:
            fprintf(stderr, "TODO ERROR: scalar initializer cannot be empty\n");
            errors++;
            break;
        case 1:
            // Q: allow initlist for single var?
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

void FunctionAnalyser::analyseDeclExpr(Expr* expr) {
    LOG_FUNC
    DeclExpr* DE = cast<DeclExpr>(expr);
    VarDecl* decl = DE->getDecl();

    // check type and convert User types
    QualType type = decl->getType();
    unsigned errs = typeResolver.checkType(type, false);
    errors += errs;
    if (!errs) {
        typeResolver.resolveCanonicals(decl, type, true);
        ArrayType* AT = dyncast<ArrayType>(type.getTypePtr());
        if (AT && AT->getSize()) {
            analyseExpr(AT->getSize(), RHS);
            // TODO check type of size expr
        }
    }

    // check name
    if (!scope.checkScopedSymbol(decl)) return;
    // check initial value
    Expr* initialValue = decl->getInitValue();
    if (initialValue && !errs) {
        QualType Q = analyseExpr(initialValue, RHS);
        if (Q.isValid()) {
            typeResolver.checkCompatible(decl->getType(), Q, initialValue->getLocation(), TypeChecker::CONV_INIT);
        }
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
    //unsigned numbits = I->Value.getActiveBits();   // unsigned
    // TEMP for now assume signed
    // Q: we can determine size, but don't know if we need signed/unsigned
    unsigned numbits = I->Value.getMinSignedBits();  // signed
    if (numbits <= 8) return Type::Int8();
    if (numbits <= 16) return Type::Int16();
    if (numbits <= 32) return Type::Int32();
    return Type::Int64();
}

QualType FunctionAnalyser::analyseBinaryOperator(Expr* expr, unsigned side) {
    LOG_FUNC
    BinaryOperator* binop = cast<BinaryOperator>(expr);
    QualType TLeft, TRight;

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
    case BO_LE:
    case BO_LT:
    case BO_GE:
    case BO_GT:
    case BO_NE:
    case BO_EQ:
    case BO_And:
    case BO_Xor:
    case BO_Or:
    case BO_LAnd:
    case BO_LOr:
        // RHS, RHS
        TLeft = analyseExpr(binop->getLHS(), RHS);
        TRight = analyseExpr(binop->getRHS(), RHS);
        expr->setCTC(combineCtc(binop->getLHS()->getCTC(), binop->getRHS()->getCTC()));
        break;
    case BO_Assign:
        // LHS, RHS
        TLeft = analyseExpr(binop->getLHS(), side | LHS);
        TRight = analyseExpr(binop->getRHS(), RHS);
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
        TLeft = analyseExpr(binop->getLHS(), LHS | RHS);
        TRight = analyseExpr(binop->getRHS(), RHS);
        break;
    case BO_Comma:
        assert(0 && "unhandled binary operator type");
        break;
    }
    if (TLeft.isNull() || TRight.isNull()) return QualType();

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
        // TODO return largetst witdth of left/right (long*short -> long)
        // TEMP just return INT
        return Type::Int32();
    case BO_Shl:
    case BO_Shr:
        return TLeft;
    case BO_LE:
    case BO_LT:
    case BO_GE:
    case BO_GT:
    case BO_NE:
    case BO_EQ:
        return Type::Bool();
    case BO_And:
    case BO_Xor:
    case BO_Or:
        return TLeft;
    case BO_LAnd:
    case BO_LOr:
        return Type::Bool();
    case BO_Assign:
        typeResolver.checkCompatible(TLeft, TRight, binop->getLocation(), TypeChecker::CONV_ASSIGN);
        checkAssignment(binop->getLHS(), TLeft);
        return TLeft;
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
        checkAssignment(binop->getLHS(), TLeft);
        return TLeft;
    case BO_Comma:
        assert(0 && "unhandled binary operator type");
        break;
    }
    return QualType();
}

QualType FunctionAnalyser::analyseConditionalOperator(Expr* expr) {
    LOG_FUNC
    ConditionalOperator* condop = cast<ConditionalOperator>(expr);
    analyseExpr(condop->getCond(), RHS);
    QualType TLeft = analyseExpr(condop->getLHS(), RHS);
    analyseExpr(condop->getRHS(), RHS);
    // TODO also check type of RHS
    return TLeft;
}

QualType FunctionAnalyser::analyseUnaryOperator(Expr* expr, unsigned side) {
    LOG_FUNC
    UnaryOperator* unaryop = cast<UnaryOperator>(expr);
    QualType LType;
    switch (unaryop->getOpcode()) {
    case UO_PostInc:
    case UO_PostDec:
    case UO_PreInc:
    case UO_PreDec:
        LType = analyseExpr(unaryop->getExpr(), side | LHS);
        if (LType.isNull()) return 0;
        // TODO check if type is Integer/Floating Point
        break;
    case UO_AddrOf:
        {
            LType = analyseExpr(unaryop->getExpr(), side | RHS);
            if (LType.isNull()) return 0;
            QualType Q = typeContext.getPointerType(LType);
            typeResolver.resolveCanonicals(0, Q, true);
            return Q;
        }
    case UO_Deref:
        LType = analyseExpr(unaryop->getExpr(), side | RHS);
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
            QualType Q = LType->getCanonicalType();
            const PointerType* P = cast<PointerType>(Q);
            return P->getPointeeType();
        }
        break;
    case UO_Plus:
    case UO_Minus:
    case UO_Not:
    case UO_LNot:
        LType = analyseExpr(unaryop->getExpr(), side | RHS);
        unaryop->setCTC(unaryop->getExpr()->getCTC());
        if (LType.isNull()) return 0;
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
    analyseExpr(func->getExpr(), RHS);
    if (func->isSizeof()) { // sizeof()
        // TODO can also be type (for sizeof)
        return Type::UInt32();
    } else { // elemsof()
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
        return QualType();
    }
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
    if (isa<PointerType>(LType2)) {
        return cast<PointerType>(LType2)->getPointeeType();
    }
    if (isa<ArrayType>(LType2)) {
        return cast<ArrayType>(LType2)->getElementType();
    }
    return 0;
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
        if (SRD) {
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
                    return analyseMember(T, member, side);
                }
                break;
            case DECL_ENUMVALUE:
                assert(0 && "TODO");
                break;
            case DECL_ARRAYVALUE:
            case DECL_USE:
                assert(0);
                break;
            }
        } else if (SR.getPackage()) {
            M->setPkgPrefix(true);
            if (isArrow) {
                fprintf(stderr, "TODO ERROR: cannot use -> for package access\n");
                // continue checking
            }
            ScopeResult res = scope.findSymbolInPackage(member->getName(), member->getLocation(), SR.getPackage());
            Decl* D = res.getDecl();
            if (D) {
                member->setDecl(D);
                if (side & RHS) D->setUsed();
                ExprCTC ctc = decl2ctc(D);
                member->setCTC(ctc);
                expr->setCTC(ctc);
                return Decl2Type(D);
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
        // NOT very nice, structs can have VarDecls or StructTypeDecls
        if (isa<VarDecl>(match)) {
            VarDecl* V = cast<VarDecl>(match);
            if (side & RHS) V->setUsed();
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
            typeResolver.checkCompatible(argType, typeGiven, argGiven->getLocation(), TypeChecker::CONV_CONV);
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
#warning "TODO check if not void"
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
    return func->getReturnType();
}

ScopeResult FunctionAnalyser::analyseIdentifier(IdentifierExpr* id) {
    LOG_FUNC
    ScopeResult res = scope.findSymbol(id->getName(), id->getLocation());
    Decl* D = res.getDecl();
    if (!res.isOK()) return res;

    if (D) {
        id->setDecl(D);
        id->setCTC(decl2ctc(D));
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
        Diags.Report(assignee->getLocation(), diag::err_typecheck_assign_const);
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
            // ..
            //return resolveUserType(VD->getType());
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
        TypeDecl* D = U->getMatch();
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

