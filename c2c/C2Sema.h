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

#ifndef C2SEMA_H
#define C2SEMA_H

#include <string>
#include <map>
#include <vector>

#include <clang/Basic/SourceLocation.h>
#include "C2Parser.h" // TODO only needed for ExprResult/StmtResult
#include "Stmt.h"
#include "Expr.h"

namespace clang {
class SourceManager;
class Token;
class DiagnosticsEngine;
}

using clang::SourceLocation;
using clang::SourceManager;

namespace C2 {

class CodeGenerator;
class FunctionDecl;
class Decl;
class Stmt;
class Expr;
class Type;
class ASTVisitor;

enum C2Type {
    TYPE_U8 = 0,
    TYPE_U16,
    TYPE_U32,
    TYPE_S8,
    TYPE_S16,
    TYPE_S32,
    TYPE_INT,
    TYPE_STRING,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_VOID,
};

class C2Sema {
public:
    C2Sema(SourceManager& sm_, DiagnosticsEngine& Diags_);
    ~C2Sema();

    // file level actions
    void ActOnPackage(const char* name, SourceLocation loc);
    void ActOnUse(const char* name, SourceLocation loc);
    void ActOnTypeDef(const char* name, SourceLocation loc, Expr* typeExpr, bool is_public);
    void ActOnVarDef(const char* name, SourceLocation loc, bool is_public, Expr* type, Expr* InitValue);
    FunctionDecl* ActOnFuncDef(const char* name, SourceLocation loc, bool is_public, Expr* rtype);
    void ActOnFinishFunctionBody(Decl* func, Stmt* body);

    // statements
    StmtResult ActOnReturnStmt(SourceLocation loc, Expr* value);
    StmtResult ActOnIfStmt(SourceLocation ifLoc,
                           ExprResult condition, StmtResult thenStmt,
                           SourceLocation elseLoc, StmtResult elseStmt);
    StmtResult ActOnCompoundStmt(SourceLocation L, SourceLocation R, StmtList& stmts);
    StmtResult ActOnDeclaration(const char* name, SourceLocation loc, Expr* type, Expr* InitValue);

    // expressions
    ExprResult ActOnNumericConstant(const Token& Tok);
    ExprResult ActOnStringLiteral(const clang::Token* StringToks, unsigned int NumStringToks);
    ExprResult ActOnCallExpr(Expr* id, Expr** args, unsigned num, SourceLocation RParenLoc);
    ExprResult ActOnIdExpression(IdentifierInfo* pkgII, SourceLocation pkgLoc,
                                 IdentifierInfo& symII, SourceLocation symLoc);
    ExprResult ActOnInitList(SourceLocation left_, SourceLocation right_, ExprList& vals);
    ExprResult ActOnArrayType(Expr* base, Expr* size);
    ExprResult ActOnPointerType(Expr* base);
    ExprResult ActOnUserType(Expr* id);
    ExprResult ActOnBuiltinType(C2Type t);
    ExprResult ActOnStructType(SourceLocation leftBrace, SourceLocation rightBrace,
                               ExprList& members, bool isStruct);
    ExprResult ActOnTypeQualifier(ExprResult R, unsigned int qualifier);
    ExprResult ActOnVarExpr(const char* name, SourceLocation loc, Expr* type, Expr* InitValue);

    // analysis
    void visitAST(ASTVisitor& visitor);

    // debugging
    void printAST() const;
    void generateC() const;

    const std::string& getPkgName() const { return pkgName; }
private:
    C2::Type* getBuiltinType(C2Type t) const;
    DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID);
    void addDecl(Decl* d);
    const Decl* findUse(const char* name) const;
    Decl* getSymbol(const std::string& name) const;

    SourceManager& SourceMgr;
    DiagnosticsEngine& Diags;

    // TEMP
    friend class CodeGenerator;

    std::string pkgName;
    SourceLocation pkgLoc;

    typedef std::vector<Decl*> DeclList;
    typedef DeclList::const_iterator DeclListConstIter;
    typedef DeclList::iterator DeclListIter;
    DeclList decls;

    // TODO move to some Package class
    // This map is just for lookups, no ownership. UseDecls are not added here
    typedef std::map<std::string, Decl*> Symbols;
    typedef Symbols::const_iterator SymbolsConstIter;
    Symbols symbols;

    C2Sema(const C2Sema&);
    C2Sema& operator= (const C2Sema&);
};

}

#endif

