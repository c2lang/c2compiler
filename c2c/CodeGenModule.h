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

#ifndef CODEGEN_MODULE_H
#define CODEGEN_MODULE_H

#include <llvm/IRBuilder.h>
#include <string>
#include <vector>

namespace llvm {
class Module;
class LLVMContext;
class Type;
class Function;
}

namespace C2 {

class AST;
class Decl;
class Expr;
class Type;
class Package;

// generates LLVM Module from (multiple) ASTs
class CodeGenModule {
public:
    CodeGenModule(const Package* pkg_);
    ~CodeGenModule();
    void addEntry(const std::string& filename, AST& ast);

    void generate();
    bool verify();
    void write(const std::string& target, const std::string& name);
    void dump();

    llvm::Type* ConvertType(const C2::Type* type);
    llvm::Function* createExternal(const Package* P, const std::string& name);

    const Package* getPackage() const { return pkg; }
    llvm::Module* getModule() const { return module; }
    llvm::LLVMContext& getContext() const { return context; }
    llvm::IRBuilder<> getBuilder() const { return builder; }

    llvm::Constant* EvaluateExprAsConstant(const Expr *E);
    llvm::Value* EvaluateExprAsBool(const Expr *E);
private:
    void EmitFunctionProto(Decl* D);
    void EmitTopLevelDecl(Decl* D);

    const Package* pkg;

    struct Entry {
        Entry(const std::string& f, AST& s)
            : filename(&f), ast(&s) {}
        const std::string* filename;
        AST* ast;
    };
    typedef std::vector<Entry> Entries;
    typedef Entries::iterator EntriesIter;
    Entries entries;

    llvm::LLVMContext& context;
    llvm::Module* module;
    llvm::IRBuilder<> builder;

    CodeGenModule(const CodeGenModule&);
    CodeGenModule& operator= (const CodeGenModule&);
};

}

#endif

