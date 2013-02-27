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

#include <llvm/Constants.h>
#include <llvm/Module.h>
#include <vector>

#include "Expr.h"
#include "StringBuilder.h"
#include "Utils.h"
#include "Type.h"
#include "CodeGenerator.h"

using namespace C2;
using namespace std;

llvm::Value* NumberExpr::codeGen(CodeGenContext& C) {
    // TODO number is always int32 (signed is true)
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(C.context), value, true);
}

llvm::Value* StringExpr::codeGen(CodeGenContext& C) {
    return C.builder.CreateGlobalStringPtr(value);
}

llvm::Value* BoolLiteralExpr::codeGen(CodeGenContext& C) {
    return llvm::ConstantInt::get(llvm::Type::getInt1Ty(C.context), value, true);
}

llvm::Value* CharLiteralExpr::codeGen(CodeGenContext& context) {
    assert(0 && "TODO");
    return 0;
}

llvm::Value* CallExpr::codeGen(CodeGenContext& C) {
    llvm::Function* function = C.module.getFunction(Fn->getName());
    assert(function && "CANNOT FIND FUNCTION");

    // NOTE: see CodeGenerator insertion of puts() and printf()
    // TODO elipsis (see CodeGenerator)
    switch (args.size()) {
    case 0:
        C.builder.CreateCall(function);
        break;
    case 1:
        C.builder.CreateCall(function, args[0]->codeGen(C));
        break;
    default:
        {
            std::vector<llvm::Value *> Args;
            for (unsigned int i=0; i<args.size(); i++) {
                Args.push_back(args[i]->codeGen(C));
            }
            llvm::ArrayRef<llvm::Value*> argsRef(Args);
            C.builder.CreateCall(function, argsRef);
        }
        break;
    }
    // TODO return something??
    return 0;
}

llvm::Value* IdentifierExpr::codeGen(CodeGenContext& context) {
    assert(0 && "SHOULD NEVER BE GENERATED?");
    return 0;
}

llvm::Value* TypeExpr::codeGen(CodeGenContext& context) {
    assert(0 && "TODO");
    return 0;
}

llvm::Value* InitListExpr::codeGen(CodeGenContext& context) {
    assert(0 && "TODO");
    return 0;
}

llvm::Value* DeclExpr::codeGen(CodeGenContext& C) {
    // TODO arrays types?
    llvm::AllocaInst* inst = C.builder.CreateAlloca(type->convert(C), 0, name);
    // TODO smart alignment
    inst->setAlignment(4);
    // TODO initValue
    return inst;
}

llvm::Value* BinOpExpr::codeGen(CodeGenContext& context) {
    assert(0 && "TODO");
    return 0;
}

llvm::Value* UnaryOpExpr::codeGen(CodeGenContext& context) {
    return 0;
}

llvm::Value* SizeofExpr::codeGen(CodeGenContext& context) {
    assert(0 && "TODO");
    return 0;
}

llvm::Value* ArraySubscriptExpr::codeGen(CodeGenContext& context) {
    assert(0 && "TODO");
    return 0;
}

llvm::Value* MemberExpr::codeGen(CodeGenContext& context) {
    return 0;
}

