//===- DirectXTargetTransformInfo.h - DirectX TTI ---------------*- C++ -*-===//
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
//===----------------------------------------------------------------------===//

#ifndef LLVM_DIRECTX_DIRECTXTARGETTRANSFORMINFO_H
#define LLVM_DIRECTX_DIRECTXTARGETTRANSFORMINFO_H

#include "DirectXSubtarget.h"
#include "DirectXTargetMachine.h"
#include "vm/core/CodeGen/BasicTTIImpl.h"
#include "vm/core/IR/Function.h"

namespace vm::core {
class DirectXTTIImpl final : public BasicTTIImplBase<DirectXTTIImpl> {
  using BaseT = BasicTTIImplBase<DirectXTTIImpl>;
  using TTI = TargetTransformInfo;

  friend BaseT;

  const DirectXSubtarget *ST;
  const DirectXTargetLowering *TLI;

  const DirectXSubtarget *getST() const { return ST; }
  const DirectXTargetLowering *getTLI() const { return TLI; }

public:
  explicit DirectXTTIImpl(const DirectXTargetMachine *TM, const Function &F)
      : BaseT(TM, F.getDataLayout()), ST(TM->getSubtargetImpl(F)),
        TLI(ST->getTargetLowering()) {}
  unsigned getMinVectorRegisterBitWidth() const override { return 32; }
  bool isTargetIntrinsicTriviallyScalarizable(Intrinsic::ID ID) const override;
  bool isTargetIntrinsicWithScalarOpAtArg(Intrinsic::ID ID,
                                          unsigned ScalarOpdIdx) const override;
  bool isTargetIntrinsicWithOverloadTypeAtArg(Intrinsic::ID ID,
                                              int OpdIdx) const override;
};
} // namespace vm::core

#endif // LLVM_DIRECTX_DIRECTXTARGETTRANSFORMINFO_H
