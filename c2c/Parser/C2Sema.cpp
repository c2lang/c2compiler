/* Copyright 2013,2014 Bas van den Berg
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

#include <llvm/ADT/APFloat.h>
#include <clang/Parse/ParseDiagnostic.h>
#include <clang/Sema/SemaDiagnostic.h>
#include <clang/Lex/LiteralSupport.h>

#include "Parser/C2Sema.h"
#include "AST/Decl.h"
#include "AST/Expr.h"
#include "AST/AST.h"
#include "Utils/StringBuilder.h"
#include "Utils/color.h"

//#define SEMA_DEBUG

using namespace C2;
using namespace clang;
using llvm::APFloat;

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


C2Sema::C2Sema(SourceManager& sm_, DiagnosticsEngine& Diags_, TypeContext& tc, AST& ast_, clang::Preprocessor& PP_)
    : SourceMgr(sm_)
    , Diags(Diags_)
    , typeContext(tc)
    , ast(ast_)
    , PP(PP_)
{}

C2Sema::~C2Sema() {}

void C2Sema::ActOnModule(const char* name, SourceLocation loc) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: module " << name << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    if (name[0] == '_' && name[1] == '_') {
        Diag(loc, diag::err_invalid_symbol_name) << name;
        return;
    }
    if (strcmp(name, "c2") == 0) {
        Diag(loc, diag::err_module_c2);
        return;
    }
    ast.setName(name, loc);
    ImportDecl* U = new ImportDecl(name, loc, true, name, SourceLocation());
    U->setType(typeContext.getModuleType(U));
    U->setUsed();
    ast.addImport(U);
    addSymbol(U);
}

void C2Sema::ActOnImport(const char* moduleName_, SourceLocation loc, Token& aliasTok, bool isLocal) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: import " << moduleName_ << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    std::string moduleName = moduleName_;
    // check if importing own module
    if (ast.getModuleName() == moduleName) {
        Diag(loc, diag::err_import_own_module) << moduleName_;
        return;
    }
    // check for duplicate import of module
    const ImportDecl* old = findModule(moduleName);
    if (old) {
        Diag(loc, diag::err_duplicate_import) << moduleName;
        Diag(old->getLocation(), diag::note_previous_import);
        return;
    }
    std::string name = moduleName;
    if (aliasTok.is(tok::identifier)) {
        name = aliasTok.getIdentifierInfo()->getNameStart();
        // check if same as normal module name
        if (name == moduleName) {
            Diag(aliasTok.getLocation(), diag::err_alias_same_as_module);
            return;
        }
    }
    ImportDecl* U = new ImportDecl(name, loc, isLocal, moduleName, aliasTok.getLocation());
    U->setType(typeContext.getModuleType(U));
    ast.addImport(U);
    addSymbol(U);
}

void C2Sema::ActOnAliasType(const char* name, SourceLocation loc, Expr* type, bool is_public) {
    assert(name);
    assert(type);
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: alias type def " << name << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    TypeExpr* typeExpr = cast<TypeExpr>(type);
    if (typeExpr->hasLocalQualifier()) {
        Diag(loc, diag::err_invalid_local_typedef);
    }
    AliasTypeDecl* T = new AliasTypeDecl(name, loc, typeExpr->getType(), is_public, ast.getFileID());
    QualType A = typeContext.getAliasType(T, typeExpr->getType());
    T->setType(A);
    ast.addType(T);
    addSymbol(T);
    delete typeExpr;
}

void C2Sema::ActOnVarDef(const char* name, SourceLocation loc,
                        bool is_public, Expr* type, Expr* InitValue) {
    TypeExpr* typeExpr = cast<TypeExpr>(type);
    if (typeExpr->hasLocalQualifier()) {
        Diag(loc, diag::err_invalid_local_globalvar);
    }
    VarDecl* V =  createVarDecl(VARDECL_GLOBAL, name, loc, typeExpr, InitValue, is_public);
    ast.addVar(V);
    addSymbol(V);
}

C2::FunctionDecl* C2Sema::createFuncDecl(const char* name, SourceLocation loc,
            bool is_public, Expr* rtype) {
    assert(rtype);
    TypeExpr* typeExpr = cast<TypeExpr>(rtype);
    if (typeExpr->hasLocalQualifier()) {
        // TODO let Parser check this (need extra arg for ParseSingleTypeSpecifier())
        // TODO need local's location
        Diag(loc, diag::err_invalid_local_returntype);
    }
    FunctionDecl* D = new FunctionDecl(name, loc, is_public, ast.getFileID(), typeExpr->getType());
    delete typeExpr;
    QualType qt =  typeContext.getFunctionType(D);
    D->setType(qt);
    return D;
}

// NOTE: takes Type* from typeExpr and deletes typeExpr;
C2::VarDecl* C2Sema::createVarDecl(VarDeclKind k, const char* name, SourceLocation loc, TypeExpr* typeExpr, Expr* InitValue, bool is_public) {
    // TODO check that type is not pre-fixed with own module
    // globals, function params, struct members
    VarDecl* V = new VarDecl(k, name, loc, typeExpr->getType(), InitValue, is_public, ast.getFileID());
    delete typeExpr;
    return V;
}

C2::FunctionDecl* C2Sema::ActOnFuncDecl(const char* name, SourceLocation loc, bool is_public, Expr* rtype) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: func decl " << name << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    FunctionDecl* D = createFuncDecl(name, loc, is_public, rtype);
    ast.addFunction(D);
    addSymbol(D);
    return D;
}

C2::FunctionDecl* C2Sema::ActOnFuncTypeDecl(const char* name, SourceLocation loc,
            bool is_public, Expr* rtype) {
#ifdef SEMA_DEBUG
    assert(name);
    std::cerr << COL_SEMA"SEMA: function type decl " << name << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    FunctionDecl* D = createFuncDecl(name, loc, is_public, rtype);
    FunctionTypeDecl* FTD = new FunctionTypeDecl(D, ast.getFileID());
    ast.addType(FTD);
    addSymbol(FTD);
    return D;
}

void C2Sema::ActOnFunctionArg(FunctionDecl* func, const char* name, SourceLocation loc, Expr* type, Expr* InitValue) {
#ifdef SEMA_DEBUG
    assert(name);
    std::cerr << COL_SEMA"SEMA: function arg" << name << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    // first create VarDecl
    TypeExpr* typeExpr = cast<TypeExpr>(type);
    if (typeExpr->hasLocalQualifier()) {
        Diag(loc, diag::err_invalid_local_functionargument);
    }
    VarDecl* var = createVarDecl(VARDECL_PARAM, name, loc, typeExpr, InitValue, func->isPublic());

    // check args for duplicates
    if (var->getName() != "") {
        VarDecl* existing = func->findArg(var->getName());
        if (existing) {
            Diag(var->getLocation(), diag::err_param_redefinition) << var->getName();
            Diag(existing->getLocation(), diag::note_previous_declaration);
        }
    }
    func->addArg(var);
    // check if already have default args
    if (var->getInitValue()) {
        func->setDefaultArgs();
    } else {
        if (func->hasDefaultArgs()) {
            if (var->getName() == "") {
                Diag(var->getLocation(), diag::err_param_default_argument_missing);
            } else {
                Diag(var->getLocation(), diag::err_param_default_argument_missing_name)
                    << var->getName();
            }
        }
    }
}

void C2Sema::ActOnFinishFunctionBody(Decl* decl, Stmt* body) {
    FunctionDecl* func = cast<FunctionDecl>(decl);
    CompoundStmt* C = cast<CompoundStmt>(body);
    func->setBody(C);
}

void C2Sema::ActOnArrayValue(const char* name, SourceLocation loc, Expr* Value) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: arrayvalue at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    ast.addArrayValue(new ArrayValueDecl(name, loc, Value));
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
    if (name[0] == '_' && name[1] == '_') {
        Diag(loc, diag::err_invalid_symbol_name) << name;
        delete type;
        delete InitValue;
        return StmtResult(true);
    }
    // TEMP extract here to Type and delete rtype Expr
    TypeExpr* typeExpr = cast<TypeExpr>(type);
    bool hasLocal = typeExpr->hasLocalQualifier();
    VarDecl* V =  createVarDecl(VARDECL_LOCAL, name, loc, typeExpr, InitValue, false);
    if (hasLocal) V->setLocalQualifier();
    return StmtResult(new DeclExpr(V));
}

C2::ExprResult C2Sema::ActOnCallExpr(Expr* Fn, Expr** args, unsigned numArgs, SourceLocation RParenLoc) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: call to " << "TODO Fn" << " at " << "TODO Fn";
    //expr2loc(Fn).dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    CallExpr* call = new CallExpr(Fn, RParenLoc);
    assert(call);
    for (unsigned i=0; i<numArgs; i++) call->addArg(args[i]);
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

C2::ExprResult C2Sema::ActOnNil(SourceLocation L) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: nil expr at ";
    L.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return ExprResult(new NilExpr(L));
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

// see clang, some GCC extension allows LHS to be null (C2 doesn't?)
C2::ExprResult C2Sema::ActOnConditionalOp(SourceLocation QuestionLoc, SourceLocation ColonLoc,
                             Expr* CondExpr, Expr* LHSExpr, Expr* RHSExpr) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: CondOp at ";
    QuestionLoc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return ExprResult(new ConditionalOperator(QuestionLoc, ColonLoc, CondExpr, LHSExpr, RHSExpr));
}

C2::ExprResult C2Sema::ActOnInitList(SourceLocation left_, SourceLocation right_, ExprList& vals) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: initlist at ";
    left_.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return ExprResult(new InitListExpr(left_, right_, vals));
}

C2::ExprResult C2Sema::ActOnArrayDesignatorExpr(SourceLocation left, ExprResult Designator, ExprResult InitValue) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: ArrayDesignatorExpr at ";
    left_.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return ExprResult(new DesignatedInitExpr(left, Designator.release(), InitValue.release()));
}

C2::ExprResult C2Sema::ActOnFieldDesignatorExpr(SourceLocation loc, IdentifierInfo* field, ExprResult InitValue) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: FieldDesignatorExpr at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return ExprResult(new DesignatedInitExpr(loc, field->getNameStart(), InitValue.release()));
}

C2::ExprResult C2Sema::ActOnArrayType(Expr* base, Expr* size) {
#ifdef SEMA_DEBUGG
    std::cerr << COL_SEMA"SEMA: Array Type"ANSI_NORMAL"\n";
#endif
    assert(base);
    TypeExpr* typeExpr = cast<TypeExpr>(base);
    QualType QT = typeContext.getArrayType(typeExpr->getType(), size, true);
    typeExpr->setType(QT);
    return ExprResult(base);
}

C2::ExprResult C2Sema::ActOnPointerType(Expr* base) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: Pointer Type"ANSI_NORMAL"\n";
#endif
    assert(base);
    TypeExpr* typeExpr = cast<TypeExpr>(base);
    QualType qt = typeContext.getPointerType(typeExpr->getType());
    typeExpr->setType(qt);
    return ExprResult(base);
}

C2::ExprResult C2Sema::ActOnUserType(IdentifierInfo* psym, SourceLocation ploc,
                                     IdentifierInfo* tsym, SourceLocation tloc) {
    assert(tsym);
    std::string tname(tsym->getNameStart(), tsym->getLength());
    std::string pname;
    if (psym) {
        pname = psym->getNameStart();
    }
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: User Type ";
    if (psym) {
        std::cerr << pname << '.';
    }
    std::cerr << tname << " at ";
    tloc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL << '\n';
#endif
    QualType qt = typeContext.getUnresolvedType(ploc, pname, tloc, tname);
    return ExprResult(new TypeExpr(qt));
}

C2::ExprResult C2Sema::ActOnBuiltinType(tok::TokenKind k) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: Builtin Type"ANSI_NORMAL"\n";
#endif
    QualType qt;
    switch (k) {
    case tok::kw_uint8:     qt = Type::UInt8(); break;
    case tok::kw_uint16:    qt = Type::UInt16(); break;
    case tok::kw_uint32:    qt = Type::UInt32(); break;
    case tok::kw_uint64:    qt = Type::UInt64(); break;
    case tok::kw_int8:      qt = Type::Int8(); break;
    case tok::kw_int16:     qt = Type::Int16(); break;
    case tok::kw_int32:     qt = Type::Int32(); break;
    case tok::kw_int64:     qt = Type::Int64(); break;
    case tok::kw_int:       qt = Type::Int32(); break;
    case tok::kw_uint:      qt = Type::UInt32(); break;
    case tok::kw_string: assert(0); break;
    case tok::kw_float: qt = Type::Float32(); break;
    case tok::kw_float32:   qt = Type::Float32(); break;
    case tok::kw_float64:   qt = Type::Float64(); break;
    case tok::kw_char:      qt = Type::Int8(); break;
    case tok::kw_bool:      qt = Type::Bool(); break;
    case tok::kw_void:      qt = Type::Void(); break;
    case tok::kw_uchar:     qt = Type::UInt8(); break;
    default:
        assert(0);
        break;
    }

    return ExprResult(new TypeExpr(qt));
}

StructTypeDecl* C2Sema::ActOnStructType(const char* name, SourceLocation loc,
            bool isStruct, bool is_public, bool is_global) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: Struct/Union Type '" << (name ? name : "<anonymous>");
    std::cerr << ANSI_NORMAL << '\n';
#endif
    QualType qt = typeContext.getStructType();
    StructTypeDecl* S = new StructTypeDecl(name, loc, qt, isStruct, is_global, is_public, ast.getFileID());
    StructType* ST = cast<StructType>(qt.getTypePtr());
    ST->setDecl(S);
    if (is_global) {
        ast.addType(S);
        addSymbol(S);
    }
    return S;
}

void C2Sema::ActOnStructVar(StructTypeDecl* S, const char* name, SourceLocation loc, Expr* type, Expr* InitValue, bool is_public) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: struct var " << name << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    TypeExpr* typeExpr = cast<TypeExpr>(type);
    if (typeExpr->hasLocalQualifier()) {
        Diag(loc, diag::err_invalid_local_structmember) << (S->isStruct() ? 0 : 1);
    }
    VarDecl* V = createVarDecl(VARDECL_MEMBER, name, loc, typeExpr, InitValue, is_public);
    ActOnStructMember(S, V);
}

void C2Sema::ActOnStructMember(StructTypeDecl* S, Decl* member) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: struct member" << ANSI_NORMAL << '\n';
#endif
    S->addMember(member);
}

void C2Sema::ActOnStructTypeFinish(StructTypeDecl* S, SourceLocation left, SourceLocation right) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: Struct finish"ANSI_NORMAL"\n";
#endif
    //S->setLocs(left, right);
    Names names;
    analyseStructNames(S, names);
}

EnumTypeDecl* C2Sema::ActOnEnumType(const char* name, SourceLocation loc, Expr* implType, bool is_public) {
    assert(implType);
    TypeExpr* T = cast<TypeExpr>(implType);
    QualType impl = T->getType();
    assert(impl->hasCanonicalType());
    delete T;

    QualType qt = typeContext.getEnumType();
    EnumTypeDecl* E = new EnumTypeDecl(name, loc, impl, qt, is_public, ast.getFileID());
    EnumType* ET = cast<EnumType>(qt.getTypePtr());
    ET->setCanonicalType(impl);
    ET->setDecl(E);
    ast.addType(E);
    addSymbol(E);
    return E;
}

void C2Sema::ActOnEnumConstant(EnumTypeDecl* Enum, IdentifierInfo* symII,
                                SourceLocation symLoc, Expr* Value) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: enum constant"ANSI_NORMAL"\n";
#endif
    EnumConstantDecl* D = new EnumConstantDecl(symII->getNameStart(), symLoc, Enum->getType(), Value,
                                               Enum->isPublic(), ast.getFileID());
    Enum->addConstant(D);
    addSymbol(D);
}

C2::ExprResult C2Sema::ActOnEnumTypeFinished(Expr* enumType,
                            SourceLocation leftBrace, SourceLocation rightBrace) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: enum Type"ANSI_NORMAL"\n";
#endif
    // TODO use left/rightBrace (add to TypeExpr, then pass to TypeDecl)
    return ExprResult(enumType);
}

C2::ExprResult C2Sema::ActOnTypeQualifier(ExprResult R, unsigned qualifier) {
    assert(R.get());
    if (qualifier) {
#ifdef SEMA_DEBUG
        std::cerr << COL_SEMA"SEMA: Qualifier Type"ANSI_NORMAL"\n";
#endif
        TypeExpr* typeExpr = cast<TypeExpr>(R.get());
        // TODO use typeExpr.addConst() and just return QualType (not ref) in getType()
        QualType qt = typeExpr->getType();
        if (qualifier & TYPE_CONST) qt.addConst();
        if (qualifier & TYPE_VOLATILE) qt.addVolatile();
        if (qualifier & TYPE_LOCAL) typeExpr->setLocalQualifier();
        typeExpr->setType(qt);
    }
    return R;
}

C2::ExprResult C2Sema::ActOnBuiltinExpression(SourceLocation Loc, Expr* expr, bool isSizeof) {
    assert(expr);
#ifdef SEMA_DEBUG
    const char* fname = (isSizeof ? "sizeof" : "elemsof");
    std::cerr << COL_SEMA"SEMA: " << fname << " at ";
    Loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    return ExprResult(new BuiltinExpr(Loc, expr, isSizeof));
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

C2::ExprResult C2Sema::ActOnMemberExpr(Expr* Base, bool isArrow, IdentifierInfo* sym, SourceLocation loc) {
    assert(Base);
    assert(sym);
    std::string member(sym->getNameStart(), sym->getLength());
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: member access " << member;
    std::cerr << ANSI_NORMAL"\n";
#endif
    return ExprResult(new MemberExpr(Base, isArrow, member, loc));
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

C2::ExprResult C2Sema::ActOnIntegerConstant(SourceLocation Loc, uint64_t Val) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: integer constant"ANSI_NORMAL"\n";
#endif
    // NOTE: always 64 bits?
    llvm::APInt ResultValue(64, Val, true);
    return ExprResult(new IntegerLiteral(Loc, ResultValue));
}

C2::ExprResult C2Sema::ActOnBooleanConstant(const Token& Tok) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: boolean constant"ANSI_NORMAL"\n";
#endif
    return ExprResult(new BooleanLiteral(Tok.getLocation(), Tok.is(tok::kw_true)));
}

C2::ExprResult C2Sema::ActOnNumericConstant(const Token& Tok) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: numeric constant"ANSI_NORMAL"\n";
#endif
    // Fast path for a single digit (which is quite common).  A single digit
    // cannot have a trigraph, escaped newline, radix prefix, or suffix.
    if (Tok.getLength() == 1) {
        const char Val = PP.getSpellingOfSingleCharacterNumericConstant(Tok);
        return ActOnIntegerConstant(Tok.getLocation(), Val-'0');
    }
  SmallString<128> SpellingBuffer;
  // NumericLiteralParser wants to overread by one character.  Add padding to
  // the buffer in case the token is copied to the buffer.  If getSpelling()
  // returns a StringRef to the memory buffer, it should have a null char at
  // the EOF, so it is also safe.
  SpellingBuffer.resize(Tok.getLength() + 1);

  // Get the spelling of the token, which eliminates trigraphs, etc.
  bool Invalid = false;
  StringRef TokSpelling = PP.getSpelling(Tok, SpellingBuffer, &Invalid);
  if (Invalid)
    return ExprError();

  NumericLiteralParser Literal(TokSpelling, Tok.getLocation(), PP);
  if (Literal.hadError)
    return ExprError();

    if (Literal.hasUDSuffix()) {
        assert(0 && "HUH?");
    }

    Expr* Res;

    if (Literal.isFloatingLiteral()) {
        // clang::Sema::BuildFloatingLiteral()
        // TEMP Hardcoded
        const llvm::fltSemantics& Format = llvm::APFloat::IEEEsingle;
        APFloat Val(Format);

        APFloat::opStatus result = Literal.GetFloatValue(Val);
      // Overflow is always an error, but underflow is only an error if
      // we underflowed to zero (APFloat reports denormals as underflow).
      if ((result & APFloat::opOverflow) ||
          ((result & APFloat::opUnderflow) && Val.isZero())) {
        assert(0 && "TODO");
#if 0
        unsigned diagnostic;
        SmallString<20> buffer;
        if (result & APFloat::opOverflow) {
          diagnostic = diag::warn_float_overflow;
          APFloat::getLargest(Format).toString(buffer);
        } else {
          diagnostic = diag::warn_float_underflow;
          APFloat::getSmallest(Format).toString(buffer);
        }

        Diag(Tok.getLocation(), diagnostic)
          << Ty
          << StringRef(buffer.data(), buffer.size());
#endif
      }

      //bool isExact = (result == APFloat::opOK);
      //return FloatingLiteral::Create(S.Context, Val, isExact, Ty, Loc);
        Res = new FloatingLiteral(Tok.getLocation(), Val);

    } else if (!Literal.isIntegerLiteral()) {
        return ExprError();
    } else {
        QualType ty;

        const unsigned MaxWidth = 64; // for now limit to 64 bits
        llvm::APInt ResultVal(MaxWidth, 0);
        if (Literal.GetIntegerValue(ResultVal)) {
            Diag(Tok.getLocation(), diag::err_integer_too_large);
        } else {
#if 0
            // Octal, Hexadecimal, and integers with a U suffix are allowed to
            // be an unsigned.
            bool AllowUnsigned = Literal.isUnsigned || Literal.getRadix() != 10;

            // Check from smallest to largest, picking the smallest type we can.
            unsigned Width = 0;
          if (!Literal.isLong && !Literal.isLongLong) {
            // Are int/unsigned possibilities?
            unsigned IntSize = 64;

            // Does it fit in a unsigned?
            if (ResultVal.isIntN(IntSize)) {
              // Does it fit in a signed int?
#if 0
              if (!Literal.isUnsigned && ResultVal[IntSize-1] == 0)
                Ty = Context.IntTy;
              else if (AllowUnsigned)
                Ty = Context.UnsignedIntTy;
#endif
              Width = IntSize;
            }
          }

          // Check long long if needed.
          if (Width == 0) {
              if (ResultVal.isIntN(64)) {
#if 0
                  if (!Literal.isUnsigned && (ResultVal[LongLongSize-1] == 0 ||
                      (getLangOpts().MicrosoftExt && Literal.isLongLong)))
                    Ty = Context.LongLongTy;
                  else if (AllowUnsigned)
                    Ty = Context.UnsignedLongLongTy;
#endif
                  Width = 64;
              }
          }

            if (Width == 0) {
                fprintf(stderr, "TOO LARGE\n");
                assert(0 && "TODO");
            }
            // set correct width
            if (ResultVal.getBitWidth() != Width) {
                ResultVal = ResultVal.trunc(Width);
            }
#endif
        }

        Res = new IntegerLiteral(Tok.getLocation(), ResultVal);
    }
    return ExprResult(Res);
}


C2::ExprResult C2Sema::ActOnStringLiteral(const Token* StringToks, unsigned NumStringToks) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: string literal"ANSI_NORMAL"\n";
#endif
    StringLiteralParser Literal(StringToks, NumStringToks, PP);
    if (Literal.hadError) return ExprError();

    llvm::StringRef ref = Literal.GetString();
    return ExprResult(new StringLiteral(StringToks[0].getLocation(), ref.data()));
}

C2::ExprResult C2Sema::ActOnCharacterConstant(const Token& Tok) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: char constant at ";
    Loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    SmallString<16> CharBuffer;
    bool Invalid = false;
    StringRef ThisTok = PP.getSpelling(Tok, CharBuffer, &Invalid);
    if (Invalid) return ExprError();

    CharLiteralParser Literal(ThisTok.begin(), ThisTok.end(), Tok.getLocation(),
                              PP, Tok.getKind());
    if (Literal.hadError())
        return ExprError();

    return ExprResult(new CharacterLiteral(Tok.getLocation(), Literal.getValue()));
}

DiagnosticBuilder C2Sema::Diag(SourceLocation Loc, unsigned DiagID) {
    return Diags.Report(Loc, DiagID);
}

void C2Sema::addSymbol(Decl* d) {
    Decl* Old = ast.findSymbol(d->getName());
    if (Old) {
        Diag(d->getLocation(), diag::err_redefinition)
        << d->getName();
        Diag(Old->getLocation(), diag::note_previous_definition);
    } else {
        ast.addSymbol(d);
    }
}

const C2::ImportDecl* C2Sema::findModule(const std::string& name) const {
    for (unsigned i=0; i<ast.numImports(); i++) {
        ImportDecl* D = ast.getImport(i);
        if (D->getModuleName() == name) return D;
    }
    return 0;
}

C2::ExprResult C2Sema::ExprError() {
    return C2::ExprResult(true);
}

void C2Sema::analyseStructNames(const StructTypeDecl* S, Names& names) {
    typedef Names::iterator NamesIter;
    for (unsigned i=0; i<S->numMembers(); i++) {
        const Decl* member = S->getMember(i);
        const std::string& name = member->getName();
        if (name == "") {
            assert(isa<StructTypeDecl>(member));
            analyseStructNames(cast<StructTypeDecl>(member), names);
        } else {
            NamesIter iter = names.find(name);
            if (iter != names.end()) {
                const Decl* existing = iter->second;
                Diag(member->getLocation(), diag::err_duplicate_member) << name;
                Diag(existing->getLocation(), diag::note_previous_declaration);
            } else {
                names[name] = member;
            }
        }
    }
}

