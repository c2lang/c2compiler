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
#include <llvm/Function.h>
#include <llvm/BasicBlock.h>
#include <vector>
// for SmallString
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/FileSystem.h>
// for tool_output_file
#include <llvm/Support/ToolOutputFile.h>
// for WriteBitcodeToFile
#include <llvm/Bitcode/ReaderWriter.h>
// For verifyModule()
#include <llvm/Analysis/Verifier.h>
//#include <stdio.h>

#include "CodeGenModule.h"
#include "CodeGenFunction.h"
#include "C2Sema.h"
#include "Decl.h"

using namespace C2;
using namespace llvm;

CodeGenModule::CodeGenModule(const std::string& pkgName_)
    : pkgName(pkgName_)
    , context(llvm::getGlobalContext())
    , module(new llvm::Module(pkgName, context))
    , builder(context)
{
    // TEMP hardcode puts function
    std::vector<llvm::Type *> putsArgs;
    putsArgs.push_back(builder.getInt8Ty()->getPointerTo());
    llvm::ArrayRef<llvm::Type*>  argsRef(putsArgs);
    llvm::FunctionType *putsType = 
        llvm::FunctionType::get(builder.getInt32Ty(), argsRef, false);
    llvm::Constant *putsFunc = module->getOrInsertFunction("puts", putsType);

    // TEMP hardcode puts function
    std::vector<llvm::Type *> printfArgs;
    printfArgs.push_back(builder.getInt8Ty()->getPointerTo());
    llvm::ArrayRef<llvm::Type*>  argsRef2(printfArgs);
    llvm::FunctionType *printfType =
        llvm::FunctionType::get(builder.getInt32Ty(), argsRef2, true);
    llvm::Constant *printfFunc = module->getOrInsertFunction("printf", printfType);
}

CodeGenModule::~CodeGenModule() {
    delete module;
}

void CodeGenModule::addEntry(const std::string& filename, C2Sema& sema) {
    entries.push_back(Entry(filename, sema));
}

void CodeGenModule::generate() {
    // TODO use ASTVisitor for this?
    for (EntriesIter iter = entries.begin(); iter != entries.end(); ++iter) {
        //printf("GENERATING CODE FOR %s\n", iter->filename->c_str());
        for (unsigned int i=0; i<iter->sema->getNumDecls(); i++) {
            Decl* decl = iter->sema->getDecl(i);
            EmitTopLevelDecl(iter->sema->getDecl(i));
        }
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

void CodeGenModule::verify() {
    if (verifyModule(*module)) {
        errs() << "Error in Module!\n";
    }
}

void CodeGenModule::dump() {
    module->dump();
}

void CodeGenModule::EmitTopLevelDecl(Decl* D) {
    switch (D->dtype()) {
    case DECL_FUNC:
        {
            FunctionDecl* F = DeclCaster<FunctionDecl>::getType(D);
            CodeGenFunction cgf(*this, F);
            cgf.generate();
        }
        break;
    case DECL_VAR:
        {
            VarDecl* Var = DeclCaster<VarDecl>::getType(D);
            llvm::Type* type = ConvertType(Var->getType());
            bool constant = false;
            llvm::GlobalValue::LinkageTypes ltype = llvm::GlobalValue::InternalLinkage;
            if (Var->isPublic()) ltype = llvm::GlobalValue::ExternalLinkage;
            /*
            // TODO use correct arguments for constant and Initializer
            // NOTE: getName() doesn't have to be virtual here
            llvm::GlobalVariable* global =
                new llvm::GlobalVariable(module, type, constant, ltype, 0, Var->getName()); 
            */
        }
        break;
    case DECL_TYPE:
        assert(0 && "TODO type");
        break;
    case DECL_ARRAYVALUE:
        assert(0 && "TODO arrayvalue");
        break;
    case DECL_USE:
        // nothing needed
        break;
    }
}

llvm::Type* CodeGenModule::ConvertType(C2::Type* type) {
    switch (type->getKind()) {
    case Type::BUILTIN:
        {
            // TODO make u8/16/32 unsigned
            switch (type->getBuiltinType()) {
            case TYPE_U8:       return builder.getInt8Ty();
            case TYPE_U16:      return builder.getInt16Ty();
            case TYPE_U32:      return builder.getInt32Ty();
            case TYPE_S8:       return builder.getInt32Ty();
            case TYPE_S16:      return builder.getInt16Ty();
            case TYPE_S32:      return builder.getInt32Ty();
            case TYPE_INT:      return builder.getInt32Ty();
            case TYPE_STRING:
                return 0;  // TODO remove this type?
            case TYPE_FLOAT:    return builder.getFloatTy();
            case TYPE_F32:      return builder.getFloatTy();
            case TYPE_F64:      return builder.getFloatTy(); // TODO find correct type (double?)
            case TYPE_CHAR:     return builder.getInt8Ty();
            case TYPE_BOOL:     return builder.getInt1Ty();
            case TYPE_VOID:     return builder.getVoidTy();
            }
        }
        break;
    case Type::USER:
    case Type::STRUCT:
    case Type::UNION:
    case Type::ENUM:
    case Type::FUNC:
        assert(0 && "TODO");
        break;
    case Type::POINTER:
        {
            llvm::Type* tt = ConvertType(type->getRefType());
            return tt->getPointerTo();
        }
    case Type::ARRAY:
        {
            // Hmm for function args, array are simply converted to pointers, do that for now
            // array: use type = ArrayType::get(elementType, numElements)
            llvm::Type* tt = ConvertType(type->getRefType());
            return tt->getPointerTo();
        }
    case Type::QUALIFIER:
        assert(0 && "TODO");
        break;
    }

    return 0;
}

