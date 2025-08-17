//===--- InterpState.h - Interpreter state for the constexpr VM -*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
// Definition of the interpreter state and entry point.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_INTERP_INTERPSTATE_H
#define LANGUAGE_CORE_AST_INTERP_INTERPSTATE_H

#include "Context.h"
#include "DynamicAllocator.h"
#include "Floating.h"
#include "Function.h"
#include "InterpFrame.h"
#include "InterpStack.h"
#include "State.h"
#include "language/Core/AST/APValue.h"
#include "language/Core/AST/ASTDiagnostic.h"
#include "language/Core/AST/Expr.h"
#include "language/Core/AST/OptionalDiagnostic.h"

namespace language::Core {
namespace interp {
class Context;
class Function;
class InterpStack;
class InterpFrame;
class SourceMapper;

struct StdAllocatorCaller {
  const Expr *Call = nullptr;
  QualType AllocType;
  explicit operator bool() { return Call; }
};

/// Interpreter context.
class InterpState final : public State, public SourceMapper {
public:
  InterpState(State &Parent, Program &P, InterpStack &Stk, Context &Ctx,
              SourceMapper *M = nullptr);
  InterpState(State &Parent, Program &P, InterpStack &Stk, Context &Ctx,
              const Function *Func);

  ~InterpState();

  void cleanup();

  InterpState(const InterpState &) = delete;
  InterpState &operator=(const InterpState &) = delete;

  bool diagnosing() const { return getEvalStatus().Diag != nullptr; }

  // Stack frame accessors.
  Frame *getSplitFrame() { return Parent.getCurrentFrame(); }
  Frame *getCurrentFrame() override;
  unsigned getCallStackDepth() override {
    return Current ? (Current->getDepth() + 1) : 1;
  }
  const Frame *getBottomFrame() const override {
    return Parent.getBottomFrame();
  }

  // Access objects from the walker context.
  Expr::EvalStatus &getEvalStatus() const override {
    return Parent.getEvalStatus();
  }
  ASTContext &getASTContext() const override { return Parent.getASTContext(); }

  // Forward status checks and updates to the walker.
  bool checkingForUndefinedBehavior() const override {
    return Parent.checkingForUndefinedBehavior();
  }
  bool keepEvaluatingAfterFailure() const override {
    return Parent.keepEvaluatingAfterFailure();
  }
  bool keepEvaluatingAfterSideEffect() const override {
    return Parent.keepEvaluatingAfterSideEffect();
  }
  bool checkingPotentialConstantExpression() const override {
    return Parent.checkingPotentialConstantExpression();
  }
  bool noteUndefinedBehavior() override {
    return Parent.noteUndefinedBehavior();
  }
  bool inConstantContext() const;
  bool hasActiveDiagnostic() override { return Parent.hasActiveDiagnostic(); }
  void setActiveDiagnostic(bool Flag) override {
    Parent.setActiveDiagnostic(Flag);
  }
  void setFoldFailureDiagnostic(bool Flag) override {
    Parent.setFoldFailureDiagnostic(Flag);
  }
  bool hasPriorDiagnostic() override { return Parent.hasPriorDiagnostic(); }
  bool noteSideEffect() override { return Parent.noteSideEffect(); }

  /// Reports overflow and return true if evaluation should continue.
  bool reportOverflow(const Expr *E, const toolchain::APSInt &Value);

  /// Deallocates a pointer.
  void deallocate(Block *B);

  /// Delegates source mapping to the mapper.
  SourceInfo getSource(const Function *F, CodePtr PC) const override {
    if (M)
      return M->getSource(F, PC);

    assert(F && "Function cannot be null");
    return F->getSource(PC);
  }

  Context &getContext() const { return Ctx; }

  void setEvalLocation(SourceLocation SL) { this->EvalLocation = SL; }

  DynamicAllocator &getAllocator() { return Alloc; }

  /// Diagnose any dynamic allocations that haven't been freed yet.
  /// Will return \c false if there were any allocations to diagnose,
  /// \c true otherwise.
  bool maybeDiagnoseDanglingAllocations();

  StdAllocatorCaller getStdAllocatorCaller(StringRef Name) const;

  void *allocate(size_t Size, unsigned Align = 8) const {
    return Allocator.Allocate(Size, Align);
  }
  template <typename T> T *allocate(size_t Num = 1) const {
    return static_cast<T *>(allocate(Num * sizeof(T), alignof(T)));
  }

  template <typename T> T allocAP(unsigned BitWidth) {
    unsigned NumWords = APInt::getNumWords(BitWidth);
    if (NumWords == 1)
      return T(BitWidth);
    uint64_t *Mem = (uint64_t *)this->allocate(NumWords * sizeof(uint64_t));
    // std::memset(Mem, 0, NumWords * sizeof(uint64_t)); // Debug
    return T(Mem, BitWidth);
  }

  Floating allocFloat(const toolchain::fltSemantics &Sem) {
    if (Floating::singleWord(Sem))
      return Floating(toolchain::APFloatBase::SemanticsToEnum(Sem));

    unsigned NumWords =
        APInt::getNumWords(toolchain::APFloatBase::getSizeInBits(Sem));
    uint64_t *Mem = (uint64_t *)this->allocate(NumWords * sizeof(uint64_t));
    // std::memset(Mem, 0, NumWords * sizeof(uint64_t)); // Debug
    return Floating(Mem, toolchain::APFloatBase::SemanticsToEnum(Sem));
  }

private:
  friend class EvaluationResult;
  friend class InterpStateCCOverride;
  /// AST Walker state.
  State &Parent;
  /// Dead block chain.
  DeadBlock *DeadBlocks = nullptr;
  /// Reference to the offset-source mapping.
  SourceMapper *M;
  /// Allocator used for dynamic allocations performed via the program.
  DynamicAllocator Alloc;

public:
  /// Reference to the module containing all bytecode.
  Program &P;
  /// Temporary stack.
  InterpStack &Stk;
  /// Interpreter Context.
  Context &Ctx;
  /// Bottom function frame.
  InterpFrame BottomFrame;
  /// The current frame.
  InterpFrame *Current = nullptr;
  /// Source location of the evaluating expression
  SourceLocation EvalLocation;
  /// Declaration we're initializing/evaluting, if any.
  const VarDecl *EvaluatingDecl = nullptr;
  /// Things needed to do speculative execution.
  SmallVectorImpl<PartialDiagnosticAt> *PrevDiags = nullptr;
  unsigned SpeculationDepth = 0;
  std::optional<bool> ConstantContextOverride;

  toolchain::SmallVector<
      std::pair<const Expr *, const LifetimeExtendedTemporaryDecl *>>
      SeenGlobalTemporaries;

  /// List of blocks we're currently running either constructors or destructors
  /// for.
  toolchain::SmallVector<const Block *> InitializingBlocks;

  mutable toolchain::BumpPtrAllocator Allocator;
};

class InterpStateCCOverride final {
public:
  InterpStateCCOverride(InterpState &Ctx, bool Value)
      : Ctx(Ctx), OldCC(Ctx.ConstantContextOverride) {
    // We only override this if the new value is true.
    Enabled = Value;
    if (Enabled)
      Ctx.ConstantContextOverride = Value;
  }
  ~InterpStateCCOverride() {
    if (Enabled)
      Ctx.ConstantContextOverride = OldCC;
  }

private:
  bool Enabled;
  InterpState &Ctx;
  std::optional<bool> OldCC;
};

} // namespace interp
} // namespace language::Core

#endif
