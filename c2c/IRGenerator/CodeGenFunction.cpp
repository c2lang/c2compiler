/* Copyright 2013-2019 Bas van den Berg
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

#include <vector>
#include <string>
#include <stdio.h>

#include <llvm/IR/Module.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>

#include "IRGenerator/CodeGenFunction.h"
#include "IRGenerator/CodeGenModule.h"
#include "AST/Decl.h"
#include "AST/Expr.h"
#include "AST/Stmt.h"
#include "AST/Type.h"
#include "AST/Module.h"
#include "Utils/StringBuilder.h"
#include "Utils/GenUtils.h"

//#define GEN_DEBUG

#ifdef GEN_DEBUG
#include <iostream>
#include "Utils/color.h"
#define LOG_FUNC std::cerr << ANSI_MAGENTA << __func__ << "()" << ANSI_NORMAL << "\n";
#else
#define LOG_FUNC
#endif

using namespace C2;
using namespace llvm;
using namespace c2lang;

CodeGenFunction::CodeGenFunction(CodeGenModule& CGM_, FunctionDecl* Func_)
    : CGM(CGM_)
    , FuncDecl(Func_)
    , context(CGM.getContext())
    , module(CGM.getModule())
    , Builder(context)
    , breakContinueStack()
{}

llvm::Function* CodeGenFunction::generateProto(const std::string& modName) {
    LOG_FUNC
    // function part
    // arguments + return type
    llvm::FunctionType *funcType;
    QualType rt = FuncDecl->getReturnType();
    llvm::Type* RT = CGM.ConvertType(rt);
    if (FuncDecl->numArgs() == 0) {
        funcType = llvm::FunctionType::get(RT, FuncDecl->isVariadic());
    } else {
        std::vector<llvm::Type*> Args;
        for (unsigned i=0; i<FuncDecl->numArgs(); i++) {
            VarDecl* arg = FuncDecl->getArg(i);
            QualType qt = arg->getType();
            Args.push_back(CGM.ConvertType(qt));
        }
        llvm::ArrayRef<llvm::Type*> argsRef(Args);
        funcType = llvm::FunctionType::get(RT, argsRef, FuncDecl->isVariadic());
    }
    StringBuilder buffer;
    GenUtils::addName(modName, FuncDecl->getName(), buffer);

    llvm::GlobalValue::LinkageTypes ltype = CGM.getLinkage(FuncDecl->isPublic());
    // override for main
    if (strcmp(FuncDecl->getName(), "main") == 0) ltype = llvm::GlobalValue::ExternalLinkage;

    llvm::Function *func =
        llvm::Function::Create(funcType, ltype, (const char*)buffer, module);

    return func;
}

void CodeGenFunction::generateBody(llvm::Function* func) {
    LOG_FUNC
    CurFn = func;

    llvm::BasicBlock *EntryBB = createBasicBlock("entry", func);

    // Create a marker to make it easy to insert allocas into the entryblock
    // later.  Don't create this with the builder, because we don't want it
    // folded.
    llvm::IntegerType* Int32Ty = llvm::Type::getInt32Ty(context);
    llvm::Value *Undef = llvm::UndefValue::get(Int32Ty);
    AllocaInsertPt = new llvm::BitCastInst(Undef, Int32Ty, "", EntryBB);
    AllocaInsertPt->setName("allocapt");

    Builder.SetInsertPoint(EntryBB);

    // arguments
    Function::arg_iterator argsValues = func->arg_begin();
    for (unsigned i=0; i<FuncDecl->numArgs(); i++) {
        VarDecl* arg = FuncDecl->getArg(i);
        EmitVarDecl(arg);
        Value* argumentValue = &*argsValues;
        argsValues++;
        argumentValue->setName(arg->getName());
        new StoreInst(argumentValue, arg->getIRValue(), false, EntryBB);
    }

    // body
    CompoundStmt* Body = FuncDecl->getBody();
    EmitCompoundStmt(Body);

    llvm::BasicBlock *CurBB = Builder.GetInsertBlock();
    if (!CurBB->getTerminator()) Builder.CreateRetVoid();

    // Remove the AllocaInsertPt instruction, which is just a convenience for us.
    llvm::Instruction *Ptr = AllocaInsertPt;
    AllocaInsertPt = 0;
    Ptr->eraseFromParent();
}

void CodeGenFunction::EmitStmt(const Stmt* S) {
    LOG_FUNC
    switch (S->getKind()) {
    case STMT_RETURN:
        EmitReturnStmt(cast<ReturnStmt>(S));
        break;
    case STMT_EXPR:
        EmitExpr(cast<Expr>(S));
        break;
    case STMT_IF:
        EmitIfStmt(cast<IfStmt>(S));
        break;
    case STMT_FOR:
        EmitForStmt(cast<ForStmt>(S));
        break;
    case STMT_WHILE:
    case STMT_DO:
    case STMT_SWITCH:
    case STMT_MATCH:
    case STMT_CASE:
    case STMT_DEFAULT:
    case STMT_BREAK:
    case STMT_CONTINUE:
    case STMT_LABEL:
    case STMT_GOTO:
        S->dump();
        TODO;
        break;
    case STMT_COMPOUND:
        EmitCompoundStmt(cast<CompoundStmt>(S));
        break;
    case STMT_DECL:
        EmitVarDecl(cast<DeclStmt>(S)->getDecl());
        break;
    case STMT_ASM:
        S->dump();
        TODO;
        break;
    }
}

void CodeGenFunction::EmitCompoundStmt(const CompoundStmt* S) {
    LOG_FUNC
    // TODO create BasicBlock here?
    Stmt** stmts = S->getStmts();
    for (unsigned i=0; i<S->numStmts(); i++) {
        EmitStmt(stmts[i]);
    }
}

void CodeGenFunction::EmitReturnStmt(const ReturnStmt* S) {
    LOG_FUNC
    // TODO type
    // check IRBuilder::getCurrentFunctionReturnType()

    const Expr* RV = S->getExpr();
    ReturnInst* R;
    if (RV) {
        R = Builder.CreateRet(EmitExpr(RV));
    } else {
        R = Builder.CreateRetVoid();
    }
    //CGM.setCurrentReturnValue(R);
}

void CodeGenFunction::EmitIfStmt(const IfStmt* S) {
    LOG_FUNC
    // C99 6.8.4.1: The first substatement is executed if the expression compares
    // unequal to 0.  The condition must be a scalar type.
    //RunCleanupsScope ConditionScope(*this);

    if (S->getConditionVariable()) {
        TODO;
        //EmitAutoVarDecl(S->getConditionVariable());
    }

    // If the condition constant folds and can be elided, try to avoid emitting
    // the condition and the dead arm of the if/else.
#if 0
    bool CondConstant;
    if (ConstantFoldsToSimpleInteger(S.getCond(), CondConstant)) {
        // Figure out which block (then or else) is executed.
        const Stmt *Executed = S.getThen();
        const Stmt *Skipped  = S.getElse();
        if (!CondConstant)  // Condition false?
            std::swap(Executed, Skipped);

        // If the skipped block has no labels in it, just emit the executed block.
        // This avoids emitting dead code and simplifies the CFG substantially.
        if (!ContainsLabel(Skipped)) {
            if (Executed) {
                RunCleanupsScope ExecutedScope(*this);
                EmitStmt(Executed);
            }
            return;
        }
    }
#endif

    // Otherwise, the condition did not fold, or we couldn't elide it.  Just emit
    // the conditional branch.
    llvm::BasicBlock *ThenBlock = createBasicBlock("if.then");
    llvm::BasicBlock *ContBlock = createBasicBlock("if.end");
    llvm::BasicBlock *ElseBlock = ContBlock;
    if (S->getElse())
        ElseBlock = createBasicBlock("if.else");
    EmitBranchOnBoolExpr(cast<Expr>(S->getCond()), ThenBlock, ElseBlock);

    // Emit the 'then' code.
    EmitBlock(ThenBlock);
    {
        //RunCleanupsScope ThenScope(*this);
        EmitStmt(S->getThen());
    }
    EmitBranch(ContBlock);

    // Emit the 'else' code if present.
    if (const Stmt *Else = S->getElse()) {
        // There is no need to emit line number for unconditional branch.
        //if (getDebugInfo())
        //  Builder.SetCurrentDebugLocation(llvm::DebugLoc());
        EmitBlock(ElseBlock);
        {
            //RunCleanupsScope ElseScope(*this);
            EmitStmt(Else);
        }
        // There is no need to emit line number for unconditional branch.
        //if (getDebugInfo())
        //  Builder.SetCurrentDebugLocation(llvm::DebugLoc());
        EmitBranch(ContBlock);
    }

    // Emit the continuation block for code after the if.
    EmitBlock(ContBlock, true);
}

void CodeGenFunction::EmitForStmt(const ForStmt *S) {
    LOG_FUNC
    TODO;
}

void CodeGenFunction::EmitBlock(llvm::BasicBlock *BB, bool IsFinished) {
    LOG_FUNC
    llvm::BasicBlock *CurBB = Builder.GetInsertBlock();

    // Fall out of the current block (if necessary).
    EmitBranch(BB);

    if (IsFinished && BB->use_empty()) {
        delete BB;
        return;
    }

    // Place the block after the current block, if possible, or else at
    // the end of the function.
    if (CurBB && CurBB->getParent()) {
        CurFn->getBasicBlockList().insertAfter(CurBB->getIterator(), BB);
    } else {
        CurFn->getBasicBlockList().push_back(BB);
    }
    Builder.SetInsertPoint(BB);
}

void CodeGenFunction::EmitBranch(llvm::BasicBlock *Target) {
    LOG_FUNC
    // Emit a branch from the current block to the target one if this
    // was a real block.  If this was just a fall-through block after a
    // terminator, don't emit it.
    llvm::BasicBlock *CurBB = Builder.GetInsertBlock();

    if (!CurBB || CurBB->getTerminator()) {
        // If there is no insert point or the previous block is already
        // terminated, don't touch it.
    } else {
        // Otherwise, create a fall-through branch.
        Builder.CreateBr(Target);
    }

    Builder.ClearInsertionPoint();
}

/// EmitBranchOnBoolExpr - Emit a branch on a boolean condition (e.g. for an if
/// statement) to the specified blocks.  Based on the condition, this might try
/// to simplify the codegen of the conditional based on the branch.
///
void CodeGenFunction::EmitBranchOnBoolExpr(const Expr *Cond,
        llvm::BasicBlock *TrueBlock,
        llvm::BasicBlock *FalseBlock) {
    LOG_FUNC
    //Cond = Cond->IgnoreParens();
    if (const BinaryOperator *CondBOp = cast<BinaryOperator>(Cond)) {
        // Handle X && Y in a condition.
        if (CondBOp->getOpcode() == BINOP_LAnd) {
#if 0
            // If we have "1 && X", simplify the code.  "0 && X" would have constant
            // folded if the case was simple enough.
            bool ConstantBool = false;
            if (ConstantFoldsToSimpleInteger(CondBOp->getLHS(), ConstantBool) &&
                    ConstantBool) {
                // br(1 && X) -> br(X).
                return EmitBranchOnBoolExpr(CondBOp->getRHS(), TrueBlock, FalseBlock);
            }

            // If we have "X && 1", simplify the code to use an uncond branch.
            // "X && 0" would have been constant folded to 0.
            if (ConstantFoldsToSimpleInteger(CondBOp->getRHS(), ConstantBool) &&
                    ConstantBool) {
                // br(X && 1) -> br(X).
                return EmitBranchOnBoolExpr(CondBOp->getLHS(), TrueBlock, FalseBlock);
            }
#endif

            // Emit the LHS as a conditional.  If the LHS conditional is false, we
            // want to jump to the FalseBlock.
            llvm::BasicBlock *LHSTrue = createBasicBlock("land.lhs.true");

            ConditionalEvaluation eval(*this);
            EmitBranchOnBoolExpr(CondBOp->getLHS(), LHSTrue, FalseBlock);
            EmitBlock(LHSTrue);

            // Any temporaries created here are conditional.
            eval.begin(*this);
            EmitBranchOnBoolExpr(CondBOp->getRHS(), TrueBlock, FalseBlock);
            eval.end(*this);

            return;
        }
        if (CondBOp->getOpcode() == BINOP_LOr) {
#if 0
            // If we have "0 || X", simplify the code.  "1 || X" would have constant
            // folded if the case was simple enough.
            bool ConstantBool = false;
            if (ConstantFoldsToSimpleInteger(CondBOp->getLHS(), ConstantBool) &&
                    !ConstantBool) {
                // br(0 || X) -> br(X).
                return EmitBranchOnBoolExpr(CondBOp->getRHS(), TrueBlock, FalseBlock);
            }

            // If we have "X || 0", simplify the code to use an uncond branch.
            // "X || 1" would have been constant folded to 1.
            if (ConstantFoldsToSimpleInteger(CondBOp->getRHS(), ConstantBool) &&
                    !ConstantBool) {
                // br(X || 0) -> br(X).
                return EmitBranchOnBoolExpr(CondBOp->getLHS(), TrueBlock, FalseBlock);
            }
#endif

            // Emit the LHS as a conditional.  If the LHS conditional is true, we
            // want to jump to the TrueBlock.
            llvm::BasicBlock *LHSFalse = createBasicBlock("lor.lhs.false");

            ConditionalEvaluation eval(*this);
            EmitBranchOnBoolExpr(CondBOp->getLHS(), TrueBlock, LHSFalse);
            EmitBlock(LHSFalse);

            // Any temporaries created here are conditional.
            eval.begin(*this);
            EmitBranchOnBoolExpr(CondBOp->getRHS(), TrueBlock, FalseBlock);
            eval.end(*this);

            return;
        }
    }
#if 0
    if (const UnaryOperator *CondUOp = dyn_cast<UnaryOperator>(Cond)) {
        // br(!x, t, f) -> br(x, f, t)
        if (CondUOp->getOpcode() == UO_LNot)
            return EmitBranchOnBoolExpr(CondUOp->getSubExpr(), FalseBlock, TrueBlock);
    }
#endif

#if 0
    if (const ConditionalOperator *CondOp = dyn_cast<ConditionalOperator>(Cond)) {
        // br(c ? x : y, t, f) -> br(c, br(x, t, f), br(y, t, f))
        llvm::BasicBlock *LHSBlock = createBasicBlock("cond.true");
        llvm::BasicBlock *RHSBlock = createBasicBlock("cond.false");

        ConditionalEvaluation cond(*this);
        EmitBranchOnBoolExpr(CondOp->getCond(), LHSBlock, RHSBlock);

        cond.begin(*this);
        EmitBlock(LHSBlock);
        EmitBranchOnBoolExpr(CondOp->getLHS(), TrueBlock, FalseBlock);
        cond.end(*this);

        cond.begin(*this);
        EmitBlock(RHSBlock);
        EmitBranchOnBoolExpr(CondOp->getRHS(), TrueBlock, FalseBlock);
        cond.end(*this);

        return;
    }
#endif

    // Emit the code with the fully general case.
    llvm::Value *CondV = EvaluateExprAsBool(Cond);
    Builder.CreateCondBr(CondV, TrueBlock, FalseBlock);
}

llvm::Value* CodeGenFunction::EmitExpr(const Expr* E) {
    LOG_FUNC
    llvm::Value* V = EmitExprNoImpCast(E);
    if (E->hasImpCast()) {
        // TODO correct InputSigned
        bool InputSigned = true;
        V = Builder.CreateIntCast(V, CGM.ConvertType(E->getImpCast()), InputSigned, "conv");
    }
    return V;
}

llvm::Value *CodeGenFunction::castIfNecessary(llvm::Type *targetType, const QualType &astTargetType, llvm::Value *value, const QualType &valueType) {

    llvm::Type *currType = value->getType();
    if (targetType == currType)
    {
        return value;
    }
    if (targetType->isFloatingPointTy())
    {
        if (currType->isIntegerTy()) {
            return valueType.isUnsignedType()
                   ? Builder.CreateUIToFP(value, targetType, "uitofp-cast")
                   : Builder.CreateSIToFP(value, targetType, "sitofp-cast");
        }
        if (currType->isFloatingPointTy()) {
            if (currType->getPrimitiveSizeInBits() >= targetType->getPrimitiveSizeInBits()) return value;
            return Builder.CreateFPExt(value, targetType, "fpext-cast");
        }
        FATAL_ERROR("Unexpected type!");
    }
    if (targetType->isIntegerTy())
    {
        if (currType->isIntegerTy()) {
            if (currType->getPrimitiveSizeInBits() > targetType->getPrimitiveSizeInBits())
            {
                return value;
            }
            return Builder.CreateZExt(value, targetType, "extend");
        }
        // Float "wins"
        if (currType->isFloatingPointTy()) return value;
    }
    FATAL_ERROR("Unexpected type!");
}



void CodeGenFunction::promoteCast(llvm::Value **lhs, const QualType &lhsType, llvm::Value **rhs, const QualType &rhsType)
{
    llvm::Type *rType = (*rhs)->getType();
    llvm::Type *lType = (*lhs)->getType();
    *rhs = castIfNecessary(lType, lhsType, *rhs, rhsType);
    *lhs = castIfNecessary(rType, rhsType, *lhs, lhsType);
}


llvm::Value* CodeGenFunction::EmitExprNoImpCast(const Expr* E) {
    LOG_FUNC

    switch (E->getKind()) {
    case EXPR_INTEGER_LITERAL:
    {
        const IntegerLiteral* N = cast<IntegerLiteral>(E);
        const Type* T = N->getType().getTypePtr();
        assert(T->isBuiltinType());
        const BuiltinType* BT = cast<BuiltinType>(T);
        uint64_t v = N->Value.getZExtValue();
        return llvm::ConstantInt::get(CGM.ConvertType(N->getType()), v, BT->isSignedInteger());
    }
    case EXPR_FLOAT_LITERAL:
    {
        const FloatingLiteral* F = cast<FloatingLiteral>(E);
        return llvm::ConstantFP::get(context, F->Value);
    }
    case EXPR_BOOL_LITERAL:
    {
        const BooleanLiteral* B = cast<BooleanLiteral>(E);
        return llvm::ConstantInt::get(llvm::Type::getInt1Ty(context), B->getValue(), true);
    }
    case EXPR_CHAR_LITERAL:
    {
        const CharacterLiteral* C = cast<CharacterLiteral>(E);
        return llvm::ConstantInt::get(CGM.ConvertType(E->getType()), C->getValue());
    }
    case EXPR_STRING_LITERAL:
    {
        const StringLiteral* S = cast<StringLiteral>(E);
        return Builder.CreateGlobalStringPtr(S->getValue());
    }
    case EXPR_NIL:
        fprintf(stderr, "WARNING: implicit casts missing for NilExpr! - invalid IR\n"); //TODO
        return llvm::ConstantPointerNull::get(CGM.getVoidPtrType());
    case EXPR_CALL:
        return EmitCallExpr(cast<CallExpr>(E));
    case EXPR_IDENTIFIER:
        return EmitIdentifierExpr(cast<IdentifierExpr>(E));
    case EXPR_INITLIST:
        E->dump();
        TODO;
    case EXPR_DESIGNATOR_INIT:
        E->dump();
        TODO;
    case EXPR_TYPE:
        break;
    case EXPR_BINOP:
        return EmitBinaryOperator(cast<BinaryOperator>(E));
    case EXPR_CONDOP:
        E->dump();
        TODO;
    case EXPR_UNARYOP:
        return EmitUnaryOperator(cast<UnaryOperator>(E));
    case EXPR_BUILTIN:
        return EmitBuiltinExpr(cast<BuiltinExpr>(E));
    case EXPR_ARRAYSUBSCRIPT:
        break;
    case EXPR_MEMBER:
    {
        TODO;
        assert(cast<MemberExpr>(E)->isModulePrefix() && "TODO not-prefix members");
        E->dump();
        TODO;
        break;
    }
    case EXPR_PAREN:
    {
        const ParenExpr* P = cast<ParenExpr>(E);
        return EmitExpr(P->getExpr());
    }
    case EXPR_BITOFFSET:
        E->dump();
        TODO;
        break;
    case EXPR_CAST:
    E->dump();
        TODO;
        break;
    }
    TODO;
    return 0;
}

llvm::Value* CodeGenFunction::EmitCallExpr(const CallExpr* E) {
    LOG_FUNC
    // Doesn't have to be a function, can also be funcptr symbol
    // Analyser should set whether direct call or not?
    // TODO only do below if direct call?
    // TODO for now assert IdentifierExpr;

    const Expr* Fn = E->getFn();
    const Decl* FD = 0;
    std::string FuncName;
    switch (Fn->getKind()) {
    case EXPR_IDENTIFIER:
    {
        const IdentifierExpr* I = cast<IdentifierExpr>(Fn);
        FD = I->getDecl();
        FuncName = I->getName();
    }
    break;
    case EXPR_MEMBER:
    {
        // NOTE: we only support module.symbol now (not struct.member)
        // So only FunctionDecl
        const MemberExpr* M = cast<MemberExpr>(Fn);
        const IdentifierExpr* rhs = M->getMember();

        FuncName = rhs->getName();
        FD = M->getDecl();
        assert(FD->getKind() == DECL_FUNC && "Only support module.symbol for now");
    }
    break;
    default:
        printf("TODO unsupported call type\n");
        TODO;
        return 0;
    };
    // TODO optimize buffer below (lots of copying)
    llvm::Function* function;
    StringBuilder fullname;
    GenUtils::addName(FD->getModule()->getCName(), FuncName, fullname);
    // TODO FIX THIS for single module!
    if (FD->getModule()->getName() == CGM.getName()) {       // same-module (find func)
        function = module->getFunction((const char*)fullname);
    } else {    // other module (find or generate decl)
        function = module->getFunction((const char*)fullname);
        if (!function) {
            function = CGM.createExternal(FD->getModule(), FuncName);
        }
    }
    assert(function && "CANNOT FIND FUNCTION");

    // NOTE: see CodeGenerator insertion of puts() and printf()
    // TODO elipsis
    llvm::Value* call = 0;
    switch (E->numArgs()) {
    case 0:
        call = Builder.CreateCall(function);
        break;
    case 1:
    {
        // TEMP only handle implicit casts in this case
        const Expr* ArgExpr = E->getArg(0);
        Value* arg = EmitExpr(ArgExpr);
        if (ArgExpr->hasImpCast()) {
            Type* DestTy = BuiltinType::get(ArgExpr->getImpCast());
            arg = Builder.CreateBitCast(arg, CGM.ConvertType(DestTy));
        }
        call = Builder.CreateCall(function, arg);
        break;
    }
    default:
    {
        std::vector<llvm::Value *> Args;
        for (unsigned i=0; i<E->numArgs(); i++) {
            // TODO match argument to callee's arg type
            Args.push_back(EmitExpr(E->getArg(i)));
        }
        llvm::ArrayRef<llvm::Value*> argsRef(Args);
        call = Builder.CreateCall(function, argsRef);
    }
    break;
    }
    return call;
}

llvm::Value* CodeGenFunction::EmitIdentifierExpr(const IdentifierExpr* E) {
    LOG_FUNC
    Decl* D = E->getDecl();
    assert(D && "Declaration missing");
    switch (D->getKind()) {
    case DECL_FUNC:
        FATAL_ERROR("Unexpected declaration");
        break;
    case DECL_VAR:
    {
        VarDecl* VD = cast<VarDecl>(D);
        llvm::BasicBlock *CurBB = Builder.GetInsertBlock();
        assert(CurBB);
        // for RHS:
        //assert(VD->getIRValue());
        //return VD->getIRValue();
        // for LHS:
        return new LoadInst(VD->getIRValue(), "", false, CurBB);
    }
    case DECL_ENUMVALUE:
    {
        //EnumConstantDecl* ECD = cast<EnumConstantDecl>(D);
        // Nasty, we need the value of the constant, but we have no place to store it.
        // We need to change the AST structure for this.
        // TODO correct width + value
        int value = 5;
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), value, true);
    }
    case DECL_ALIASTYPE:
    case DECL_STRUCTTYPE:
    case DECL_ENUMTYPE:
    case DECL_FUNCTIONTYPE:
    case DECL_ARRAYVALUE:
    case DECL_IMPORT:
    case DECL_LABEL:
        break;
    }
    TODO;
    return 0;
}

llvm::Value* CodeGenFunction::EmitBuiltinExpr(const BuiltinExpr* E) {
    uint64_t v = E->getValue().getZExtValue();
    return llvm::ConstantInt::get(CGM.ConvertType(E->getType()), v, true);
}

void CodeGenFunction::EmitVarDecl(const VarDecl* D) {
    LOG_FUNC
    QualType qt = D->getType();
    StringBuilder name(64);
    name << D->getName();
    if (D->isParameter()) name << ".addr";
    llvm::AllocaInst *inst = CreateTempAlloca(CGM.ConvertType(qt), (const char*)name);
    D->setIRValue(inst);

    // set alignment
    if (!D->isParameter()) inst->setAlignment(CGM.getAlignment(qt));

    const Expr* I = D->getInitValue();
    // NOTE: for function params, we do this during Body generation
    if (I && !D->isParameter()) {
        llvm::Value* val = EmitExpr(I);
        Builder.CreateStore(val, inst, qt.isVolatileQualified());
    }
}



llvm::Value* CodeGenFunction::EmitBinaryOperator(const BinaryOperator* B) {
    LOG_FUNC
    Value* L = EmitExpr(B->getLHS());
    Value* R = EmitExpr(B->getRHS());
    if (L == 0 || R == 0) return 0;

    // Note that I'm not sure whether this is good.
    // Basically at this point the AST still code lite i32-expr * f32-expr
    // Instead of doing a (desperate) cast here, The AST
    // should have generated all implicit casts already.
    // At that point this method will completely go away.
    promoteCast(&L, B->getLHS()->getType(), &R, B->getRHS()->getType());

    // At this point this is not quite correct due to implicits casts not done in the AST.
    bool is_float = B->getLHS()->getType().isFloatType();
    bool is_unsigned_int = !is_float && B->getLHS()->getType().isUnsignedType();

    switch (B->getOpcode()) {
    case BINOP_Mul:
        return is_float ? Builder.CreateFMul(L, R, "") : Builder.CreateMul(L, R, "");
    case BINOP_Div:
        if (is_float) return Builder.CreateFDiv(L, R, "");
        return is_unsigned_int ? Builder.CreateUDiv(L, R, "") : Builder.CreateSDiv(L, R, "");
    case BINOP_Rem:
        if (is_float) return Builder.CreateFRem(L, R, "");
        return is_unsigned_int ? Builder.CreateURem(L, R, "") : Builder.CreateSRem(L, R, "");
    case BINOP_Add:
        return is_float
               ? Builder.CreateFAdd(L, R, "fadd")
               : Builder.CreateAdd(L, R, "add");
    case BINOP_Sub:
        return is_float
               ? Builder.CreateFSub(L, R, "fsub")
               : Builder.CreateSub(L, R, "sub");
    case BINOP_Shl:
        assert(!is_float);
        return Builder.CreateShl(L, R, "");
    case BINOP_Shr:
        assert(!is_float);
        return is_unsigned_int ? Builder.CreateLShr(L, R, "") : Builder.CreateAShr(L, R, "");
    case BINOP_LT:
        if (is_float) return Builder.CreateFCmpOLT(L, R, "");
        return is_unsigned_int ? Builder.CreateICmpULT(L, R) : Builder.CreateICmpSLT(L, R);
    case BINOP_GT:
        if (is_float) return Builder.CreateFCmpOGT(L, R, "");
        return is_unsigned_int ? Builder.CreateICmpUGT(L, R) : Builder.CreateICmpSGT(L, R);
    case BINOP_LE:
        if (is_float) return Builder.CreateFCmpOLE(L, R, "");
        return is_unsigned_int ? Builder.CreateICmpULE(L, R) : Builder.CreateICmpSLE(L, R);
    case BINOP_GE:
        if (is_float) return Builder.CreateFCmpOGE(L, R, "");
        return is_unsigned_int ? Builder.CreateICmpUGE(L, R) : Builder.CreateICmpSGE(L, R);
    case BINOP_EQ:
        return is_float ? Builder.CreateFCmpOEQ(L, R, "") : Builder.CreateICmpEQ(L, R);
    case BINOP_NE:
        return is_float ? Builder.CreateFCmpONE(L, R, "") : Builder.CreateICmpNE(L, R);
    case BINOP_And:
        assert(!is_float);
        return Builder.CreateAnd(L, R, "and");
    case BINOP_Xor:
        return Builder.CreateXor(L, R, "xor");
    case BINOP_Or:
        return Builder.CreateOr(L, R, "or");
    case BINOP_LAnd:
    case BINOP_LOr:
        TODO;
        break;
    case BINOP_Assign:
    {
        // TODO correct volatileQualified
        return Builder.CreateStore(R, L, false);
        //return Builder.CreateStore(R, L, qt.isVolatileQualified());
    }
    case BINOP_MulAssign:
    case BINOP_DivAssign:
    case BINOP_RemAssign:
    case BINOP_AddAssign:
    case BINOP_SubAssign:
    case BINOP_ShlAssign:
    case BINOP_ShrAssign:
    case BINOP_AndAssign:
    case BINOP_XorAssign:
    case BINOP_OrAssign:
    case BINOP_Comma:
        TODO;
        break;
    }
    B->dump();
    TODO;
    return 0;
}

llvm::Value* CodeGenFunction::EmitUnaryOperator(const UnaryOperator* unaryOperator) {
    LOG_FUNC
    Expr *expression = unaryOperator->getExpr();

    // At this point this is not quite correct due to implicits casts not done in the AST yet.
    bool is_float = expression->getType().isFloatType();

    switch (unaryOperator->getOpcode()) {
        case UO_Not:
            return Builder.CreateNot(EmitExpr(expression), "not");
        case UO_Minus:
            assert(!is_unsigned_int);
            return is_float
                ? Builder.CreateFNeg(EmitExpr(expression), "fneg")
                : Builder.CreateNeg(EmitExpr(expression), "neg");
        case UO_LNot:
        {
            Value *rhs = EmitExpr(expression);
            return is_float
                   ? Builder.CreateFCmp(CmpInst::Predicate::FCMP_OEQ, rhs,
                                        llvm::ConstantFP::get(rhs->getType(), 0.0))
                   : Builder.CreateICmp(CmpInst::Predicate::ICMP_EQ, EmitExpr(expression),
                                       llvm::ConstantInt::get(rhs->getType(), 0, false));

        }
        case UO_Deref:break;
        case UO_PostInc:break;
        case UO_PostDec:break;
        case UO_PreInc:break;
        case UO_PreDec:break;
        case UO_AddrOf: break;
    }
    TODO;
}

llvm::Value *CodeGenFunction::EvaluateExprAsBool(const Expr *E) {
    if (const IntegerLiteral* N = dyncast<IntegerLiteral>(E)) {
        return llvm::ConstantInt::get(llvm::Type::getInt1Ty(context), N->Value.getSExtValue(), true);
    }

    if (const IdentifierExpr* I = dyncast<IdentifierExpr>(E)) {
        // TODO for now only support identifier to base type
        Decl* D = I->getDecl();
        assert(D && "No declaration found");
        assert(D->getKind() == DECL_VAR && "TODO only support ref to vardecl, need anything else?");
        VarDecl* VD = cast<VarDecl>(D);

        llvm::BasicBlock *CurBB = Builder.GetInsertBlock();
        assert(CurBB);
        Value* load = new LoadInst(VD->getIRValue(), "", false, CurBB);
        return Builder.CreateIsNotNull(load, "tobool");
    }
    if (const BinaryOperator* BinOp = dyncast<BinaryOperator>(E)) {
        return EmitBinaryOperator(BinOp);
        // TODO convert to Bool if not already
    }
    TODO;
    return 0;
}

/// CreateTempAlloca - This creates a alloca and inserts it into the entry
/// block.
llvm::AllocaInst *CodeGenFunction::CreateTempAlloca(llvm::Type *Ty,
        const Twine &Name) {
    return new llvm::AllocaInst(Ty, 0, Name, AllocaInsertPt);
}




