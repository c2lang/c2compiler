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

#include <llvm/Module.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Function.h>
#include <llvm/BasicBlock.h>
#include <llvm/IRBuilder.h>
#include <llvm/GlobalValue.h>
#include <stdio.h>

#include "Decl.h"
#include "Stmt.h"
#include "Expr.h"
#include "StringBuilder.h"
#include "Type.h"
#include "Utils.h"
#include "CodeGenerator.h"

using namespace C2;
using namespace std;

llvm::Value* FunctionDecl::codeGen(CodeGenContext& C) {
    // arguments + return type
    llvm::FunctionType *funcType;
    if (args.size() == 0) {
        funcType = llvm::FunctionType::get(rtype->convert(C), false);
    } else {
        std::vector<llvm::Type*> Args;
        for (unsigned int i=0; i<args.size(); i++) {
            // TODO already store as DeclExpr?
            DeclExpr* de = ExprCaster<DeclExpr>::getType(args[i]);
            assert(de);
            Args.push_back(de->getType()->convert(C));
        }
        llvm::ArrayRef<llvm::Type*> argsRef(Args);
        // TODO handle ellipsis
        funcType = llvm::FunctionType::get(rtype->convert(C), argsRef, false);
    }
    //TODO linkage type (is_public)
    StringBuilder buffer;
    Utils::addName(C.pkgName, name, buffer);
    llvm::Function *func =
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, (const char*)buffer, &C.module);
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(C.context, "entry", func);
    C.builder.SetInsertPoint(entry);
    body->codeGen(C);
    return func;
}

llvm::Value* VarDecl::codeGen(CodeGenContext& C) {
    llvm::Type* type = getType()->convert(C);
    bool constant = false;
    llvm::GlobalValue::LinkageTypes ltype = llvm::GlobalValue::InternalLinkage;
    if (is_public) ltype = llvm::GlobalValue::ExternalLinkage;
    // TODO use correct arguments for constant and Initializer
    llvm::GlobalVariable* global =
        new llvm::GlobalVariable(C.module, type, constant, ltype, 0, getName()); 
    return 0;
}

llvm::Value* TypeDecl::codeGen(CodeGenContext& C) {
    fprintf(stderr, "TODO TypeDecl::codeGen()\n");
    return 0;
}

llvm::Value* ArrayValueDecl::codeGen(CodeGenContext& C) {
    fprintf(stderr, "TODO ArrayValueDecl::codeGen()\n");
    return 0;
}

llvm::Value* UseDecl::codeGen(CodeGenContext& C) {
    return 0;
}

