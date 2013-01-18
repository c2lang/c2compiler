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
#include "llvm/ADT/ArrayRef.h"
#include "llvm/LLVMContext.h"
#include "llvm/Function.h"
#include "llvm/BasicBlock.h"
#include "llvm/IRBuilder.h"
#include <vector>
#include <string>
// for SmallString
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/FileSystem.h"
// for tool_output_file
#include <llvm/Support/ToolOutputFile.h>
// for WriteBitcodeToFile
#include <llvm/Bitcode/ReaderWriter.h>

#include "CodeGenerator.h"
#include "C2Sema.h"
#include "Decl.h"

using namespace C2;
using namespace llvm;

CodeGenerator::CodeGenerator(C2Sema& sema_)
    : sema(sema_)
    , context(llvm::getGlobalContext())
    , module(new llvm::Module(sema.pkgName, context))
    , builder(context)
{
    // TEMP hardcode puts function
    std::vector<llvm::Type *> putsArgs;
    putsArgs.push_back(builder.getInt8Ty()->getPointerTo());
    llvm::ArrayRef<llvm::Type*>  argsRef(putsArgs);
    llvm::FunctionType *putsType = 
        llvm::FunctionType::get(builder.getInt32Ty(), argsRef, false);
    llvm::Constant *putsFunc = module->getOrInsertFunction("puts", putsType);
}

CodeGenerator::~CodeGenerator() {
    delete module;
}

void CodeGenerator::generate() {
    CodeGenContext mycontext(context, *module, builder);
    for (unsigned int i=0; i<sema.decls.size(); i++) {
        Decl* decl = sema.decls[i];
        decl->codeGen(mycontext);
    }
#if 0
    // print to ascii file (/tmp/temp-%%%%%%.bc)
    {
        StringRef name("temp");
        int FD;
        SmallString<128> TempPath = name;
        TempPath += "-%%%%%%%%";
        TempPath += ".ll";
        if (llvm::sys::fs::unique_file(TempPath.str(), FD, TempPath, /*makeAbsolute=*/true) != llvm::errc::success) {
            //ImportingInstance.getDiagnostics().Report(diag::err_module_map_temp_file)
            //<< TempModuleMapFileName;
            fprintf(stderr, "NO SUCCESS\n");
            return;
        }
        llvm::raw_fd_ostream OS(FD, /*shouldClose=*/true);
        module->print(OS, 0);
        fprintf(stderr, "RESULTNAME=%s\n", TempPath.str().str().c_str());
    }
#endif
#if 0
    // print to binary file
    {
        std::string ErrInfo;
        const char* outfile = "./out.bc";
        tool_output_file Out(outfile, ErrInfo, raw_fd_ostream::F_Binary);
        if (!ErrInfo.empty()) {
            fprintf(stderr, "Could not open file %s\n", outfile);
            return;
        }
        llvm::WriteBitcodeToFile(module, Out.os());
        Out.os().close();
        if (Out.os().has_error()) {
            fprintf(stderr, "Error writing file %s\n", outfile);
        }
        Out.keep();
        printf("written %s\n", outfile);
    }
#endif
#if 0
    llvm::FunctionType *funcType = llvm::FunctionType::get(builder.getVoidTy(), false);
    llvm::Function *mainFunc = 
    llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", module);
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entrypoint", mainFunc);
    builder.SetInsertPoint(entry);

    llvm::Value *helloWorld = builder.CreateGlobalStringPtr("hello world!\n");

    std::vector<llvm::Type *> putsArgs;
    putsArgs.push_back(builder.getInt8Ty()->getPointerTo());
    llvm::ArrayRef<llvm::Type*>  argsRef(putsArgs);

    llvm::FunctionType *putsType = 
    llvm::FunctionType::get(builder.getInt32Ty(), argsRef, false);
    llvm::Constant *putsFunc = module->getOrInsertFunction("puts", putsType);

    builder.CreateCall(putsFunc, helloWorld);
    builder.CreateRetVoid();
    module->dump();
    // print to file (/tmp/temp.bc)
    {
        StringRef name("temp");
        int FD;
        SmallString<128> TempPath = name;
        TempPath += "-%%%%%%%%";
        TempPath += ".ll";
        if (llvm::sys::fs::unique_file(TempPath.str(), FD, TempPath, /*makeAbsolute=*/true) != llvm::errc::success) {
            //ImportingInstance.getDiagnostics().Report(diag::err_module_map_temp_file)
            //<< TempModuleMapFileName;
            fprintf(stderr, "NO SUCCESS\n");
            return;
        }
        llvm::raw_fd_ostream OS(FD, /*shouldClose=*/true);
        module->print(OS, 0);
        fprintf(stderr, "RESULTNAME=%s\n", TempPath.str().str().c_str());
    }
#endif
}

void CodeGenerator::dump() {
    module->dump();
}

