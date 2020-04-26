/* Copyright 2013-2020 Bas van den Berg
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

#ifndef ANALYSER_FILE_ANALYSER_H
#define ANALYSER_FILE_ANALYSER_H

#include <memory>
#include <map>
#include <string>
#include "AST/Type.h"
#include "AST/Module.h"
#include "AST/Expr.h"
#include "Analyser/Scope.h"
#include "Analyser/FunctionAnalyser.h"
#include "Analyser/TypeResolver.h"
#include "Analyser/ExprAnalyser.h"

namespace c2lang {
class DiagnosticsEngine;
}

namespace C2 {

class IdentifierExpr;
class Scope;
class TypeDecl;
class VarDecl;
class FunctionDecl;
class StructTypeDecl;
class ArrayValueDecl;
class AST;
class TypeResolver;
class TargetInfo;
class RefType;

typedef std::vector<FunctionDecl*> StructFunctionEntries;
typedef std::map<StructTypeDecl*, StructFunctionEntries> StructFunctionList;
typedef StructFunctionList::const_iterator StructFunctionListIter;

typedef std::map<VarDecl*, ExprList> IncrementalArrayVals;
typedef IncrementalArrayVals::const_iterator IncrementalArrayValsIter;

typedef std::vector<C2::EnumConstantDecl*> EnumConstants;
typedef std::map<EnumTypeDecl*, EnumConstants> IncrementalEnums;
typedef IncrementalEnums::const_iterator IncrementalEnumsIter;

class FileAnalyser {
public:
    FileAnalyser(const Module& module_,
                 const Modules& allModules,
                 c2lang::DiagnosticsEngine& Diags_,
                 const TargetInfo& target_,
                 AST& ast_,
                 bool verbose);
    ~FileAnalyser() {}

    // call in this order
    bool collectIncrementals(IncrementalArrayVals& values, IncrementalEnums& enums);
    bool collectStructFunctions(StructFunctionList& structFuncs);
    bool analyseTypes();
    bool analyseVars();
    bool analyseStaticAsserts();
    bool analyseFunctionProtos();
    void analyseFunctionBodies();
    void checkUnusedDecls();

private:
    bool pushCheck(Decl* d);
    void popCheck();
    bool isTop(Decl* d);

    bool collectIncremental(ArrayValueDecl* D, IncrementalArrayVals& values, IncrementalEnums& enums);
    bool collectStructFunction(FunctionDecl* F, StructFunctionList& structFuncs);

    // Decls
    bool analyseStaticAssertInteger(const Expr* lhs, const Expr* rhs);
    //bool analyseStaticAssertString(const Expr* lhs, const Expr* rhs);
    bool analyseStaticAssert(StaticAssertDecl* D);

    bool analyseDecl(Decl* D);
    bool analyseVarDecl(VarDecl* D);
    bool analyseTypeDecl(TypeDecl* D);
    bool analyseStructTypeDecl(StructTypeDecl* D);
    bool analyseStructMember(QualType T, MemberExpr* M, bool isStatic);
    bool analyseFunctionDecl(FunctionDecl* D);
    bool checkIfStaticStructFunction(FunctionDecl* F) const;
    bool analyseEnumConstants(EnumTypeDecl* ETD);
    QualType analyseType(QualType Q, SourceLocation loc, bool usedPublic, bool full);
    QualType analyseRefType(QualType Q, bool usedPublic, bool full);

    // Expressions
    bool analyseExpr(Expr* expr, bool usedPublic);
    bool analyseArraySubscript(Expr* expr, bool usedPublic);
    bool analyseBuiltinExpr(Expr* expr, bool usedPublic);
    bool analyseToContainer(BuiltinExpr* B, bool usedPublic);
    bool analyseOffsetOf(BuiltinExpr* B, bool usedPublic);
    StructTypeDecl* builtinExprToStructTypeDecl(BuiltinExpr* B, bool usedPublic);
    bool analyseSizeOfExpr(BuiltinExpr* B, bool usedPublic);
    bool analyseEnumMinMaxExpr(BuiltinExpr* B, bool isMin, bool usedPublic);
    bool analyseElemsOfExpr(BuiltinExpr* B, bool usedPublic);
    bool analyseBinaryOperator(Expr* expr, bool usedPublic);
    bool analyseUnaryOperator(Expr* expr, bool usedPublic);
    bool analyseParenExpr(Expr* expr, bool usedPublic);
    bool analyseMemberExpr(Expr* expr, bool usedPublic);
    Decl* analyseIdentifier(IdentifierExpr* id, bool usedPublic);

    // Init expressions
    bool analyseInitExpr(Expr* expr, QualType expectedType, bool usedPublic);
    bool analyseInitList(InitListExpr* expr, QualType Q, bool usedPublic);
    bool analyseInitListArray(InitListExpr* expr, QualType Q, unsigned numValues, Expr** values, bool usedPublic);
    bool analyseInitListStruct(InitListExpr* expr, QualType Q, unsigned numValues, Expr** values, bool usedPublic);
    bool checkArrayDesignators(InitListExpr* expr, int64_t* size);
    bool analyseDesignatorInitExpr(Expr* expr, QualType expectedType, bool usedPublic);
    typedef std::vector<Expr*> Fields;
    bool analyseFieldInDesignatedInitExpr(DesignatedInitExpr* D,
                                                        StructTypeDecl* STD,
                                                        QualType Q,
                                                        Fields &fields,
                                                        Expr* value,
                                                        bool &haveDesignators,
                                                        bool usedPublic);
    typedef std::map<const std::string, const Decl*> Names;
    bool analyseStructNames(const StructTypeDecl* S, Names& names, bool isStruct);
    bool checkVarDeclAttributes(VarDecl* D);
    bool checkAttributes(Decl* D);
    bool analyseStaticStructMember(QualType T, MemberExpr* M, const StructTypeDecl* S);

    void checkStructMembersForUsed(const StructTypeDecl* S);
    bool checkAddressOfOperand(Expr* expr);

    void error(SourceLocation loc, QualType left, QualType right) const;
    c2lang::DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) const;

    AST& ast;
    const Module& module;
    std::unique_ptr<Scope> scope;
    std::unique_ptr<TypeResolver> TR;
    c2lang::DiagnosticsEngine& Diags;
    ExprAnalyser EA;
    FunctionAnalyser functionAnalyser;
    // TEMP array with index
    Decl* checkStack[8];
    unsigned checkIndex;

    bool verbose;
    FileAnalyser(const FileAnalyser&);
    FileAnalyser& operator= (const FileAnalyser&);
};

}

#endif

