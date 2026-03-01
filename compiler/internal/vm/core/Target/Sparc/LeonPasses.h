//===------- LeonPasses.h - Define passes specific to LEON ----------------===//
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
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPARC_LEON_PASSES_H
#define LLVM_LIB_TARGET_SPARC_LEON_PASSES_H

#include "vm/core/CodeGen/MachineFunctionPass.h"

namespace vm::core {
class SparcSubtarget;

class LLVM_LIBRARY_VISIBILITY LEONMachineFunctionPass
    : public MachineFunctionPass {
protected:
  const SparcSubtarget *Subtarget = nullptr;
  const int LAST_OPERAND = -1;

  // this vector holds free registers that we allocate in groups for some of the
  // LEON passes
  std::vector<int> UsedRegisters;

protected:
  LEONMachineFunctionPass(char &ID);

  void clearUsedRegisterList() { UsedRegisters.clear(); }

  void markRegisterUsed(int registerIndex) {
    UsedRegisters.push_back(registerIndex);
  }
};

class LLVM_LIBRARY_VISIBILITY ErrataWorkaround : public MachineFunctionPass {
protected:
  const SparcSubtarget *ST;
  const TargetInstrInfo *TII;
  const TargetRegisterInfo *TRI;

  bool checkSeqTN0009A(MachineBasicBlock::iterator I);
  bool checkSeqTN0009B(MachineBasicBlock::iterator I);
  bool checkSeqTN0010First(MachineBasicBlock &MBB);
  bool checkSeqTN0010(MachineBasicBlock::iterator I);
  bool checkSeqTN0012(MachineBasicBlock::iterator I);
  bool checkSeqTN0013(MachineBasicBlock::iterator I);

  bool moveNext(MachineBasicBlock::iterator &I);
  bool isFloat(MachineBasicBlock::iterator I);
  bool isDivSqrt(MachineBasicBlock::iterator I);
  void insertNop(MachineBasicBlock::iterator I);

public:
  static char ID;

  ErrataWorkaround();
  bool runOnMachineFunction(MachineFunction &MF) override;

  StringRef getPassName() const override { return "Errata workaround pass"; };
};

class LLVM_LIBRARY_VISIBILITY InsertNOPLoad : public LEONMachineFunctionPass {
public:
  static char ID;

  InsertNOPLoad();
  bool runOnMachineFunction(MachineFunction &MF) override;

  StringRef getPassName() const override {
    return "InsertNOPLoad: Erratum Fix LBR35: insert a NOP instruction after "
           "every single-cycle load instruction when the next instruction is "
           "another load/store instruction";
  }
};

class LLVM_LIBRARY_VISIBILITY DetectRoundChange
    : public LEONMachineFunctionPass {
public:
  static char ID;

  DetectRoundChange();
  bool runOnMachineFunction(MachineFunction &MF) override;

  StringRef getPassName() const override {
    return "DetectRoundChange: Leon erratum detection: detect any rounding "
           "mode change request: use only the round-to-nearest rounding mode";
  }
};

class LLVM_LIBRARY_VISIBILITY FixAllFDIVSQRT : public LEONMachineFunctionPass {
public:
  static char ID;

  FixAllFDIVSQRT();
  bool runOnMachineFunction(MachineFunction &MF) override;

  StringRef getPassName() const override {
    return "FixAllFDIVSQRT: Erratum Fix LBR34: fix FDIVS/FDIVD/FSQRTS/FSQRTD "
           "instructions with NOPs and floating-point store";
  }
};
} // namespace vm::core

#endif // LLVM_LIB_TARGET_SPARC_LEON_PASSES_H
