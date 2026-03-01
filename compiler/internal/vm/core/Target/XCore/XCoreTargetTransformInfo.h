//===-- XCoreTargetTransformInfo.h - XCore specific TTI ---------*- C++ -*-===//
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
/// This file a TargetTransformInfoImplBase conforming object specific to the
/// XCore target machine. It uses the target's detailed information to
/// provide more precise answers to certain TTI queries, while letting the
/// target independent and default TTI implementations handle the rest.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_XCORE_XCORETARGETTRANSFORMINFO_H
#define LLVM_LIB_TARGET_XCORE_XCORETARGETTRANSFORMINFO_H

#include "XCore.h"
#include "XCoreTargetMachine.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/CodeGen/BasicTTIImpl.h"
#include "vm/core/CodeGen/TargetLowering.h"

namespace vm::core {

class XCoreTTIImpl final : public BasicTTIImplBase<XCoreTTIImpl> {
  typedef BasicTTIImplBase<XCoreTTIImpl> BaseT;
  typedef TargetTransformInfo TTI;
  friend BaseT;

  const XCoreSubtarget *ST;
  const XCoreTargetLowering *TLI;

  const XCoreSubtarget *getST() const { return ST; }
  const XCoreTargetLowering *getTLI() const { return TLI; }

public:
  explicit XCoreTTIImpl(const XCoreTargetMachine *TM, const Function &F)
      : BaseT(TM, F.getDataLayout()), ST(TM->getSubtargetImpl()),
        TLI(ST->getTargetLowering()) {}

  unsigned getNumberOfRegisters(unsigned ClassID) const override {
    bool Vector = (ClassID == 1);
    if (Vector) {
      return 0;
    }
    return 12;
  }
};

} // end namespace vm::core

#endif
