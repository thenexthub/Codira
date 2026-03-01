//===-- BPFSelectionDAGInfo.cpp - BPF SelectionDAG Info -------------------===//
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
// This file implements the BPFSelectionDAGInfo class.
//
//===----------------------------------------------------------------------===//

#include "BPFSelectionDAGInfo.h"
#include "BPFTargetMachine.h"
#include "vm/core/CodeGen/SelectionDAG.h"

#define GET_SDNODE_DESC
#include "BPFGenSDNodeInfo.inc"

using namespace vm::core;

#define DEBUG_TYPE "bpf-selectiondag-info"

BPFSelectionDAGInfo::BPFSelectionDAGInfo()
    : SelectionDAGGenTargetInfo(BPFGenSDNodeInfo) {}

SDValue BPFSelectionDAGInfo::EmitTargetCodeForMemcpy(
    SelectionDAG &DAG, const SDLoc &dl, SDValue Chain, SDValue Dst, SDValue Src,
    SDValue Size, Align Alignment, bool isVolatile, bool AlwaysInline,
    MachinePointerInfo DstPtrInfo, MachinePointerInfo SrcPtrInfo) const {
  // Requires the copy size to be a constant.
  ConstantSDNode *ConstantSize = dyn_cast<ConstantSDNode>(Size);
  if (!ConstantSize)
    return SDValue();

  unsigned CopyLen = ConstantSize->getZExtValue();
  unsigned StoresNumEstimate = alignTo(CopyLen, Alignment) >> Log2(Alignment);
  // Impose the same copy length limit as MaxStoresPerMemcpy.
  if (StoresNumEstimate > getCommonMaxStoresPerMemFunc())
    return SDValue();

  return DAG.getNode(BPFISD::MEMCPY, dl, MVT::Other, Chain, Dst, Src,
                     DAG.getConstant(CopyLen, dl, MVT::i64),
                     DAG.getConstant(Alignment.value(), dl, MVT::i64));
}
