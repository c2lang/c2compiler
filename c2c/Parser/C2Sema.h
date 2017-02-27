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

#ifndef PARSER_C2SEMA_H
#define PARSER_C2SEMA_H

#include <clang/Basic/SourceLocation.h>
#include <vector>
#include <map>

#include "AST/Decl.h"
#include "AST/Stmt.h"
#include "AST/Expr.h"
#include "AST/Type.h"
#include "Parser/ParserTypes.h"

namespace clang {
class SourceManager;
class Token;
class DiagnosticsEngine;
class Preprocessor;
}

using clang::SourceLocation;
using clang::SourceManager;
using clang::Token;
using clang::DiagnosticsEngine;

// TEMP
using namespace clang;

namespace C2 {

class AST;
class ASTContext;
class Component;
class Module;

typedef std::vector<Decl*> DeclList;
typedef std::vector<VarDecl*> VarDeclList;

class C2Sema {
public:
    C2Sema(SourceManager& sm_, DiagnosticsEngine& Diags_, clang::Preprocessor& PP_,
           Component& comp_, Module* existingMod, const std::string& filename_);
    ~C2Sema();

    void printAST() const;

    // file level actions
    void ActOnModule(const char* name, SourceLocation loc);
    void ActOnImport(const char* name, SourceLocation loc, Token& aliasTok, bool isLocal);
    VarDecl* ActOnVarDef(const char* name, SourceLocation loc, bool is_public, Expr* type);

    // function decls
    FunctionDecl* ActOnFuncDecl(const char* name, SourceLocation loc, bool is_public, Expr* rtype);
    FunctionTypeDecl* ActOnFuncTypeDecl(const char* name, SourceLocation loc, bool is_public, Expr* rtype);
    VarDeclResult ActOnFunctionArg(FunctionDecl* func, const char* name, SourceLocation loc, Expr* type, Expr* InitValue);
    void ActOnFinishFunctionArgs(FunctionDecl* func, VarDeclList& args);
    void ActOnFinishFunctionBody(FunctionDecl* func, Stmt* body);

    void ActOnArrayValue(const char* name, SourceLocation loc, Expr* Value);

    // struct decls
    Decl* ActOnAliasType(const char* name, SourceLocation loc, Expr* typeExpr, bool is_public);
    StructTypeDecl* ActOnStructType(const char* name, SourceLocation loc, bool isStruct, bool is_public, bool is_global);
    Decl* ActOnStructVar(StructTypeDecl* S, const char* name, SourceLocation loc, Expr* type, Expr* InitValue);
    void ActOnStructMembers(StructTypeDecl* S, DeclList& members);

    // statements
    StmtResult ActOnReturnStmt(SourceLocation loc, Expr* value);
    StmtResult ActOnIfStmt(SourceLocation ifLoc,
                           Stmt* condition, StmtResult thenStmt,
                           SourceLocation elseLoc, StmtResult elseStmt);
    StmtResult ActOnWhileStmt(SourceLocation loc, Stmt* condition, StmtResult thenStmt);
    StmtResult ActOnDoStmt(SourceLocation loc, ExprResult condition, StmtResult thenStmt);
    StmtResult ActOnForStmt(SourceLocation loc, Stmt* Init, Expr* Cond, Expr* Incr, Stmt* Body);
    StmtResult ActOnSwitchStmt(SourceLocation loc, Stmt* condition, StmtList& cases);
    StmtResult ActOnCaseStmt(SourceLocation loc, Expr* condition, StmtList& stmts);
    StmtResult ActOnDefaultStmt(SourceLocation loc, StmtList& stmts);
    StmtResult ActOnBreakStmt(SourceLocation loc);
    StmtResult ActOnContinueStmt(SourceLocation loc);
    StmtResult ActOnLabelStmt(const char* name, SourceLocation loc, Stmt* subStmt);
    StmtResult ActOnGotoStmt(const char* name, SourceLocation GotoLoc, SourceLocation LabelLoc);
    StmtResult ActOnCompoundStmt(SourceLocation L, SourceLocation R, StmtList& stmts);
    StmtResult ActOnDeclaration(const char* name, SourceLocation loc, Expr* type, Expr* InitValue);

    // expressions
    ExprResult ActOnBooleanConstant(const Token& Tok);
    ExprResult ActOnNumericConstant(const Token& Tok);
    ExprResult ActOnStringLiteral(ArrayRef<Token> StringToks);
    ExprResult ActOnCharacterConstant(const Token& Tok);
    ExprResult ActOnCallExpr(Expr* id, Expr** args, unsigned num, SourceLocation RParenLoc);
    ExprResult ActOnIdExpression(IdentifierInfo& symII, SourceLocation symLoc);
    ExprResult ActOnParenExpr(SourceLocation L, SourceLocation R, Expr* E);
    ExprResult ActOnNil(SourceLocation L);

    ExprResult ActOnBinOp(SourceLocation opLoc, tok::TokenKind Kind, Expr* LHS, Expr* RHS);
    ExprResult ActOnConditionalOp(SourceLocation QuestionLoc, SourceLocation ColonLoc,
                                 Expr* CondExpr, Expr* LHSExpr, Expr* RHSExpr);

    ExprResult ActOnInitList(SourceLocation left_, SourceLocation right_, ExprList& vals);
    ExprResult ActOnArrayDesignatorExpr(SourceLocation left, ExprResult Designator, ExprResult InitValue);
    ExprResult ActOnFieldDesignatorExpr(SourceLocation loc, IdentifierInfo* field, ExprResult InitValue);
    ExprResult ActOnArrayType(Expr* base, Expr* size, bool isIncremental);
    ExprResult ActOnPointerType(Expr* base, unsigned qualifier);
    ExprResult ActOnUserType(Expr* mName, Expr* tName);
    ExprResult ActOnBuiltinType(tok::TokenKind k);
    EnumTypeDecl* ActOnEnumType(const char* name, SourceLocation loc, Expr* implType, bool is_public, bool is_incr);
    void ActOnEnumTypeFinished(EnumTypeDecl* Enum, EnumConstantDecl** constants, unsigned numConstants);
    EnumConstantDecl* ActOnEnumConstant(EnumTypeDecl* Enum, IdentifierInfo* symII, SourceLocation symLoc, Expr* Value);
    ExprResult ActOnTypeQualifier(ExprResult R, unsigned qualifier);
    ExprResult ActOnBuiltinExpression(SourceLocation Loc, Expr* expr, BuiltinExpr::BuiltinKind kind_);
    ExprResult ActOnArraySubScriptExpr(SourceLocation RLoc, Expr* Base, Expr* Idx);
    ExprResult ActOnMemberExpr(Expr* Base, Expr* member);
    ExprResult ActOnPostfixUnaryOp(SourceLocation OpLoc, tok::TokenKind Kind, Expr* Input);
    ExprResult ActOnUnaryOp(SourceLocation OpLoc, tok::TokenKind Kind, Expr* Input);
    ExprResult ActOnBitOffset(SourceLocation colLoc, Expr* LHS, Expr* RHS);
    ExprResult ActOnExplicitCast(SourceLocation castLoc, Expr* type, Expr* expr);

    // Attributes
    void ActOnAttr(Decl* D, const char* name, SourceRange range, Expr* arg);
private:
    ExprResult ActOnIntegerConstant(SourceLocation Loc, uint64_t Val);
    FunctionDecl* createFuncDecl(const char* name, SourceLocation loc, bool is_public, Expr* rtype);
    VarDecl* createVarDecl(VarDeclKind k, const char* name, SourceLocation loc, TypeExpr* typeExpr,
                                    Expr* InitValue, bool is_public);

    C2::ExprResult ExprError();

    DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID);
    void addSymbol(Decl* d);
    Decl* findSymbol(const char* name) const;
    const ImportDecl* findModule(const char* name_) const;

    SourceManager& SourceMgr;
    DiagnosticsEngine& Diags;
    clang::Preprocessor& PP;

    Component& component;
    Module* module;
    AST& ast;
    ASTContext& Context;

    typedef std::map<std::string, Decl*> Symbols;
    typedef Symbols::const_iterator SymbolsConstIter;
    Symbols imports;

    C2Sema(const C2Sema&);
    C2Sema& operator= (const C2Sema&);
};

}

#endif

