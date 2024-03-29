/* Copyright 2013-2023 Bas van den Berg
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

#include "CGenerator/TypeSorter.h"
#include "AST/Module.h"
#include "Utils/StringBuilder.h"

namespace C2 {

class Decl;
class VarDecl;
class TypeDecl;
class FunctionDecl;
class StructTypeDecl;
class FunctionTypeDecl;
class EnumTypeDecl;
class QualType;
class Expr;
class StringLiteral;
class Stmt;
class CompoundStmt;
class AsmStmt;
class AssertStmt;
class HeaderNamer;
class TargetInfo;

// generates LLVM Module from (multiple) Module(s)
class CCodeGenerator : public CTypeWriter {
public:
    enum Mode { MULTI_FILE, SINGLE_FILE };
    CCodeGenerator(const std::string& filename_,
                   Mode mode_,
                   const Modules& modules_,
                   const ModuleList& mods_,
                   const HeaderNamer& namer_,
                   const TargetInfo& targetInfo_,
                   bool genChecks_,
                   bool genAsserts_);
    ~CCodeGenerator();

    void generate(bool printCode, const std::string& outputDir);
    void createLibHeader(bool printCode, const std::string& outputDir);

    // for CTypeWriter
    virtual void forwardDecl(const Decl* D);
    virtual void fullDecl(const Decl* D);
private:
    void EmitAll();
    void EmitIncludeGuard();
    void EmitIncludes();

    void EmitFunctionForward(const FunctionDecl* F);
    void EmitFunction(const FunctionDecl* F);
    void EmitFunctionArgs(const FunctionDecl* F, StringBuilder& output);
    void EmitConstant(const VarDecl* D);
    void EmitGlobalVariable(const VarDecl* V);
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
    void EmitSSwitchStmt(const Stmt* S, unsigned indent);
    void EmitDeclStmt(const Stmt* S, unsigned indent);
    void EmitAsmStmt(const AsmStmt* S, unsigned indent);
    void EmitAssertStmt(const AssertStmt* S, unsigned indent);
    void EmitAsmPart(bool multiline, unsigned indent);
    void EmitAsmOperand(const char* name, const StringLiteral* c, const Expr* e);

    void EmitExpr(const Expr* E, StringBuilder& output);
    void EmitBuiltinExpr(const Expr* E, StringBuilder& output);
    void EmitBinaryOperator(const Expr* E, StringBuilder& output);
    void EmitConditionalOperator(const Expr* E, StringBuilder& output);
    void EmitUnaryOperator(const Expr* E, StringBuilder& output);
    void EmitMemberExpr(const Expr* E, StringBuilder& output);
    void EmitCallExpr(const Expr* E, StringBuilder& output);
    void EmitIdentifierExpr(const Expr* E, StringBuilder& output);
    void EmitBitOffsetExpr(const Expr* Base, Expr* E, StringBuilder& output);

    // Helpers
    void EmitDecl(const Decl* D, StringBuilder& output);
    void EmitStructDecl(const StructTypeDecl* D, StringBuilder& output);
    void EmitEnumConstant(const EnumConstantDecl* D, const char* typeName, StringBuilder& output);
    void EmitFunctionProto(const FunctionDecl* F, StringBuilder& output);
    void EmitTypePreName(QualType type, StringBuilder& output);
    void EmitTypePostName(QualType type, StringBuilder& output);
    void EmitConditionPre(const Stmt* S, unsigned indent);
    void EmitConditionPost(const Stmt* S);
    bool EmitAttributes(const Decl* D, StringBuilder& output, bool addStartSpace);

    bool EmitAsStatic(const Decl* D) const;
    bool EmitAsDefine(const VarDecl* V) const;

    const Expr* stripImplicitCast(const Expr* E);

    const std::string& filename;
    Mode mode;
    bool inInterface;
    bool generateChecks;
    bool generateAsserts;

    const Modules& modules;
    const ModuleList& mods;
    const HeaderNamer& headerNamer;
    const TargetInfo& targetInfo;

    StringBuilder cbuf;
    StringBuilder hbuf;
    std::string cfilename;
    std::string hfilename;

    CCodeGenerator(const CCodeGenerator&);
    CCodeGenerator& operator= (const CCodeGenerator&);
};

}

#endif

