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

#ifndef CODEGEN_MODULE_H
#define CODEGEN_MODULE_H

#include <string>
#include <vector>

#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/IRBuilder.h>
#include "AST/Module.h"
#include "AST/Type.h"
#include "AST/Expr.h"

namespace llvm {
class Module;
class LLVMContext;
class Type;
class Function;
}

namespace C2 {

class Decl;
class VarDecl;
class Type;
class StringLiteral;

// generates LLVM Module from (multiple) Module(s)
class CodeGenModule {
public:
    CodeGenModule(const std::string& name_, bool single, const ModuleList& mods_, llvm::LLVMContext& context_);
    ~CodeGenModule();

    void generate();
    bool verify();
    void write(const std::string& outputDir, const std::string& name_);
    void dump();

    llvm::Type* ConvertType(BuiltinType::Kind K);
    llvm::Type* ConvertType(QualType Q);
    llvm::PointerType* getVoidPtrType();
    llvm::Function* createExternal(const C2::Module* P, const std::string& name_);
    llvm::GlobalValue::LinkageTypes getLinkage(bool isPublic);

    const std::string& getName() const { return name; }
    llvm::Module* getModule() const { return module; }
    llvm::LLVMContext& getContext() const { return context; }
    llvm::IRBuilder<> getBuilder() const { return builder; }

    unsigned getAlignment(QualType Q) const;
private:
    llvm::Type* ConvertStructType(const StructType* S);

    void EmitGlobalVariable(VarDecl* V);
    llvm::Constant* EvaluateExprAsConstant(const Expr *E);
    llvm::Constant* GetConstantArrayFromStringLiteral(const StringLiteral* E);
    llvm::Constant* EmitDefaultInit(QualType Q);
    llvm::Constant* EmitStructInit(const StructType *AT, Expr** Vals, unsigned numValues);
    llvm::Constant* EmitArrayInit(const ArrayType *AT, Expr** Vals, unsigned numValues);
    llvm::Constant* EmitConstantDecl(const Decl* D);

    const std::string name;
    bool single_module;     // multiple modules in single module
    const ModuleList& mods;

    llvm::LLVMContext& context;
    llvm::Module* module;
    llvm::IRBuilder<> builder;

    CodeGenModule(const CodeGenModule&);
    CodeGenModule& operator= (const CodeGenModule&);
};

}

#endif

