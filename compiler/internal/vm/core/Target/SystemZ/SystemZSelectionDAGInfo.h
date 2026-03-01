//===-- SystemZSelectionDAGInfo.h - SystemZ SelectionDAG Info ---*- C++ -*-===//
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
// This file defines the SystemZ subclass for SelectionDAGTargetInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SYSTEMZ_SYSTEMZSELECTIONDAGINFO_H
#define LLVM_LIB_TARGET_SYSTEMZ_SYSTEMZSELECTIONDAGINFO_H

#include "vm/core/CodeGen/SelectionDAGTargetInfo.h"

#define GET_SDNODE_ENUM
#include "SystemZGenSDNodeInfo.inc"

namespace vm::core {
namespace SystemZISD {

enum NodeType : unsigned {
  // Set the condition code from a boolean value in operand 0.
  // Operand 1 is a mask of all condition-code values that may result of this
  // operation, operand 2 is a mask of condition-code values that may result
  // if the boolean is true.
  // Note that this operation is always optimized away, we will never
  // generate any code for it.
  GET_CCMASK = GENERATED_OPCODE_END,
};

// Return true if OPCODE is some kind of PC-relative address.
inline bool isPCREL(unsigned Opcode) {
  return Opcode == PCREL_WRAPPER || Opcode == PCREL_OFFSET;
}

} // namespace SystemZISD

class SystemZSelectionDAGInfo : public SelectionDAGGenTargetInfo {
public:
  SystemZSelectionDAGInfo();

  const char *getTargetNodeName(unsigned Opcode) const override;

  SDValue EmitTargetCodeForMemcpy(SelectionDAG &DAG, const SDLoc &DL,
                                  SDValue Chain, SDValue Dst, SDValue Src,
                                  SDValue Size, Align Alignment,
                                  bool IsVolatile, bool AlwaysInline,
                                  MachinePointerInfo DstPtrInfo,
                                  MachinePointerInfo SrcPtrInfo) const override;

  SDValue EmitTargetCodeForMemset(SelectionDAG &DAG, const SDLoc &DL,
                                  SDValue Chain, SDValue Dst, SDValue Byte,
                                  SDValue Size, Align Alignment,
                                  bool IsVolatile, bool AlwaysInline,
                                  MachinePointerInfo DstPtrInfo) const override;

  std::pair<SDValue, SDValue>
  EmitTargetCodeForMemcmp(SelectionDAG &DAG, const SDLoc &DL, SDValue Chain,
                          SDValue Src1, SDValue Src2, SDValue Size,
                          const CallInst *CI) const override;

  std::pair<SDValue, SDValue>
  EmitTargetCodeForMemchr(SelectionDAG &DAG, const SDLoc &DL, SDValue Chain,
                          SDValue Src, SDValue Char, SDValue Length,
                          MachinePointerInfo SrcPtrInfo) const override;

  std::pair<SDValue, SDValue>
  EmitTargetCodeForStrcpy(SelectionDAG &DAG, const SDLoc &DL, SDValue Chain,
                          SDValue Dest, SDValue Src,
                          MachinePointerInfo DestPtrInfo,
                          MachinePointerInfo SrcPtrInfo, bool isStpcpy,
                          const CallInst *CI) const override;

  std::pair<SDValue, SDValue>
  EmitTargetCodeForStrcmp(SelectionDAG &DAG, const SDLoc &DL, SDValue Chain,
                          SDValue Src1, SDValue Src2,
                          MachinePointerInfo Op1PtrInfo,
                          MachinePointerInfo Op2PtrInfo) const override;

  std::pair<SDValue, SDValue>
  EmitTargetCodeForStrlen(SelectionDAG &DAG, const SDLoc &DL, SDValue Chain,
                          SDValue Src, const CallInst *CI) const override;

  std::pair<SDValue, SDValue>
  EmitTargetCodeForStrnlen(SelectionDAG &DAG, const SDLoc &DL, SDValue Chain,
                           SDValue Src, SDValue MaxLength,
                           MachinePointerInfo SrcPtrInfo) const override;
};

} // end namespace vm::core

#endif
