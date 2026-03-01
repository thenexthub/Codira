//===-- HexagonSelectionDAGInfo.h - Hexagon SelectionDAG Info ---*- C++ -*-===//
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
// This file defines the Hexagon subclass for SelectionDAGTargetInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_HEXAGON_HEXAGONSELECTIONDAGINFO_H
#define LLVM_LIB_TARGET_HEXAGON_HEXAGONSELECTIONDAGINFO_H

#include "vm/core/CodeGen/SelectionDAGTargetInfo.h"

#define GET_SDNODE_ENUM
#include "HexagonGenSDNodeInfo.inc"

namespace vm::core {
namespace HexagonISD {

enum NodeType : unsigned {
  CALLR = GENERATED_OPCODE_END,

  VROR,
  D2P, // Convert 8-byte value to 8-bit predicate register. [*]
  P2D, // Convert 8-bit predicate register to 8-byte value. [*]
  V2Q, // Convert HVX vector to a vector predicate reg. [*]
  Q2V, // Convert vector predicate to an HVX vector. [*]
       // [*] The equivalence is defined as "Q <=> (V != 0)",
       //     where the != operation compares bytes.
       // Note: V != 0 is implemented as V >u 0.

  TL_EXTEND,   // Wrappers for ISD::*_EXTEND and ISD::TRUNCATE to prevent DAG
  TL_TRUNCATE, // from auto-folding operations, e.g.
               // (i32 ext (i16 ext i8)) would be folded to (i32 ext i8).
               // To simplify the type legalization, we want to keep these
               // single steps separate during type legalization.
               // TL_[EXTEND|TRUNCATE] Inp, i128 _, i32 Opc
               // * Inp is the original input to extend/truncate,
               // * _ is a dummy operand with an illegal type (can be undef),
               // * Opc is the original opcode.
               // The legalization process (in Hexagon lowering code) will
               // first deal with the "real" types (i.e. Inp and the result),
               // and once all of them are processed, the wrapper node will
               // be replaced with the original ISD node. The dummy illegal
               // operand is there to make sure that the legalization hooks
               // are called again after everything else is legal, giving
               // us the opportunity to undo the wrapping.

  TYPECAST, // No-op that's used to convert between different legal
            // types in a register.
  ISEL,     // Marker for nodes that were created during ISel, and
            // which need explicit selection (would have been left
            // unselected otherwise).
};

} // namespace HexagonISD

class HexagonSelectionDAGInfo : public SelectionDAGGenTargetInfo {
public:
  HexagonSelectionDAGInfo();

  const char *getTargetNodeName(unsigned Opcode) const override;

  void verifyTargetNode(const SelectionDAG &DAG,
                        const SDNode *N) const override;

  SDValue EmitTargetCodeForMemcpy(SelectionDAG &DAG, const SDLoc &dl,
                                  SDValue Chain, SDValue Dst, SDValue Src,
                                  SDValue Size, Align Alignment,
                                  bool isVolatile, bool AlwaysInline,
                                  MachinePointerInfo DstPtrInfo,
                                  MachinePointerInfo SrcPtrInfo) const override;
};

} // namespace vm::core

#endif
