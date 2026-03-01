//===- AVRTargetTransformInfo.h - AVR specific TTI --------------*- C++ -*-===//
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
/// \file
/// This file defines a TargetTransformInfoImplBase conforming object specific
/// to the AVR target machine. It uses the target's detailed information to
/// provide more precise answers to certain TTI queries, while letting the
/// target independent and default TTI implementations handle the rest.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AVR_AVRTARGETTRANSFORMINFO_H
#define LLVM_LIB_TARGET_AVR_AVRTARGETTRANSFORMINFO_H

#include "AVRSubtarget.h"
#include "AVRTargetMachine.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/CodeGen/BasicTTIImpl.h"
#include "vm/core/IR/Function.h"

namespace vm::core {

class AVRTTIImpl final : public BasicTTIImplBase<AVRTTIImpl> {
  using BaseT = BasicTTIImplBase<AVRTTIImpl>;
  using TTI = TargetTransformInfo;

  friend BaseT;

  const AVRSubtarget *ST;
  const AVRTargetLowering *TLI;

  const AVRSubtarget *getST() const { return ST; }
  const AVRTargetLowering *getTLI() const { return TLI; }

public:
  explicit AVRTTIImpl(const AVRTargetMachine *TM, const Function &F)
      : BaseT(TM, F.getDataLayout()), ST(TM->getSubtargetImpl(F)),
        TLI(ST->getTargetLowering()) {}

  bool isLSRCostLess(const TargetTransformInfo::LSRCost &C1,
                     const TargetTransformInfo::LSRCost &C2) const override;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_AVR_AVRTARGETTRANSFORMINFO_H
