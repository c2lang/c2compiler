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

#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <llvm/IRBuilder.h>

namespace llvm {
class Module;
class LLVMContext;
}

namespace C2 {

class C2Sema;

class CodeGenContext {
public:
    CodeGenContext(llvm::LLVMContext& context_, llvm::Module& module_, llvm::IRBuilder<>& builder_)
        : context(context_)
        , module(module_)
        , builder(builder_)
    {}
    llvm::LLVMContext& context;
    llvm::Module& module;
    llvm::IRBuilder<> &builder;
};

class CodeGenerator {
public:
    CodeGenerator(C2Sema& sema_);
    ~CodeGenerator();
    void generate();
    void dump();
private:
    C2Sema& sema;
    llvm::LLVMContext& context;
    llvm::Module* module;
    llvm::IRBuilder<> builder;

    CodeGenerator(const CodeGenerator&);
    CodeGenerator& operator= (const CodeGenerator&);
};

}

#endif

