/* Copyright 2013-2018 Bas van den Berg
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
#include "Analyser/FunctionAnalyser.h"
#include "AST/Type.h"
#include "AST/Module.h"
#include "AST/Expr.h"
#include "Analyser/Scope.h"
#include "Analyser/TypeResolver.h"

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

typedef std::vector<FunctionDecl*> StructFunctionEntries;
typedef std::map<StructTypeDecl*, StructFunctionEntries> StructFunctionList;
typedef StructFunctionList::const_iterator StructFunctionListIter;

typedef std::map<VarDecl*, ExprList> IncrementalArrayVals;
typedef IncrementalArrayVals::const_iterator IncrementalArrayValsIter;

enum class FileAnalyserPass {
    NOT_STARTED,
    ADD_IMPORTS,
    RESOLVE_TYPES,
    RESOLVE_TYPE_CANONICALS,
    RESOLVE_STRUCT_MEMBERS,
    RESOLVE_VARS,
    RESOLVE_ENUM_CONSTANTS,
    CHECK_ARRAY_VALUES,
    CHECK_FUNCTION_PROTOS,
    CHECK_VAR_INITS,
    CHECK_FUNCTION_BODIES,
    CHECK_DECLS_FOR_USED
};

class FileAnalyser {
public:
    FileAnalyser(const Module& module_,
                 const Modules& modules,
                 c2lang::DiagnosticsEngine& Diags_,
                 const TargetInfo& target_,
                 AST& ast_,
                 bool verbose);
    ~FileAnalyser() {}

    void printAST(bool printInterface) const;

    // call in this order
    void addImports();
    void resolveTypes();
    unsigned resolveTypeCanonicals();
    unsigned resolveStructMembers();
    unsigned resolveVars();
    unsigned checkArrayValues(IncrementalArrayVals& values);
    unsigned checkFunctionProtos(StructFunctionList& structFuncs);
    void checkVarInits();
    unsigned resolveEnumConstants();
    void checkFunctionBodies();
    void checkDeclsForUsed();

private:
    void beginNewPass(FileAnalyserPass newPass);
    unsigned checkTypeDecl(TypeDecl* D);
    unsigned checkStructTypeDecl(StructTypeDecl* D);
    unsigned resolveVarDecl(VarDecl* D);
    unsigned resolveFunctionDecl(FunctionDecl* D, bool checkArgs);
    unsigned checkArrayValue(ArrayValueDecl* D, IncrementalArrayVals& values);
    typedef std::map<const std::string, const Decl*> Names;
    void checkStructFunction(FunctionDecl* F, StructFunctionList& structFuncs);
    void analyseStructNames(const StructTypeDecl* S, Names& names, bool isStruct);
    void checkVarDeclAttributes(VarDecl* D);
    void checkAttributes(Decl* D);
    void checkStructMembersForUsed(const StructTypeDecl* S);
    bool isStaticStructFunc(QualType T,  FunctionDecl* func) const;
    Decl* getStructDecl(QualType T) const;

    AST& ast;
    const Module& module;
    std::unique_ptr<Scope> globals;
    std::unique_ptr<TypeResolver> TR;
    c2lang::DiagnosticsEngine& Diags;
    FunctionAnalyser functionAnalyser;
    bool verbose;
    FileAnalyserPass currentPass = FileAnalyserPass::NOT_STARTED;
    FileAnalyser(const FileAnalyser&);
    FileAnalyser& operator= (const FileAnalyser&);
};

}

#endif

