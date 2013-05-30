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

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <clang/Parse/ParseDiagnostic.h>
#include <clang/Sema/SemaDiagnostic.h>

#include "C2Sema.h"
#include "Decl.h"
#include "Expr.h"
#include "StringBuilder.h"
#include "color.h"
#include "ASTVisitor.h"

#define SEMA_DEBUG

#define COL_SEMA ANSI_RED

using namespace C2;
using namespace clang;

static inline clang::BinaryOperatorKind ConvertTokenKindToBinaryOpcode(tok::TokenKind Kind) {
  clang::BinaryOperatorKind Opc;
  switch (Kind) {
  default: llvm_unreachable("Unknown binop!");
  case tok::periodstar:           Opc = BO_PtrMemD; break;
  case tok::arrowstar:            Opc = BO_PtrMemI; break;
  case tok::star:                 Opc = BO_Mul; break;
  case tok::slash:                Opc = BO_Div; break;
  case tok::percent:              Opc = BO_Rem; break;
  case tok::plus:                 Opc = BO_Add; break;
  case tok::minus:                Opc = BO_Sub; break;
  case tok::lessless:             Opc = BO_Shl; break;
  case tok::greatergreater:       Opc = BO_Shr; break;
  case tok::lessequal:            Opc = BO_LE; break;
  case tok::less:                 Opc = BO_LT; break;
  case tok::greaterequal:         Opc = BO_GE; break;
  case tok::greater:              Opc = BO_GT; break;
  case tok::exclaimequal:         Opc = BO_NE; break;
  case tok::equalequal:           Opc = BO_EQ; break;
  case tok::amp:                  Opc = BO_And; break;
  case tok::caret:                Opc = BO_Xor; break;
  case tok::pipe:                 Opc = BO_Or; break;
  case tok::ampamp:               Opc = BO_LAnd; break;
  case tok::pipepipe:             Opc = BO_LOr; break;
  case tok::equal:                Opc = BO_Assign; break;
  case tok::starequal:            Opc = BO_MulAssign; break;
  case tok::slashequal:           Opc = BO_DivAssign; break;
  case tok::percentequal:         Opc = BO_RemAssign; break;
  case tok::plusequal:            Opc = BO_AddAssign; break;
  case tok::minusequal:           Opc = BO_SubAssign; break;
  case tok::lesslessequal:        Opc = BO_ShlAssign; break;
  case tok::greatergreaterequal:  Opc = BO_ShrAssign; break;
  case tok::ampequal:             Opc = BO_AndAssign; break;
  case tok::caretequal:           Opc = BO_XorAssign; break;
  case tok::pipeequal:            Opc = BO_OrAssign; break;
  case tok::comma:                Opc = BO_Comma; break;
  }
  return Opc;
}

static inline UnaryOperatorKind ConvertTokenKindToUnaryOpcode(
  tok::TokenKind Kind) {
  UnaryOperatorKind Opc;
  switch (Kind) {
  default: llvm_unreachable("Unknown unary op!");
  case tok::plusplus:     Opc = UO_PreInc; break;
  case tok::minusminus:   Opc = UO_PreDec; break;
  case tok::amp:          Opc = UO_AddrOf; break;
  case tok::star:         Opc = UO_Deref; break;
  case tok::plus:         Opc = UO_Plus; break;
  case tok::minus:        Opc = UO_Minus; break;
  case tok::tilde:        Opc = UO_Not; break;
  case tok::exclaim:      Opc = UO_LNot; break;
  case tok::kw___real:    Opc = UO_Real; break;
  case tok::kw___imag:    Opc = UO_Imag; break;
  case tok::kw___extension__: Opc = UO_Extension; break;
  }
  return Opc;
}


C2Sema::C2Sema(SourceManager& sm_, DiagnosticsEngine& Diags_, TypeContext& tc)
    : SourceMgr(sm_)
    , Diags(Diags_)
    , typeContext(tc)
{
}

C2Sema::~C2Sema() {
    for (unsigned int i=0; i<decls.size(); i++) {
        delete decls[i];
    }
}

void C2Sema::ActOnPackage(const char* name, SourceLocation loc) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: package " << name << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    if (strcmp(name, "c2") == 0) {
        Diag(loc, diag::err_package_c2);
        return;
    }
    pkgName = name;
    pkgLoc = loc;
}

void C2Sema::ActOnUse(const char* name, SourceLocation loc, Token& aliasTok, bool isLocal) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: use " << name << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    // check if use-ing own package
    if (pkgName == name) {
        Diag(loc, diag::err_use_own_package) << name;
        return;
    }
    // check for duplicate use
    const Decl* old = findUse(name);
    if (old) {
        Diag(loc, diag::err_duplicate_use) << name;
        Diag(old->getLocation(), diag::note_previous_use);
        return;
    }
    const char* aliasName = "";
    if (aliasTok.is(tok::identifier)) {
        aliasName = aliasTok.getIdentifierInfo()->getNameStart();
        // check if same as normal package name
        if (strcmp(name, aliasName) == 0) {
            Diag(aliasTok.getLocation(), diag::err_alias_same_as_package);
            return;
        }
        const UseDecl* old = findAlias(aliasName);
        if (old) {
            Diag(aliasTok.getLocation(), diag::err_duplicate_use) << aliasName;
            Diag(old->getAliasLocation(), diag::note_previous_use);
            return;
        }
    }
    addDecl(new UseDecl(name, loc, isLocal, aliasName, aliasTok.getLocation()));
}

void C2Sema::ActOnTypeDef(const char* name, SourceLocation loc, Expr* type, bool is_public) {
    assert(name);
    assert(type);
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: type def " << name << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    // TEMP extract here to Type and delete rtype Expr
    TypeExpr* typeExpr = ExprCaster<TypeExpr>::getType(type);
    assert(typeExpr);
    TypeDecl* decl = new TypeDecl(name, loc, typeExpr->getType(), is_public);
    addDecl(decl);
    delete type;
}

void C2Sema::ActOnVarDef(const char* name, SourceLocation loc,
                        bool is_public, Expr* type, Expr* InitValue) {
    assert(type);
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: var def " << name << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    // TEMP extract here to Type and delete rtype Expr
    TypeExpr* typeExpr = ExprCaster<TypeExpr>::getType(type);
    assert(typeExpr);
    // TODO check that type is not pre-fixed with own package
    DeclExpr* declExpr = new DeclExpr(name, loc, typeExpr->getType(), InitValue);
    VarDecl* decl = new VarDecl(declExpr, is_public, false);
    addDecl(decl);
    delete type;
}

C2::FunctionDecl* C2Sema::ActOnFuncDef(const char* name, SourceLocation loc, bool is_public, Expr* rtype) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: func def " << name << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    // TEMP extract here to Type and delete rtype Expr
    assert(rtype);
    TypeExpr* typeExpr = ExprCaster<TypeExpr>::getType(rtype);
    assert(typeExpr);
    FunctionDecl* decl = new FunctionDecl(name, loc, is_public, typeExpr->getType());
    addDecl(decl);
    delete rtype;
    return decl;
}

void C2Sema::ActOnFunctionArgs(Decl* decl, ExprList params) {
    FunctionDecl* func = DeclCaster<FunctionDecl>::getType(decl);
    assert(func);
    for (unsigned int i=0; i<params.size(); i++) {
        DeclExpr* de = ExprCaster<DeclExpr>::getType(params[i]);
        assert(de);
        // check args for duplicates
        DeclExpr* existing = func->findArg(de->getName());
        if (existing) {
            Diag(de->getLocation(), diag::err_param_redefinition) << de->getName();
            Diag(existing->getLocation(), diag::note_previous_declaration);
            continue;
        }
        func->addArg(de);
    }
}

void C2Sema::ActOnFinishFunctionBody(Decl* decl, Stmt* body) {
    FunctionDecl* func = DeclCaster<FunctionDecl>::getType(decl);
    assert(func);
    func->setBody(body);
}

void C2Sema::ActOnArrayValue(const char* name, SourceLocation loc, Expr* Value) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: arrayvalue at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    ArrayValueDecl* decl = new ArrayValueDecl(name, loc, Value);
    addDecl(decl);
}

C2::StmtResult C2Sema::ActOnReturnStmt(SourceLocation loc, Expr* value) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: return at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return StmtResult(new ReturnStmt(loc, value));
}

C2::StmtResult C2Sema::ActOnIfStmt(SourceLocation ifLoc,
                                   ExprResult condition, StmtResult thenStmt,
                                   SourceLocation elseLoc, StmtResult elseStmt) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: if statement at ";
    ifLoc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return StmtResult(new IfStmt(ifLoc, condition.release(), thenStmt.release(), elseLoc, elseStmt.release()));
}

C2::StmtResult C2Sema::ActOnWhileStmt(SourceLocation loc, ExprResult Cond, StmtResult Then) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: while statement at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return StmtResult(new WhileStmt(loc, Cond.release(), Then.release()));
}

C2::StmtResult C2Sema::ActOnDoStmt(SourceLocation loc, ExprResult Cond, StmtResult Then) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: do statement at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return StmtResult(new DoStmt(loc, Cond.release(), Then.release()));
}

C2::StmtResult C2Sema::ActOnForStmt(SourceLocation loc, Stmt* Init, Expr* Cond, Expr* Incr, Stmt* Body) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: for statement at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return StmtResult(new ForStmt(loc, Init, Cond, Incr, Body));
}

C2::StmtResult C2Sema::ActOnSwitchStmt(SourceLocation loc, Expr* Cond, StmtList& cases) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: switch statement at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return StmtResult(new SwitchStmt(loc, Cond, cases));
}

C2::StmtResult C2Sema::ActOnCaseStmt(SourceLocation loc, Expr* Cond, StmtList& stmts) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: case statement at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return StmtResult(new CaseStmt(loc, Cond, stmts));
}

C2::StmtResult C2Sema::ActOnDefaultStmt(SourceLocation loc, StmtList& stmts) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: default statement at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return StmtResult(new DefaultStmt(loc, stmts));
}

C2::StmtResult C2Sema::ActOnBreakStmt(SourceLocation loc) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: break statement at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return StmtResult(new BreakStmt(loc));
}

C2::StmtResult C2Sema::ActOnContinueStmt(SourceLocation loc) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: continue statement at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return StmtResult(new ContinueStmt(loc));
}

C2::StmtResult C2Sema::ActOnLabelStmt(const char* name, SourceLocation loc, Stmt* subStmt) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: label statement at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return StmtResult(new LabelStmt(name, loc, subStmt));
}

C2::StmtResult C2Sema::ActOnGotoStmt(const char* name, SourceLocation GotoLoc, SourceLocation LabelLoc) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: goto statement at ";
    GotoLoc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return StmtResult(new GotoStmt(name, GotoLoc, LabelLoc));
}

C2::StmtResult C2Sema::ActOnCompoundStmt(SourceLocation L, SourceLocation R, StmtList& stmts) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: compound statement at ";
    L.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return StmtResult(new CompoundStmt(L, R, stmts));
}

C2::StmtResult C2Sema::ActOnDeclaration(const char* name, SourceLocation loc, Expr* type, Expr* InitValue) {
    assert(type);
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: decl at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    // TEMP extract here to Type and delete rtype Expr
    TypeExpr* typeExpr = ExprCaster<TypeExpr>::getType(type);
    assert(typeExpr);
    DeclExpr* declExpr = new DeclExpr(name, loc, typeExpr->getType(), InitValue);
    declExpr->setStatementFlag();
    delete type;
    return StmtResult(declExpr);
}

C2::ExprResult C2Sema::ActOnCallExpr(Expr* Fn, Expr** args, unsigned numArgs, SourceLocation RParenLoc) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: call to " << "TODO Fn" << " at " << "TODO Fn";
    //expr2loc(Fn).dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    CallExpr* call = new CallExpr(Fn);
    assert(call);
    for (unsigned int i=0; i<numArgs; i++) call->addArg(args[i]);
    return ExprResult(call);
}

C2::ExprResult C2Sema::ActOnIdExpression(IdentifierInfo& symII, SourceLocation symLoc) {
    std::string id(symII.getNameStart(), symII.getLength());
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: identifier " << id << " at ";
    symLoc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return ExprResult(new IdentifierExpr(symLoc, id));
}

C2::ExprResult C2Sema::ActOnParenExpr(SourceLocation L, SourceLocation R, Expr* E) {
    assert(E && "missing expr");
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: paren expr at ";
    L.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return ExprResult(new ParenExpr(L, R, E));
}

C2::ExprResult C2Sema::ActOnBinOp(SourceLocation opLoc, tok::TokenKind Kind, Expr* LHS, Expr* RHS) {
    assert(LHS);
    assert(RHS);
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: BinOp " << tok::getTokenSimpleSpelling(Kind) << " at ";
    opLoc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    clang::BinaryOperatorKind Opc = ConvertTokenKindToBinaryOpcode(Kind);

    // Emit warnings for tricky precedence issues, e.g. "bitfield & 0x4 == 0"
    //DiagnoseBinOpPrecedence(*this, Opc, TokLoc, LHSExpr, RHSExpr);

    return ExprResult(new BinaryOperator(LHS, RHS, Opc, opLoc));
}

C2::ExprResult C2Sema::ActOnInitList(SourceLocation left_, SourceLocation right_, ExprList& vals) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: initlist at ";
    left_.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return ExprResult(new InitListExpr(left_, right_, vals));
}

C2::ExprResult C2Sema::ActOnArrayType(Expr* base, Expr* size) {
#ifdef SEMA_DEBUGG
    std::cerr << COL_SEMA"SEMA: Array Type"ANSI_NORMAL"\n";
#endif
    assert(base);
    TypeExpr* typeExpr = ExprCaster<TypeExpr>::getType(base);
    assert(typeExpr);
    Type* arr = typeContext.getArray(typeExpr->getType(), size, true);
    typeExpr->setType(arr);
    return ExprResult(base);
}

C2::ExprResult C2Sema::ActOnPointerType(Expr* base) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: Pointer Type"ANSI_NORMAL"\n";
#endif
    assert(base);
    TypeExpr* typeExpr = ExprCaster<TypeExpr>::getType(base);
    assert(typeExpr);
    Type* ptr = typeContext.getPointer(typeExpr->getType());
    typeExpr->setType(ptr);
    return ExprResult(base);
}

C2::ExprResult C2Sema::ActOnUserType(Expr* expr) {
    assert(expr);
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: User Type"ANSI_NORMAL"\n";
#endif
    Type* type = typeContext.getUser();
    type->setUserType(expr);
    return ExprResult(new TypeExpr(type));
}

C2::ExprResult C2Sema::ActOnBuiltinType(C2Type t) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: Builtin Type"ANSI_NORMAL"\n";
#endif
    Type* type = BuiltinType::get(t);
    return ExprResult(new TypeExpr(type));
}

C2::ExprResult C2Sema::ActOnStructType(SourceLocation leftBrace, SourceLocation rightBrace,
                                       ExprList& members, bool isStruct) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: Struct/Union Type"ANSI_NORMAL"\n";
#endif
    Type* type = typeContext.getStruct(isStruct);
    // TODO use left/rightBrace (add to TypeExpr, then pass to TypeDecl)
    MemberList members2;
    for (unsigned int i=0; i<members.size(); i++) {
        DeclExpr* member = ExprCaster<DeclExpr>::getType(members[i]);
        assert(member);
        members2.push_back(member);
    }
    type->setMembers(members2);
    return ExprResult(new TypeExpr(type));
}

C2::ExprResult C2Sema::ActOnEnumType() {
    Type* type = typeContext.getEnum();
    return ExprResult(new TypeExpr(type));
}

C2::ExprResult C2Sema::ActOnEnumTypeFinished(Expr* enumType,
                            SourceLocation leftBrace, SourceLocation rightBrace,
                           ExprList& values) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: enum Type"ANSI_NORMAL"\n";
#endif
    TypeExpr* typeExpr = ExprCaster<TypeExpr>::getType(enumType);
    assert(typeExpr);
    // TODO use left/rightBrace (add to TypeExpr, then pass to TypeDecl)
/*
    MemberList members2;
    for (unsigned int i=0; i<members.size(); i++) {
        DeclExpr* member = ExprCaster<DeclExpr>::getType(members[i]);
        assert(member);
        members2.push_back(member);
    }
    enumType->setMembers(members2);
*/
    values.clear();  // remove entries from original list
    return ExprResult(enumType);
}

C2::ExprResult C2Sema::ActOnEnumConstant(Expr* enumType, IdentifierInfo* symII,
                                SourceLocation symLoc, Expr* Value) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: enum constant"ANSI_NORMAL"\n";
#endif
    TypeExpr* typeExpr = ExprCaster<TypeExpr>::getType(enumType);
    assert(typeExpr);
    return ExprResult(new DeclExpr(symII->getNameStart(), symLoc, typeExpr->getType(), Value));
}

C2::ExprResult C2Sema::ActOnTypeQualifier(ExprResult R, unsigned int qualifier) {
    if (qualifier) {
#ifdef SEMA_DEBUG
        std::cerr << COL_SEMA"SEMA: Qualifier Type"ANSI_NORMAL"\n";
#endif
        assert(R.get());
        TypeExpr* typeExpr = ExprCaster<TypeExpr>::getType(R.get());
        assert(typeExpr);
        Type* qual = typeContext.getQualifier(typeExpr->getType(), qualifier);
        typeExpr->setType(qual);
    }
    return R;
}

C2::ExprResult C2Sema::ActOnVarExpr(const char* name, SourceLocation loc, Expr* type, Expr* InitValue) {
    assert(type);
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: var expr " << name << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    // TEMP extract here to Type and delete rtype Expr
    TypeExpr* typeExpr = ExprCaster<TypeExpr>::getType(type);
    assert(typeExpr);
    DeclExpr* declExpr = new DeclExpr(name, loc, typeExpr->getType(), InitValue);
    delete type;
    return ExprResult(declExpr);
}

C2::ExprResult C2Sema::ActOnSizeofExpression(SourceLocation Loc, Expr* expr) {
    assert(expr);
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: sizeof at ";
    Loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return ExprResult(new SizeofExpr(Loc, expr));
}

C2::ExprResult C2Sema::ActOnArraySubScriptExpr(SourceLocation RLoc, Expr* Base, Expr* Idx) {
    assert(Base);
    assert(Idx);
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: array subscript at ";
    RLoc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return ExprResult(new ArraySubscriptExpr(RLoc, Base, Idx));
}

C2::ExprResult C2Sema::ActOnMemberExpr(Expr* Base, bool isArrow, Expr* Member) {
    assert(Base);
    assert(Member);
    IdentifierExpr* Id = ExprCaster<IdentifierExpr>::getType(Member);
    assert(Id);
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: member access";
    std::cerr << ANSI_NORMAL"\n";
#endif
    return ExprResult(new MemberExpr(Base, isArrow, Id));
}

C2::ExprResult C2Sema::ActOnPostfixUnaryOp(SourceLocation OpLoc, tok::TokenKind Kind, Expr* Input) {
    assert(Input);
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: postop at ";
    OpLoc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    UnaryOperatorKind Opc;
    switch (Kind) {
    default: llvm_unreachable("Unknown unary op!");
    case tok::plusplus:   Opc = UO_PostInc; break;
    case tok::minusminus: Opc = UO_PostDec; break;
    }
#if 0
    // Since this might is a postfix expression, get rid of ParenListExprs.
    ExprResult Result = MaybeConvertParenListExprToParenExpr(S, Input);
    if (Result.isInvalid()) return ExprError();
    Input = Result.take();
#endif
    return ExprResult(new UnaryOperator(OpLoc, Opc, Input));
}

C2::ExprResult C2Sema::ActOnUnaryOp(SourceLocation OpLoc, tok::TokenKind Kind, Expr* Input) {
    assert(Input);
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: unary op at ";
    OpLoc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    UnaryOperatorKind Opc = ConvertTokenKindToUnaryOpcode(Kind);
    return ExprResult(new UnaryOperator(OpLoc, Opc, Input));
}

void C2Sema::visitAST(ASTVisitor& visitor) {
    for (unsigned int i=0; i<decls.size(); i++) {
        bool stop = visitor.handle(decls[i]);
        if (stop) break;
    }
}

C2::ExprResult C2Sema::ActOnBooleanConstant(const Token& Tok) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: boolean constant"ANSI_NORMAL"\n";
#endif
    return ExprResult(new BoolLiteralExpr(Tok.getLocation(), Tok.is(tok::kw_true)));
}

C2::ExprResult C2Sema::ActOnNumericConstant(const Token& Tok) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: numeric constant"ANSI_NORMAL"\n";
#endif
    // TEMP, only support integers
    const char* D = Tok.getLiteralData();
    char buffer[30];
    memset(buffer, 0, sizeof(buffer));
    strncpy(buffer, Tok.getLiteralData(), Tok.getLength());

    unsigned int size = 4;  // size of int
    //return Owned(IntegerLiteral::Create(Context, :e
    // (see ActOnIntegerConstant)
    return ExprResult(new NumberExpr(Tok.getLocation(), atoi(buffer)));
}


C2::ExprResult C2Sema::ActOnStringLiteral(const Token* StringToks, unsigned int NumStringToks) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: string literal"ANSI_NORMAL"\n";
#endif
    // TEMP just add all the strings together
    std::string result;
    for (unsigned int i=0; i<NumStringToks; i++) {
        // Strip off double-quotes here
        std::string text(StringToks[0].getLiteralData()+1, StringToks[0].getLength()-2);
        result += text;
    }

    return ExprResult(new StringExpr(StringToks[0].getLocation(), result));
}

C2::ExprResult C2Sema::ActOnCharacterConstant(SourceLocation Loc, const std::string& value) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: char constant at ";
    Loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    // TODO parse value (\0, octal, \0x, etc) (see clang::CharLiteralParser)
    const char* str = value.c_str();
    assert(str[0] == '\'' && str[2] == '\'' && "char constant parsing not supported yet");
    unsigned cvalue = str[1];
    return ExprResult(new CharLiteralExpr(Loc, cvalue));
}

void C2Sema::printAST(const std::string& filename) const {
    StringBuilder buffer;
    buffer << "---- AST " << "(pkg=" << pkgName << ") " << filename << " ----\n";
    for (DeclListConstIter iter = decls.begin(); iter != decls.end(); ++iter) {
        (*iter)->print(buffer);
        buffer << '\n';
    }
    printf("%s", (const char*)buffer);
}

DiagnosticBuilder C2Sema::Diag(SourceLocation Loc, unsigned DiagID) {
    return Diags.Report(Loc, DiagID);
}

void C2Sema::addDecl(Decl* d) {
    decls.push_back(d);

    // UseDecl's dont define a symbol
    if (Decl::isSymbol(d->dtype())) {
        Decl* Old = getSymbol(d->getName());
        if (Old) {
            Diag(d->getLocation(), diag::err_redefinition)
            << d->getName();
            Diag(Old->getLocation(), diag::note_previous_definition);
        } else {
            symbols[d->getName()] = d;
        }
    }
}

const C2::Decl* C2Sema::findUse(const char* name) const {
    for (unsigned int i=0; i<decls.size(); i++) {
        Decl* d = decls[i];
        if (d->dtype() != DECL_USE) break;
        if (d->getName() == name) return d;
    }
    return 0;
}

const C2::UseDecl* C2Sema::findAlias(const char* name) const {
    for (unsigned int i=0; i<decls.size(); i++) {
        Decl* d = decls[i];
        if (d->dtype() != DECL_USE) break;
        UseDecl* useDecl = DeclCaster<UseDecl>::getType(d);
        assert(useDecl);
        if (useDecl->getAlias() == name) return useDecl;
    }
    return 0;
}

C2::Decl* C2Sema::getSymbol(const std::string& name) const {
    SymbolsConstIter iter = symbols.find(name);
    if (iter == symbols.end()) return 0;
    else return iter->second;
}

