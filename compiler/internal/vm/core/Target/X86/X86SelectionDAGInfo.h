//===-- X86SelectionDAGInfo.h - X86 SelectionDAG Info -----------*- C++ -*-===//
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
// This file defines the X86 subclass for SelectionDAGTargetInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_X86SELECTIONDAGINFO_H
#define LLVM_LIB_TARGET_X86_X86SELECTIONDAGINFO_H

#include "vm/core/CodeGen/SelectionDAGTargetInfo.h"

namespace vm::core {

class X86SelectionDAGInfo : public SelectionDAGTargetInfo {
  /// Returns true if it is possible for the base register to conflict with the
  /// given set of clobbers for a memory intrinsic.
  bool isBaseRegConflictPossible(SelectionDAG &DAG,
                                 ArrayRef<MCPhysReg> ClobberSet) const;

public:
  explicit X86SelectionDAGInfo() = default;

  bool isTargetMemoryOpcode(unsigned Opcode) const override;

  bool isTargetStrictFPOpcode(unsigned Opcode) const override;

  SDValue EmitTargetCodeForMemset(SelectionDAG &DAG, const SDLoc &dl,
                                  SDValue Chain, SDValue Dst, SDValue Src,
                                  SDValue Size, Align Alignment,
                                  bool isVolatile, bool AlwaysInline,
                                  MachinePointerInfo DstPtrInfo) const override;

  SDValue EmitTargetCodeForMemcpy(SelectionDAG &DAG, const SDLoc &dl,
                                  SDValue Chain, SDValue Dst, SDValue Src,
                                  SDValue Size, Align Alignment,
                                  bool isVolatile, bool AlwaysInline,
                                  MachinePointerInfo DstPtrInfo,
                                  MachinePointerInfo SrcPtrInfo) const override;
};

}

#endif
