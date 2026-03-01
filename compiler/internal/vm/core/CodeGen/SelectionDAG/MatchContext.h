//===---------------- toolchain/CodeGen/MatchContext.h  --------------*- C++ -*-===//
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
// This file declares the EmptyMatchContext class and VPMatchContext class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_CODEGEN_SELECTIONDAG_MATCHCONTEXT_H
#define LLVM_LIB_CODEGEN_SELECTIONDAG_MATCHCONTEXT_H

#include "vm/core/CodeGen/SelectionDAG.h"
#include "vm/core/CodeGen/TargetLowering.h"

namespace vm::core {

class EmptyMatchContext {
  SelectionDAG &DAG;
  const TargetLowering &TLI;
  SDNode *Root;

public:
  EmptyMatchContext(SelectionDAG &DAG, const TargetLowering &TLI, SDNode *Root)
      : DAG(DAG), TLI(TLI), Root(Root) {}

  unsigned getRootBaseOpcode() { return Root->getOpcode(); }
  bool match(SDValue OpN, unsigned Opcode) const {
    return Opcode == OpN->getOpcode();
  }

  // Same as SelectionDAG::getNode().
  template <typename... ArgT> SDValue getNode(ArgT &&...Args) {
    return DAG.getNode(std::forward<ArgT>(Args)...);
  }

  bool isOperationLegal(unsigned Op, EVT VT) const {
    return TLI.isOperationLegal(Op, VT);
  }

  bool isOperationLegalOrCustom(unsigned Op, EVT VT,
                                bool LegalOnly = false) const {
    return TLI.isOperationLegalOrCustom(Op, VT, LegalOnly);
  }

  unsigned getNumOperands(SDValue N) const { return N->getNumOperands(); }
};

class VPMatchContext {
  SelectionDAG &DAG;
  const TargetLowering &TLI;
  SDValue RootMaskOp;
  SDValue RootVectorLenOp;
  SDNode *Root;

public:
  VPMatchContext(SelectionDAG &DAG, const TargetLowering &TLI, SDNode *_Root)
      : DAG(DAG), TLI(TLI), RootMaskOp(), RootVectorLenOp() {
    Root = _Root;
    assert(Root->isVPOpcode());
    if (auto RootMaskPos = ISD::getVPMaskIdx(Root->getOpcode()))
      RootMaskOp = Root->getOperand(*RootMaskPos);
    else if (Root->getOpcode() == ISD::VP_SELECT)
      RootMaskOp = DAG.getAllOnesConstant(SDLoc(Root),
                                          Root->getOperand(0).getValueType());

    if (auto RootVLenPos = ISD::getVPExplicitVectorLengthIdx(Root->getOpcode()))
      RootVectorLenOp = Root->getOperand(*RootVLenPos);
  }

  unsigned getRootBaseOpcode() {
    std::optional<unsigned> Opcode = ISD::getBaseOpcodeForVP(
        Root->getOpcode(), !Root->getFlags().hasNoFPExcept());
    assert(Opcode.has_value());
    return *Opcode;
  }

  /// whether \p OpVal is a node that is functionally compatible with the
  /// NodeType \p Opc
  bool match(SDValue OpVal, unsigned Opc) const {
    if (!OpVal->isVPOpcode())
      return OpVal->getOpcode() == Opc;

    auto BaseOpc = ISD::getBaseOpcodeForVP(OpVal->getOpcode(),
                                           !OpVal->getFlags().hasNoFPExcept());
    if (BaseOpc != Opc)
      return false;

    // Make sure the mask of OpVal is true mask or is same as Root's.
    unsigned VPOpcode = OpVal->getOpcode();
    if (auto MaskPos = ISD::getVPMaskIdx(VPOpcode)) {
      SDValue MaskOp = OpVal.getOperand(*MaskPos);
      if (RootMaskOp != MaskOp &&
          !ISD::isConstantSplatVectorAllOnes(MaskOp.getNode()))
        return false;
    }

    // Make sure the EVL of OpVal is same as Root's.
    if (auto VLenPos = ISD::getVPExplicitVectorLengthIdx(VPOpcode))
      if (RootVectorLenOp != OpVal.getOperand(*VLenPos))
        return false;
    return true;
  }

  // Specialize based on number of operands.
  // TODO emit VP intrinsics where MaskOp/VectorLenOp != null
  // SDValue getNode(unsigned Opcode, const SDLoc &DL, EVT VT) { return
  // DAG.getNode(Opcode, DL, VT); }
  SDValue getNode(unsigned Opcode, const SDLoc &DL, EVT VT, SDValue Operand) {
    unsigned VPOpcode = *ISD::getVPForBaseOpcode(Opcode);
    assert(ISD::getVPMaskIdx(VPOpcode) == 1 &&
           ISD::getVPExplicitVectorLengthIdx(VPOpcode) == 2);
    return DAG.getNode(VPOpcode, DL, VT,
                       {Operand, RootMaskOp, RootVectorLenOp});
  }

  SDValue getNode(unsigned Opcode, const SDLoc &DL, EVT VT, SDValue N1,
                  SDValue N2) {
    unsigned VPOpcode = *ISD::getVPForBaseOpcode(Opcode);
    assert(ISD::getVPMaskIdx(VPOpcode) == 2 &&
           ISD::getVPExplicitVectorLengthIdx(VPOpcode) == 3);
    return DAG.getNode(VPOpcode, DL, VT, {N1, N2, RootMaskOp, RootVectorLenOp});
  }

  SDValue getNode(unsigned Opcode, const SDLoc &DL, EVT VT, SDValue N1,
                  SDValue N2, SDValue N3) {
    unsigned VPOpcode = *ISD::getVPForBaseOpcode(Opcode);
    assert(ISD::getVPMaskIdx(VPOpcode) == 3 &&
           ISD::getVPExplicitVectorLengthIdx(VPOpcode) == 4);
    return DAG.getNode(VPOpcode, DL, VT,
                       {N1, N2, N3, RootMaskOp, RootVectorLenOp});
  }

  SDValue getNode(unsigned Opcode, const SDLoc &DL, EVT VT, SDValue Operand,
                  SDNodeFlags Flags) {
    unsigned VPOpcode = *ISD::getVPForBaseOpcode(Opcode);
    assert(ISD::getVPMaskIdx(VPOpcode) == 1 &&
           ISD::getVPExplicitVectorLengthIdx(VPOpcode) == 2);
    return DAG.getNode(VPOpcode, DL, VT, {Operand, RootMaskOp, RootVectorLenOp},
                       Flags);
  }

  SDValue getNode(unsigned Opcode, const SDLoc &DL, EVT VT, SDValue N1,
                  SDValue N2, SDNodeFlags Flags) {
    unsigned VPOpcode = *ISD::getVPForBaseOpcode(Opcode);
    assert(ISD::getVPMaskIdx(VPOpcode) == 2 &&
           ISD::getVPExplicitVectorLengthIdx(VPOpcode) == 3);
    return DAG.getNode(VPOpcode, DL, VT, {N1, N2, RootMaskOp, RootVectorLenOp},
                       Flags);
  }

  SDValue getNode(unsigned Opcode, const SDLoc &DL, EVT VT, SDValue N1,
                  SDValue N2, SDValue N3, SDNodeFlags Flags) {
    unsigned VPOpcode = *ISD::getVPForBaseOpcode(Opcode);
    assert(ISD::getVPMaskIdx(VPOpcode) == 3 &&
           ISD::getVPExplicitVectorLengthIdx(VPOpcode) == 4);
    return DAG.getNode(VPOpcode, DL, VT,
                       {N1, N2, N3, RootMaskOp, RootVectorLenOp}, Flags);
  }

  bool isOperationLegal(unsigned Op, EVT VT) const {
    unsigned VPOp = *ISD::getVPForBaseOpcode(Op);
    return TLI.isOperationLegal(VPOp, VT);
  }

  bool isOperationLegalOrCustom(unsigned Op, EVT VT,
                                bool LegalOnly = false) const {
    unsigned VPOp = *ISD::getVPForBaseOpcode(Op);
    return TLI.isOperationLegalOrCustom(VPOp, VT, LegalOnly);
  }

  unsigned getNumOperands(SDValue N) const {
    return N->isVPOpcode() ? N->getNumOperands() - 2 : N->getNumOperands();
  }
};

} // namespace vm::core

#endif // LLVM_LIB_CODEGEN_SELECTIONDAG_MATCHCONTEXT_H
