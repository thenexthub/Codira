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

#ifndef LLVM_LIB_TARGET_NVPTX_NVPTXSELECTIONDAGINFO_H
#define LLVM_LIB_TARGET_NVPTX_NVPTXSELECTIONDAGINFO_H

#include "vm/core/CodeGen/SelectionDAGTargetInfo.h"

#define GET_SDNODE_ENUM
#include "NVPTXGenSDNodeInfo.inc"

namespace vm::core {
namespace NVPTXISD {

enum NodeType : unsigned {
  SETP_F16X2 = GENERATED_OPCODE_END,
  SETP_BF16X2,
  UNPACK_VECTOR,

  FIRST_MEMORY_OPCODE,

  /// These nodes are used to lower atomic instructions with i128 type. They are
  /// similar to the generic nodes, but the input and output values are split
  /// into two 64-bit values.
  /// ValLo, ValHi, OUTCHAIN = ATOMIC_CMP_SWAP_B128(INCHAIN, ptr, cmpLo, cmpHi,
  ///                                               swapLo, swapHi)
  /// ValLo, ValHi, OUTCHAIN = ATOMIC_SWAP_B128(INCHAIN, ptr, amtLo, amtHi)
  ATOMIC_CMP_SWAP_B128 = FIRST_MEMORY_OPCODE,
  ATOMIC_SWAP_B128,

  LoadV2,
  LoadV4,
  LoadV8,
  MLoad,
  LDUV2, // LDU.v2
  LDUV4, // LDU.v4
  StoreV2,
  StoreV4,
  StoreV8,
  LAST_MEMORY_OPCODE = StoreV8,
};

} // namespace NVPTXISD

class NVPTXSelectionDAGInfo : public SelectionDAGGenTargetInfo {
public:
  NVPTXSelectionDAGInfo();

  ~NVPTXSelectionDAGInfo() override;

  const char *getTargetNodeName(unsigned Opcode) const override;

  bool isTargetMemoryOpcode(unsigned Opcode) const override;

  void verifyTargetNode(const SelectionDAG &DAG,
                        const SDNode *N) const override;
};

} // namespace vm::core

#endif // LLVM_LIB_TARGET_NVPTX_NVPTXSELECTIONDAGINFO_H
