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

#ifndef FILE_ANALYSER_H
#define FILE_ANALYSER_H

#include "Type.h"
#include "Package.h"

namespace clang {
class DiagnosticsEngine;
}

namespace C2 {

class IdentifierExpr;
class FileScope;
class TypeDecl;
class VarDecl;
class FunctionDecl;
class ArrayValueDecl;
class AST;

class FileAnalyser {
public:
    FileAnalyser(const Pkgs& pkgs, clang::DiagnosticsEngine& Diags_,
                 AST& ast_, TypeContext& typeContext_, bool verbose);
    ~FileAnalyser();

    // call in this order
    unsigned checkUses();
    unsigned resolveTypes();
    unsigned resolveTypeCanonicals();
    unsigned resolveStructMembers();
    unsigned resolveVars();
    unsigned checkVarInits();
    unsigned resolveEnumConstants();
    unsigned checkFunctionProtos();
    unsigned checkFunctionBodies();
private:
    unsigned checkTypeDecl(TypeDecl* D);
    unsigned checkStructTypeDecl(StructTypeDecl* D);
    unsigned checkType(QualType Q, bool used_public);
    QualType resolveCanonical(QualType Q, bool set);
    unsigned resolveVarDecl(VarDecl* D);
    unsigned resolveFunctionDecl(FunctionDecl* D);
    unsigned checkFunctionBody(FunctionDecl* D);
    unsigned checkArrayValue(ArrayValueDecl* D);

    unsigned checkInitValue(Expr* initVal, QualType expected);

    // TODO use auto_ptr on globals
    AST& ast;
    TypeContext& typeContext;
    FileScope* globals;
    clang::DiagnosticsEngine& Diags;
    bool verbose;

    FileAnalyser(const FileAnalyser&);
    FileAnalyser& operator= (const FileAnalyser&);
};

}

#endif

