//===----- CGOpenCLRuntime.h - Interface to OpenCL Runtimes -----*- C++ -*-===//
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
// This provides an abstract class for OpenCL code generation.  Concrete
// subclasses of this implement code generation for specific OpenCL
// runtime libraries.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_CODEGEN_CGOPENCLRUNTIME_H
#define LANGUAGE_CORE_LIB_CODEGEN_CGOPENCLRUNTIME_H

#include "language/Core/AST/Expr.h"
#include "language/Core/AST/Type.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/IR/Type.h"
#include "toolchain/IR/Value.h"

namespace language::Core {

class BlockExpr;
class Expr;
class VarDecl;

namespace CodeGen {

class CodeGenFunction;
class CodeGenModule;

class CGOpenCLRuntime {
protected:
  CodeGenModule &CGM;
  toolchain::Type *PipeROTy;
  toolchain::Type *PipeWOTy;
  toolchain::Type *SamplerTy;

  /// Structure for enqueued block information.
  struct EnqueuedBlockInfo {
    toolchain::Function *InvokeFunc; /// Block invoke function.
    toolchain::Value *KernelHandle;  /// Enqueued block kernel reference.
    toolchain::Value *BlockArg;      /// The first argument to enqueued block kernel.
    toolchain::Type *BlockTy;        /// Type of the block argument.
  };
  /// Maps block expression to block information.
  toolchain::DenseMap<const Expr *, EnqueuedBlockInfo> EnqueuedBlockMap;

  virtual toolchain::Type *getPipeType(const PipeType *T, StringRef Name,
                                  toolchain::Type *&PipeTy);
  toolchain::PointerType *getPointerType(const Type *T);

public:
  CGOpenCLRuntime(CodeGenModule &CGM) : CGM(CGM),
    PipeROTy(nullptr), PipeWOTy(nullptr), SamplerTy(nullptr) {}
  virtual ~CGOpenCLRuntime();

  /// Emit the IR required for a work-group-local variable declaration, and add
  /// an entry to CGF's LocalDeclMap for D.  The base class does this using
  /// CodeGenFunction::EmitStaticVarDecl to emit an internal global for D.
  virtual void EmitWorkGroupLocalVarDecl(CodeGenFunction &CGF,
                                         const VarDecl &D);

  virtual toolchain::Type *convertOpenCLSpecificType(const Type *T);

  virtual toolchain::Type *getPipeType(const PipeType *T);

  toolchain::Type *getSamplerType(const Type *T);

  // Returns a value which indicates the size in bytes of the pipe
  // element.
  virtual toolchain::Value *getPipeElemSize(const Expr *PipeArg);

  // Returns a value which indicates the alignment in bytes of the pipe
  // element.
  virtual toolchain::Value *getPipeElemAlign(const Expr *PipeArg);

  /// \return __generic void* type.
  toolchain::PointerType *getGenericVoidPointerType();

  /// \return enqueued block information for enqueued block.
  EnqueuedBlockInfo emitOpenCLEnqueuedBlock(CodeGenFunction &CGF,
                                            const Expr *E);

  /// Record invoke function and block literal emitted during normal
  /// codegen for a block expression. The information is used by
  /// emitOpenCLEnqueuedBlock to emit wrapper kernel.
  ///
  /// \param InvokeF invoke function emitted for the block expression.
  /// \param Block block literal emitted for the block expression.
  void recordBlockInfo(const BlockExpr *E, toolchain::Function *InvokeF,
                       toolchain::Value *Block, toolchain::Type *BlockTy);

  /// \return LLVM block invoke function emitted for an expression derived from
  /// the block expression.
  toolchain::Function *getInvokeFunction(const Expr *E);
};

}
}

#endif
