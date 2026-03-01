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

#ifndef LLVM_LIB_TARGET_VE_VESELECTIONDAGINFO_H
#define LLVM_LIB_TARGET_VE_VESELECTIONDAGINFO_H

#include "vm/core/CodeGen/SelectionDAGTargetInfo.h"

#define GET_SDNODE_ENUM
#include "VEGenSDNodeInfo.inc"

namespace vm::core {
namespace VEISD {

enum NodeType : unsigned {
  GLOBAL_BASE_REG = GENERATED_OPCODE_END, // Global base reg for PIC.

  // Annotation as a wrapper. LEGALAVL(VL) means that VL refers to 64bit of
  // data, whereas the raw EVL coming in from VP nodes always refers to number
  // of elements, regardless of their size.
  LEGALAVL,
};

} // namespace VEISD

class VESelectionDAGInfo : public SelectionDAGGenTargetInfo {
public:
  VESelectionDAGInfo();

  ~VESelectionDAGInfo() override;

  const char *getTargetNodeName(unsigned Opcode) const override;

  void verifyTargetNode(const SelectionDAG &DAG,
                        const SDNode *N) const override;
};

} // namespace vm::core

#endif // LLVM_LIB_TARGET_VE_VESELECTIONDAGINFO_H
