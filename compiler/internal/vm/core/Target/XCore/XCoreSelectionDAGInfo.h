//===-- XCoreSelectionDAGInfo.h - XCore SelectionDAG Info -------*- C++ -*-===//
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
// This file defines the XCore subclass for SelectionDAGTargetInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_XCORE_XCORESELECTIONDAGINFO_H
#define LLVM_LIB_TARGET_XCORE_XCORESELECTIONDAGINFO_H

#include "vm/core/CodeGen/SelectionDAGTargetInfo.h"

#define GET_SDNODE_ENUM
#include "XCoreGenSDNodeInfo.inc"

namespace vm::core {

class XCoreSelectionDAGInfo : public SelectionDAGGenTargetInfo {
public:
  XCoreSelectionDAGInfo();

  SDValue EmitTargetCodeForMemcpy(SelectionDAG &DAG, const SDLoc &dl,
                                  SDValue Chain, SDValue Op1, SDValue Op2,
                                  SDValue Op3, Align Alignment, bool isVolatile,
                                  bool AlwaysInline,
                                  MachinePointerInfo DstPtrInfo,
                                  MachinePointerInfo SrcPtrInfo) const override;
};

} // namespace vm::core

#endif
