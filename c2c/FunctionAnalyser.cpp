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
#include "Stmt.h"
#include "Package.h"
#include "Scope.h"
#include "color.h"
#include "Utils.h"
#include "StringBuilder.h"

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

// TODO extract to constants.h
const unsigned MAX_TYPENAME = 128;
const unsigned MAX_VARNAME = 64;

// 0 = ok,
// 1 = loss of integer precision,
// 2 = sign-conversion,
// 3 = float->integer,
// 4 = incompatible,
// 5 = loss of FP precision
static int type_conversions[14][14] = {
    // I8  I16  I32  I64   U8  U16  U32  U64  F32  F64  Bool  Void
    // I8 ->
    {   0,   0,   0,   0,   2,   2,   2,   2,   0,   0,    0,   4},
    // I16 ->
    {   1,   0,   0,   0,   2,   2,   2,   2,   0,   0,    0,   4},
    // I32 ->
    {   1,   1,   0,   0,   2,   2,   2,   2,   0,   0,    0,   4},
    // I64 ->
    {   1,   1,   1,   0,   2,   2,   2,   2,   0,   0,    0,   4},
    // U8 ->
    {   2,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,   4},
    // U16 ->
    {   1,   2,   0,   0,   1,   0,   0,   0,   0,   0,    0,   4},
    // U32 ->
    {   1,   1,   2,   0,   1,   1,   0,   0,   0,   0,    0,   4},
    // U64 ->
    {   1,   1,   1,   2,   1,   1,   1,   0,   0,   0,    0,   4},
    // F32 ->
    {   3,   3,   3,   3,   3,   3,   3,   3,   0,   0,    4,   4},
    // F64 ->
    {   3,   3,   3,   3,   3,   3,   3,   3,   5,   0,    4,   4},
    // BOOL ->
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,   4},
    // VOID ->
    {  4,    4,   4,   4,  4,   4,   4,   4,   4,   4,    4,    0},
};

FunctionAnalyser::FunctionAnalyser(FileScope& scope_,
                                           TypeContext& tc,
                                           clang::DiagnosticsEngine& Diags_)
    : globalScope(scope_)
    , typeContext(tc)
    , scopeIndex(0)
    , curScope(0)
    , Diags(Diags_)
    , errors(0)
    , Function(0)
    , constDiagID(0)
    , inConstExpr(false)
{
    Scope* parent = 0;
    for (int i=0; i<MAX_SCOPE_DEPTH; i++) {
        scopes[i].InitOnce(globalScope, parent);
        parent = &scopes[i];
    }
}

unsigned FunctionAnalyser::check(FunctionDecl* func) {
    LOG_FUNC
    errors = 0;
    Function = func;
    EnterScope(Scope::FnScope | Scope::DeclScope);
    // add arguments to new scope

    // NOTE: arguments have already been checked
    for (unsigned i=0; i<func->numArgs(); i++) {
        VarDecl* arg = func->getArg(i);
        if (arg->getName() != "") {
            // check that argument names dont clash with globals
            ScopeResult res = globalScope.findSymbol(arg->getName(), arg->getLocation());
            if (res.decl) {
                // TODO check other attributes?
                Diags.Report(arg->getLocation(), diag::err_redefinition)
                    << arg->getName();
                Diags.Report(res.decl->getLocation(), diag::note_previous_definition);
                errors++;
                continue;
            }
            curScope->addDecl(arg);
        }
    }
    if (errors) return errors;

    analyseCompoundStmt(func->getBody());
    if (errors) return errors;

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

    ExitScope();
    Function = 0;
    return errors;
}

void FunctionAnalyser::EnterScope(unsigned flags) {
    LOG_FUNC
    assert (scopeIndex < MAX_SCOPE_DEPTH && "out of scopes");
    scopes[scopeIndex].Init(flags);
    curScope = &scopes[scopeIndex];
    scopeIndex++;
}

void FunctionAnalyser::ExitScope() {
    LOG_FUNC
    for (unsigned i=0; i<curScope->numDecls(); i++) {
        VarDecl* D = curScope->getDecl(i);
        if (!D->isUsed()) {
            unsigned msg = diag::warn_unused_variable;
            if (D->isParameter()) msg = diag::warn_unused_parameter;
            Diags.Report(D->getLocation(), msg) << D->getName();
        }
    }
    scopeIndex--;
    Scope* parent = curScope->getParent();
    curScope = parent;
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
        if (!haveScope) EnterScope(Scope::DeclScope);
        analyseCompoundStmt(S);
        if (!haveScope) ExitScope();
        break;
    }
}

void FunctionAnalyser::analyseCompoundStmt(Stmt* stmt) {
    LOG_FUNC
    assert(Function);
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
        checkCompatible(Type::Bool(), Q1, cond->getLocation(), CONV_CONV);
    }
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
    LOG_FUNC
    WhileStmt* W = cast<WhileStmt>(stmt);
    analyseStmt(W->getCond());
    EnterScope(Scope::BreakScope | Scope::ContinueScope | Scope::DeclScope | Scope::ControlScope);
    analyseStmt(W->getBody(), true);
    ExitScope();

}

void FunctionAnalyser::analyseDoStmt(Stmt* stmt) {
    LOG_FUNC
    DoStmt* D = cast<DoStmt>(stmt);
    EnterScope(Scope::BreakScope | Scope::ContinueScope | Scope::DeclScope);
    analyseStmt(D->getBody());
    ExitScope();
    analyseStmt(D->getCond());
}

void FunctionAnalyser::analyseForStmt(Stmt* stmt) {
    LOG_FUNC
    ForStmt* F = cast<ForStmt>(stmt);
    EnterScope(Scope::BreakScope | Scope::ContinueScope | Scope::DeclScope | Scope::ControlScope);
    if (F->getInit()) analyseStmt(F->getInit());
    if (F->getCond()) analyseExpr(F->getCond(), RHS);
    if (F->getIncr()) analyseExpr(F->getIncr(), RHS);
    analyseStmt(F->getBody(), true);
    ExitScope();
}

void FunctionAnalyser::analyseSwitchStmt(Stmt* stmt) {
    LOG_FUNC
    SwitchStmt* S = cast<SwitchStmt>(stmt);
    analyseExpr(S->getCond(), RHS);
    const StmtList& Cases = S->getCases();
    Stmt* defaultStmt = 0;
    EnterScope(Scope::BreakScope | Scope::SwitchScope);
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
    ExitScope();
}

void FunctionAnalyser::analyseBreakStmt(Stmt* stmt) {
    LOG_FUNC
    if (!curScope->allowBreak()) {
        BreakStmt* B = cast<BreakStmt>(stmt);
        Diags.Report(B->getLocation(), diag::err_break_not_in_loop_or_switch);
    }
}

void FunctionAnalyser::analyseContinueStmt(Stmt* stmt) {
    LOG_FUNC
    if (!curScope->allowContinue()) {
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
    QualType rtype = Function->getReturnType();
    bool no_rvalue = (rtype.getTypePtr() == Type::Void());
    if (value) {
        QualType type = analyseExpr(value, RHS);
        if (no_rvalue) {
            Diags.Report(ret->getLocation(), diag::ext_return_has_expr) << Function->getName() << 0;
            // TODO value->getSourceRange()
        } else {
            if (type.isValid()) {
                checkCompatible(rtype, type, value->getLocation(), CONV_CONV);
            }
        }
    } else {
        if (!no_rvalue) {
            Diags.Report(ret->getLocation(), diag::ext_return_missing_expr) << Function->getName() << 0;
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
        {
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
    case EXPR_STRING_LITERAL:
        {
            // return type: 'const char*'
            QualType Q = typeContext.getPointerType(Type::Int8());
            Q.addConst();
            if (!Q->hasCanonicalType()) Q->setCanonicalType(Q);
            return Q;
        }
    case EXPR_BOOL_LITERAL:
        return Type::Bool();
    case EXPR_CHAR_LITERAL:
        return Type::Int8();
    case EXPR_FLOAT_LITERAL:
        // For now always return type float
        return Type::Float32();
    case EXPR_CALL:
        return analyseCall(expr);
    case EXPR_IDENTIFIER:
        {
            IdentifierExpr* id = cast<IdentifierExpr>(expr);
            //fprintf(stderr, "%s  %s %s\n", id->getName().c_str(), side&LHS ? "LHS" : "", side&RHS ? "RHS": "");
            ScopeResult Res = analyseIdentifier(id);
            if (!Res.ok) break;
            if (!Res.decl) break;
            id->setDecl(Res.decl);
            if (Res.pkg) id->setPackage(Res.pkg);
            // NOTE: expr should not be package name (handled above)
            // TODO LHS: check if VarDecl
            if (side & LHS) checkDeclAssignment(Res.decl, expr);
            if (side & RHS) Res.decl->setUsed();
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
        return analyseBinaryOperator(expr, side);
    case EXPR_CONDOP:
        return analyseConditionalOperator(expr);
    case EXPR_UNARYOP:
        return analyseUnaryOperator(expr, side);
    case EXPR_BUILTIN:
        analyseBuiltinExpr(expr);
        break;
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
    case EXPR_STRING_LITERAL:
    case EXPR_BOOL_LITERAL:
    case EXPR_CHAR_LITERAL:
    case EXPR_FLOAT_LITERAL:
        // TODO check if compatible
        break;
    case EXPR_CALL:
        // TODO check return type (if void -> size of array has non-integer type 'void')
        assert(constDiagID);
        Diags.Report(expr->getLocation(), constDiagID);
        return;
    case EXPR_IDENTIFIER:
        {
            IdentifierExpr* id = cast<IdentifierExpr>(expr);
            ScopeResult Res = analyseIdentifier(id);
            if (!Res.ok) return;
            if (!Res.decl) return;
            id->setDecl(Res.decl);
            if (Res.pkg) id->setPackage(Res.pkg);
            switch (Res.decl->getKind()) {
            case DECL_FUNC:
                // can be ok for const
                assert(0 && "TODO");
                break;
            case DECL_VAR:
                {
                    VarDecl* VD = cast<VarDecl>(Res.decl);
                    if (inConstExpr) {
                        QualType T = VD->getType();
                        if (!T.isConstQualified()) {
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
                assert(0 && "TODO");
                break;
            default:
                assert(0 && "shouldn't come here");
                return;
            }
        }
        break;
    case EXPR_INITLIST:
        analyseInitList(expr, expectedType);
        break;
    case EXPR_TYPE:
        assert(0 && "??");
        break;
    case EXPR_DECL:
        assert(0 && "TODO ERROR");
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

void FunctionAnalyser::analyseInitList(Expr* expr, QualType expectedType) {
    LOG_FUNC

#if 0
    InitListExpr* I = cast<InitListExpr>(expr);
    assert(expectedType.isValid());
    const Type* type = expectedType.getTypePtr();

    // TODO use canonical type
    switch (type->getTypeClass()) {
    case TC_BUILTIN:
    case TC_POINTER:
    case TC_ARRAY:
    case TC_UNRESOLVED:
    case TC_ALIAS:
    case TC_STRUCT:
    case TC_ENUM:
    case TC_FUNCTION:
    }

    switch (type->getKind()) {
    case Type::USER:
        {
            analyseInitList(expr, type->getRefType());
            return;
        }
    case Type::STRUCT:
    case Type::UNION:
        {
#if 0
            MemberList* members = type->getMembers();
            assert(members);
            // check array member type with each value in initlist
            ExprList& values = I->getValues();
            for (unsigned i=0; i<values.size(); i++) {
                if (i >= members->size()) {
                    // TODO error: 'excess elements in array initializer'
                    return;
                }
                DeclExpr* member = (*members)[i];
                analyseInitExpr(values[i], member->getType());
            }
#endif
        }
        break;
    case Type::ARRAY:
        {
            // check array member type with each value in initlist
            ExprList& values = I->getValues();
            for (unsigned i=0; i<values.size(); i++) {
                QualType ref = type->getRefType();
                // TEMP CONST CAST
                analyseInitExpr(values[i], (Type*)ref.getTypePtr());
            }
        }
        break;
    default:
        {
        char typeName[MAX_TYPENAME];
        StringBuilder buf(MAX_TYPENAME, typeName);
        type->DiagName(buf);
        Diags.Report(expr->getLocation(), diag::err_invalid_type_initializer_list) << typeName;
        }
        break;
    }
#endif
}

void FunctionAnalyser::analyseDeclExpr(Expr* expr) {
    LOG_FUNC
    DeclExpr* DE = cast<DeclExpr>(expr);
    VarDecl* decl = DE->getDecl();

    // check type and convert User types
    QualType type = decl->getType();
    unsigned errs = globalScope.checkType(type, false);
    errors += errs;
    if (!errs) {
        globalScope.resolveCanonicals(decl, type, true);
    }

    // check name
    ScopeResult res = curScope->findSymbol(decl->getName(), decl->getLocation());
    if (res.decl) {
        // TODO check other attributes?
        Diags.Report(decl->getLocation(), diag::err_redefinition)
            << decl->getName();
        Diags.Report(res.decl->getLocation(), diag::note_previous_definition);
        return;
    }
    // check initial value
    Expr* initialValue = decl->getInitValue();
    if (initialValue && !errs) {
        QualType Q = analyseExpr(initialValue, RHS);
        if (Q.isValid()) {
            checkCompatible(decl->getType(), Q, initialValue->getLocation(), CONV_INIT);
        }
    }
    if (type.isConstQualified() && !initialValue) {
        Diags.Report(decl->getLocation(), diag::err_uninitialized_const_var) << decl->getName();
    }
    curScope->addDecl(decl);
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
        checkCompatible(TLeft, TRight, binop->getLocation(), CONV_ASSIGN);
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
            globalScope.resolveCanonicals(0, Q, true);
            return Q;
        }
    case UO_Deref:
        LType = analyseExpr(unaryop->getExpr(), side | RHS);
        if (LType.isNull()) return 0;
        if (!LType.isPointerType()) {
            char typeName[MAX_TYPENAME];
            StringBuilder buf(MAX_TYPENAME, typeName);
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
        if (LType.isNull()) return 0;
        break;
    default:
        assert(0 && "TODO");
        break;
    }
    return LType;
}

void FunctionAnalyser::analyseBuiltinExpr(Expr* expr) {
    LOG_FUNC
    BuiltinExpr* func = cast<BuiltinExpr>(expr);
    analyseExpr(func->getExpr(), RHS);
    if (func->isSizeof()) { // sizeof()
        // TODO can also be type (for sizeof)
    } else { // elemsof()
        Expr* E = func->getExpr();
        IdentifierExpr* I = cast<IdentifierExpr>(E);
        Decl* D = I->getDecl();
        // should be VarDecl(for array/enum) or TypeDecl(array/enum)
        switch (D->getKind()) {
        case DECL_FUNC:
             fprintf(stderr, "TODO Function err\n");
            // ERROR
            return;
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
                return;
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
    // we dont know what we're looking at here, can be:
    // pkg.type
    // pkg.var
    // pkg.func
    // var(Type=struct>.member
    // var[index].member
    // var->member
    // At least check if it exists for now
    Expr* base = M->getBase();
    if (base->getKind() == EXPR_IDENTIFIER) {
        IdentifierExpr* base_id = cast<IdentifierExpr>(base);
        ScopeResult SR = analyseIdentifier(base_id);
        if (!SR.ok) return QualType();
        if (SR.decl) {
            base_id->setDecl(SR.decl);
            if (SR.pkg) base_id->setPackage(SR.pkg);
            switch (SR.decl->getKind()) {
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
                    VarDecl* VD = cast<VarDecl>(SR.decl);
                    if (side & RHS) VD->setUsed();
                    QualType T = Decl2Type(VD);
                    assert(T.isValid());  // analyser should set

                    // for arrow it should be Ptr to StructType, otherwise StructType
                    if (isArrow) {
                        if (!T.isPointerType()) {
                            fprintf(stderr, "TODO using -> with non-pointer type\n");
                            char typeName[MAX_TYPENAME];
                            StringBuilder buf(MAX_TYPENAME, typeName);
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
                            StringBuilder buf(MAX_TYPENAME);
                            T.DiagName(buf);
                            Diags.Report(M->getLocation(), diag::err_typecheck_member_reference_suggestion)
                                << buf << 0 << 1;
                            return QualType();
                        }
                    }
                    // Q: resolve UnresolvedTypes?

                    // check if struct/union type
                    if (!T->isStructType()) {
                        StringBuilder buf(MAX_TYPENAME);
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
                return QualType();
            }
            if (SR.external && !D->isPublic()) {
                Diags.Report(member->getLocation(), diag::err_not_public)
                    << Utils::fullName(SR.pkg->getName(), D->getName());
                return QualType();
            }
            member->setPackage(SR.pkg);
            return Decl2Type(D);
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
    char temp1[MAX_TYPENAME];
    StringBuilder buf1(MAX_TYPENAME, temp1);
    T->DiagName(buf1);

    char temp2[MAX_VARNAME];
    StringBuilder buf2(MAX_VARNAME, temp2);
    buf2 << '\'' << member->getName() << '\'';
    Diags.Report(member->getLocation(), diag::err_no_member) << temp2 << temp1;
    return QualType();
}

QualType FunctionAnalyser::analyseParenExpr(Expr* expr) {
    LOG_FUNC
    ParenExpr* P = cast<ParenExpr>(expr);
    return analyseExpr(P->getExpr(), RHS);
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
        fprintf(stderr, "error: NOT a function type TODO\n");
        char typeName[MAX_TYPENAME];
        StringBuilder buf(MAX_TYPENAME, typeName);
        LType2.DiagName(buf);
        Diags.Report(call->getLocation(), diag::err_typecheck_call_not_function) << typeName;
        return QualType();
    }

    const FunctionType* FT = cast<FunctionType>(LType2);
    const FunctionDecl* func = FT->getDecl();
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
            checkCompatible(argType, typeGiven, argGiven->getLocation(), CONV_CONV);
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
    ScopeResult res = curScope->findSymbol(id->getName(), id->getLocation());
    if (!res.ok) return res;
    if (res.decl) {
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
            Diags.Report(id->getLocation(), diag::err_undeclared_var_use)
                << id->getName();
            ScopeResult res2 = globalScope.findSymbolInUsed(id->getName());
            if (res2.decl) {
                Diags.Report(res2.decl->getLocation(), diag::note_function_suggestion)
                    << Utils::fullName(res2.pkg->getName(), id->getName());
            }
        }
    }
    return res;
}

bool FunctionAnalyser::checkAssignee(Expr* expr) const {
    switch (expr->getKind()) {
    case EXPR_INTEGER_LITERAL:
    case EXPR_STRING_LITERAL:
    case EXPR_BOOL_LITERAL:
    case EXPR_CHAR_LITERAL:
    case EXPR_FLOAT_LITERAL:
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

bool FunctionAnalyser::checkCompatible(QualType left, QualType right, SourceLocation Loc, ConvType conv) const {
    LOG_FUNC
    assert(left.isValid());
    const Type* canon = left.getTypePtr()->getCanonicalType();
    switch (canon->getTypeClass()) {
    case TC_BUILTIN:
        return checkBuiltin(left, right, Loc, conv);
    case TC_POINTER:
        return checkPointer(left, right, Loc, conv);
    case TC_ARRAY:
        break;
    case TC_UNRESOLVED:
        break;
    case TC_ALIAS:
        break;
    case TC_STRUCT:
        break;
    case TC_ENUM:
        break;
    case TC_FUNCTION:
        break;
    }
    return false;
}

bool FunctionAnalyser::checkBuiltin(QualType left, QualType right, SourceLocation Loc, ConvType conv) const {
    LOG_FUNC
    if (right->isBuiltinType()) {
        // NOTE: canonical is builtin, var itself my be UnresolvedType etc
        const BuiltinType* Right = cast<BuiltinType>(right->getCanonicalType());
        const BuiltinType* Left = cast<BuiltinType>(left->getCanonicalType());
        int rule = type_conversions[Right->getKind()][Left->getKind()];
        // 0 = ok, 1 = loss of precision, 2 sign-conversion, 3=float->integer, 4 incompatible, 5 loss of FP prec.
        // TODO use matrix with allowed conversions: 3 options: ok, error, warn
        if (rule == 0) return true;

        int errorMsg = 0;
        switch (rule) {
        case 1: // loss of precision
            errorMsg = diag::warn_impcast_integer_precision;
            break;
        case 2: // sign-conversion
            errorMsg = diag::warn_impcast_integer_sign;
            break;
        case 3: // float->integer
            errorMsg = diag::warn_impcast_float_integer;
            break;
        case 4: // incompatible
            errorMsg = diag::err_illegal_type_conversion;
            break;
        case 5: // loss of fp-precision
            errorMsg = diag::warn_impcast_float_precision;
            break;
        default:
            assert(0 && "should not come here");
        }
        StringBuilder buf1(MAX_TYPENAME);
        StringBuilder buf2(MAX_TYPENAME);
        right.DiagName(buf1);
        left.DiagName(buf2);
        // TODO error msg depends on conv type (see clang errors)
        Diags.Report(Loc, errorMsg) << buf1 << buf2;
        return false;
    }

    StringBuilder buf1(MAX_TYPENAME);
    StringBuilder buf2(MAX_TYPENAME);
    right.DiagName(buf1);
    left.DiagName(buf2);
    // TODO error msg depends on conv type (see clang errors)
    Diags.Report(Loc, diag::err_illegal_type_conversion) << buf1 << buf2;
    return false;
}

bool FunctionAnalyser::checkPointer(QualType left, QualType right, SourceLocation Loc, ConvType conv) const {
    LOG_FUNC
    if (right->isPointerType()) {
#warning "TODO dereference types (can be Alias etc) and check those"
        return true;
    }
    if (right->isArrayType()) {
#warning "TODO dereference types (can be Alias etc) and check those"
        return true;
    }
    StringBuilder buf1(MAX_TYPENAME);
    StringBuilder buf2(MAX_TYPENAME);
    right.DiagName(buf1);
    left.DiagName(buf2);
    // TODO error msg depends on conv type (see clang errors)
    Diags.Report(Loc, diag::err_illegal_type_conversion) << buf1 << buf2;
    return false;
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
    assert(inConstExpr == false);
    inConstExpr = true;
    constDiagID = DiagID;
}

void FunctionAnalyser::popMode() {
    LOG_FUNC
    assert(inConstExpr == true);
    inConstExpr = false;
    constDiagID = 0;
}

