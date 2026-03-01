//===-- MipsSEISelDAGToDAG.h - A Dag to Dag Inst Selector for MipsSE -----===//
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
// Subclass of MipsDAGToDAGISel specialized for mips32/64.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MIPS_MIPSSEISELDAGTODAG_H
#define LLVM_LIB_TARGET_MIPS_MIPSSEISELDAGTODAG_H

#include "MipsISelDAGToDAG.h"

namespace vm::core {

class MipsSEDAGToDAGISel : public MipsDAGToDAGISel {

public:
  explicit MipsSEDAGToDAGISel(MipsTargetMachine &TM, CodeGenOptLevel OL)
      : MipsDAGToDAGISel(TM, OL) {}

private:

  bool runOnMachineFunction(MachineFunction &MF) override;

  void addDSPCtrlRegOperands(bool IsDef, MachineInstr &MI,
                             MachineFunction &MF);

  MCRegister getMSACtrlReg(const SDValue RegIdx) const;

  bool replaceUsesWithZeroReg(MachineRegisterInfo *MRI, const MachineInstr&);

  std::pair<SDNode *, SDNode *> selectMULT(SDNode *N, unsigned Opc,
                                           const SDLoc &dl, EVT Ty, bool HasLo,
                                           bool HasHi);

  void selectAddE(SDNode *Node, const SDLoc &DL) const;

  bool selectAddrFrameIndex(SDValue Addr, SDValue &Base, SDValue &Offset) const;
  bool selectAddrFrameIndexOffset(SDValue Addr, SDValue &Base, SDValue &Offset,
                                  unsigned OffsetBits,
                                  unsigned ShiftAmount) const;

  bool selectAddrRegImm(SDValue Addr, SDValue &Base,
                        SDValue &Offset) const override;

  bool selectAddrDefault(SDValue Addr, SDValue &Base,
                         SDValue &Offset) const override;

  bool selectIntAddr(SDValue Addr, SDValue &Base,
                     SDValue &Offset) const override;

  bool selectAddrRegImm9(SDValue Addr, SDValue &Base,
                         SDValue &Offset) const;

  bool selectAddrRegImm11(SDValue Addr, SDValue &Base,
                          SDValue &Offset) const;

  bool selectAddrRegImm12(SDValue Addr, SDValue &Base,
                          SDValue &Offset) const;

  bool selectAddrRegImm16(SDValue Addr, SDValue &Base,
                          SDValue &Offset) const;

  bool selectIntAddr11MM(SDValue Addr, SDValue &Base,
                         SDValue &Offset) const override;

  bool selectIntAddr12MM(SDValue Addr, SDValue &Base,
                         SDValue &Offset) const override;

  bool selectIntAddr16MM(SDValue Addr, SDValue &Base,
                         SDValue &Offset) const override;

  bool selectIntAddrLSL2MM(SDValue Addr, SDValue &Base,
                           SDValue &Offset) const override;

  bool selectIntAddrSImm10(SDValue Addr, SDValue &Base,
                           SDValue &Offset) const override;

  bool selectIntAddrSImm10Lsl1(SDValue Addr, SDValue &Base,
                               SDValue &Offset) const override;

  bool selectIntAddrSImm10Lsl2(SDValue Addr, SDValue &Base,
                               SDValue &Offset) const override;

  bool selectIntAddrSImm10Lsl3(SDValue Addr, SDValue &Base,
                               SDValue &Offset) const override;

  /// Select constant vector splats.
  bool selectVSplat(SDNode *N, APInt &Imm,
                    unsigned MinSizeInBits) const override;
  /// Select constant vector splats whose value fits in a given integer.
  bool selectVSplatCommon(SDValue N, SDValue &Imm, bool Signed,
                          unsigned ImmBitSize) const override;
  /// Select constant vector splats whose value is a power of 2.
  bool selectVSplatUimmPow2(SDValue N, SDValue &Imm) const override;
  /// Select constant vector splats whose value is the inverse of a
  /// power of 2.
  bool selectVSplatUimmInvPow2(SDValue N, SDValue &Imm) const override;
  /// Select constant vector splats whose value is a run of set bits
  /// ending at the most significant bit.
  bool selectVSplatMaskL(SDValue N, SDValue &Imm) const override;
  /// Select constant vector splats whose value is a run of set bits
  /// starting at bit zero.
  bool selectVSplatMaskR(SDValue N, SDValue &Imm) const override;

  /// Select constant vector splats whose value is 1.
  bool selectVSplatImmEq1(SDValue N) const override;

  bool trySelect(SDNode *Node) override;

  // Emits proper ABI for _mcount profiling calls.
  void emitMCountABI(MachineInstr &MI, MachineBasicBlock &MBB,
                     MachineFunction &MF);

  void processFunctionAfterISel(MachineFunction &MF) override;

  bool SelectInlineAsmMemoryOperand(const SDValue &Op,
                                    InlineAsm::ConstraintCode ConstraintID,
                                    std::vector<SDValue> &OutOps) override;
};

class MipsSEDAGToDAGISelLegacy : public MipsDAGToDAGISelLegacy {
public:
  explicit MipsSEDAGToDAGISelLegacy(MipsTargetMachine &TM, CodeGenOptLevel OL);
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

FunctionPass *createMipsSEISelDag(MipsTargetMachine &TM,
                                  CodeGenOptLevel OptLevel);
}

#endif
