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
#include <string>

// TEMP for debugging
#include <stdio.h>

#include "CodeGenFunction.h"
#include "CodeGenModule.h"
#include "C2Sema.h"
#include "Decl.h"
#include "Stmt.h"
#include "StringBuilder.h"
#include "Utils.h"
#include "Package.h"

using namespace C2;
using namespace llvm;

CodeGenFunction::CodeGenFunction(CodeGenModule& CGM_, FunctionDecl* Func_)
    : CGM(CGM_)
    , Func(Func_)
    , context(CGM.getContext())
    , module(CGM.getModule())
    , builder(context)
{}

llvm::Function* CodeGenFunction::generateProto(const std::string& pkgname) {
    // function part
    // arguments + return type
    llvm::FunctionType *funcType;
    llvm::Type* RT = CGM.ConvertType(Func->rtype);
    if (Func->args.size() == 0) {
        funcType = llvm::FunctionType::get(RT, false);
    } else {
        std::vector<llvm::Type*> Args;
        for (unsigned int i=0; i<Func->args.size(); i++) {
            // TODO already store as DeclExpr?
            DeclExpr* de = ExprCaster<DeclExpr>::getType(Func->args[i]);
            assert(de);
            Args.push_back(CGM.ConvertType(de->getType()));
        }
        llvm::ArrayRef<llvm::Type*> argsRef(Args);
        // TODO handle ellipsis
        funcType = llvm::FunctionType::get(RT, argsRef, false);
    }
    StringBuilder buffer;
    Utils::addName(pkgname, Func->name, buffer);

    llvm::GlobalValue::LinkageTypes ltype = llvm::GlobalValue::InternalLinkage;
    if (Func->isPublic()) ltype = llvm::GlobalValue::ExternalLinkage;

    llvm::Function *func =
        llvm::Function::Create(funcType, ltype, (const char*)buffer, module);
    return func;
}

void CodeGenFunction::generateBody(llvm::Function* func) {
    // body part
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entry", func);
    builder.SetInsertPoint(entry);

    EmitStmt(*Func->body);
}

void CodeGenFunction::EmitStmt(const Stmt& S) {
    switch (S.stype()) {
    case STMT_RETURN:
        EmitReturnStmt(StmtCaster<ReturnStmt>::getType(S));
        break;
    case STMT_EXPR:
        EmitExpr(StmtCaster<Expr>::getType(S));
        break;
    case STMT_IF:
    case STMT_WHILE:
    case STMT_DO:
    case STMT_FOR:
    case STMT_SWITCH:
    case STMT_CASE:
    case STMT_DEFAULT:
    case STMT_BREAK:
    case STMT_CONTINUE:
    case STMT_LABEL:
    case STMT_GOTO:
        fprintf(stderr, "CODEGEN TODO:\n");
        S.dump();
        break;
    case STMT_COMPOUND:
        EmitCompoundStmt(StmtCaster<CompoundStmt>::getType(S));
        break;
    }
}

void CodeGenFunction::EmitCompoundStmt(const CompoundStmt* S) {
    // TODO create BasicBlock here?
    const StmtList& stmts = S->getStmts();
    for (unsigned i=0; i<stmts.size(); ++i) {
        EmitStmt(*stmts[i]);
    }
}

void CodeGenFunction::EmitReturnStmt(const ReturnStmt* S) {
    // TODO type
    // check IRBuilder::getCurrentFunctionReturnType()
    const Expr* RV = S->getExpr();
    if (RV) {
        builder.CreateRet(EmitExpr(RV));
    } else {
        builder.CreateRetVoid();
    }
}

llvm::Value* CodeGenFunction::EmitExpr(const Expr* E) {
    switch (E->etype()) {
    case EXPR_NUMBER:
        {
            NumberExpr* N = ExprCaster<NumberExpr>::getType(E);
            // TODO number is always int32 (signed is true)
            return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), N->value, true);
        }
    case EXPR_STRING:
        {
            StringExpr* S = ExprCaster<StringExpr>::getType(E);
            return builder.CreateGlobalStringPtr(S->value);
        }
    case EXPR_BOOL:
        {
            BoolLiteralExpr* B = ExprCaster<BoolLiteralExpr>::getType(E);
            return llvm::ConstantInt::get(llvm::Type::getInt1Ty(context), B->value, true);
        }
    case EXPR_CHARLITERAL:
        break;
    case EXPR_CALL:
        return EmitCallExpr(ExprCaster<CallExpr>::getType(E));
    case EXPR_IDENTIFIER:
    case EXPR_INITLIST:
    case EXPR_TYPE:
        break;
    case EXPR_DECL:
        {
            DeclExpr* D = ExprCaster<DeclExpr>::getType(E);
            // TODO arrays types?
            llvm::AllocaInst* inst = builder.CreateAlloca(CGM.ConvertType(D->getType()), 0, D->getName());
            // TODO smart alignment
            inst->setAlignment(4);
            // TODO initValue
            return inst;
        }
    case EXPR_BINOP:
    case EXPR_UNARYOP:
    case EXPR_SIZEOF:
    case EXPR_ARRAYSUBSCRIPT:
    case EXPR_MEMBER:
    case EXPR_PAREN:
        break;
    }
    assert(0 && "TODO");
    return 0;
}

llvm::Value* CodeGenFunction::EmitCallExpr(const CallExpr* E) {
    // Doesn't have to be a function, can also be funcptr symbol
    // Analyser should set whether direct call or not?
    // TODO only do below if direct call?
    // TODO for now assert IdentifierExpr;

    const Expr* Fn = E->getFn();
    IdentifierExpr* FuncName = 0;
    switch (Fn->etype()) {
    default:
        assert(0 && "TODO unsupported call type");
        return 0;
    case EXPR_IDENTIFIER:
        FuncName = ExprCaster<IdentifierExpr>::getType(Fn);
        break;
    case EXPR_MEMBER:
        {
            // NOTE: we only support pkg.symbol now (not struct.member)
            MemberExpr* M = ExprCaster<MemberExpr>::getType(Fn);
            FuncName = M->getMember();
            assert(FuncName->getPackage() && "Only support pkg.symbol for now");
        }
        break;
    };
    // TODO optimize buffer below (lots of copying)
    llvm::Function* function;
    StringBuilder fullname;
    Utils::addName(FuncName->getPackage()->getCName(), FuncName->getName(), fullname);
    if (FuncName->getPackage() == CGM.getPackage()) {       // same-package (find func)
        function = module->getFunction((const char*)fullname);
    } else {    // other package (find or generate decl)
        function = module->getFunction((const char*)fullname);
        if (!function) {
            function = CGM.createExternal(FuncName->getPackage(), FuncName->getName());
        }
    }
    assert(function && "CANNOT FIND FUNCTION");

    // NOTE: see CodeGenerator insertion of puts() and printf()
    // TODO elipsis
    llvm::Value* call = 0;
    switch (E->numArgs()) {
    case 0:
        call = builder.CreateCall(function);
        break;
    case 1:
        call = builder.CreateCall(function, EmitExpr(E->getArg(0)));
        break;
    default:
        {
            std::vector<llvm::Value *> Args;
            for (unsigned int i=0; i<E->numArgs(); i++) {
                Args.push_back(EmitExpr(E->getArg(i)));
            }
            llvm::ArrayRef<llvm::Value*> argsRef(Args);
            call = builder.CreateCall(function, argsRef);
        }
        break;
    }
    return call;
}

