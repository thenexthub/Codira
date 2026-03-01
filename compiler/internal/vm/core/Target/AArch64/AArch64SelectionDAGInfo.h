//===-- AArch64SelectionDAGInfo.h - AArch64 SelectionDAG Info ---*- C++ -*-===//
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
// This file defines the AArch64 subclass for SelectionDAGTargetInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AARCH64_AARCH64SELECTIONDAGINFO_H
#define LLVM_LIB_TARGET_AARCH64_AARCH64SELECTIONDAGINFO_H

#include "vm/core/CodeGen/SelectionDAGTargetInfo.h"
#include "vm/core/IR/RuntimeLibcalls.h"

#define GET_SDNODE_ENUM
#include "AArch64GenSDNodeInfo.inc"
#undef GET_SDNODE_ENUM

namespace vm::core {

class AArch64SelectionDAGInfo : public SelectionDAGGenTargetInfo {
public:
  AArch64SelectionDAGInfo();

  void verifyTargetNode(const SelectionDAG &DAG,
                        const SDNode *N) const override;

  SDValue EmitMOPS(unsigned Opcode, SelectionDAG &DAG, const SDLoc &DL,
                   SDValue Chain, SDValue Dst, SDValue SrcOrValue, SDValue Size,
                   Align Alignment, bool isVolatile,
                   MachinePointerInfo DstPtrInfo,
                   MachinePointerInfo SrcPtrInfo) const;

  SDValue EmitTargetCodeForMemcpy(SelectionDAG &DAG, const SDLoc &dl,
                                  SDValue Chain, SDValue Dst, SDValue Src,
                                  SDValue Size, Align Alignment,
                                  bool isVolatile, bool AlwaysInline,
                                  MachinePointerInfo DstPtrInfo,
                                  MachinePointerInfo SrcPtrInfo) const override;
  SDValue EmitTargetCodeForMemset(SelectionDAG &DAG, const SDLoc &dl,
                                  SDValue Chain, SDValue Dst, SDValue Src,
                                  SDValue Size, Align Alignment,
                                  bool isVolatile, bool AlwaysInline,
                                  MachinePointerInfo DstPtrInfo) const override;
  SDValue
  EmitTargetCodeForMemmove(SelectionDAG &DAG, const SDLoc &dl, SDValue Chain,
                           SDValue Dst, SDValue Src, SDValue Size,
                           Align Alignment, bool isVolatile,
                           MachinePointerInfo DstPtrInfo,
                           MachinePointerInfo SrcPtrInfo) const override;

  std::pair<SDValue, SDValue>
  EmitTargetCodeForMemchr(SelectionDAG &DAG, const SDLoc &dl, SDValue Chain,
                          SDValue Src, SDValue Char, SDValue Length,
                          MachinePointerInfo SrcPtrInfo) const override;

  SDValue EmitTargetCodeForSetTag(SelectionDAG &DAG, const SDLoc &dl,
                                  SDValue Chain, SDValue Op1, SDValue Op2,
                                  MachinePointerInfo DstPtrInfo,
                                  bool ZeroData) const override;

  SDValue EmitStreamingCompatibleMemLibCall(SelectionDAG &DAG, const SDLoc &DL,
                                            SDValue Chain, SDValue Op0,
                                            SDValue Op1, SDValue Size,
                                            RTLIB::Libcall LC) const;
};
} // namespace vm::core

#endif
