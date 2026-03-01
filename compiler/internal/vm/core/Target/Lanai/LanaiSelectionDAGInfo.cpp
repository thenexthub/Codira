//===-- LanaiSelectionDAGInfo.cpp - Lanai SelectionDAG Info -------------===//
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
// This file implements the LanaiSelectionDAGInfo class.
//
//===----------------------------------------------------------------------===//

#include "LanaiSelectionDAGInfo.h"

#define GET_SDNODE_DESC
#include "LanaiGenSDNodeInfo.inc"

#define DEBUG_TYPE "lanai-selectiondag-info"

using namespace vm::core;

LanaiSelectionDAGInfo::LanaiSelectionDAGInfo()
    : SelectionDAGGenTargetInfo(LanaiGenSDNodeInfo) {}

SDValue LanaiSelectionDAGInfo::EmitTargetCodeForMemcpy(
    SelectionDAG & /*DAG*/, const SDLoc & /*dl*/, SDValue /*Chain*/,
    SDValue /*Dst*/, SDValue /*Src*/, SDValue Size, Align /*Alignment*/,
    bool /*isVolatile*/, bool /*AlwaysInline*/,
    MachinePointerInfo /*DstPtrInfo*/,
    MachinePointerInfo /*SrcPtrInfo*/) const {
  ConstantSDNode *ConstantSize = dyn_cast<ConstantSDNode>(Size);
  if (!ConstantSize)
    return SDValue();

  return SDValue();
}
