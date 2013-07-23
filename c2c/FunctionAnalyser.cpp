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

// TODO extract to constants.h
const unsigned MAX_TYPENAME = 128;
const unsigned MAX_VARNAME = 64;

// 0 = ok, 1 = illegal
static int type_conversions[14][14] = {
    // TYPE_U8 ->
    //U8,  U16, U32, U64, I8, I16, I32, I64, F32, F64, INT, CHAR, BOOL, STRING, VOID,
    {  0,    1},
    //U8,  U16, U32, U64, I8, I16, I32, I64, F32, F64, INT, CHAR, BOOL, STRING, VOID,
};
/*
    TYPE_U8,
    TYPE_U16,
    TYPE_U32,
    TYPE_U64,
    TYPE_I8,
    TYPE_I16,   // 5
    TYPE_I32,
    TYPE_I64,
    TYPE_F32,
    TYPE_F64,
    TYPE_INT,   // 10
    TYPE_CHAR,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_VOID,
*/

FunctionAnalyser::FunctionAnalyser(FileScope& scope_,
                                           TypeContext& tc,
                                           clang::DiagnosticsEngine& Diags_)
    : globalScope(scope_)
    , typeContext(tc)
    , scopeIndex(0)
    , curScope(0)
    , Diags(Diags_)
    , errors(0)
    , func(0)
    , inConstExpr(false)
    , constDiagID(0)
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
    LOG_FUNC
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
            QualType rtype = func->getReturnType();
            bool need_rvalue = (rtype.getTypePtr() != BuiltinType::get(TYPE_VOID));
            if (need_rvalue) {
                CompoundStmt* compound = func->getBody();
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
            QualType QT = VD->getType();
            const Type* T = QT.getTypePtr();
            if (T->isArrayType() && T->getArrayExpr()) {
                ConstModeSetter setter(*this, diag::err_vla_decl_in_file_scope);
                EnterScope(0);
                analyseInitExpr(T->getArrayExpr(),  BuiltinType::get(TYPE_INT));
                ExitScope();
            }
            Expr* Init = VD->getInitValue();
            if (Init) {
                ConstModeSetter setter(*this, diag::err_init_element_not_constant);
                EnterScope(0);
                // TEMP CONST CAST
                analyseInitExpr(Init, (Type*)VD->getType().getTypePtr());
                ExitScope();
            }
            if (QT.isConstQualified() && !Init) {
                Diags.Report(VD->getLocation(), diag::err_uninitialized_const_var) << VD->getName();
            }
        }
        break;
    case DECL_ENUMVALUE:
        assert(0 && "TODO");
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
    LOG_FUNC
    assert (scopeIndex < MAX_SCOPE_DEPTH && "out of scopes");
    scopes[scopeIndex].Init(flags);
    curScope = &scopes[scopeIndex];
    scopeIndex++;
}

void FunctionAnalyser::ExitScope() {
    LOG_FUNC
    scopeIndex--;
    Scope* parent = curScope->getParent();
    curScope = parent;
}

void FunctionAnalyser::analyseStmt(Stmt* S, bool haveScope) {
    LOG_FUNC
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
    LOG_FUNC
    CompoundStmt* compound = StmtCaster<CompoundStmt>::getType(stmt);
    assert(compound);
    const StmtList& stmts = compound->getStmts();
    for (unsigned int i=0; i<stmts.size(); i++) {
        analyseStmt(stmts[i]);
    }
}

void FunctionAnalyser::analyseIfStmt(Stmt* stmt) {
    LOG_FUNC
    IfStmt* I = StmtCaster<IfStmt>::getType(stmt);
    assert(I);
    Expr* cond = I->getCond();
    QualType Q1 = analyseExpr(cond);
    checkConversion(cond->getLocation(), Q1, QualType(BuiltinType::get(TYPE_BOOL)));
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
    WhileStmt* W = StmtCaster<WhileStmt>::getType(stmt);
    assert(W);
    analyseStmt(W->getCond());
    EnterScope(Scope::BreakScope | Scope::ContinueScope | Scope::DeclScope | Scope::ControlScope);
    analyseStmt(W->getBody(), true);
    ExitScope();

}

void FunctionAnalyser::analyseDoStmt(Stmt* stmt) {
    LOG_FUNC
    DoStmt* D = StmtCaster<DoStmt>::getType(stmt);
    assert(D);
    EnterScope(Scope::BreakScope | Scope::ContinueScope | Scope::DeclScope);
    analyseStmt(D->getBody());
    ExitScope();
    analyseStmt(D->getCond());
}

void FunctionAnalyser::analyseForStmt(Stmt* stmt) {
    LOG_FUNC
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
    LOG_FUNC
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
        BreakStmt* B = StmtCaster<BreakStmt>::getType(stmt);
        assert(B);
        Diags.Report(B->getLocation(), diag::err_break_not_in_loop_or_switch);
    }
}

void FunctionAnalyser::analyseContinueStmt(Stmt* stmt) {
    LOG_FUNC
    if (!curScope->allowContinue()) {
        ContinueStmt* C = StmtCaster<ContinueStmt>::getType(stmt);
        assert(C);
        Diags.Report(C->getLocation(), diag::err_continue_not_in_loop);
    }
}

void FunctionAnalyser::analyseCaseStmt(Stmt* stmt) {
    LOG_FUNC
    CaseStmt* C = StmtCaster<CaseStmt>::getType(stmt);
    assert(C);
    analyseExpr(C->getCond());
    const StmtList& stmts = C->getStmts();
    for (unsigned int i=0; i<stmts.size(); i++) {
        analyseStmt(stmts[i]);
    }
}

void FunctionAnalyser::analyseDefaultStmt(Stmt* stmt) {
    LOG_FUNC
    DefaultStmt* D = StmtCaster<DefaultStmt>::getType(stmt);
    assert(D);
    const StmtList& stmts = D->getStmts();
    for (unsigned int i=0; i<stmts.size(); i++) {
        analyseStmt(stmts[i]);
    }
}

void FunctionAnalyser::analyseReturnStmt(Stmt* stmt) {
    LOG_FUNC
    ReturnStmt* ret = StmtCaster<ReturnStmt>::getType(stmt);
    assert(ret);
    Expr* value = ret->getExpr();
    QualType rtype = func->getReturnType();
    bool no_rvalue = (rtype.getTypePtr() == BuiltinType::get(TYPE_VOID));
    if (value) {
        QualType type = analyseExpr(value);
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
    LOG_FUNC
    Expr* expr = StmtCaster<Expr>::getType(stmt);
    assert(expr);
    analyseExpr(expr);
}

C2::QualType FunctionAnalyser::Decl2Type(Decl* decl) {
    LOG_FUNC
    assert(decl);
    switch (decl->dtype()) {
    case DECL_FUNC:
        {
            FunctionDecl* FD = DeclCaster<FunctionDecl>::getType(decl);
            return FD->getType();
        }
    case DECL_VAR:
        {
            VarDecl* VD = DeclCaster<VarDecl>::getType(decl);
            return VD->getType();
        }
    case DECL_ENUMVALUE:
        {
            EnumConstantDecl* EC = DeclCaster<EnumConstantDecl>::getType(decl);
            return EC->getType();
        }
    case DECL_TYPE:
        {
            TypeDecl* TD = DeclCaster<TypeDecl>::getType(decl);
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

C2::QualType FunctionAnalyser::analyseExpr(Expr* expr) {
    LOG_FUNC
    switch (expr->etype()) {
    case EXPR_NUMBER:
        // TEMP for now always return type int
        return QualType(BuiltinType::get(TYPE_INT));
    case EXPR_STRING:
        {
            // return type: 'const char*'
            QualType stype = typeContext.getPointer(BuiltinType::get(TYPE_CHAR));
            stype.addConst();
            return stype;
        }
    case EXPR_BOOL:
        return QualType(BuiltinType::get(TYPE_BOOL));
    case EXPR_CHARLITERAL:
        return QualType(BuiltinType::get(TYPE_I8));
    case EXPR_FLOAT_LITERAL:
        // For now always return type float
        return QualType(BuiltinType::get(TYPE_F32));
    case EXPR_CALL:
        return analyseCall(expr);
    case EXPR_IDENTIFIER:
        {
            ScopeResult Res = analyseIdentifier(expr);
            if (!Res.ok) break;
            if (!Res.decl) break;
            if (Res.pkg) {
                IdentifierExpr* id = ExprCaster<IdentifierExpr>::getType(expr);
                id->setPackage(Res.pkg);
                id->setDecl(Res.decl);
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
    case EXPR_BUILTIN:
        analyseBuiltinExpr(expr);
        break;
    case EXPR_ARRAYSUBSCRIPT:
        return analyseArraySubscript(expr);
    case EXPR_MEMBER:
        return analyseMemberExpr(expr);
    case EXPR_PAREN:
        return analyseParenExpr(expr);
    }
    return QualType();
}

void FunctionAnalyser::analyseInitExpr(Expr* expr, QualType expectedType) {
    LOG_FUNC

    switch (expr->etype()) {
    case EXPR_NUMBER:
    case EXPR_STRING:
    case EXPR_BOOL:
    case EXPR_CHARLITERAL:
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
            ScopeResult Res = analyseIdentifier(expr);
            IdentifierExpr* id = ExprCaster<IdentifierExpr>::getType(expr);
            if (!Res.ok) return;
            if (!Res.decl) return;
            if (Res.pkg) {
                id->setPackage(Res.pkg);
                id->setDecl(Res.decl);
            }
            switch (Res.decl->dtype()) {
            case DECL_FUNC:
                // can be ok for const
                assert(0 && "TODO");
                break;
            case DECL_VAR:
                {
                    VarDecl* VD = DeclCaster<VarDecl>::getType(Res.decl);
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
            case DECL_TYPE:
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
        analyseBinaryOperator(expr);
        break;
    case EXPR_CONDOP:
        analyseConditionalOperator(expr);
        break;
    case EXPR_UNARYOP:
        analyseUnaryOperator(expr);
        break;
    case EXPR_BUILTIN:
        analyseBuiltinExpr(expr);
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

void FunctionAnalyser::analyseInitList(Expr* expr, QualType expectedType) {
    LOG_FUNC
    InitListExpr* I = ExprCaster<InitListExpr>::getType(expr);
    assert(I);
    assert(expectedType.isValid());

    const Type* type = expectedType.getTypePtr();
    switch (type->getKind()) {
    case Type::USER:
        {
            analyseInitList(expr, type->getRefType());
            return;
        }
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
                analyseInitExpr(values[i], member->getType());
            }
        }
        break;
    case Type::ARRAY:
        {
            // check array member type with each value in initlist
            ExprList& values = I->getValues();
            for (unsigned int i=0; i<values.size(); i++) {
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
}

void FunctionAnalyser::analyseDeclExpr(Expr* expr) {
    LOG_FUNC
    DeclExpr* decl = ExprCaster<DeclExpr>::getType(expr);
    assert(decl);

    // check type and convert User types
    QualType type = decl->getType();
    // TODO CONST CAST
    errors += globalScope.checkType((Type*)type.getTypePtr(), false);

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
    if (type.isConstQualified() && !initialValue) {
        Diags.Report(decl->getLocation(), diag::err_uninitialized_const_var) << decl->getName();
    }
    curScope->addDecl(new VarDecl(decl, false, true));
}

QualType FunctionAnalyser::analyseBinaryOperator(Expr* expr) {
    LOG_FUNC
    BinaryOperator* binop = ExprCaster<BinaryOperator>::getType(expr);
    assert(binop);
    QualType TLeft = analyseExpr(binop->getLHS());
    QualType TRight = analyseExpr(binop->getRHS());
    // assigning to 'A' from incompatible type 'B'
    // diag::err_typecheck_convert_incompatible
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
        return QualType(BuiltinType::get(TYPE_INT));
    case BO_Shl:
    case BO_Shr:
        return TLeft;
    case BO_LE:
    case BO_LT:
    case BO_GE:
    case BO_GT:
    case BO_NE:
    case BO_EQ:
        return QualType(BuiltinType::get(TYPE_BOOL));
    case BO_And:
    case BO_Xor:
    case BO_Or:
        return TLeft;
    case BO_LAnd:
    case BO_LOr:
        return QualType(BuiltinType::get(TYPE_BOOL));
    case BO_Assign:
        //checkAssignmentOperands(TLeft, TRight);
        checkConversion(binop->getLocation(), TRight, TLeft);
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
        return TLeft;
    case BO_Comma:
        assert(0 && "unhandled binary operator type");
        break;
    }
    return QualType();
}

QualType FunctionAnalyser::analyseConditionalOperator(Expr* expr) {
    LOG_FUNC
    ConditionalOperator* condop = ExprCaster<ConditionalOperator>::getType(expr);
    assert(condop);
    analyseExpr(condop->getCond());
    QualType TLeft = analyseExpr(condop->getLHS());
    analyseExpr(condop->getRHS());
    // TODO also check type of RHS
    return TLeft;
}

QualType FunctionAnalyser::analyseUnaryOperator(Expr* expr) {
    LOG_FUNC
    UnaryOperator* unaryop = ExprCaster<UnaryOperator>::getType(expr);
    assert(unaryop);
    QualType LType = analyseExpr(unaryop->getExpr());
    if (LType.isNull()) return 0;
    switch (unaryop->getOpcode()) {
    case UO_AddrOf:
        return typeContext.getPointer(LType);
    case UO_Deref:
        // TODO handle user types
        if (!LType.isPointerType()) {
            char typeName[MAX_TYPENAME];
            StringBuilder buf(MAX_TYPENAME, typeName);
            LType.DiagName(buf);
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

void FunctionAnalyser::analyseBuiltinExpr(Expr* expr) {
    LOG_FUNC
    BuiltinExpr* func = ExprCaster<BuiltinExpr>::getType(expr);
    assert(func);
    analyseExpr(func->getExpr());
    if (func->isSizeFunc()) { // sizeof()
        // TODO can also be type (for sizeof)
    } else { // elemsof()
        Expr* E = func->getExpr();
        IdentifierExpr* I = ExprCaster<IdentifierExpr>::getType(E);
        assert(I && "expr should be IdentifierExpr");
        Decl* D = I->getDecl();
        // should be VarDecl(for array/enum) or TypeDecl(array/enum)
        switch (D->dtype()) {
        case DECL_FUNC:
             fprintf(stderr, "TODO Function err\n");
            // ERROR
            return;
        case DECL_VAR:
            {
                VarDecl* VD = DeclCaster<VarDecl>::getType(D);
                QualType Q = VD->getType();
                if (!Q.isArrayType() && !Q.isEnumType()) {
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
        case DECL_TYPE:
            {
                assert(0 && "TODO");
                return;
            }
        case DECL_ARRAYVALUE:
        case DECL_USE:
            assert(0);
            break;
        }
    }
}

QualType FunctionAnalyser::analyseArraySubscript(Expr* expr) {
    LOG_FUNC
    ArraySubscriptExpr* sub = ExprCaster<ArraySubscriptExpr>::getType(expr);
    assert(sub);
    QualType LType = analyseExpr(sub->getBase());
    if (LType.isNull()) return 0;
    // TODO this should be done in analyseExpr()
    QualType LType2 = resolveUserType(LType);
    if (LType2.isNull()) return 0;
    if (!LType2.isSubscriptable()) {
        Diags.Report(expr->getLocation(), diag::err_typecheck_subscript);
        return 0;
    }
    analyseExpr(sub->getIndex());
    return LType2.getTypePtr()->getRefType();
}

QualType FunctionAnalyser::analyseMemberExpr(Expr* expr) {
    LOG_FUNC
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
        if (!SR.ok) return QualType();
        if (SR.decl) {
            IdentifierExpr* base_id = ExprCaster<IdentifierExpr>::getType(base);
            switch (SR.decl->dtype()) {
            case DECL_FUNC:
            case DECL_TYPE:
                fprintf(stderr, "error: member reference base 'type' is not a structure, union or package\n");
                return QualType();
            case DECL_VAR:
                {
                    // TODO extract to function?
                    VarDecl* VD = DeclCaster<VarDecl>::getType(SR.decl);
                    QualType T = VD->getType();
                    assert(T.isValid());  // analyser should set

                    if (isArrow) {
                        if (!T.isPointerType()) {
                            fprintf(stderr, "TODO using -> with non-pointer type\n");
                            // continue analysing
                        } else {
                            // deref
                            T = T->getRefType();
                        }
                    } else {
                        if (T.isPointerType()) {
                            fprintf(stderr, "TODO using . with pointer type\n");
                            // just deref and continue for now
                            T = T.getTypePtr()->getRefType();
                            // TODO qualifiers?
                        }
                    }
                    if (T.isUserType()) {
                        T = T.getTypePtr()->getRefType();
                        // TODO qualifiers?
                        assert(T.isValid() && "analyser should set refType");
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
                        return QualType();
                    }
                    // find member in struct
                    MemberList* members = T->getMembers();
                    for (unsigned i=0; i<members->size(); i++) {
                        DeclExpr* de = (*members)[i];
                        if (de->getName() == member->getName()) { // found
                            return de->getType();
                        }
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
        QualType LType = analyseExpr(base);
        if (LType.isNull()) return QualType();
        // TODO this should be done in analyseExpr()
        QualType LType2 = resolveUserType(LType);
        if (LType2.isNull()) return QualType();
        if (!LType2.isStructOrUnionType()) {
            fprintf(stderr, "error: not a struct or union type\n");
            LType2->dump();
            return QualType();
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
        fprintf(stderr, "error: (1) Type 'todo' has no member '%s'\n", member->getName().c_str());
        return QualType();
    }
    return QualType();
}

QualType FunctionAnalyser::analyseParenExpr(Expr* expr) {
    LOG_FUNC
    ParenExpr* P = ExprCaster<ParenExpr>::getType(expr);
    assert(P);
    return analyseExpr(P->getExpr());
}

QualType FunctionAnalyser::analyseCall(Expr* expr) {
    LOG_FUNC
    CallExpr* call = ExprCaster<CallExpr>::getType(expr);
    assert(call);
    // analyse function
    QualType LType = analyseExpr(call->getFn());
    if (LType.isNull()) {
        fprintf(stderr, "CALL unknown function (already error)\n");
        call->getFn()->dump();
        return QualType();
    }
    // TODO this should be done in analyseExpr()
    QualType LType2 = resolveUserType(LType);
    if (LType2.isNull()) return QualType();
    if (!LType2.isFuncType()) {
        fprintf(stderr, "error: NOT a function type TODO\n");
        char typeName[MAX_TYPENAME];
        StringBuilder buf(MAX_TYPENAME, typeName);
        LType2.DiagName(buf);
        Diags.Report(call->getLocation(), diag::err_typecheck_call_not_function) << typeName;
        return 0;
    }

    // TODO check if Ellipsoid otherwise compare num args with num params
    const Type* T = LType2.getTypePtr();
    for (unsigned i=0; i<call->numArgs(); i++) {
        Expr* arg = call->getArg(i);
        QualType ArgGot = analyseExpr(arg);
        QualType ArgNeed = T->getArgument(i);
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
    return T->getReturnType();
}

ScopeResult FunctionAnalyser::analyseIdentifier(Expr* expr) {
    LOG_FUNC
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

C2::QualType FunctionAnalyser::checkAssignmentOperands(QualType left, QualType right) {
    LOG_FUNC
    assert(left.isValid());
    assert(right.isValid());

    // TEMP only check for (int) = (float) for now
    if (right.getTypePtr() == BuiltinType::get(TYPE_F32) &&
        left.getTypePtr() == BuiltinType::get(TYPE_U32))
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
    QualType qt;
    return qt;
}

void FunctionAnalyser::checkConversion(SourceLocation Loc, QualType from, QualType to) {
#ifdef ANALYSER_DEBUG
    StringBuilder buf;
    buf <<ANSI_MAGENTA << __func__ << "() converversion ";
    from.DiagName(buf);
    buf << " to ";
    to.DiagName(buf);
    buf << ANSI_NORMAL;
    fprintf(stderr, "%s\n", (const char*)buf);
#endif
    // TEMP only check float -> bool
    const Type* t1 = from.getTypePtr();
    const Type* t2 = to.getTypePtr();
    // TODO use getCanonicalType()
    if (t1->isBuiltinType() && t2->isBuiltinType()) {
        C2Type tt1 = t1->getBuiltinType();
        C2Type tt2 = t2->getBuiltinType();
        // TODO use matrix with allowed conversions: 3 options: ok, error, warn
        if (tt1 == TYPE_F32 && tt2 == TYPE_BOOL) {
            StringBuilder buf1(MAX_TYPENAME);
            StringBuilder buf2(MAX_TYPENAME);
            from.DiagName(buf1);
            to.DiagName(buf2);
            Diags.Report(Loc, diag::err_illegal_type_conversion)
                << buf1 << buf2;
        }
    }
}

C2::QualType FunctionAnalyser::resolveUserType(QualType T) {
    if (T->isUserType()) {
        QualType t2 = T.getTypePtr()->getRefType();
        assert(t2.isValid());
        // TODO Qualifiers correct?
        return t2;
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

