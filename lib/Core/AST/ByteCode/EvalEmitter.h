//===--- EvalEmitter.h - Instruction emitter for the VM ---------*- C++ -*-===//
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
// Defines the instruction emitters.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_INTERP_EVALEMITTER_H
#define LANGUAGE_CORE_AST_INTERP_EVALEMITTER_H

#include "EvaluationResult.h"
#include "InterpState.h"
#include "PrimType.h"
#include "Source.h"

namespace language::Core {
namespace interp {
class Context;
class Function;
class InterpStack;
class Program;
enum Opcode : uint32_t;

/// An emitter which evaluates opcodes as they are emitted.
class EvalEmitter : public SourceMapper {
public:
  using LabelTy = uint32_t;
  using AddrTy = uintptr_t;
  using Local = Scope::Local;
  using PtrCallback = toolchain::function_ref<bool(const Pointer &)>;

  EvaluationResult interpretExpr(const Expr *E,
                                 bool ConvertResultToRValue = false,
                                 bool DestroyToplevelScope = false);
  EvaluationResult interpretDecl(const VarDecl *VD, bool CheckFullyInitialized);
  /// Interpret the given Expr to a Pointer.
  EvaluationResult interpretAsPointer(const Expr *E, PtrCallback PtrCB);
  /// Interpret the given expression as if it was in the body of the given
  /// function, i.e. the parameters of the function are available for use.
  bool interpretCall(const FunctionDecl *FD, const Expr *E);

  /// Clean up all resources.
  void cleanup();

protected:
  EvalEmitter(Context &Ctx, Program &P, State &Parent, InterpStack &Stk);

  virtual ~EvalEmitter();

  /// Define a label.
  void emitLabel(LabelTy Label);
  /// Create a label.
  LabelTy getLabel();

  /// Methods implemented by the compiler.
  virtual bool visitExpr(const Expr *E, bool DestroyToplevelScope) = 0;
  virtual bool visitDeclAndReturn(const VarDecl *VD, bool ConstantContext) = 0;
  virtual bool visitFunc(const FunctionDecl *F) = 0;
  virtual bool visit(const Expr *E) = 0;
  virtual bool emitBool(bool V, const Expr *E) = 0;

  /// Emits jumps.
  bool jumpTrue(const LabelTy &Label);
  bool jumpFalse(const LabelTy &Label);
  bool jump(const LabelTy &Label);
  bool fallthrough(const LabelTy &Label);
  /// Speculative execution.
  bool speculate(const CallExpr *E, const LabelTy &EndLabel);

  /// Since expressions can only jump forward, predicated execution is
  /// used to deal with if-else statements.
  bool isActive() const { return CurrentLabel == ActiveLabel; }
  bool checkingForUndefinedBehavior() const {
    return S.checkingForUndefinedBehavior();
  }

  /// Callback for registering a local.
  Local createLocal(Descriptor *D);

  /// Returns the source location of the current opcode.
  SourceInfo getSource(const Function *F, CodePtr PC) const override {
    return (F && F->hasBody()) ? F->getSource(PC) : CurrentSource;
  }

  /// Parameter indices.
  toolchain::DenseMap<const ParmVarDecl *, ParamOffset> Params;
  /// Lambda captures.
  toolchain::DenseMap<const ValueDecl *, ParamOffset> LambdaCaptures;
  /// Offset of the This parameter in a lambda record.
  ParamOffset LambdaThisCapture{0, false};
  /// Local descriptors.
  toolchain::SmallVector<SmallVector<Local, 8>, 2> Descriptors;
  std::optional<SourceInfo> LocOverride = std::nullopt;

private:
  /// Current compilation context.
  Context &Ctx;
  /// Current program.
  Program &P;
  /// Callee evaluation state.
  InterpState S;
  /// Location to write the result to.
  EvaluationResult EvalResult;
  /// Whether the result should be converted to an RValue.
  bool ConvertResultToRValue = false;
  /// Whether we should check if the result has been fully
  /// initialized.
  bool CheckFullyInitialized = false;
  /// Callback to call when using interpretAsPointer.
  std::optional<PtrCallback> PtrCB;

  /// Temporaries which require storage.
  toolchain::SmallVector<std::unique_ptr<char[]>> Locals;

  Block *getLocal(unsigned Index) const {
    assert(Index < Locals.size());
    return reinterpret_cast<Block *>(Locals[Index].get());
  }

  void updateGlobalTemporaries();

  // The emitter always tracks the current instruction and sets OpPC to a token
  // value which is mapped to the location of the opcode being evaluated.
  CodePtr OpPC;
  /// Location of the current instruction.
  SourceInfo CurrentSource;

  /// Next label ID to generate - first label is 1.
  LabelTy NextLabel = 1;
  /// Label being executed - 0 is the entry label.
  LabelTy CurrentLabel = 0;
  /// Active block which should be executed.
  LabelTy ActiveLabel = 0;

protected:
#define GET_EVAL_PROTO
#include "Opcodes.inc"
#undef GET_EVAL_PROTO
};

} // namespace interp
} // namespace language::Core

#endif
