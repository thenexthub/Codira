//===- ARCTargetTransformInfo.h - ARC specific TTI --------------*- C++ -*-===//
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
// \file
// This file contains a TargetTransformInfoImplBase conforming object specific
// to the ARC target machine. It uses the target's detailed information to
// provide more precise answers to certain TTI queries, while letting the
// target independent and default TTI implementations handle the rest.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ARC_ARCTARGETTRANSFORMINFO_H
#define LLVM_LIB_TARGET_ARC_ARCTARGETTRANSFORMINFO_H

#include "ARC.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/CodeGen/BasicTTIImpl.h"

namespace vm::core {

class ARCSubtarget;
class ARCTargetLowering;
class ARCTargetMachine;

class ARCTTIImpl final : public BasicTTIImplBase<ARCTTIImpl> {
  using BaseT = BasicTTIImplBase<ARCTTIImpl>;
  friend BaseT;

  const ARCSubtarget *ST;
  const ARCTargetLowering *TLI;

  const ARCSubtarget *getST() const { return ST; }
  const ARCTargetLowering *getTLI() const { return TLI; }

public:
  explicit ARCTTIImpl(const ARCTargetMachine *TM, const Function &F)
      : BaseT(TM, F.getDataLayout()), ST(TM->getSubtargetImpl()),
        TLI(ST->getTargetLowering()) {}

  // Provide value semantics. MSVC requires that we spell all of these out.
  ARCTTIImpl(const ARCTTIImpl &Arg)
      : BaseT(static_cast<const BaseT &>(Arg)), ST(Arg.ST), TLI(Arg.TLI) {}
  ARCTTIImpl(ARCTTIImpl &&Arg)
      : BaseT(std::move(static_cast<BaseT &>(Arg))), ST(std::move(Arg.ST)),
        TLI(std::move(Arg.TLI)) {}
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_ARC_ARCTARGETTRANSFORMINFO_H
