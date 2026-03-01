//===-- AVRISelLowering.h - AVR DAG Lowering Interface ----------*- C++ -*-===//
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
// This file defines the interfaces that AVR uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_AVR_ISEL_LOWERING_H
#define LLVM_AVR_ISEL_LOWERING_H

#include "vm/core/CodeGen/CallingConvLower.h"
#include "vm/core/CodeGen/TargetLowering.h"

namespace vm::core {

class AVRSubtarget;
class AVRTargetMachine;

/// Performs target lowering for the AVR.
class AVRTargetLowering : public TargetLowering {
public:
  explicit AVRTargetLowering(const AVRTargetMachine &TM,
                             const AVRSubtarget &STI);

public:
  MVT getScalarShiftAmountTy(const DataLayout &, EVT LHSTy) const override {
    return MVT::i8;
  }

  MVT::SimpleValueType getCmpLibcallReturnType() const override {
    return MVT::i8;
  }

  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

  void ReplaceNodeResults(SDNode *N, SmallVectorImpl<SDValue> &Results,
                          SelectionDAG &DAG) const override;

  bool isLegalAddressingMode(const DataLayout &DL, const AddrMode &AM, Type *Ty,
                             unsigned AS,
                             Instruction *I = nullptr) const override;

  bool getPreIndexedAddressParts(SDNode *N, SDValue &Base, SDValue &Offset,
                                 ISD::MemIndexedMode &AM,
                                 SelectionDAG &DAG) const override;

  bool getPostIndexedAddressParts(SDNode *N, SDNode *Op, SDValue &Base,
                                  SDValue &Offset, ISD::MemIndexedMode &AM,
                                  SelectionDAG &DAG) const override;

  bool isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const override;

  EVT getSetCCResultType(const DataLayout &DL, LLVMContext &Context,
                         EVT VT) const override;

  MachineBasicBlock *
  EmitInstrWithCustomInserter(MachineInstr &MI,
                              MachineBasicBlock *MBB) const override;

  ConstraintType getConstraintType(StringRef Constraint) const override;

  ConstraintWeight
  getSingleConstraintMatchWeight(AsmOperandInfo &info,
                                 const char *constraint) const override;

  std::pair<unsigned, const TargetRegisterClass *>
  getRegForInlineAsmConstraint(const TargetRegisterInfo *TRI,
                               StringRef Constraint, MVT VT) const override;

  InlineAsm::ConstraintCode
  getInlineAsmMemConstraint(StringRef ConstraintCode) const override;

  void LowerAsmOperandForConstraint(SDValue Op, StringRef Constraint,
                                    std::vector<SDValue> &Ops,
                                    SelectionDAG &DAG) const override;

  Register getRegisterByName(const char *RegName, LLT VT,
                             const MachineFunction &MF) const override;

  bool shouldSplitFunctionArgumentsAsLittleEndian(
      const DataLayout &DL) const override {
    return false;
  }

  ShiftLegalizationStrategy
  preferredShiftLegalizationStrategy(SelectionDAG &DAG, SDNode *N,
                                     unsigned ExpansionFactor) const override {
    return ShiftLegalizationStrategy::LowerToLibcall;
  }

private:
  SDValue getAVRCmp(SDValue LHS, SDValue RHS, ISD::CondCode CC, SDValue &AVRcc,
                    SelectionDAG &DAG, SDLoc dl) const;
  SDValue getAVRCmp(SDValue LHS, SDValue RHS, SelectionDAG &DAG,
                    SDLoc dl) const;
  SDValue LowerShifts(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerDivRem(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerBlockAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerBR_CC(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerINLINEASM(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSETCC(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerVASTART(SDValue Op, SelectionDAG &DAG) const;

  bool CanLowerReturn(CallingConv::ID CallConv, MachineFunction &MF,
                      bool isVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      LLVMContext &Context, const Type *RetTy) const override;

  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, const SDLoc &dl,
                      SelectionDAG &DAG) const override;
  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool isVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &dl, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;
  SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
                    SmallVectorImpl<SDValue> &InVals) const override;
  SDValue LowerCallResult(SDValue Chain, SDValue InGlue,
                          CallingConv::ID CallConv, bool isVarArg,
                          const SmallVectorImpl<ISD::InputArg> &Ins,
                          const SDLoc &dl, SelectionDAG &DAG,
                          SmallVectorImpl<SDValue> &InVals) const;

protected:
  const AVRSubtarget &Subtarget;

private:
  MachineBasicBlock *insertShift(MachineInstr &MI, MachineBasicBlock *BB,
                                 bool Tiny) const;
  MachineBasicBlock *insertWideShift(MachineInstr &MI,
                                     MachineBasicBlock *BB) const;
  MachineBasicBlock *insertMul(MachineInstr &MI, MachineBasicBlock *BB) const;
  MachineBasicBlock *insertCopyZero(MachineInstr &MI,
                                    MachineBasicBlock *BB) const;
  MachineBasicBlock *insertAtomicArithmeticOp(MachineInstr &MI,
                                              MachineBasicBlock *BB,
                                              unsigned Opcode, int Width) const;
};

} // end namespace vm::core

#endif // LLVM_AVR_ISEL_LOWERING_H
