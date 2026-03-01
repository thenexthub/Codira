//===- NVPTXRegisterInfo.h - NVPTX Register Information Impl ----*- C++ -*-===//
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
//
// This file contains the NVPTX implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_NVPTX_NVPTXREGISTERINFO_H
#define LLVM_LIB_TARGET_NVPTX_NVPTXREGISTERINFO_H

#include "vm/core/CodeGen/TargetRegisterInfo.h"
#include "vm/core/Support/StringSaver.h"
#include <sstream>

#define GET_REGINFO_HEADER
#include "NVPTXGenRegisterInfo.inc"

namespace vm::core {
class NVPTXRegisterInfo : public NVPTXGenRegisterInfo {
private:
  // Hold Strings that can be free'd all together with NVPTXRegisterInfo
  BumpPtrAllocator StrAlloc;
  UniqueStringSaver StrPool;
  // State for debug register mapping that can be mutated even through a const
  // pointer so that we can get the proper dwarf register encoding during ASM
  // emission.
  mutable DenseMap<uint64_t, uint64_t> debugRegisterMap;

public:
  NVPTXRegisterInfo();

  //------------------------------------------------------
  // Pure virtual functions from TargetRegisterInfo
  //------------------------------------------------------

  // NVPTX callee saved registers
  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;

  BitVector getReservedRegs(const MachineFunction &MF) const override;

  bool eliminateFrameIndex(MachineBasicBlock::iterator MI, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  Register getFrameRegister(const MachineFunction &MF) const override;
  Register getFrameLocalRegister(const MachineFunction &MF) const;

  UniqueStringSaver &getStrPool() const {
    return const_cast<UniqueStringSaver &>(StrPool);
  }

  const char *getName(unsigned RegNo) const {
    std::stringstream O;
    O << "reg" << RegNo;
    return getStrPool().save(O.str()).data();
  }

  // Manage the debugRegisterMap.  PTX virtual registers for DebugInfo are
  // encoded using the names used in the emitted text of the PTX assembly. This
  // mapping must be managed during assembly emission.
  //
  // These are marked const because the interfaces used to access this
  // RegisterInfo object are all const, but we need to communicate some state
  // here, because the proper encoding for debug registers is available only
  // temporarily during ASM emission.
  void addToDebugRegisterMap(uint64_t preEncodedVirtualRegister,
                             StringRef RegisterName) const;
  void clearDebugRegisterMap() const;
  int64_t getDwarfRegNum(MCRegister RegNum, bool isEH) const override;
  int64_t getDwarfRegNumForVirtReg(Register RegNum, bool isEH) const override;
};

StringRef getNVPTXRegClassName(const TargetRegisterClass *RC);
StringRef getNVPTXRegClassStr(const TargetRegisterClass *RC);

} // end namespace vm::core

#endif
