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

#ifndef CODEGEN_FUNCTION_H
#define CODEGEN_FUNCTION_H

#include <llvm/IRBuilder.h>

namespace llvm {
class Module;
class LLVMContext;
class Value;
class Function;
}

namespace C2 {

class CodeGenModule;
class FunctionDecl;
class Stmt;
class CompoundStmt;
class ReturnStmt;
class Expr;
class CallExpr;

// This class organizes the per-function state that is used
// while generating LLVM code.
class CodeGenFunction {
public:
    CodeGenFunction(CodeGenModule& CGM_, FunctionDecl* Func_);
    ~CodeGenFunction() {}

    llvm::Function* generateProto(const std::string& pkgname);
    void generateBody(llvm::Function* func);
private:
    void EmitStmt(const Stmt& S);
    void EmitCompoundStmt(const CompoundStmt* S);
    void EmitReturnStmt(const ReturnStmt* S);

    llvm::Value* EmitExpr(const Expr* E);
    llvm::Value* EmitCallExpr(const CallExpr* E);

    CodeGenModule& CGM;
    FunctionDecl* Func;

    llvm::LLVMContext& context;
    llvm::Module* module;
    llvm::IRBuilder<> builder;  // NOTE: do we really need to create a new builder?

    CodeGenFunction(const CodeGenFunction&);
    CodeGenFunction& operator= (const CodeGenFunction&);
};

}

#endif

