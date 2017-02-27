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

#ifndef CODEGEN_INTERFACE_GENERATOR_H
#define CODEGEN_INTERFACE_GENERATOR_H

#include <string>

#include "Utils/StringBuilder.h"
#include "Utils/StringList.h"

namespace C2 {

class AST;
class Decl;
class VarDecl;
class TypeDecl;
class FunctionDecl;
class ImportDecl;
class StructTypeDecl;
class FunctionTypeDecl;
class EnumTypeDecl;
class QualType;
class Expr;
class IdentifierExpr;
class Module;

class InterfaceGenerator {
public:
    InterfaceGenerator(const Module& module_);
    ~InterfaceGenerator() {}

    void write(const std::string& outputDir, bool printCode);

private:
    void EmitImport(const ImportDecl* D, StringList& importList);
    void EmitTypeDecl(const TypeDecl* D);
    void EmitVarDecl(const VarDecl* D, unsigned indent);
    void EmitFunctionDecl(const FunctionDecl* D);

    void EmitFunctionArgs(const FunctionDecl* F);
    void EmitArgVarDecl(const VarDecl* D, unsigned index);

    void EmitType(QualType type);
    void EmitAliasType(const TypeDecl* T);
    void EmitStructType(const StructTypeDecl* S, unsigned indent);
    void EmitEnumType(const EnumTypeDecl* E);
    void EmitFunctionType(const FunctionTypeDecl* F);
    void EmitPrefixedDecl(const Decl* D);

    void EmitExpr(const Expr* E);
    void EmitBinaryOperator(const Expr* E);
    void EmitConditionalOperator(const Expr* E);
    void EmitUnaryOperator(const Expr* E);
    void EmitMemberExpr(const Expr* E);
    void EmitIdentifierExpr(const IdentifierExpr* E);

    // Helpers
    void EmitStringLiteral(const std::string& input);
    void EmitAttributes(const Decl* D);

    const Module& module;
    StringBuilder iface;
    const AST* currentAST;

    InterfaceGenerator(const InterfaceGenerator&);
    InterfaceGenerator& operator= (const InterfaceGenerator&);
};

}

#endif

