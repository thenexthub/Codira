//===- ARCISelLowering.h - ARC DAG Lowering Interface -----------*- C++ -*-===//
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
// This file defines the interfaces that ARC uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ARC_ARCISELLOWERING_H
#define LLVM_LIB_TARGET_ARC_ARCISELLOWERING_H

#include "ARC.h"
#include "vm/core/CodeGen/SelectionDAG.h"
#include "vm/core/CodeGen/TargetLowering.h"

namespace vm::core {

// Forward delcarations
class ARCSubtarget;
class ARCTargetMachine;

//===--------------------------------------------------------------------===//
// TargetLowering Implementation
//===--------------------------------------------------------------------===//
class ARCTargetLowering : public TargetLowering {
public:
  explicit ARCTargetLowering(const TargetMachine &TM,
                             const ARCSubtarget &Subtarget);

  /// Provide custom lowering hooks for some operations.
  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

  /// Return true if the addressing mode represented by AM is legal for this
  /// target, for a load/store of the specified type.
  bool isLegalAddressingMode(const DataLayout &DL, const AddrMode &AM, Type *Ty,
                             unsigned AS,
                             Instruction *I = nullptr) const override;

private:
  const ARCSubtarget &Subtarget;

  void ReplaceNodeResults(SDNode *N, SmallVectorImpl<SDValue> &Results,
                          SelectionDAG &DAG) const override;

  // Lower Operand helpers
  SDValue LowerCallArguments(SDValue Chain, CallingConv::ID CallConv,
                             bool isVarArg,
                             const SmallVectorImpl<ISD::InputArg> &Ins,
                             SDLoc dl, SelectionDAG &DAG,
                             SmallVectorImpl<SDValue> &InVals) const;
  // Lower Operand specifics
  SDValue LowerJumpTable(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerFRAMEADDR(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerBR_CC(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSIGN_EXTEND_INREG(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI) const override;

  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool isVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &dl, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
                    SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, const SDLoc &dl,
                      SelectionDAG &DAG) const override;

  bool CanLowerReturn(CallingConv::ID CallConv, MachineFunction &MF,
                      bool isVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &ArgsFlags,
                      LLVMContext &Context, const Type *RetTy) const override;

  bool mayBeEmittedAsTailCall(const CallInst *CI) const override;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_ARC_ARCISELLOWERING_H
