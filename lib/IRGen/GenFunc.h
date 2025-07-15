//===--- GenFunc.h - Codira IR generation for functions ----------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
//  This file provides the private interface to the function and
//  function-type emission code.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_GENFUNC_H
#define LANGUAGE_IRGEN_GENFUNC_H

#include "language/AST/Types.h"

namespace toolchain {
  class Function;
  class Value;
}

namespace language {
namespace irgen {
  class Address;
  class Explosion;
  class ForeignFunctionInfo;
  class IRGenFunction;

  /// Project the capture address from on-stack block storage.
  Address projectBlockStorageCapture(IRGenFunction &IGF,
                                     Address storageAddr,
                                     CanSILBlockStorageType storageTy);

  /// Load the stored isolation of an @isolated(any) function type, which
  /// is assumed to be at a known offset within a closure object.
  void emitExtractFunctionIsolation(IRGenFunction &IGF,
                                    toolchain::Value *fnContext,
                                    Explosion &result);
  
  /// Emit the block header into a block storage slot.
  void emitBlockHeader(IRGenFunction &IGF,
                       Address storage,
                       CanSILBlockStorageType blockTy,
                       toolchain::Constant *invokeFunction,
                       CanSILFunctionType invokeTy,
                       ForeignFunctionInfo foreignInfo);

  /// Emit a partial application thunk for a function pointer applied to a
  /// partial set of argument values.
  std::optional<StackAddress> emitFunctionPartialApplication(
      IRGenFunction &IGF, SILFunction &SILFn, const FunctionPointer &fnPtr,
      toolchain::Value *fnContext, Explosion &args,
      ArrayRef<SILParameterInfo> argTypes, SubstitutionMap subs,
      CanSILFunctionType origType, CanSILFunctionType substType,
      CanSILFunctionType outType, Explosion &out, bool isOutlined);
  CanType getArgumentLoweringType(CanType type, SILParameterInfo paramInfo,
                                  bool isNoEscape);

  /// Stub function that weakly links againt the language_coroFrameAlloc
  /// function. This is required for back-deployment.
  toolchain::Constant *getCoroFrameAllocStubFn(IRGenModule &IGM);
} // end namespace irgen
} // end namespace language

#endif
