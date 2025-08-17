//===--- ByteCodeEmitter.h - Instruction emitter for the VM -----*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_AST_INTERP_LINKEMITTER_H
#define LANGUAGE_CORE_AST_INTERP_LINKEMITTER_H

#include "Context.h"
#include "PrimType.h"
#include "Program.h"
#include "Source.h"

namespace language::Core {
namespace interp {
enum Opcode : uint32_t;

/// An emitter which links the program to bytecode for later use.
class ByteCodeEmitter {
protected:
  using LabelTy = uint32_t;
  using AddrTy = uintptr_t;
  using Local = Scope::Local;

public:
  /// Compiles the function into the module.
  void compileFunc(const FunctionDecl *FuncDecl, Function *Func = nullptr);

protected:
  ByteCodeEmitter(Context &Ctx, Program &P) : Ctx(Ctx), P(P) {}

  virtual ~ByteCodeEmitter() {}

  /// Define a label.
  void emitLabel(LabelTy Label);
  /// Create a label.
  LabelTy getLabel() { return ++NextLabel; }

  /// Methods implemented by the compiler.
  virtual bool visitFunc(const FunctionDecl *E) = 0;
  virtual bool visitExpr(const Expr *E, bool DestroyToplevelScope) = 0;
  virtual bool visitDeclAndReturn(const VarDecl *E, bool ConstantContext) = 0;
  virtual bool visit(const Expr *E) = 0;
  virtual bool emitBool(bool V, const Expr *E) = 0;

  /// Emits jumps.
  bool jumpTrue(const LabelTy &Label);
  bool jumpFalse(const LabelTy &Label);
  bool jump(const LabelTy &Label);
  bool fallthrough(const LabelTy &Label);
  /// Speculative execution.
  bool speculate(const CallExpr *E, const LabelTy &EndLabel);

  /// We're always emitting bytecode.
  bool isActive() const { return true; }
  bool checkingForUndefinedBehavior() const { return false; }

  /// Callback for local registration.
  Local createLocal(Descriptor *D);

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
  /// Program to link to.
  Program &P;
  /// Index of the next available label.
  LabelTy NextLabel = 0;
  /// Offset of the next local variable.
  unsigned NextLocalOffset = 0;
  /// Label information for linker.
  toolchain::DenseMap<LabelTy, unsigned> LabelOffsets;
  /// Location of label relocations.
  toolchain::DenseMap<LabelTy, toolchain::SmallVector<unsigned, 5>> LabelRelocs;
  /// Program code.
  toolchain::SmallVector<std::byte> Code;
  /// Opcode to expression mapping.
  SourceMap SrcMap;

  /// Returns the offset for a jump or records a relocation.
  int32_t getOffset(LabelTy Label);

  /// Emits an opcode.
  template <typename... Tys>
  bool emitOp(Opcode Op, const Tys &...Args, const SourceInfo &L);

protected:
#define GET_LINK_PROTO
#include "Opcodes.inc"
#undef GET_LINK_PROTO
};

} // namespace interp
} // namespace language::Core

#endif
