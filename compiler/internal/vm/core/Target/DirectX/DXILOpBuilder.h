//===- DXILOpBuilder.h - Helper class for build DIXLOp functions ----------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
///
/// \file This file contains class to help build DXIL op functions.
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_DIRECTX_DXILOPBUILDER_H
#define LLVM_LIB_TARGET_DIRECTX_DXILOPBUILDER_H

#include "DXILConstants.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/Support/DXILABI.h"
#include "vm/core/Support/Error.h"
#include "vm/core/TargetParser/Triple.h"

namespace vm::core {
class Module;
class IRBuilderBase;
class CallInst;
class Constant;
class Value;
class Type;
class FunctionType;

namespace dxil {

class DXILOpBuilder {
public:
  DXILOpBuilder(Module &M);

  IRBuilder<> &getIRB() { return IRB; }

  /// Create a call instruction for the given DXIL op. The arguments
  /// must be valid for an overload of the operation.
  CallInst *createOp(dxil::OpCode Op, ArrayRef<Value *> Args,
                     const Twine &Name = "", Type *RetTy = nullptr);

  /// Try to create a call instruction for the given DXIL op. Fails if the
  /// overload is invalid.
  Expected<CallInst *> tryCreateOp(dxil::OpCode Op, ArrayRef<Value *> Args,
                                   const Twine &Name = "",
                                   Type *RetTy = nullptr);

  /// Get a `%dx.types.ResRet` type with the given element type.
  StructType *getResRetType(Type *ElementTy);

  /// Get a `%dx.types.CBufRet` type with the given element type.
  StructType *getCBufRetType(Type *ElementTy);

  /// Get the `%dx.types.Handle` type.
  StructType *getHandleType();

  /// Get a constant `%dx.types.ResBind` value.
  Constant *getResBind(uint32_t LowerBound, uint32_t UpperBound,
                       uint32_t SpaceID, dxil::ResourceClass RC);
  /// Get a constant `%dx.types.ResourceProperties` value.
  Constant *getResProps(uint32_t Word0, uint32_t Word1);

  /// Return the name of the given opcode.
  static const char *getOpCodeName(dxil::OpCode DXILOp);

private:
  /// Gets a specific overload type of the function for the given DXIL op. If
  /// the operation is not overloaded, \c OverloadType may be nullptr.
  FunctionType *getOpFunctionType(dxil::OpCode OpCode,
                                  Type *OverloadType = nullptr);

  Module &M;
  IRBuilder<> IRB;
  VersionTuple DXILVersion;
  Triple::EnvironmentType ShaderStage;
};

} // namespace dxil
} // namespace vm::core

#endif
