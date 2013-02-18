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

#ifndef FUNCTION_BODY_ANALYSER_H
#define FUNCTION_BODY_ANALYSER_H

#include "ASTVisitor.h"
#include "Package.h"

namespace clang {
class DiagnosticsEngine;
}

namespace C2 {

class Type;
class GlobalScope;
class Scope;
class Decl;
class Stmt;
class Expr;
class IdentifierExpr;

class FunctionBodyAnalyser : public ASTVisitor {
public:
    FunctionBodyAnalyser(GlobalScope& scope_, clang::DiagnosticsEngine& Diags_);
    virtual ~FunctionBodyAnalyser();

    virtual bool handle(Decl* decl);
    unsigned int getErrors() const { return errors; }
private:
    void EnterScope();
    void ExitScope();
    void analyseCompound(Stmt* stmt);
    void analyseReturnStmt(Stmt* stmt);
    void analyseStmtExpr(Stmt* stmt);
    void analyseExpr(Expr* expr);
    void analyseDeclExpr(Expr* expr);

    GlobalScope& globalScope;
    Scope* curScope;
    //const Pkgs& pkgs;
    clang::DiagnosticsEngine& Diags;
    unsigned int errors;

    FunctionBodyAnalyser(const FunctionBodyAnalyser&);
    FunctionBodyAnalyser& operator= (const FunctionBodyAnalyser&);
};

}

#endif

