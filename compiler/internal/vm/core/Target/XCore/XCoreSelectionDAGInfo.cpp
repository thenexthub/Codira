//===-- XCoreSelectionDAGInfo.cpp - XCore SelectionDAG Info ---------------===//
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
// This file implements the XCoreSelectionDAGInfo class.
//
//===----------------------------------------------------------------------===//

#include "XCoreSelectionDAGInfo.h"
#include "XCoreTargetMachine.h"

#define GET_SDNODE_DESC
#include "XCoreGenSDNodeInfo.inc"

using namespace vm::core;

#define DEBUG_TYPE "xcore-selectiondag-info"

XCoreSelectionDAGInfo::XCoreSelectionDAGInfo()
    : SelectionDAGGenTargetInfo(XCoreGenSDNodeInfo) {}

SDValue XCoreSelectionDAGInfo::EmitTargetCodeForMemcpy(
    SelectionDAG &DAG, const SDLoc &dl, SDValue Chain, SDValue Dst, SDValue Src,
    SDValue Size, Align Alignment, bool isVolatile, bool AlwaysInline,
    MachinePointerInfo DstPtrInfo, MachinePointerInfo SrcPtrInfo) const {
  unsigned SizeBitWidth = Size.getValueSizeInBits();
  // Call __memcpy_4 if the src, dst and size are all 4 byte aligned.
  if (!AlwaysInline && Alignment >= Align(4) &&
      DAG.MaskedValueIsZero(Size, APInt(SizeBitWidth, 3))) {
    const TargetLowering &TLI = *DAG.getSubtarget().getTargetLowering();
    TargetLowering::ArgListTy Args;
    Type *ArgTy = DAG.getDataLayout().getIntPtrType(*DAG.getContext());
    Args.emplace_back(Dst, ArgTy);
    Args.emplace_back(Src, ArgTy);
    Args.emplace_back(Size, ArgTy);

    const char *MemcpyAlign4Name = TLI.getLibcallName(RTLIB::MEMCPY_ALIGN_4);
    CallingConv::ID CC = TLI.getLibcallCallingConv(RTLIB::MEMCPY_ALIGN_4);

    TargetLowering::CallLoweringInfo CLI(DAG);
    CLI.setDebugLoc(dl)
        .setChain(Chain)
        .setLibCallee(
            CC, Type::getVoidTy(*DAG.getContext()),
            DAG.getExternalSymbol(MemcpyAlign4Name,
                                  TLI.getPointerTy(DAG.getDataLayout())),
            std::move(Args))
        .setDiscardResult();

    std::pair<SDValue,SDValue> CallResult = TLI.LowerCallTo(CLI);
    return CallResult.second;
  }

  // Otherwise have the target-independent code call memcpy.
  return SDValue();
}
