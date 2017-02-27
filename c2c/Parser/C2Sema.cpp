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

#include <string.h>
#include <ctype.h>

#include <llvm/ADT/APFloat.h>
#include <clang/AST/Expr.h>
#include <clang/Parse/ParseDiagnostic.h>
#include <clang/Sema/SemaDiagnostic.h>
#include <clang/Lex/LiteralSupport.h>

#include "Parser/C2Sema.h"
#include "AST/AST.h"
#include "AST/Attr.h"
#include "AST/Component.h"

//#define SEMA_DEBUG
//#define SEMA_MEMSIZE

#ifdef SEMA_DEBUG
#include <iostream>
#include "Utils/color.h"
#endif

using namespace C2;
using namespace clang;
using llvm::APFloat;

#ifdef SEMA_MEMSIZE
static int counter;

static uint64_t declCounters[DECL_LABEL+1];
static uint32_t declSizes[DECL_LABEL+1];
static const char* declNames[DECL_LABEL+1];

static uint64_t stmtCounters[STMT_DECL+1];
static uint32_t stmtSizes[STMT_DECL+1];
static const char* stmtNames[STMT_DECL+1];

static uint64_t exprCounters[EXPR_CAST+1];
static uint32_t exprSizes[EXPR_CAST+1];
static const char* exprNames[EXPR_CAST+1];

// TODO attributes
// TODO types

#define MEM_ADD(x)
#define MEM_DECL(x) do { declCounters[x]++; } while(0)
#define MEM_STMT(x) do { stmtCounters[x]++; } while(0)
#define MEM_EXPR(x) do { exprCounters[x]++; } while(0)
#else
#define MEM_ADD(x)
#define MEM_DECL(x)
#define MEM_STMT(x)
#define MEM_EXPR(x)
#endif

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


C2Sema::C2Sema(SourceManager& sm_, DiagnosticsEngine& Diags_, clang::Preprocessor& PP_,
               Component& component_, Module* existingMod, const std::string& filename_)
    : SourceMgr(sm_)
    , Diags(Diags_)
    , PP(PP_)
    , component(component_)
    , module(existingMod)
    , ast(*new AST(filename_, component.isExternal()))
    , Context(ast.getASTContext())
{
#ifdef SEMA_MEMSIZE
    memset(declCounters, 0, sizeof(declCounters));
    memset(stmtCounters, 0, sizeof(stmtCounters));
    memset(exprCounters, 0, sizeof(exprCounters));
    if (counter == 0) {
#define DECL_INIT(x, y) \
    do { \
        declSizes[x] = sizeof(y); \
        declNames[x] = #y; \
    } while(0);
        DECL_INIT(DECL_FUNC, FunctionDecl);
        DECL_INIT(DECL_VAR, VarDecl);
        DECL_INIT(DECL_ENUMVALUE, EnumConstantDecl);
        DECL_INIT(DECL_ALIASTYPE, AliasTypeDecl);
        DECL_INIT(DECL_STRUCTTYPE, StructTypeDecl);
        DECL_INIT(DECL_ENUMTYPE, EnumTypeDecl);
        DECL_INIT(DECL_FUNCTIONTYPE, FunctionTypeDecl);
        DECL_INIT(DECL_ARRAYVALUE, ArrayValueDecl);
        DECL_INIT(DECL_IMPORT, ImportDecl);
        DECL_INIT(DECL_LABEL, LabelDecl);
#define STMT_INIT(x, y) \
    do { \
        stmtSizes[x] = sizeof(y); \
        stmtNames[x] = #y; \
    } while(0);
        STMT_INIT(STMT_RETURN, ReturnStmt);
        STMT_INIT(STMT_EXPR, Expr);
        STMT_INIT(STMT_IF, IfStmt);
        STMT_INIT(STMT_WHILE, WhileStmt);
        STMT_INIT(STMT_DO, DoStmt);
        STMT_INIT(STMT_FOR, ForStmt);
        STMT_INIT(STMT_SWITCH, SwitchStmt);
        STMT_INIT(STMT_CASE, CaseStmt);
        STMT_INIT(STMT_DEFAULT, DefaultStmt);
        STMT_INIT(STMT_BREAK, BreakStmt);
        STMT_INIT(STMT_CONTINUE, ContinueStmt);
        STMT_INIT(STMT_LABEL, LabelStmt);
        STMT_INIT(STMT_GOTO, GotoStmt);
        STMT_INIT(STMT_COMPOUND, CompoundStmt);
        STMT_INIT(STMT_DECL, DeclStmt);
#define EXPR_INIT(x, y) \
    do { \
        exprSizes[x] = sizeof(y); \
        exprNames[x] = #y; \
    } while(0);
        EXPR_INIT(EXPR_INTEGER_LITERAL, IntegerLiteral);
        EXPR_INIT(EXPR_FLOAT_LITERAL, FloatingLiteral);
        EXPR_INIT(EXPR_BOOL_LITERAL, BooleanLiteral);
        EXPR_INIT(EXPR_CHAR_LITERAL, CharacterLiteral);
        EXPR_INIT(EXPR_STRING_LITERAL, StringLiteral);
        EXPR_INIT(EXPR_NIL, NilExpr);
        EXPR_INIT(EXPR_IDENTIFIER, IdentifierExpr);
        EXPR_INIT(EXPR_TYPE, TypeExpr);
        EXPR_INIT(EXPR_CALL, CallExpr);
        EXPR_INIT(EXPR_INITLIST, InitListExpr);
        EXPR_INIT(EXPR_DESIGNATOR_INIT, DesignatedInitExpr);
        EXPR_INIT(EXPR_BINOP, BinaryOperator);
        EXPR_INIT(EXPR_CONDOP, ConditionalOperator);
        EXPR_INIT(EXPR_UNARYOP, UnaryOperator);
        EXPR_INIT(EXPR_BUILTIN, BuiltinExpr);
        EXPR_INIT(EXPR_ARRAYSUBSCRIPT, ArraySubscriptExpr);
        EXPR_INIT(EXPR_MEMBER, MemberExpr);
        EXPR_INIT(EXPR_PAREN, ParenExpr);
        EXPR_INIT(EXPR_BITOFFSET, BitOffsetExpr);
        EXPR_INIT(EXPR_CAST, CastExpr);
    }
    counter++;
#endif
}

C2Sema::~C2Sema() {
#if 0
    printf("%u  BuiltinType\n", (int)sizeof(BuiltinType));
    printf("%u  PointerType\n", (int)sizeof(PointerType));
    printf("%u  ArrayType\n", (int)sizeof(ArrayType));
    printf("%u  UnresolvedType\n", (int)sizeof(UnresolvedType));
    printf("%u  AliasType\n", (int)sizeof(AliasType));
    printf("%u  StructType\n", (int)sizeof(StructType));
    printf("%u  EnumType\n", (int)sizeof(EnumType));
    printf("%u  FunctionType\n", (int)sizeof(FunctionType));
    printf("%u  ModuleType\n", (int)sizeof(ModuleType));
#endif
#ifdef SEMA_MEMSIZE
    uint64_t total_elems = 0;
    uint64_t total_size = 0;
    for (unsigned i=0; i<(sizeof(declCounters)/sizeof(declCounters[0])); i++) {
        total_elems += declCounters[i];
        total_size += declCounters[i] * declSizes[i];
    }
    for (unsigned i=0; i<(sizeof(stmtCounters)/sizeof(stmtCounters[0])); i++) {
        total_elems += stmtCounters[i];
        total_size += stmtCounters[i] * stmtSizes[i];
    }
    for (unsigned i=0; i<(sizeof(exprCounters)/sizeof(exprCounters[0])); i++) {
        total_elems += exprCounters[i];
        total_size += exprCounters[i] * exprSizes[i];
    }

#define PRINT_STATS \
    if (count != 0) { \
        int percCount = (count * 100) / total_elems; \
        uint64_t size = count * elemsize; \
        int percSize = (size * 100) / total_size; \
        printf("   %6" PRIu64"(%2d%%)   %8" PRIu64"(%2d%%)   %3d   %s\n", count, percCount, size, percSize, elemsize, name); \
    }

    printf("AST COUNT            SIZE       ELEM   NAME\n");
    printf("   %6" PRIu64"        %8" PRIu64"\n", total_elems, total_size);
    for (unsigned i=0; i<(sizeof(declCounters)/sizeof(declCounters[0])); i++) {
        uint64_t count = declCounters[i];
        uint32_t elemsize = declSizes[i];
        const char* name = declNames[i];
        PRINT_STATS;
    }
    for (unsigned i=0; i<(sizeof(stmtCounters)/sizeof(stmtCounters[0])); i++) {
        uint64_t count = stmtCounters[i];
        uint32_t elemsize = stmtSizes[i];
        const char* name = stmtNames[i];
        PRINT_STATS;
    }
    for (unsigned i=0; i<(sizeof(exprCounters)/sizeof(exprCounters[0])); i++) {
        uint64_t count = exprCounters[i];
        uint32_t elemsize = exprSizes[i];
        const char* name = exprNames[i];
        PRINT_STATS;
    }
    Context.dump();
#endif
}

void C2Sema::printAST() const {
    ast.print(true);
    module->printAttributes(true);
}

void C2Sema::ActOnModule(const char* name_, SourceLocation loc) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA << "SEMA: module " << name_ << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    if (name_[0] == '_' && name_[1] == '_') {
        Diag(loc, diag::err_invalid_symbol_name) << name_;
        return;
    }
    if (!islower(name_[0]) && !ast.isInterface()) {
        Diag(loc, diag::err_module_casing);
        return;
    }

    if (strcmp(name_, "c2") == 0) {
        Diag(loc, diag::err_module_c2);
        return;
    }
    if (strcmp(name_, "main") == 0) {
        Diag(loc, diag::err_module_invalid_name) << name_;
        return;
    }

    // First create Module, then AST, the get Context (from Module?)
    const char* name = Context.addIdentifier(name_, strlen(name_));
    ast.setName(name, loc);

    if (module) {
        // for external modules, filename should match module name
        if (module->getName() != name) {
            Diag(loc, diag::err_file_wrong_module) << module->getName() << name;
        }
    } else {
        module = component.getModule(name);
    }
    module->addAST(&ast);

    MEM_DECL(DECL_IMPORT);
    ImportDecl* U = new (Context) ImportDecl(name, loc, true, name, SourceLocation());
    U->setType(Context.getModuleType(U));
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
    const char* moduleName = Context.addIdentifier(moduleName_, strlen(moduleName_));
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
    const char* name = moduleName;
    clang::SourceLocation realLoc = loc;
    if (aliasTok.is(tok::identifier)) {
        IdentifierInfo* aliasSym = aliasTok.getIdentifierInfo();
        name = Context.addIdentifier(aliasSym->getNameStart(), aliasSym->getLength());
        // check if same as normal module name
        if (strcmp(name, moduleName) == 0) {
            Diag(aliasTok.getLocation(), diag::err_alias_same_as_module);
            return;
        }
        if (!islower(name[0]) && !ast.isInterface()) {
            Diag(aliasTok.getLocation(), diag::err_module_casing);
            return;
        }
        realLoc = aliasTok.getLocation();
    }

    MEM_DECL(DECL_IMPORT);
    ImportDecl* U = new (Context) ImportDecl(name, realLoc, isLocal, moduleName, aliasTok.getLocation());
    U->setType(Context.getModuleType(U));
    ast.addImport(U);
    addSymbol(U);
}

C2::Decl* C2Sema::ActOnAliasType(const char* name_, SourceLocation loc, Expr* type, bool is_public) {
    assert(name_);
    assert(type);
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: alias type def " << name_ << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    if (ast.isInterface()) {
        if (is_public) Diag(loc, diag::err_public_in_interface);
        is_public = true;
    }
    TypeExpr* typeExpr = cast<TypeExpr>(type);
    if (typeExpr->hasLocalQualifier()) {
        Diag(loc, diag::err_invalid_local_typedef);
    }
    const char* name = Context.addIdentifier(name_, strlen(name_));
    MEM_DECL(DECL_ALIASTYPE);
    AliasTypeDecl* T = new (Context) AliasTypeDecl(name, loc, typeExpr->getType(), is_public);
    QualType A = Context.getAliasType(T, typeExpr->getType());
    T->setType(A);
    ast.addType(T);
    addSymbol(T);
    Context.freeTypeExpr(typeExpr);
    return T;
}

C2::VarDecl* C2Sema::ActOnVarDef(const char* name, SourceLocation loc, bool is_public, Expr* type) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: VarDef " << name << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    TypeExpr* typeExpr = cast<TypeExpr>(type);
    if (typeExpr->hasLocalQualifier()) {
        Diag(loc, diag::err_invalid_local_globalvar);
    }
    if (ast.isInterface()) {
        if (is_public) Diag(loc, diag::err_public_in_interface);
        is_public = true;
    }
    VarDecl* V = createVarDecl(VARDECL_GLOBAL, name, loc, typeExpr, 0, is_public);
    ast.addVar(V);
    addSymbol(V);
    return V;
}

C2::FunctionDecl* C2Sema::createFuncDecl(const char* name_, SourceLocation loc,
        bool is_public, Expr* rtype) {
    assert(rtype);
    TypeExpr* typeExpr = cast<TypeExpr>(rtype);
    if (typeExpr->hasLocalQualifier()) {
        // TODO let Parser check this (need extra arg for ParseSingleTypeSpecifier())
        // TODO need local's location
        Diag(loc, diag::err_invalid_local_returntype);
    }
    MEM_DECL(DECL_FUNC);
    const char* name = Context.addIdentifier(name_, strlen(name_));
    FunctionDecl* D = new (Context) FunctionDecl(name, loc, is_public, typeExpr->getType());
    Context.freeTypeExpr(typeExpr);
    QualType qt =  Context.getFunctionType(D);
    D->setType(qt);
    return D;
}

// NOTE: takes Type* from typeExpr and deletes typeExpr;
C2::VarDecl* C2Sema::createVarDecl(VarDeclKind k, const char* name_, SourceLocation loc, TypeExpr* typeExpr, Expr* InitValue, bool is_public) {
    // TODO check that type is not pre-fixed with own module
    // globals, function params, struct members

    MEM_DECL(DECL_VAR);
    const char* name = Context.addIdentifier(name_, strlen(name_));
    VarDecl* V = new (Context) VarDecl(k, name, loc, typeExpr->getType(), InitValue, is_public);
    Context.freeTypeExpr(typeExpr);
    return V;
}

C2::FunctionDecl* C2Sema::ActOnFuncDecl(const char* name_, SourceLocation loc, bool is_public, Expr* rtype) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: func decl " << name_ << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    if (ast.isInterface()) {
        if (is_public) Diag(loc, diag::err_public_in_interface);
        is_public = true;
    }
    const char* name = Context.addIdentifier(name_, strlen(name_));
    FunctionDecl* D = createFuncDecl(name, loc, is_public, rtype);
    if (strcmp(D->getName(), "main") == 0) D->setExported();
    ast.addFunction(D);
    addSymbol(D);
    return D;
}

C2::FunctionTypeDecl* C2Sema::ActOnFuncTypeDecl(const char* name_, SourceLocation loc,
        bool is_public, Expr* rtype) {
#ifdef SEMA_DEBUG
    assert(name_);
    std::cerr << COL_SEMA"SEMA: function type decl " << name_ << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    if (ast.isInterface()) {
        if (is_public) Diag(loc, diag::err_public_in_interface);
        is_public = true;
    }
    FunctionDecl* D = createFuncDecl(name_, loc, is_public, rtype);
    MEM_DECL(DECL_FUNCTIONTYPE);
    FunctionTypeDecl* FTD = new (Context) FunctionTypeDecl(D);
    ast.addType(FTD);
    addSymbol(FTD);
    return FTD;
}

C2::VarDeclResult C2Sema::ActOnFunctionArg(FunctionDecl* func, const char* name, SourceLocation loc, Expr* type, Expr* InitValue) {
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

    // check if already have default args
    if (var->getInitValue()) {
        func->setDefaultArgs();
    } else {
        if (func->hasDefaultArgs()) {
            if (var->hasEmptyName()) {
                Diag(var->getLocation(), diag::err_param_default_argument_missing);
            } else {
                Diag(var->getLocation(), diag::err_param_default_argument_missing_name)
                        << var->getName();
            }
        }
    }
    return VarDeclResult(var);
}

void C2Sema::ActOnFinishFunctionArgs(FunctionDecl* func, VarDeclList& args_) {
    unsigned numArgs = args_.size();
    VarDecl** args = (VarDecl**)Context.Allocate(sizeof(VarDecl*)*numArgs);
    memcpy(args, &args_[0], sizeof(VarDecl*)*numArgs);
    func->setArgs(args, numArgs);
}

void C2Sema::ActOnFinishFunctionBody(FunctionDecl* func, Stmt* body) {
    CompoundStmt* C = cast<CompoundStmt>(body);
    func->setBody(C);
}

void C2Sema::ActOnArrayValue(const char* name_, SourceLocation loc, Expr* Value) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: arrayvalue at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_DECL(DECL_ARRAYVALUE);
    const char* name = Context.addIdentifier(name_, strlen(name_));
    ArrayValueDecl* decl = new (Context) ArrayValueDecl(name, loc, Value);
    decl->setModule(module);
    ast.addArrayValue(decl);
}

C2::StmtResult C2Sema::ActOnReturnStmt(SourceLocation loc, Expr* value) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: return at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_STMT(STMT_RETURN);
    return StmtResult(new (Context) ReturnStmt(loc, value));
}

C2::StmtResult C2Sema::ActOnIfStmt(SourceLocation ifLoc,
                                   Stmt* condition, StmtResult thenStmt,
                                   SourceLocation elseLoc, StmtResult elseStmt) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: if statement at ";
    ifLoc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_STMT(STMT_IF);
    return StmtResult(new (Context) IfStmt(ifLoc, condition, thenStmt.get(), elseLoc, elseStmt.get()));
}

C2::StmtResult C2Sema::ActOnWhileStmt(SourceLocation loc, Stmt* Cond, StmtResult Then) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: while statement at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_STMT(STMT_WHILE);
    return StmtResult(new (Context) WhileStmt(loc, Cond, Then.get()));
}

C2::StmtResult C2Sema::ActOnDoStmt(SourceLocation loc, ExprResult Cond, StmtResult Then) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: do statement at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_STMT(STMT_DO);
    return StmtResult(new (Context) DoStmt(loc, Cond.get(), Then.get()));
}

C2::StmtResult C2Sema::ActOnForStmt(SourceLocation loc, Stmt* Init, Expr* Cond, Expr* Incr, Stmt* Body) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: for statement at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_STMT(STMT_FOR);
    return StmtResult(new (Context) ForStmt(loc, Init, Cond, Incr, Body));
}

C2::StmtResult C2Sema::ActOnSwitchStmt(SourceLocation loc, Stmt* Cond, StmtList& cases_) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: switch statement at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_STMT(STMT_SWITCH);
    unsigned numCases = cases_.size();
    Stmt** cases = (Stmt**)Context.Allocate(sizeof(Stmt*)*numCases);
    memcpy(cases, &cases_[0], sizeof(Stmt*)*numCases);
    return StmtResult(new (Context) SwitchStmt(loc, Cond, cases, numCases));
}

C2::StmtResult C2Sema::ActOnCaseStmt(SourceLocation loc, Expr* Cond, StmtList& stmts_) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: case statement at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_STMT(STMT_CASE);
    unsigned numStmts = stmts_.size();
    Stmt** stmts = (Stmt**)Context.Allocate(sizeof(Stmt*)*numStmts);
    memcpy(stmts, &stmts_[0], sizeof(Stmt*)*numStmts);
    return StmtResult(new (Context) CaseStmt(loc, Cond, stmts, numStmts));
}

C2::StmtResult C2Sema::ActOnDefaultStmt(SourceLocation loc, StmtList& stmts_) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: default statement at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_STMT(STMT_DEFAULT);
    unsigned numStmts = stmts_.size();
    Stmt** stmts = (Stmt**)Context.Allocate(sizeof(Stmt*)*numStmts);
    memcpy(stmts, &stmts_[0], sizeof(Stmt*)*numStmts);
    return StmtResult(new (Context) DefaultStmt(loc, stmts, numStmts));
}

C2::StmtResult C2Sema::ActOnBreakStmt(SourceLocation loc) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: break statement at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_STMT(STMT_BREAK);
    return StmtResult(new (Context) BreakStmt(loc));
}

C2::StmtResult C2Sema::ActOnContinueStmt(SourceLocation loc) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: continue statement at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_STMT(STMT_CONTINUE);
    return StmtResult(new (Context) ContinueStmt(loc));
}

C2::StmtResult C2Sema::ActOnLabelStmt(const char* name_, SourceLocation loc, Stmt* subStmt) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: label statement at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_STMT(STMT_LABEL);
    const char* name = Context.addIdentifier(name_, strlen(name_));
    return StmtResult(new (Context) LabelStmt(name, loc, subStmt));
}

C2::StmtResult C2Sema::ActOnGotoStmt(const char* name_, SourceLocation GotoLoc, SourceLocation LabelLoc) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: goto statement at ";
    GotoLoc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_STMT(STMT_GOTO);
    const char* name = Context.addIdentifier(name_, strlen(name_));
    return StmtResult(new (Context) GotoStmt(name, GotoLoc, LabelLoc));
}

C2::StmtResult C2Sema::ActOnCompoundStmt(SourceLocation L, SourceLocation R, StmtList& stmts_) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: compound statement at ";
    L.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_STMT(STMT_COMPOUND);
    unsigned numStmts = stmts_.size();
    Stmt** stmts = (Stmt**)Context.Allocate(sizeof(Stmt*)*numStmts);
    memcpy(stmts, &stmts_[0], sizeof(Stmt*)*numStmts);
    return StmtResult(new (Context) CompoundStmt(L, R, stmts, numStmts));
}

C2::StmtResult C2Sema::ActOnDeclaration(const char* name_, SourceLocation loc, Expr* type, Expr* InitValue) {
    assert(type);
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: decl at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    if (name_[0] == '_' && name_[1] == '_') {
        Diag(loc, diag::err_invalid_symbol_name) << name_;
        return StmtResult(true);
    }
    // TEMP extract here to Type and delete rtype Expr
    TypeExpr* typeExpr = cast<TypeExpr>(type);
    bool hasLocal = typeExpr->hasLocalQualifier();
    VarDecl* V = createVarDecl(VARDECL_LOCAL, name_, loc, typeExpr, InitValue, false);
    if (hasLocal) V->setLocalQualifier();
    MEM_STMT(STMT_DECL);
    return StmtResult(new (Context) DeclStmt(V));
}

C2::ExprResult C2Sema::ActOnCallExpr(Expr* Fn, Expr** args_, unsigned numArgs, SourceLocation RParenLoc) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: call to " << "TODO Fn" << " at " << "TODO Fn";
    //expr2loc(Fn).dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_EXPR(EXPR_CALL);
    Expr** args = (Expr**)Context.Allocate(sizeof(Expr*)*numArgs);
    memcpy(args, args_, sizeof(Expr*)*numArgs);
    CallExpr* call = new (Context) CallExpr(Fn, RParenLoc, args, numArgs);
    return ExprResult(call);
}

C2::ExprResult C2Sema::ActOnIdExpression(IdentifierInfo& symII, SourceLocation symLoc) {
    const char* name = Context.addIdentifier(symII.getNameStart(), symII.getLength());
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: identifier " << name << " at ";
    symLoc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_EXPR(EXPR_IDENTIFIER);
    return ExprResult(new (Context) IdentifierExpr(symLoc, name));
}

C2::ExprResult C2Sema::ActOnParenExpr(SourceLocation L, SourceLocation R, Expr* E) {
    assert(E && "missing expr");
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: paren expr at ";
    L.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_EXPR(EXPR_PAREN);
    return ExprResult(new (Context) ParenExpr(L, R, E));
}

C2::ExprResult C2Sema::ActOnNil(SourceLocation L) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: nil expr at ";
    L.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_EXPR(EXPR_NIL);
    return ExprResult(new (Context) NilExpr(L));
}


C2::ExprResult C2Sema::ActOnBinOp(SourceLocation opLoc, tok::TokenKind Kind, Expr* LHS, Expr* RHS) {
    assert(LHS);
    assert(RHS);
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: BinOp " << tok::getPunctuatorSpelling(Kind) << " at ";
    opLoc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    clang::BinaryOperatorKind Opc = ConvertTokenKindToBinaryOpcode(Kind);

    // Emit warnings for tricky precedence issues, e.g. "bitfield & 0x4 == 0"
    //DiagnoseBinOpPrecedence(*this, Opc, TokLoc, LHSExpr, RHSExpr);

    MEM_EXPR(EXPR_BINOP);
    return ExprResult(new (Context) BinaryOperator(LHS, RHS, Opc, opLoc));
}

// see clang, some GCC extension allows LHS to be null (C2 doesn't?)
C2::ExprResult C2Sema::ActOnConditionalOp(SourceLocation QuestionLoc, SourceLocation ColonLoc,
        Expr* CondExpr, Expr* LHSExpr, Expr* RHSExpr) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: CondOp at ";
    QuestionLoc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_EXPR(EXPR_CONDOP);
    return ExprResult(new (Context) ConditionalOperator(QuestionLoc, ColonLoc, CondExpr, LHSExpr, RHSExpr));
}

C2::ExprResult C2Sema::ActOnInitList(SourceLocation left_, SourceLocation right_, ExprList& vals) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: initlist at ";
    left_.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    unsigned numValues = vals.size();
    // TODO allow size = 0?
    Expr** values = (Expr**)Context.Allocate(sizeof(Expr*)*numValues);
    memcpy(values, &vals[0], sizeof(Expr*)*numValues);
    MEM_EXPR(EXPR_INITLIST);
    return ExprResult(new (Context) InitListExpr(left_, right_, values, numValues));
}

C2::ExprResult C2Sema::ActOnArrayDesignatorExpr(SourceLocation left, ExprResult Designator, ExprResult InitValue) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: ArrayDesignatorExpr at ";
    left.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_EXPR(EXPR_DESIGNATOR_INIT);
    return ExprResult(new (Context) DesignatedInitExpr(left, Designator.get(), InitValue.get()));
}

C2::ExprResult C2Sema::ActOnFieldDesignatorExpr(SourceLocation loc, IdentifierInfo* field, ExprResult InitValue) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: FieldDesignatorExpr at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_EXPR(EXPR_DESIGNATOR_INIT);
    const char* name = Context.addIdentifier(field->getNameStart(), field->getLength());
    return ExprResult(new (Context) DesignatedInitExpr(loc, name, InitValue.get()));
}

C2::ExprResult C2Sema::ActOnArrayType(Expr* base, Expr* size, bool isIncremental) {
#ifdef SEMA_DEBUGG
    std::cerr << COL_SEMA << "SEMA: Array Type" << ANSI_NORMAL"\n";
#endif
    assert(base);
    TypeExpr* typeExpr = cast<TypeExpr>(base);
    QualType QT = Context.getArrayType(typeExpr->getType(), size, isIncremental);
    typeExpr->setType(QT);
    return ExprResult(base);
}

C2::ExprResult C2Sema::ActOnPointerType(Expr* base, unsigned qualifier) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA << "SEMA: Pointer Type" << ANSI_NORMAL"\n";
#endif
    assert(base);
    TypeExpr* typeExpr = cast<TypeExpr>(base);
    QualType qt = typeExpr->getType();
    if (qualifier) {
        if (qualifier & TYPE_CONST) qt.addConst();
        if (qualifier & TYPE_VOLATILE) qt.addVolatile();
        // HMM local keyword is lost? -> TODO propagate to outer type
        if (qualifier & TYPE_LOCAL) typeExpr->setLocalQualifier();
    }
    typeExpr->setType(Context.getPointerType(qt));
    return ExprResult(base);
}

C2::ExprResult C2Sema::ActOnUserType(Expr* mName_, Expr* tName_) {
    assert(isa<IdentifierExpr>(tName_));
    IdentifierExpr* tName = cast<IdentifierExpr>(tName_);
    IdentifierExpr* mName = 0;
    if (mName_) {
        assert(isa<IdentifierExpr>(mName_));
        mName = cast<IdentifierExpr>(mName_);
    }
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA << "SEMA: User Type ";
    if (mName) {
        std::cerr << mName->getName() << '.';
    }
    std::cerr << tName->getName() << " at ";
    tName->getLocation().dump(SourceMgr);
    std::cerr << ANSI_NORMAL << '\n';
#endif
    QualType qt = Context.getUnresolvedType(mName, tName);
    TypeExpr* te = new (Context.allocTypeExpr())TypeExpr(qt);
    return ExprResult(te);
}

C2::ExprResult C2Sema::ActOnBuiltinType(tok::TokenKind k) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA << "SEMA: Builtin Type" << ANSI_NORMAL"\n";
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
    case tok::kw_float: qt = Type::Float32(); break;
    case tok::kw_float32:   qt = Type::Float32(); break;
    case tok::kw_float64:   qt = Type::Float64(); break;
    case tok::kw_char:      qt = Type::UInt8(); break;
    case tok::kw_bool:      qt = Type::Bool(); break;
    case tok::kw_void:      qt = Type::Void(); break;
    default:
        assert(0);
        break;
    }

    TypeExpr* te = new (Context.allocTypeExpr())TypeExpr(qt);
    return ExprResult(te);
}

StructTypeDecl* C2Sema::ActOnStructType(const char* name_, SourceLocation loc,
                                        bool isStruct, bool is_public, bool is_global) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA << "SEMA: Struct/Union Type '" << (name_ ? name_ : "<anonymous>");
    std::cerr << ANSI_NORMAL << '\n';
#endif
    if (ast.isInterface()) {
        if (is_public) Diag(loc, diag::err_public_in_interface);
        is_public = true;
    }
    QualType qt = Context.getStructType();
    MEM_DECL(DECL_STRUCTTYPE);
    const char* name = Context.addIdentifier(name_, strlen(name_));
    StructTypeDecl* S = new (Context) StructTypeDecl(name, loc, qt, isStruct, is_global, is_public);
    StructType* ST = cast<StructType>(qt.getTypePtr());
    ST->setDecl(S);
    if (is_global) {
        ast.addType(S);
        addSymbol(S);
    }
    return S;
}

C2::Decl* C2Sema::ActOnStructVar(StructTypeDecl* S, const char* name_, SourceLocation loc, Expr* type, Expr* InitValue) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA << "SEMA: struct var " << name_ << " at ";
    loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    TypeExpr* typeExpr = cast<TypeExpr>(type);
    if (typeExpr->hasLocalQualifier()) {
        Diag(loc, diag::err_invalid_local_structmember) << (S->isStruct() ? 0 : 1);
    }
    return createVarDecl(VARDECL_MEMBER, name_, loc, typeExpr, InitValue, S->isPublic());
}

void C2Sema::ActOnStructMembers(StructTypeDecl* S, DeclList& members_) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA << "SEMA: struct member" << ANSI_NORMAL << '\n';
#endif
    unsigned numMembers = members_.size();
    Decl** members = (Decl**)Context.Allocate(sizeof(Decl*)*numMembers);
    memcpy(members, &members_[0], sizeof(Decl*)*numMembers);
    S->setMembers(members, numMembers);
}

EnumTypeDecl* C2Sema::ActOnEnumType(const char* name_, SourceLocation loc, Expr* implType, bool is_public, bool is_incr) {
    assert(implType);
    TypeExpr* T = cast<TypeExpr>(implType);
    QualType impl = T->getType();
    assert(impl->hasCanonicalType());
    Context.freeTypeExpr(T);

    if (ast.isInterface()) {
        if (is_public) Diag(loc, diag::err_public_in_interface);
        is_public = true;
    }

    QualType qt = Context.getEnumType();
    MEM_DECL(DECL_ENUMTYPE);
    const char* name = Context.addIdentifier(name_, strlen(name_));
    EnumTypeDecl* E = new (Context) EnumTypeDecl(name, loc, impl, qt, is_incr, is_public);
    EnumType* ET = cast<EnumType>(qt.getTypePtr());
    ET->setCanonicalType(impl);
    ET->setDecl(E);
    ast.addType(E);
    addSymbol(E);
    return E;
}

C2::EnumConstantDecl* C2Sema::ActOnEnumConstant(EnumTypeDecl* Enum, IdentifierInfo* symII,
        SourceLocation symLoc, Expr* Value) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA << "SEMA: enum constant" << ANSI_NORMAL"\n";
#endif
    MEM_DECL(DECL_ENUMVALUE);

    const char* name = Context.addIdentifier(symII->getNameStart(), symII->getLength());
    EnumConstantDecl* D = new (Context) EnumConstantDecl(name, symLoc, Enum->getType(), Value,
            Enum->isPublic());
    addSymbol(D);

    if (!isupper(name[0]) && !ast.isInterface()) {
        Diag(symLoc, diag::err_enumconst_casing);
    }
    return D;
}

void C2Sema::ActOnEnumTypeFinished(EnumTypeDecl* Enum, EnumConstantDecl** constants_, unsigned numConstants) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA << "SEMA: enum finished" << ANSI_NORMAL"\n";
#endif
    EnumConstantDecl** constants = (EnumConstantDecl**)Context.Allocate(sizeof(EnumConstantDecl*)*numConstants);
    memcpy(constants, constants_, sizeof(EnumConstantDecl*)*numConstants);
    Enum->setConstants(constants, numConstants);
}

C2::ExprResult C2Sema::ActOnTypeQualifier(ExprResult R, unsigned qualifier) {
    assert(R.get());
    if (qualifier) {
#ifdef SEMA_DEBUG
        std::cerr << COL_SEMA << "SEMA: Qualifier Type" << ANSI_NORMAL"\n";
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

C2::ExprResult C2Sema::ActOnBuiltinExpression(SourceLocation Loc, Expr* expr, BuiltinExpr::BuiltinKind kind_) {
    assert(expr);
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA << "SEMA: " << BuiltinExpr::Str(kind_) << " at ";
    Loc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_EXPR(EXPR_BUILTIN);
    return ExprResult(new (Context) BuiltinExpr(Loc, expr, kind_));
}

C2::ExprResult C2Sema::ActOnArraySubScriptExpr(SourceLocation RLoc, Expr* Base, Expr* Idx) {
    assert(Base);
    assert(Idx);
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA << "SEMA: array subscript at ";
    RLoc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_EXPR(EXPR_ARRAYSUBSCRIPT);
    return ExprResult(new (Context) ArraySubscriptExpr(RLoc, Base, Idx));
}

C2::ExprResult C2Sema::ActOnMemberExpr(Expr* Base, Expr* member) {
    assert(Base);
    assert(member);
    IdentifierExpr* I = dyncast<IdentifierExpr>(member);
    assert(I);
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA << "SEMA: member access" << I->getName();
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_EXPR(EXPR_MEMBER);
    return ExprResult(new (Context) MemberExpr(Base, I));
}

C2::ExprResult C2Sema::ActOnPostfixUnaryOp(SourceLocation OpLoc, tok::TokenKind Kind, Expr* Input) {
    assert(Input);
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA << "SEMA: postop at ";
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
    MEM_EXPR(EXPR_UNARYOP);
    return ExprResult(new (Context) UnaryOperator(OpLoc, Opc, Input));
}

C2::ExprResult C2Sema::ActOnUnaryOp(SourceLocation OpLoc, tok::TokenKind Kind, Expr* Input) {
    assert(Input);
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA << "SEMA: unary op at ";
    OpLoc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    UnaryOperatorKind Opc = ConvertTokenKindToUnaryOpcode(Kind);
    MEM_EXPR(EXPR_UNARYOP);
    return ExprResult(new (Context) UnaryOperator(OpLoc, Opc, Input));
}

C2::ExprResult C2Sema::ActOnBitOffset(SourceLocation colLoc, Expr* LHS, Expr* RHS) {
    assert(LHS);
    assert(RHS);
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: bitoffset at ";
    colLoc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    MEM_EXPR(EXPR_BITOFFSET);
    return ExprResult(new (Context) BitOffsetExpr(LHS, RHS, colLoc));
}

C2::ExprResult C2Sema::ActOnExplicitCast(SourceLocation castLoc, Expr* type, Expr* expr) {
    assert(type);
    assert(expr);
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA"SEMA: explicit cast at ";
    castLoc.dump(SourceMgr);
    std::cerr << ANSI_NORMAL"\n";
#endif
    TypeExpr* typeExpr = cast<TypeExpr>(type);
    if (typeExpr->hasLocalQualifier()) {
        // TODO correct error message: cannot use keyword local with cast type
        Diag(castLoc, diag::err_invalid_local_typedef);
    }
    MEM_EXPR(EXPR_CAST);
    ExplicitCastExpr* E = new (Context) ExplicitCastExpr(castLoc, typeExpr->getType(), expr);
    Context.freeTypeExpr(typeExpr);
    return ExprResult(E);
}

void C2Sema::ActOnAttr(Decl* D, const char* name, SourceRange range, Expr* arg) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA << "SEMA: attribute " << name << ANSI_NORMAL"\n";
#endif
    AttrKind kind = Attr::name2kind(name);
    if (kind == ATTR_UNKNOWN) {
        Diag(range.getBegin(), diag::err_attribute_unknown) << name << range;
        return;
    }
    const AttrInfo& ai = Attr::getInfo(kind);
    // check if allowed for type of Decl
    if (isa<TypeDecl>(D) && !ai.isAllowedInType()) {
        Diag(range.getBegin(), diag::err_attribute_invalid_decl) << name << 0 << range;
        return;
    }
    if (isa<FunctionDecl>(D) && !ai.isAllowedInFunction()) {
        Diag(range.getBegin(), diag::err_attribute_invalid_decl) << name << 1 << range;
        return;
    }
    if (isa<VarDecl>(D) && !ai.isAllowedInVar()) {
        Diag(range.getBegin(), diag::err_attribute_invalid_decl) << name << 2 << range;
        return;
    }

    // check if it requires an argument or has argument while not needing one
    if (arg) {
        if (!ai.requiresArgument) {
            Diag(range.getBegin(), diag::err_attribute_wrong_number_arguments) << name << 0 << range;
            return;
        }
    } else {
        if (ai.requiresArgument) {
            Diag(range.getBegin(), diag::err_attribute_wrong_number_arguments) << name << 1 << range;
            return;
        }
    }

    // check for duplicates
    if (module->hasAttribute(D, kind)) {
        Diag(range.getBegin(), diag::warn_duplicate_attribute_exact) << name << range;
        return;
    }

    D->setHasAttributes();
    MEM_ADD(Attr);
    module->addAttribute(D, new (Context) Attr(kind, range, arg));

    // Fixup opaque structs; members are not public!
    if (kind == ATTR_OPAQUE && isa<StructTypeDecl>(D)) {
        StructTypeDecl* S = cast<StructTypeDecl>(D);
        S->setOpaqueMembers();
    }
}

C2::ExprResult C2Sema::ActOnIntegerConstant(SourceLocation Loc, uint64_t Val) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA << "SEMA: integer constant" << ANSI_NORMAL"\n";
#endif
    // NOTE: always 64 bits?
    llvm::APInt ResultValue(64, Val, true);
    unsigned radix = 10;
    MEM_EXPR(EXPR_INTEGER_LITERAL);
    return ExprResult(new (Context) IntegerLiteral(Loc, ResultValue, radix));
}

C2::ExprResult C2Sema::ActOnBooleanConstant(const Token& Tok) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA << "SEMA: boolean constant" << ANSI_NORMAL"\n";
#endif
    MEM_EXPR(EXPR_BOOL_LITERAL);
    return ExprResult(new (Context) BooleanLiteral(Tok.getLocation(), Tok.is(tok::kw_true)));
}

C2::ExprResult C2Sema::ActOnNumericConstant(const Token& Tok) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA << "SEMA: numeric constant" << ANSI_NORMAL"\n";
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
        const llvm::fltSemantics& Format = llvm::APFloat::IEEEsingle();
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
        MEM_EXPR(EXPR_FLOAT_LITERAL);
        Res = new (Context) FloatingLiteral(Tok.getLocation(), Val);

    } else if (!Literal.isIntegerLiteral()) {
        return ExprError();
    } else {
        QualType ty;

        const unsigned MaxWidth = 64; // for now limit to 64 bits
        llvm::APInt ResultVal(MaxWidth, 0);
        if (Literal.GetIntegerValue(ResultVal)) {
            Diag(Tok.getLocation(), diag::err_integer_literal_too_large) << 1;
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

        MEM_EXPR(EXPR_INTEGER_LITERAL);
        Res = new (Context) IntegerLiteral(Tok.getLocation(), ResultVal, Literal.getRadix());
    }
    return ExprResult(Res);
}


C2::ExprResult C2Sema::ActOnStringLiteral(ArrayRef<Token> StringToks) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA << "SEMA: string literal" << ANSI_NORMAL"\n";
#endif
    StringLiteralParser Literal(StringToks, PP);
    if (Literal.hadError) return ExprError();

    MEM_EXPR(EXPR_STRING_LITERAL);
    llvm::StringRef ref = Literal.GetString();
    const char* text = Context.addIdentifier(ref.data(), ref.size());
    return ExprResult(new (Context) StringLiteral(StringToks[0].getLocation(), text));
}

C2::ExprResult C2Sema::ActOnCharacterConstant(const Token& Tok) {
#ifdef SEMA_DEBUG
    std::cerr << COL_SEMA << "SEMA: char constant at ";
    Tok.getLocation().dump(SourceMgr);
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

    MEM_EXPR(EXPR_CHAR_LITERAL);
    return ExprResult(new (Context) CharacterLiteral(Tok.getLocation(), Literal.getValue()));
}

DiagnosticBuilder C2Sema::Diag(SourceLocation Loc, unsigned DiagID) {
    return Diags.Report(Loc, DiagID);
}

void C2Sema::addSymbol(Decl* d) {
    Decl* Old = findSymbol(d->getName());
    if (Old) {
        Diag(d->getLocation(), diag::err_redefinition) << d->getName();
        Diag(Old->getLocation(), diag::note_previous_definition);
    } else {
        if (isa<ImportDecl>(d)) {
            imports[d->getName()]  = d;
            d->setModule(module);   // Will be changed if it points external
        } else {
            if (d->isPublic() && module->isExported()) d->setExported();
            module->addSymbol(d);
        }
    }
}

C2::Decl* C2Sema::findSymbol(const char* name) const {
    SymbolsConstIter iter = imports.find(name);
    if (iter != imports.end()) return iter->second;

    return module->findSymbol(name);
}

const C2::ImportDecl* C2Sema::findModule(const char* name_) const {
    for (unsigned i=0; i<ast.numImports(); i++) {
        ImportDecl* D = ast.getImport(i);
        if (strcmp(D->getModuleName(), name_) == 0) return D;
    }
    return 0;
}

C2::ExprResult C2Sema::ExprError() {
    return C2::ExprResult(true);
}

