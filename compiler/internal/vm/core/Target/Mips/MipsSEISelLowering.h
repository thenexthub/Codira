//===- MipsSEISelLowering.h - MipsSE DAG Lowering Interface -----*- C++ -*-===//
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
// Subclass of MipsTargetLowering specialized for mips32/64.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MIPS_MIPSSEISELLOWERING_H
#define LLVM_LIB_TARGET_MIPS_MIPSSEISELLOWERING_H

#include "MipsISelLowering.h"
#include "vm/core/CodeGen/SelectionDAGNodes.h"
#include "vm/core/CodeGenTypes/MachineValueType.h"

namespace vm::core {

class MachineBasicBlock;
class MachineInstr;
class MipsSubtarget;
class MipsTargetMachine;
class SelectionDAG;
class TargetRegisterClass;

  class MipsSETargetLowering : public MipsTargetLowering  {
  public:
    explicit MipsSETargetLowering(const MipsTargetMachine &TM,
                                  const MipsSubtarget &STI);

    /// Enable MSA support for the given integer type and Register
    /// class.
    void addMSAIntType(MVT::SimpleValueType Ty, const TargetRegisterClass *RC);

    /// Enable MSA support for the given floating-point type and
    /// Register class.
    void addMSAFloatType(MVT::SimpleValueType Ty,
                         const TargetRegisterClass *RC);

    bool allowsMisalignedMemoryAccesses(
        EVT VT, unsigned AS = 0, Align Alignment = Align(1),
        MachineMemOperand::Flags Flags = MachineMemOperand::MONone,
        unsigned *Fast = nullptr) const override;

    TargetLoweringBase::LegalizeTypeAction
    getPreferredVectorAction(MVT VT) const override;

    SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

    SDValue PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI) const override;

    MachineBasicBlock *
    EmitInstrWithCustomInserter(MachineInstr &MI,
                                MachineBasicBlock *MBB) const override;

    bool isShuffleMaskLegal(ArrayRef<int> Mask, EVT VT) const override {
      return false;
    }

    const TargetRegisterClass *getRepRegClassFor(MVT VT) const override;

  private:
    bool isEligibleForTailCallOptimization(
        const CCState &CCInfo, unsigned NextStackOffset,
        const MipsFunctionInfo &FI) const override;

    void
    getOpndList(SmallVectorImpl<SDValue> &Ops,
                std::deque<std::pair<unsigned, SDValue>> &RegsToPass,
                bool IsPICCall, bool GlobalOrExternal, bool InternalLinkage,
                bool IsCallReloc, CallLoweringInfo &CLI, SDValue Callee,
                SDValue Chain) const override;

    SDValue lowerLOAD(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerSTORE(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerBITCAST(SDValue Op, SelectionDAG &DAG) const;

    SDValue lowerMulDiv(SDValue Op, unsigned NewOpc, bool HasLo, bool HasHi,
                        SelectionDAG &DAG) const;

    SDValue lowerINTRINSIC_WO_CHAIN(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerINTRINSIC_W_CHAIN(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerINTRINSIC_VOID(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerEXTRACT_VECTOR_ELT(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerBUILD_VECTOR(SDValue Op, SelectionDAG &DAG) const;
    /// Lower VECTOR_SHUFFLE into one of a number of instructions
    /// depending on the indices in the shuffle.
    SDValue lowerVECTOR_SHUFFLE(SDValue Op, SelectionDAG &DAG) const;
    SDValue lowerSELECT(SDValue Op, SelectionDAG &DAG) const;

    MachineBasicBlock *emitBPOSGE32(MachineInstr &MI,
                                    MachineBasicBlock *BB) const;
    MachineBasicBlock *emitMSACBranchPseudo(MachineInstr &MI,
                                            MachineBasicBlock *BB,
                                            unsigned BranchOp) const;
    /// Emit the COPY_FW pseudo instruction
    MachineBasicBlock *emitCOPY_FW(MachineInstr &MI,
                                   MachineBasicBlock *BB) const;
    /// Emit the COPY_FD pseudo instruction
    MachineBasicBlock *emitCOPY_FD(MachineInstr &MI,
                                   MachineBasicBlock *BB) const;
    /// Emit the INSERT_FW pseudo instruction
    MachineBasicBlock *emitINSERT_FW(MachineInstr &MI,
                                     MachineBasicBlock *BB) const;
    /// Emit the INSERT_FD pseudo instruction
    MachineBasicBlock *emitINSERT_FD(MachineInstr &MI,
                                     MachineBasicBlock *BB) const;
    /// Emit the INSERT_([BHWD]|F[WD])_VIDX pseudo instruction
    MachineBasicBlock *emitINSERT_DF_VIDX(MachineInstr &MI,
                                          MachineBasicBlock *BB,
                                          unsigned EltSizeInBytes,
                                          bool IsFP) const;
    /// Emit the FILL_FW pseudo instruction
    MachineBasicBlock *emitFILL_FW(MachineInstr &MI,
                                   MachineBasicBlock *BB) const;
    /// Emit the FILL_FD pseudo instruction
    MachineBasicBlock *emitFILL_FD(MachineInstr &MI,
                                   MachineBasicBlock *BB) const;
    /// Emit the FEXP2_W_1 pseudo instructions.
    MachineBasicBlock *emitFEXP2_W_1(MachineInstr &MI,
                                     MachineBasicBlock *BB) const;
    /// Emit the FEXP2_D_1 pseudo instructions.
    MachineBasicBlock *emitFEXP2_D_1(MachineInstr &MI,
                                     MachineBasicBlock *BB) const;
    /// Emit the FILL_FW pseudo instruction
    MachineBasicBlock *emitLD_F16_PSEUDO(MachineInstr &MI,
                                   MachineBasicBlock *BB) const;
    /// Emit the FILL_FD pseudo instruction
    MachineBasicBlock *emitST_F16_PSEUDO(MachineInstr &MI,
                                   MachineBasicBlock *BB) const;
    /// Emit the FEXP2_W_1 pseudo instructions.
    MachineBasicBlock *emitFPEXTEND_PSEUDO(MachineInstr &MI,
                                           MachineBasicBlock *BB,
                                           bool IsFGR64) const;
    /// Emit the FEXP2_D_1 pseudo instructions.
    MachineBasicBlock *emitFPROUND_PSEUDO(MachineInstr &MI,
                                          MachineBasicBlock *BBi,
                                          bool IsFGR64) const;
  };

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_MIPS_MIPSSEISELLOWERING_H
