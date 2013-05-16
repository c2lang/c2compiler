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
// TODO REMOVE
#include <stdio.h>


#include "CodeGenModule.h"
#include "CodeGenFunction.h"
#include "Package.h"
#include "C2Sema.h"
#include "Decl.h"
#include "StringBuilder.h"

//#define DEBUG_CODEGEN

using namespace C2;
using namespace llvm;

CodeGenModule::CodeGenModule(const Package* pkg_)
    : pkg(pkg_)
    , context(llvm::getGlobalContext())
    , module(new llvm::Module(pkg->getName(), context))
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

    // pass 1: generate all function proto's
    for (EntriesIter iter = entries.begin(); iter != entries.end(); ++iter) {
#ifdef DEBUG_CODEGEN
        printf("CodeGen for %s - pass 1\n", iter->filename->c_str());
#endif
        C2Sema* sema = iter->sema;
        for (unsigned int i=0; i<sema->getNumDecls(); i++) {
            Decl* decl = sema->getDecl(i);
            if (decl->dtype() == DECL_FUNC) EmitFunctionProto(decl);
        }
    }

    // pass 2: generate all function bodies (and possibly external function decls)
    for (EntriesIter iter = entries.begin(); iter != entries.end(); ++iter) {
#ifdef DEBUG_CODEGEN
        printf("CodeGen for %s - pass 2\n", iter->filename->c_str());
#endif
        C2Sema* sema = iter->sema;
        for (unsigned int i=0; i<sema->getNumDecls(); i++) {
            Decl* decl = sema->getDecl(i);
            EmitTopLevelDecl(decl);
        }
    }
}

void CodeGenModule::verify() {
    if (verifyModule(*module)) {
        errs() << "Error in Module!\n";
    }
}

void CodeGenModule::dump() {
    module->dump();
}

void CodeGenModule::write(const std::string& target, const std::string& name) {
    // write IR Module to output/<target>/<package>.ll
    StringBuilder filename;
    filename << "output/" << target << '/';
    bool existed;
    llvm::Twine path(filename);
    if (llvm::sys::fs::create_directories(path, existed) != llvm::errc::success) {
        fprintf(stderr, "Could not create directory: %s\n", (const char*)filename);
        return;
    }

    filename << name << ".ll";
    std::string ErrorInfo;
    llvm::raw_fd_ostream OS((const char*)filename, ErrorInfo);
    if (!ErrorInfo.empty()) {
        fprintf(stderr, "%s\n", ErrorInfo.c_str());
        return;
    }
    module->print(OS, 0);
    fprintf(stderr, "written %s\n", (const char*)filename);
#if 0
    // print to binary file
    {
        std::string ErrInfo;
        StringBuilder filename;
        filename << "output/" << target << '/' << name;
        const char* outfile = (const char*)filename;
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
}

void CodeGenModule::EmitFunctionProto(Decl* D) {
    FunctionDecl* F = DeclCaster<FunctionDecl>::getType(D);
    assert(F);
    CodeGenFunction cgf(*this, F);
    llvm::Function* proto = cgf.generateProto(pkg->getName());
    F->setIRProto(proto);
}

void CodeGenModule::EmitTopLevelDecl(Decl* D) {
    switch (D->dtype()) {
    case DECL_FUNC:
        {
            FunctionDecl* F = DeclCaster<FunctionDecl>::getType(D);
            CodeGenFunction cgf(*this, F);
            cgf.generateBody(F->getIRProto());
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

llvm::Function* CodeGenModule::createExternal(const Package* P, const std::string& name) {
    Decl* D = P->findSymbol(name);
    assert(D);
    FunctionDecl* F = DeclCaster<FunctionDecl>::getType(D);
    assert(F);
    // NOTE: use getCanonicalType()? and generate function for that?
    CodeGenFunction cgf(*this, F);
    llvm::Function* proto = cgf.generateProto(P->getCName());
    return proto;
}

