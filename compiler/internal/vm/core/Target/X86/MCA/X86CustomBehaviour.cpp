//===------------------- X86CustomBehaviour.cpp -----------------*-C++ -* -===//
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
///
/// This file implements methods from the X86CustomBehaviour class.
///
//===----------------------------------------------------------------------===//

#include "X86CustomBehaviour.h"
#include "MCTargetDesc/X86BaseInfo.h"
#include "TargetInfo/X86TargetInfo.h"
#include "vm/core-c/Visibility.h"
#include "vm/core/MC/TargetRegistry.h"

namespace vm::core {
namespace mca {

void X86InstrPostProcess::setMemBarriers(Instruction &Inst, const MCInst &MCI) {
  switch (MCI.getOpcode()) {
  case X86::MFENCE:
    Inst.setLoadBarrier(true);
    Inst.setStoreBarrier(true);
    break;
  case X86::LFENCE:
    Inst.setLoadBarrier(true);
    break;
  case X86::SFENCE:
    Inst.setStoreBarrier(true);
    break;
  }
}

void X86InstrPostProcess::useStackEngine(Instruction &Inst, const MCInst &MCI) {
  // TODO(boomanaiden154): We currently do not handle PUSHF/POPF because we
  // have not done the necessary benchmarking to see if they are also
  // optimized by the stack engine.
  // TODO: We currently just remove all RSP writes from stack operations. This
  // is not fully correct because we do not model sync uops which will
  // delay subsequent rsp using non-stack instructions.
  if (X86::isPOP(MCI.getOpcode()) || X86::isPUSH(MCI.getOpcode())) {
    auto *StackRegisterDef =
        toolchain::find_if(Inst.getDefs(), [](const WriteState &State) {
          return State.getRegisterID() == X86::RSP;
        });
    assert(
        StackRegisterDef != Inst.getDefs().end() &&
        "Expected push instruction to implicitly use stack pointer register.");
    Inst.getDefs().erase(StackRegisterDef);
  }
}

void X86InstrPostProcess::postProcessInstruction(Instruction &Inst,
                                                 const MCInst &MCI) {
  // Set IsALoadBarrier and IsAStoreBarrier flags.
  setMemBarriers(Inst, MCI);
  useStackEngine(Inst, MCI);
}

} // namespace mca
} // namespace vm::core

using namespace vm::core;
using namespace mca;

static InstrPostProcess *createX86InstrPostProcess(const MCSubtargetInfo &STI,
                                                   const MCInstrInfo &MCII) {
  return new X86InstrPostProcess(STI, MCII);
}

/// Extern function to initialize the targets for the X86 backend

extern "C" LLVM_C_ABI void LLVMInitializeX86TargetMCA() {
  TargetRegistry::RegisterInstrPostProcess(getTheX86_32Target(),
                                           createX86InstrPostProcess);
  TargetRegistry::RegisterInstrPostProcess(getTheX86_64Target(),
                                           createX86InstrPostProcess);
}
