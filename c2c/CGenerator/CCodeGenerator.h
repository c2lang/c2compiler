/* Copyright 2013,2014,2015 Bas van den Berg
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

#include "CGenerator/TypeSorter.h"
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
class CCodeGenerator : public CTypeWriter {
public:
    enum Mode { MULTI_FILE, SINGLE_FILE };
    CCodeGenerator(const std::string& filename_, Mode mode_, const Modules& modules_);
    ~CCodeGenerator();

    void addEntry(AST& ast) { entries.push_back(&ast); }
    void generate();
    void write(const std::string& outputDir, const std::string& name);
    void dump();

    // for CTypeWriter
    virtual void forwardDecl(const Decl* D);
    virtual void fullDecl(const Decl* D);
private:
    void EmitIncludes();

    void EmitFunctionForward(const FunctionDecl* F);
    void EmitFunction(const FunctionDecl* F);
    void EmitFunctionArgs(const FunctionDecl* F, StringBuilder& output);
    void EmitConstant(const VarDecl* D);
    void EmitGlobalVariable(const VarDecl* D);
    void EmitTypeDecl(const TypeDecl* D);
    void EmitForwardTypeDecl(const TypeDecl* D);
    void EmitStructType(const StructTypeDecl* S, StringBuilder& output, unsigned indent);
    void EmitEnumType(const EnumTypeDecl* E, StringBuilder& output);
    void EmitFunctionType(const FunctionTypeDecl* F, StringBuilder& output);
    void EmitArgVarDecl(const VarDecl* D, StringBuilder& output, unsigned index);
    void EmitVarDecl(const VarDecl* D, StringBuilder& output, unsigned indent);

    void EmitStmt(const Stmt* S, unsigned indent);
    void EmitCompoundStmt(const CompoundStmt* C, unsigned indent, bool startOnNewLine);
    void EmitIfStmt(const Stmt* S, unsigned indent);
    void EmitWhileStmt(const Stmt* S, unsigned indent);
    void EmitDoStmt(const Stmt* S, unsigned indent);
    void EmitForStmt(const Stmt* S, unsigned indent);
    void EmitSwitchStmt(const Stmt* S, unsigned indent);

    void EmitExpr(const Expr* E, StringBuilder& output);
    void EmitBinaryOperator(const Expr* E, StringBuilder& output);
    void EmitConditionalOperator(const Expr* E, StringBuilder& output);
    void EmitUnaryOperator(const Expr* E, StringBuilder& output);
    void EmitMemberExpr(const Expr* E, StringBuilder& output);
    void EmitDeclExpr(const DeclExpr* D, StringBuilder& output, unsigned indent);
    void EmitCallExpr(const Expr* E, StringBuilder& output);
    void EmitIdentifierExpr(const Expr* E, StringBuilder& output);
    void EmitBitOffsetExpr(const Expr* Base, Expr* E, StringBuilder& output);

    // Helpers
    void EmitDecl(const Decl* D, StringBuilder& output);
    void EmitFunctionProto(const FunctionDecl* F, StringBuilder& output);
    void EmitTypePreName(QualType type, StringBuilder& output);
    void EmitTypePostName(QualType type, StringBuilder& output);
    void EmitStringLiteral(const std::string& input, StringBuilder& output);

    const std::string& filename;
    Mode mode;

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

