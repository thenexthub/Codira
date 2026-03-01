//===------------------- AMDGPUCustomBehaviour.h ----------------*-C++ -* -===//
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
/// This file defines the AMDGPUCustomBehaviour class which inherits from
/// CustomBehaviour. This class is used by the tool toolchain-mca to enforce
/// target specific behaviour that is not expressed well enough in the
/// scheduling model for mca to enforce it automatically.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AMDGPU_MCA_AMDGPUCUSTOMBEHAVIOUR_H
#define LLVM_LIB_TARGET_AMDGPU_MCA_AMDGPUCUSTOMBEHAVIOUR_H

#include "Utils/AMDGPUBaseInfo.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/MCA/CustomBehaviour.h"
#include "vm/core/TargetParser/TargetParser.h"

namespace vm::core {
namespace mca {

class AMDGPUInstrPostProcess : public InstrPostProcess {
  void processWaitCnt(Instruction &Inst, const MCInst &MCI);

public:
  AMDGPUInstrPostProcess(const MCSubtargetInfo &STI, const MCInstrInfo &MCII)
      : InstrPostProcess(STI, MCII) {}

  ~AMDGPUInstrPostProcess() override = default;

  void postProcessInstruction(Instruction &Inst, const MCInst &MCI) override;
};

struct WaitCntInfo {
  bool VmCnt = false;
  bool ExpCnt = false;
  bool LgkmCnt = false;
  bool VsCnt = false;
};

class AMDGPUCustomBehaviour : public CustomBehaviour {
  /// Whenever MCA would like to dispatch an s_waitcnt instructions,
  /// we must check all the instruction that are still executing to see if
  /// they modify the same CNT as we need to wait for. This vector
  /// gets built in the constructor and contains 1 WaitCntInfo struct
  /// for each instruction within the SrcManager. Each element
  /// tells us which CNTs that instruction may interact with.
  /// We conservatively assume some instructions interact with more
  /// CNTs than they do in reality, so we will occasionally wait
  /// longer than necessary, but we shouldn't ever wait for shorter.
  std::vector<WaitCntInfo> InstrWaitCntInfo;

  /// This method gets called from the constructor and is
  /// where we setup the InstrWaitCntInfo vector.
  /// The core logic for determining which CNTs an instruction
  /// interacts with is taken from SIInsertWaitcnts::updateEventWaitcntAfter().
  /// Unfortunately, some of the logic from that function is not available to us
  /// in this scope so we conservatively end up assuming that some
  /// instructions interact with more CNTs than they do in reality.
  void generateWaitCntInfo();
  /// Helper function used in generateWaitCntInfo()
  bool hasModifiersSet(const std::unique_ptr<Instruction> &Inst,
                       AMDGPU::OpName OpName) const;
  /// Helper function used in generateWaitCntInfo()
  bool isGWS(uint16_t Opcode) const;
  /// Helper function used in generateWaitCntInfo()
  bool isAlwaysGDS(uint16_t Opcode) const;
  /// Helper function used in generateWaitCntInfo()
  bool isVMEM(const MCInstrDesc &MCID);
  /// This method gets called from checkCustomHazard when mca is attempting to
  /// dispatch an s_waitcnt instruction (or one of its variants). The method
  /// looks at each of the instructions that are still executing in the pipeline
  /// to determine if the waitcnt should force a wait.
  unsigned handleWaitCnt(ArrayRef<InstRef> IssuedInst, const InstRef &IR);
  /// Based on the type of s_waitcnt instruction we are looking at, and what its
  /// operands are, this method will set the values for each of the cnt
  /// references provided as arguments.
  void computeWaitCnt(const InstRef &IR, unsigned &Vmcnt, unsigned &Expcnt,
                      unsigned &Lgkmcnt, unsigned &Vscnt);

public:
  AMDGPUCustomBehaviour(const MCSubtargetInfo &STI,
                        const mca::SourceMgr &SrcMgr, const MCInstrInfo &MCII);

  ~AMDGPUCustomBehaviour() override = default;
  /// This method is used to determine if an instruction
  /// should be allowed to be dispatched. The return value is
  /// how many cycles until the instruction can be dispatched.
  /// This method is called after MCA has already checked for
  /// register and hardware dependencies so this method should only
  /// implement custom behaviour and dependencies that are not picked up
  /// by MCA naturally.
  unsigned checkCustomHazard(ArrayRef<InstRef> IssuedInst,
                             const InstRef &IR) override;
};
} // namespace mca
} // namespace vm::core

#endif
