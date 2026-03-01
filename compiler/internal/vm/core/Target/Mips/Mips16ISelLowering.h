//===-- Mips16ISelLowering.h - Mips16 DAG Lowering Interface ----*- C++ -*-===//
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
// Subclass of MipsTargetLowering specialized for mips16.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MIPS_MIPS16ISELLOWERING_H
#define LLVM_LIB_TARGET_MIPS_MIPS16ISELLOWERING_H

#include "MipsISelLowering.h"

namespace vm::core {
  class Mips16TargetLowering : public MipsTargetLowering  {
  public:
    explicit Mips16TargetLowering(const MipsTargetMachine &TM,
                                  const MipsSubtarget &STI);

    bool allowsMisalignedMemoryAccesses(EVT VT, unsigned AddrSpace,
                                        Align Alignment,
                                        MachineMemOperand::Flags Flags,
                                        unsigned *Fast) const override;

    MachineBasicBlock *
    EmitInstrWithCustomInserter(MachineInstr &MI,
                                MachineBasicBlock *MBB) const override;

  private:
    bool isEligibleForTailCallOptimization(
        const CCState &CCInfo, unsigned NextStackOffset,
        const MipsFunctionInfo &FI) const override;

    unsigned int
      getMips16HelperFunctionStubNumber(ArgListTy &Args) const;

    const char *getMips16HelperFunction
      (Type* RetTy, ArgListTy &Args, bool &needHelper) const;

    void
    getOpndList(SmallVectorImpl<SDValue> &Ops,
                std::deque< std::pair<unsigned, SDValue> > &RegsToPass,
                bool IsPICCall, bool GlobalOrExternal, bool InternalLinkage,
                bool IsCallReloc, CallLoweringInfo &CLI, SDValue Callee,
                SDValue Chain) const override;

    MachineBasicBlock *emitSel16(unsigned Opc, MachineInstr &MI,
                                 MachineBasicBlock *BB) const;

    MachineBasicBlock *emitSeliT16(unsigned Opc1, unsigned Opc2,
                                   MachineInstr &MI,
                                   MachineBasicBlock *BB) const;

    MachineBasicBlock *emitSelT16(unsigned Opc1, unsigned Opc2,
                                  MachineInstr &MI,
                                  MachineBasicBlock *BB) const;

    MachineBasicBlock *emitFEXT_T8I816_ins(unsigned BtOpc, unsigned CmpOpc,
                                           MachineInstr &MI,
                                           MachineBasicBlock *BB) const;

    MachineBasicBlock *emitFEXT_T8I8I16_ins(unsigned BtOpc, unsigned CmpiOpc,
                                            unsigned CmpiXOpc, bool ImmSigned,
                                            MachineInstr &MI,
                                            MachineBasicBlock *BB) const;

    MachineBasicBlock *emitFEXT_CCRX16_ins(unsigned SltOpc, MachineInstr &MI,
                                           MachineBasicBlock *BB) const;

    MachineBasicBlock *emitFEXT_CCRXI16_ins(unsigned SltiOpc, unsigned SltiXOpc,
                                            MachineInstr &MI,
                                            MachineBasicBlock *BB) const;
  };
}

#endif
