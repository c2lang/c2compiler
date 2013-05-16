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
    , FuncDecl(Func_)
    , context(CGM.getContext())
    , module(CGM.getModule())
    , Builder(context)
{}

llvm::Function* CodeGenFunction::generateProto(const std::string& pkgname) {
    // function part
    // arguments + return type
    llvm::FunctionType *funcType;
    llvm::Type* RT = CGM.ConvertType(FuncDecl->rtype);
    if (FuncDecl->args.size() == 0) {
        funcType = llvm::FunctionType::get(RT, false);
    } else {
        std::vector<llvm::Type*> Args;
        for (unsigned int i=0; i<FuncDecl->args.size(); i++) {
            // TODO already store as DeclExpr?
            DeclExpr* de = ExprCaster<DeclExpr>::getType(FuncDecl->args[i]);
            assert(de);
            Args.push_back(CGM.ConvertType(de->getType()));
        }
        llvm::ArrayRef<llvm::Type*> argsRef(Args);
        // TODO handle ellipsis
        funcType = llvm::FunctionType::get(RT, argsRef, false);
    }
    StringBuilder buffer;
    Utils::addName(pkgname, FuncDecl->name, buffer);

    llvm::GlobalValue::LinkageTypes ltype = llvm::GlobalValue::InternalLinkage;
    if (FuncDecl->isPublic()) ltype = llvm::GlobalValue::ExternalLinkage;

    llvm::Function *func =
        llvm::Function::Create(funcType, ltype, (const char*)buffer, module);
    return func;
}

void CodeGenFunction::generateBody(llvm::Function* func) {
    CurFn = func;
    // body part
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entry", func);
    Builder.SetInsertPoint(entry);

    EmitStmt(FuncDecl->body);
}

void CodeGenFunction::EmitStmt(const Stmt* S) {
    switch (S->stype()) {
    case STMT_RETURN:
        EmitReturnStmt(StmtCaster<ReturnStmt>::getType(S));
        break;
    case STMT_EXPR:
        EmitExpr(StmtCaster<Expr>::getType(S));
        break;
    case STMT_IF:
        EmitIfStmt(StmtCaster<IfStmt>::getType(S));
        break;
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
        S->dump();
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
        EmitStmt(stmts[i]);
    }
}

void CodeGenFunction::EmitReturnStmt(const ReturnStmt* S) {
    // TODO type
    // check IRBuilder::getCurrentFunctionReturnType()
    const Expr* RV = S->getExpr();
    if (RV) {
        Builder.CreateRet(EmitExpr(RV));
    } else {
        Builder.CreateRetVoid();
    }
}

void CodeGenFunction::EmitIfStmt(const IfStmt* S) {
  // C99 6.8.4.1: The first substatement is executed if the expression compares
  // unequal to 0.  The condition must be a scalar type.
  //RunCleanupsScope ConditionScope(*this);

  //if (S.getConditionVariable())
  //  EmitAutoVarDecl(*S.getConditionVariable());

  // If the condition constant folds and can be elided, try to avoid emitting
  // the condition and the dead arm of the if/else.
  bool CondConstant;
#if 0
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
  EmitBranchOnBoolExpr(S->getCond(), ThenBlock, ElseBlock);

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

void CodeGenFunction::EmitBlock(llvm::BasicBlock *BB, bool IsFinished) {
  llvm::BasicBlock *CurBB = Builder.GetInsertBlock();

  // Fall out of the current block (if necessary).
  EmitBranch(BB);

  if (IsFinished && BB->use_empty()) {
    delete BB;
    return;
  }

  // Place the block after the current block, if possible, or else at
  // the end of the function.
  if (CurBB && CurBB->getParent())
    CurFn->getBasicBlockList().insertAfter(CurBB, BB);
  else
    CurFn->getBasicBlockList().push_back(BB);
  Builder.SetInsertPoint(BB);
}

void CodeGenFunction::EmitBranch(llvm::BasicBlock *Target) {
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
  //Cond = Cond->IgnoreParens();
  if (const BinaryOperator *CondBOp = ExprCaster<BinaryOperator>::getType(Cond)) {
    // Handle X && Y in a condition.
    if (CondBOp->getOpcode() == BO_LAnd) {
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
    if (CondBOp->getOpcode() == BO_LOr) {
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
            return Builder.CreateGlobalStringPtr(S->value);
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
            EmitVarDecl(D);
            return 0;
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
        call = Builder.CreateCall(function);
        break;
    case 1:
        call = Builder.CreateCall(function, EmitExpr(E->getArg(0)));
        break;
    default:
        {
            std::vector<llvm::Value *> Args;
            for (unsigned int i=0; i<E->numArgs(); i++) {
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

void CodeGenFunction::EmitVarDecl(const DeclExpr* D) {
    // TODO arrays types?
    llvm::AllocaInst* inst = Builder.CreateAlloca(CGM.ConvertType(D->getType()), 0, D->getName());
    // TODO smart alignment
    inst->setAlignment(D->getType()->getWidth());
    // TODO initValue
    const Expr* I = D->getInitValue();
    if (I) {
        //llvm::Value* val = EmitExpr(I);
        //llvm::Constant* Init= CGM.EmitConstantValueFor(I, D->getType(), this);
        //llvm::Value* Loc;
        //Builder.createStore(Init, Loc, false);
    }
}

/// EvaluateExprAsBool - Perform the usual unary conversions on the specified
/// expression and compare the result against zero, returning an Int1Ty value.
llvm::Value *CodeGenFunction::EvaluateExprAsBool(const Expr *E) {
    // NOTE: for now only support numbers and convert those to bools
    NumberExpr* N = ExprCaster<NumberExpr>::getType(E);
    assert(N && "Only support constants for now");
    return llvm::ConstantInt::get(llvm::Type::getInt1Ty(context), N->value, true);

#if 0
  if (const MemberPointerType *MPT = E->getType()->getAs<MemberPointerType>()) {
    llvm::Value *MemPtr = EmitScalarExpr(E);
    return CGM.getCXXABI().EmitMemberPointerIsNotNull(*this, MemPtr, MPT);
  }

  QualType BoolTy = getContext().BoolTy;
  if (!E->getType()->isAnyComplexType())
    return EmitScalarConversion(EmitScalarExpr(E), E->getType(), BoolTy);

  return EmitComplexToScalarConversion(EmitComplexExpr(E), E->getType(),BoolTy);
#endif
}

