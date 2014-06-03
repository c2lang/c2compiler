/* Copyright 2013,2014 Bas van den Berg
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

#ifndef CODEGEN_FUNCTION_H
#define CODEGEN_FUNCTION_H

#include <llvm/IR/IRBuilder.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/ValueHandle.h>

namespace llvm {
class Module;
class LLVMContext;
class Value;
class Function;
class Twine;
}

namespace C2 {
class CodeGenModule;
class FunctionDecl;
class Stmt;
class CompoundStmt;
class ReturnStmt;
class IfStmt;
class Expr;
class CallExpr;
class IdentifierExpr;
class VarDecl;
class BinaryOperator;
class BuiltinExpr;

// This class organizes the per-function state that is used
// while generating LLVM code.
class CodeGenFunction {
public:
    CodeGenFunction(CodeGenModule& CGM_, FunctionDecl* Func_);
    ~CodeGenFunction() {}

    llvm::Function* generateProto(const std::string& modName);
    void generateBody(llvm::Function* func);
private:
    void EmitStmt(const Stmt* S);
    void EmitCompoundStmt(const CompoundStmt* S);
    void EmitReturnStmt(const ReturnStmt* S);
    void EmitIfStmt(const IfStmt* S);

    llvm::Value* EmitExpr(const Expr* E);
    llvm::Value* EmitExprNoImpCast(const Expr* E);
    llvm::Value* EmitCallExpr(const CallExpr* E);
    llvm::Value* EmitIdentifierExpr(const IdentifierExpr* E);
    llvm::Value* EmitBuiltinExpr(const BuiltinExpr* E);
    void EmitVarDecl(const VarDecl* D);
    llvm::Value* EmitBinaryOperator(const BinaryOperator* E);

    llvm::BasicBlock* createBasicBlock(const llvm::Twine &name = "",
                                      llvm::Function* parent = 0,
                                      llvm::BasicBlock* before = 0) {
#ifdef NDEBUG
    return llvm::BasicBlock::Create(context, "", parent, before);
#else
    return llvm::BasicBlock::Create(context, name, parent, before);
#endif
    }

    /// EmitBlock - Emit the given block \arg BB and set it as the insert point,
    /// adding a fall-through branch from the current insert block if
    /// necessary. It is legal to call this function even if there is no current
    /// insertion point.
    ///
    /// IsFinished - If true, indicates that the caller has finished emitting
    /// branches to the given block and does not expect to emit code into it. This
    /// means the block can be ignored if it is unreachable.
    void EmitBlock(llvm::BasicBlock *BB, bool IsFinished=false);

    /// EmitBranch - Emit a branch to the specified basic block from the current
    /// insert block, taking care to avoid creation of branches from dummy
    /// blocks. It is legal to call this function even if there is no current
    /// insertion point.
    ///
    /// This function clears the current insertion point. The caller should follow
    /// calls to this function with calls to Emit*Block prior to generation new
    /// code.
    void EmitBranch(llvm::BasicBlock *Block);

    /// EmitBranchOnBoolExpr - Emit a branch on a boolean condition (e.g. for an
    /// if statement) to the specified blocks.  Based on the condition, this might
    /// try to simplify the codegen of the conditional based on the branch.
    void EmitBranchOnBoolExpr(const Expr *Cond, llvm::BasicBlock *TrueBlock,
                            llvm::BasicBlock *FalseBlock);

    llvm::Value* EvaluateExprAsBool(const Expr *E);

    llvm::AllocaInst* CreateTempAlloca(llvm::Type *Ty, const llvm::Twine &Name);

  /// An object to manage conditionally-evaluated expressions.
  class ConditionalEvaluation {
    llvm::BasicBlock *StartBB;

  public:
    ConditionalEvaluation(CodeGenFunction &CGF)
      : StartBB(CGF.Builder.GetInsertBlock()) {}

    void begin(CodeGenFunction &CGF) {
      assert(CGF.OutermostConditional != this);
      if (!CGF.OutermostConditional)
        CGF.OutermostConditional = this;
    }

    void end(CodeGenFunction &CGF) {
      assert(CGF.OutermostConditional != 0);
      if (CGF.OutermostConditional == this)
        CGF.OutermostConditional = 0;
    }

    /// Returns a block which will be executed prior to each
    /// evaluation of the conditional code.
    llvm::BasicBlock *getStartingBlock() const {
      return StartBB;
    }
  };


    CodeGenModule& CGM;
    FunctionDecl* FuncDecl;
    llvm::Function* CurFn;      // only set for generateBody() not generateProto()

    llvm::LLVMContext& context;
    llvm::Module* module;
    llvm::IRBuilder<> Builder;  // NOTE: do we really need to create a new builder?

    /// AllocaInsertPoint - This is an instruction in the entry block before which
    /// we prefer to insert allocas.
    llvm::AssertingVH<llvm::Instruction> AllocaInsertPt;

    /// OutermostConditional - Points to the outermost active
    /// conditional control.  This is used so that we know if a
    /// temporary should be destroyed conditionally.
    ConditionalEvaluation *OutermostConditional;

    CodeGenFunction(const CodeGenFunction&);
    CodeGenFunction& operator= (const CodeGenFunction&);
};

}

#endif

