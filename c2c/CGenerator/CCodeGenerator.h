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

#ifndef CGENERATOR_CCODE_GENERATOR_H
#define CGENERATOR_CCODE_GENERATOR_H

#include <string>
#include <vector>

#include "AST/Module.h"
#include "AST/Type.h"
#include "Utils/StringBuilder.h"

namespace C2 {

class AST;
class Type;
class Decl;
class VarDecl;
class ImportDecl;
class TypeDecl;
class FunctionDecl;
class StructTypeDecl;
class FunctionTypeDecl;
class FunctionDecl;
class Expr;
class DeclExpr;
class Stmt;
class CompoundStmt;

// generates LLVM Module from (multiple) ASTs
class CCodeGenerator {
public:
    enum Mode { MULTI_FILE, SINGLE_FILE };
    CCodeGenerator(const std::string& filename_, Mode mode_, const Modules& modules_, bool prefix);
    ~CCodeGenerator();

    void addEntry(AST& ast) { entries.push_back(&ast); }
    void generate();
    void write(const std::string& target, const std::string& name);
    void dump();

private:
    void EmitIncludes();

    void EmitFunction(FunctionDecl* F);
    void EmitFunctionArgs(FunctionDecl* F, StringBuilder& output);
    void EmitVariable(VarDecl* D);
    void EmitTypeDecl(TypeDecl* D);
    void EmitStructType(StructTypeDecl* S, StringBuilder& output, unsigned indent);
    void EmitEnumType(EnumTypeDecl* E, StringBuilder& output);
    void EmitFunctionType(FunctionTypeDecl* F, StringBuilder& output);
    void EmitVarDecl(VarDecl* D, StringBuilder& output, unsigned indent);

    void EmitStmt(Stmt* S, unsigned indent);
    void EmitCompoundStmt(CompoundStmt* C, unsigned indent, bool startOnNewLine);
    void EmitIfStmt(Stmt* S, unsigned indent);
    void EmitWhileStmt(Stmt* S, unsigned indent);
    void EmitDoStmt(Stmt* S, unsigned indent);
    void EmitForStmt(Stmt* S, unsigned indent);
    void EmitSwitchStmt(Stmt* S, unsigned indent);

    void EmitExpr(Expr* E, StringBuilder& output);
    void EmitBinaryOperator(Expr* E, StringBuilder& output);
    void EmitConditionalOperator(Expr* E, StringBuilder& output);
    void EmitUnaryOperator(Expr* E, StringBuilder& output);
    void EmitMemberExpr(Expr* E, StringBuilder& output);
    void EmitDeclExpr(DeclExpr* D, StringBuilder& output, unsigned indent);
    void EmitCallExpr(Expr* E, StringBuilder& output);
    void EmitIdentifierExpr(Expr* E, StringBuilder& output);

    // Helpers
    void EmitDecl(const Decl* D, StringBuilder& output);
    void EmitFunctionProto(FunctionDecl* F, StringBuilder& output);
    void EmitTypePreName(QualType type, StringBuilder& output);
    void EmitTypePostName(QualType type, StringBuilder& output);
    void EmitStringLiteral(const std::string& input, StringBuilder& output);
    void addPrefix(const std::string& modName, const std::string& name, StringBuilder& buffer) const;

    const std::string& filename;
    const std::string* curmod;
    Mode mode;
    bool no_local_prefix;

    const Modules& modules;

    typedef std::vector<AST*> Entries;
    typedef Entries::iterator EntriesIter;
    Entries entries;

    StringBuilder cbuf;
    StringBuilder hbuf;
    std::string cfilename;
    std::string hfilename;

    CCodeGenerator(const CCodeGenerator&);
    CCodeGenerator& operator= (const CCodeGenerator&);
};

}

#endif

