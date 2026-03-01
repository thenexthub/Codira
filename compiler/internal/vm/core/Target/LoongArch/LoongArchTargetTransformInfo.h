//===- LoongArchTargetTransformInfo.h - LoongArch specific TTI --*- C++ -*-===//
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
/// LoongArch target machine. It uses the target's detailed information to
/// provide more precise answers to certain TTI queries, while letting the
/// target independent and default TTI implementations handle the rest.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LOONGARCH_LOONGARCHTARGETTRANSFORMINFO_H
#define LLVM_LIB_TARGET_LOONGARCH_LOONGARCHTARGETTRANSFORMINFO_H

#include "LoongArchSubtarget.h"
#include "LoongArchTargetMachine.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/CodeGen/BasicTTIImpl.h"

namespace vm::core {

class LoongArchTTIImpl : public BasicTTIImplBase<LoongArchTTIImpl> {
  typedef BasicTTIImplBase<LoongArchTTIImpl> BaseT;
  typedef TargetTransformInfo TTI;
  friend BaseT;

  enum LoongArchRegisterClass { GPRRC, FPRRC, VRRC };
  const LoongArchSubtarget *ST;
  const LoongArchTargetLowering *TLI;

  const LoongArchSubtarget *getST() const { return ST; }
  const LoongArchTargetLowering *getTLI() const { return TLI; }

public:
  explicit LoongArchTTIImpl(const LoongArchTargetMachine *TM, const Function &F)
      : BaseT(TM, F.getDataLayout()), ST(TM->getSubtargetImpl(F)),
        TLI(ST->getTargetLowering()) {}

  TypeSize
  getRegisterBitWidth(TargetTransformInfo::RegisterKind K) const override;
  unsigned getNumberOfRegisters(unsigned ClassID) const override;
  unsigned getRegisterClassForType(bool Vector,
                                   Type *Ty = nullptr) const override;
  unsigned getMaxInterleaveFactor(ElementCount VF) const override;
  const char *getRegisterClassName(unsigned ClassID) const override;
  TTI::PopcntSupportKind getPopcntSupport(unsigned TyWidth) const override;

  unsigned getCacheLineSize() const override;
  unsigned getPrefetchDistance() const override;
  bool enableWritePrefetching() const override;

  bool shouldExpandReduction(const IntrinsicInst *II) const override;

  TTI::MemCmpExpansionOptions
  enableMemCmpExpansion(bool OptSize, bool IsZeroCmp) const override;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_LOONGARCH_LOONGARCHTARGETTRANSFORMINFO_H
