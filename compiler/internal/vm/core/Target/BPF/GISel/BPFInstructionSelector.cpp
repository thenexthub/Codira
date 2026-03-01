//===- BPFInstructionSelector.cpp --------------------------------*- C++ -*-==//
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
/// This file implements the targeting of the InstructionSelector class for BPF.
//===----------------------------------------------------------------------===//

#include "BPFInstrInfo.h"
#include "BPFRegisterBankInfo.h"
#include "BPFSubtarget.h"
#include "BPFTargetMachine.h"
#include "vm/core/CodeGen/GlobalISel/GIMatchTableExecutorImpl.h"
#include "vm/core/CodeGen/GlobalISel/InstructionSelector.h"
#include "vm/core/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "vm/core/IR/IntrinsicsBPF.h"

#define DEBUG_TYPE "bpf-gisel"

using namespace vm::core;

namespace {

#define GET_GLOBALISEL_PREDICATE_BITSET
#include "BPFGenGlobalISel.inc"
#undef GET_GLOBALISEL_PREDICATE_BITSET

class BPFInstructionSelector : public InstructionSelector {
public:
  BPFInstructionSelector(const BPFTargetMachine &TM, const BPFSubtarget &STI,
                         const BPFRegisterBankInfo &RBI);

  bool select(MachineInstr &I) override;
  static const char *getName() { return DEBUG_TYPE; }

private:
  /// tblgen generated 'select' implementation that is used as the initial
  /// selector for the patterns that do not require complex C++.
  bool selectImpl(MachineInstr &I, CodeGenCoverage &CoverageInfo) const;

  const BPFInstrInfo &TII;
  const BPFRegisterInfo &TRI;
  const BPFRegisterBankInfo &RBI;

#define GET_GLOBALISEL_PREDICATES_DECL
#include "BPFGenGlobalISel.inc"
#undef GET_GLOBALISEL_PREDICATES_DECL

#define GET_GLOBALISEL_TEMPORARIES_DECL
#include "BPFGenGlobalISel.inc"
#undef GET_GLOBALISEL_TEMPORARIES_DECL
};

} // namespace

#define GET_GLOBALISEL_IMPL
#include "BPFGenGlobalISel.inc"
#undef GET_GLOBALISEL_IMPL

BPFInstructionSelector::BPFInstructionSelector(const BPFTargetMachine &TM,
                                               const BPFSubtarget &STI,
                                               const BPFRegisterBankInfo &RBI)
    : TII(*STI.getInstrInfo()), TRI(*STI.getRegisterInfo()), RBI(RBI),
#define GET_GLOBALISEL_PREDICATES_INIT
#include "BPFGenGlobalISel.inc"
#undef GET_GLOBALISEL_PREDICATES_INIT
#define GET_GLOBALISEL_TEMPORARIES_INIT
#include "BPFGenGlobalISel.inc"
#undef GET_GLOBALISEL_TEMPORARIES_INIT
{
}

bool BPFInstructionSelector::select(MachineInstr &I) {
  if (!isPreISelGenericOpcode(I.getOpcode()))
    return true;
  if (selectImpl(I, *CoverageInfo))
    return true;
  return false;
}

namespace vm::core {
InstructionSelector *
createBPFInstructionSelector(const BPFTargetMachine &TM,
                             const BPFSubtarget &Subtarget,
                             const BPFRegisterBankInfo &RBI) {
  return new BPFInstructionSelector(TM, Subtarget, RBI);
}
} // namespace vm::core
