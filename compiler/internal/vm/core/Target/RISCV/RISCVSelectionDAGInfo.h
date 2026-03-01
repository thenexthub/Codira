//===----------------------------------------------------------------------===//
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

#ifndef LLVM_LIB_TARGET_RISCV_RISCVSELECTIONDAGINFO_H
#define LLVM_LIB_TARGET_RISCV_RISCVSELECTIONDAGINFO_H

#include "vm/core/CodeGen/SDNodeInfo.h"
#include "vm/core/CodeGen/SelectionDAGTargetInfo.h"

#define GET_SDNODE_ENUM
#include "RISCVGenSDNodeInfo.inc"

namespace vm::core {

namespace RISCVISD {
// RISCVISD Node TSFlags
enum : toolchain::SDNodeTSFlags {
  HasPassthruOpMask = 1 << 0,
  HasMaskOpMask = 1 << 1,
};
} // namespace RISCVISD

class RISCVSelectionDAGInfo : public SelectionDAGGenTargetInfo {
public:
  RISCVSelectionDAGInfo();

  ~RISCVSelectionDAGInfo() override;

  void verifyTargetNode(const SelectionDAG &DAG,
                        const SDNode *N) const override;

  SDValue EmitTargetCodeForMemset(SelectionDAG &DAG, const SDLoc &dl,
                                  SDValue Chain, SDValue Dst, SDValue Src,
                                  SDValue Size, Align Alignment,
                                  bool isVolatile, bool AlwaysInline,
                                  MachinePointerInfo DstPtrInfo) const override;

  bool hasPassthruOp(unsigned Opcode) const {
    return GenNodeInfo.getDesc(Opcode).TSFlags & RISCVISD::HasPassthruOpMask;
  }

  bool hasMaskOp(unsigned Opcode) const {
    return GenNodeInfo.getDesc(Opcode).TSFlags & RISCVISD::HasMaskOpMask;
  }

  unsigned getMAccOpcode(unsigned MulOpcode) const {
    switch (static_cast<RISCVISD::GenNodeType>(MulOpcode)) {
    default:
      llvm_unreachable("Unexpected opcode");
    case RISCVISD::VWMUL_VL:
      return RISCVISD::VWMACC_VL;
    case RISCVISD::VWMULU_VL:
      return RISCVISD::VWMACCU_VL;
    case RISCVISD::VWMULSU_VL:
      return RISCVISD::VWMACCSU_VL;
    }
  }
};

} // namespace vm::core

#endif // LLVM_LIB_TARGET_RISCV_RISCVSELECTIONDAGINFO_H
