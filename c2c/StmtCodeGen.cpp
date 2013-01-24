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

#include <stdio.h>
#include <string.h>
#include "Stmt.h"
#include "Expr.h"
#include "StringBuilder.h"
#include "Utils.h"
#include "CodeGenerator.h"

using namespace C2;
using namespace std;

llvm::Value* ReturnStmt::codeGen(CodeGenContext& C) {
    // TODO type
    // check IRBuilder::getCurrentFunctionReturnType()
    if (value) {
        C.builder.CreateRet(value->codeGen(C));
    } else {
        C.builder.CreateRetVoid();
    }
    return 0;
}
llvm::Value* IfStmt::codeGen(CodeGenContext& C) {
    assert(0);
    return 0;
}

llvm::Value* WhileStmt::codeGen(CodeGenContext& context) {
    // TODO
    return 0;
}

llvm::Value* DoStmt::codeGen(CodeGenContext& context) {
    // TODO
    return 0;
}

llvm::Value* BreakStmt::codeGen(CodeGenContext& context) {
    return 0;
}

llvm::Value* ContinueStmt::codeGen(CodeGenContext& context) {
    return 0;
}

llvm::Value* LabelStmt::codeGen(CodeGenContext& context) {
    return 0;
}

llvm::Value* CompoundStmt::codeGen(CodeGenContext& C) {
    // TODO create BasicBlock here
    for (unsigned int i=0; i<NumStmts; i++) {
        Body[i]->codeGen(C);
    }
    return 0;
}

