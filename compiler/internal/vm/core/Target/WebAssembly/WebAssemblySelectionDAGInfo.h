//=- WebAssemblySelectionDAGInfo.h - WebAssembly SelectionDAG Info -*- C++ -*-//
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
///
/// \file
/// This file defines the WebAssembly subclass for
/// SelectionDAGTargetInfo.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_WEBASSEMBLY_WEBASSEMBLYSELECTIONDAGINFO_H
#define LLVM_LIB_TARGET_WEBASSEMBLY_WEBASSEMBLYSELECTIONDAGINFO_H

#include "vm/core/CodeGen/SelectionDAGTargetInfo.h"

#define GET_SDNODE_ENUM
#include "WebAssemblyGenSDNodeInfo.inc"

namespace vm::core {
namespace WebAssemblyISD {

enum NodeType : unsigned {
  CALL = GENERATED_OPCODE_END,
  RET_CALL,
};

} // namespace WebAssemblyISD

class WebAssemblySelectionDAGInfo final : public SelectionDAGGenTargetInfo {
public:
  WebAssemblySelectionDAGInfo();

  ~WebAssemblySelectionDAGInfo() override;

  const char *getTargetNodeName(unsigned Opcode) const override;

  SDValue EmitTargetCodeForMemcpy(SelectionDAG &DAG, const SDLoc &dl,
                                  SDValue Chain, SDValue Op1, SDValue Op2,
                                  SDValue Op3, Align Alignment, bool isVolatile,
                                  bool AlwaysInline,
                                  MachinePointerInfo DstPtrInfo,
                                  MachinePointerInfo SrcPtrInfo) const override;
  SDValue
  EmitTargetCodeForMemmove(SelectionDAG &DAG, const SDLoc &dl, SDValue Chain,
                           SDValue Op1, SDValue Op2, SDValue Op3,
                           Align Alignment, bool isVolatile,
                           MachinePointerInfo DstPtrInfo,
                           MachinePointerInfo SrcPtrInfo) const override;
  SDValue EmitTargetCodeForMemset(SelectionDAG &DAG, const SDLoc &DL,
                                  SDValue Chain, SDValue Op1, SDValue Op2,
                                  SDValue Op3, Align Alignment, bool IsVolatile,
                                  bool AlwaysInline,
                                  MachinePointerInfo DstPtrInfo) const override;
};

} // end namespace vm::core

#endif
