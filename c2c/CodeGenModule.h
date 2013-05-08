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
}

namespace C2 {

class C2Sema;
class Decl;
class Type;

// generates LLVM Module from (multiple) ASTs
class CodeGenModule {
public:
    CodeGenModule(const std::string& pkgName_);
    ~CodeGenModule();
    void addEntry(const std::string& filename, C2Sema& sema);

    void generate();
    void verify();
    void dump();

    llvm::Type* ConvertType(C2::Type* type);
    const std::string& getPkgName() const { return pkgName; }
    llvm::Module* getModule() const { return module; }
    llvm::LLVMContext& getContext() const { return context; }
    llvm::IRBuilder<> getBuilder() const { return builder; }
private:
    void EmitTopLevelDecl(Decl* D);

    const std::string& pkgName;

    struct Entry {
        Entry(const std::string& f, C2Sema& s)
            : filename(&f), sema(&s) {}
        const std::string* filename;
        C2Sema* sema;
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

