/* Copyright 2013-2017 Bas van den Berg
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

#include <assert.h>

#include "Algo/DepVisitor.h"
#include "AST/Type.h"
#include "AST/Decl.h"
#include "AST/Stmt.h"
#include "AST/Expr.h"

using namespace C2;

void DepVisitor::run() {
    //fprintf(stderr, "CHECKING %s\n", decl->getName().c_str());
    checkDecl(decl);
}

void DepVisitor::checkDecl(const Decl* D) {
    switch (D->getKind()) {
    case DECL_FUNC:
        checkFunctionDecl(cast<FunctionDecl>(D));
        break;
    case DECL_VAR:
        checkVarDecl(cast<VarDecl>(D));
        break;
    case DECL_ENUMVALUE:
        assert(0);
        break;
    case DECL_ALIASTYPE:
        checkType(cast<AliasTypeDecl>(D)->getRefType());
        break;
    case DECL_STRUCTTYPE:
    {
        const StructTypeDecl* S = cast<StructTypeDecl>(D);
        for (unsigned i=0; i<S->numMembers(); i++) {
            checkDecl(S->getMember(i));
        }
        break;
    }
    case DECL_ENUMTYPE:
    {
        const EnumTypeDecl* E = cast<EnumTypeDecl>(D);
        for (unsigned i=0; i<E->numConstants(); i++) {
            const EnumConstantDecl* ECD = E->getConstant(i);
            if (ECD->getInitValue()) checkExpr(ECD->getInitValue());
        }
        break;
    }
    case DECL_FUNCTIONTYPE:
    {
        const FunctionTypeDecl* F = cast<FunctionTypeDecl>(D);
        checkFunctionDecl(F->getDecl());
        break;
    }
    case DECL_ARRAYVALUE:
        assert(0 && "TODO");
        break;
    case DECL_IMPORT:
    case DECL_LABEL:
        break;
    }
}

void DepVisitor::checkFunctionDecl(const FunctionDecl* F) {
    // return Type
    checkType(F->getReturnType());

    // args
    for (unsigned i=0; i<F->numArgs(); i++) {
        checkVarDecl(F->getArg(i));
    }

    // check body
    if (F->getBody()) checkCompoundStmt(F->getBody());
}

void DepVisitor::checkVarDecl(const VarDecl* V) {
    checkType(V->getType());

    if (V->getInitValue()) checkExpr(V->getInitValue());
}

void DepVisitor::checkType(QualType Q, bool isFull) {
    const Type* T = Q.getTypePtr();
    switch (T->getTypeClass()) {
    case TC_BUILTIN:
        return;
    case TC_POINTER:
        checkType(cast<PointerType>(T)->getPointeeType(), false);
        break;
    case TC_ARRAY:
    {
        const ArrayType* A = cast<ArrayType>(T);
        checkType(A->getElementType(), isFull);
        if (A->getSizeExpr()) checkExpr(A->getSizeExpr());
        break;
    }
    case TC_UNRESOLVED:
        addDep(cast<UnresolvedType>(T)->getDecl(), isFull);
        break;
    case TC_ALIAS:
        addDep(cast<AliasType>(T)->getDecl(), isFull);
        break;
    case TC_STRUCT:
        addDep(cast<StructType>(T)->getDecl(), isFull);
        break;
    case TC_ENUM:
        addDep(cast<EnumType>(T)->getDecl(), isFull);
        break;
    case TC_FUNCTION:
        addDep(cast<FunctionType>(T)->getDecl(), isFull);
        break;
    case TC_MODULE:
        assert(0);
        break;
    }
}

void DepVisitor::checkStmt(const Stmt* S) {
    assert(S);
    switch (S->getKind()) {
    case STMT_RETURN:
    {
        const ReturnStmt* R = cast<ReturnStmt>(S);
        if (R->getExpr()) checkExpr(R->getExpr());
        break;
    }
    case STMT_EXPR:
        checkExpr(cast<Expr>(S));
        break;
    case STMT_IF:
    {
        const IfStmt* I = cast<IfStmt>(S);
        checkStmt(I->getCond());
        checkStmt(I->getThen());
        if (I->getElse()) checkStmt(I->getElse());
        break;
    }
    case STMT_WHILE:
    {
        const WhileStmt* W = cast<WhileStmt>(S);
        checkStmt(W->getCond());
        checkStmt(W->getBody());
        break;
    }
    case STMT_DO:
    {
        const DoStmt* D = cast<DoStmt>(S);
        checkStmt(D->getCond());
        checkStmt(D->getBody());
        break;
    }
    case STMT_FOR:
    {
        const ForStmt* F = cast<ForStmt>(S);
        if (F->getInit()) checkStmt(F->getInit());
        if (F->getCond()) checkExpr(F->getCond());
        if (F->getIncr()) checkExpr(F->getIncr());
        checkStmt(F->getBody());
        break;
    }
    case STMT_SWITCH:
    {
        const SwitchStmt* SW = cast<SwitchStmt>(S);
        checkStmt(SW->getCond());
        Stmt** cases = SW->getCases();
        for (unsigned i=0; i<SW->numCases(); i++) {
            checkStmt(cases[i]);
        }
        break;
    }
    case STMT_CASE:
    {
        const CaseStmt* C = cast<CaseStmt>(S);
        checkExpr(C->getCond());
        Stmt** stmts = C->getStmts();
        for (unsigned i=0; i<C->numStmts(); i++) {
            checkStmt(stmts[i]);
        }
        break;
    }
    case STMT_DEFAULT:
    {
        const DefaultStmt* D = cast<DefaultStmt>(S);
        Stmt** stmts = D->getStmts();
        for (unsigned i=0; i<D->numStmts(); i++) {
            checkStmt(stmts[i]);
        }
        break;
    }
    case STMT_BREAK:
    case STMT_CONTINUE:
    case STMT_LABEL:
    case STMT_GOTO:
        break;
    case STMT_COMPOUND:
        checkCompoundStmt(cast<CompoundStmt>(S));
        break;
    case STMT_DECL:
        checkVarDecl(cast<DeclStmt>(S)->getDecl());
        break;
    }
}

void DepVisitor::checkCompoundStmt(const CompoundStmt* C) {
    Stmt** stmts = C->getStmts();
    for (unsigned i=0; i<C->numStmts(); i++) {
        checkStmt(stmts[i]);
    }
}


void DepVisitor::checkExpr(const Expr* E) {
    assert(E);
    switch (E->getKind()) {
    case EXPR_INTEGER_LITERAL:
    case EXPR_FLOAT_LITERAL:
    case EXPR_BOOL_LITERAL:
    case EXPR_CHAR_LITERAL:
    case EXPR_STRING_LITERAL:
    case EXPR_NIL:
        break;
    case EXPR_IDENTIFIER:
        addDep(cast<IdentifierExpr>(E)->getDecl());
        break;
    case EXPR_TYPE:
        // only in sizeof(int), so no need to check here
        break;
    case EXPR_CALL:
    {
        const CallExpr* C = cast<CallExpr>(E);
        checkExpr(C->getFn());
        for (unsigned i=0; i<C->numArgs(); i++) {
            checkExpr(C->getArg(i));
        }
        break;
    }
    case EXPR_INITLIST:
    {
        const InitListExpr* I = cast<InitListExpr>(E);
        Expr** values = I->getValues();
        for (unsigned i=0; i<I->numValues(); i++) {
            checkExpr(values[i]);
        }
        break;
    }
    case EXPR_DESIGNATOR_INIT:
        break;
    case EXPR_BINOP:
    {
        const BinaryOperator* B = cast<BinaryOperator>(E);
        checkExpr(B->getLHS());
        checkExpr(B->getRHS());
        break;
    }
    case EXPR_CONDOP:
    {
        const ConditionalOperator* C = cast<ConditionalOperator>(E);
        checkExpr(C->getCond());
        checkExpr(C->getLHS());
        checkExpr(C->getRHS());
        break;
    }
    case EXPR_UNARYOP:
        checkExpr(cast<UnaryOperator>(E)->getExpr());
        break;
    case EXPR_BUILTIN:
        checkExpr(cast<BuiltinExpr>(E)->getExpr());
        break;
    case EXPR_ARRAYSUBSCRIPT:
    {
        const ArraySubscriptExpr* A = cast<ArraySubscriptExpr>(E);
        checkExpr(A->getBase());
        checkExpr(A->getIndex());
        break;
    }
    case EXPR_MEMBER:
    {
        const MemberExpr* M = cast<MemberExpr>(E);
        if (M->isModulePrefix()) {
            addDep(M->getDecl());
        } else {
            checkExpr(M->getBase());
            if (M->isStructFunction()) addDep(M->getDecl());
        }
        break;
    }
    case EXPR_PAREN:
        checkExpr(cast<ParenExpr>(E)->getExpr());
        break;
    case EXPR_BITOFFSET:
    {
        const BitOffsetExpr* B = cast<BitOffsetExpr>(E);
        checkExpr(B->getLHS());
        checkExpr(B->getRHS());
        break;
    }
    break;
    case EXPR_CAST:
    {
        const ExplicitCastExpr* ECE = cast<ExplicitCastExpr>(E);
        checkExpr(ECE->getInner());
        checkType(ECE->getDestType());
    }
    break;
    }
}

void DepVisitor::addDep(const Decl* D, bool isFull) {
    assert(D);

    // Skip local VarDecls
    if (const VarDecl* V = dyncast<VarDecl>(D)) {
        switch (V->getVarKind()) {
        case VARDECL_GLOBAL:
            break;
        case VARDECL_LOCAL:
        case VARDECL_PARAM:
            return;
        case VARDECL_MEMBER:
            break;
        }
    }

    if (!checkExternals && D->getModule() != decl->getModule()) return;

    // Convert EnumConstants -> EnumTypeDecl (via EnumType)
    if (const EnumConstantDecl* ECD = dyncast<EnumConstantDecl>(D)) {
        QualType Q = ECD->getType();
        const EnumType* T = cast<EnumType>(Q.getTypePtr());
        D = T->getDecl();
    }

    for (unsigned i=0; i<deps.size(); i++) {
        if (getDep(i) == D) {
            // update pointer to full if needed
            if (deps[i] != (uintptr_t)D) deps[i] |= 0x1;
            return;
        }
    }
    deps.push_back((uintptr_t)D | isFull);
    //fprintf(stderr, "  %s -> %s %p (%s)\n", decl->getName().c_str(), D->getName().c_str(), D, isFull ? "full" : "pointer");
}

