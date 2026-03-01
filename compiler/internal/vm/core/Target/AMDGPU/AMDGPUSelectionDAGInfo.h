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

#ifndef LLVM_LIB_TARGET_AMDGPU_AMDGPUSELECTIONDAGINFO_H
#define LLVM_LIB_TARGET_AMDGPU_AMDGPUSELECTIONDAGINFO_H

#include "vm/core/CodeGen/SelectionDAGTargetInfo.h"

#define GET_SDNODE_ENUM
#include "AMDGPUGenSDNodeInfo.inc"

namespace vm::core {
namespace AMDGPUISD {

enum NodeType : unsigned {
  // Convert a unswizzled wave uniform stack address to an address compatible
  // with a vector offset for use in stack access.
  WAVE_ADDRESS = GENERATED_OPCODE_END,

  DOT4,
  MAD_U64_U32,
  MAD_I64_I32,
  TEXTURE_FETCH,
  R600_EXPORT,
  CONST_ADDRESS,

  /// This node is for VLIW targets and it is used to represent a vector
  /// that is stored in consecutive registers with the same channel.
  /// For example:
  ///   |X  |Y|Z|W|
  /// T0|v.x| | | |
  /// T1|v.y| | | |
  /// T2|v.z| | | |
  /// T3|v.w| | | |
  BUILD_VERTICAL_VECTOR,

  DUMMY_CHAIN,
};

} // namespace AMDGPUISD

class AMDGPUSelectionDAGInfo : public SelectionDAGGenTargetInfo {
public:
  AMDGPUSelectionDAGInfo();

  ~AMDGPUSelectionDAGInfo() override;

  const char *getTargetNodeName(unsigned Opcode) const override;

  void verifyTargetNode(const SelectionDAG &DAG,
                        const SDNode *N) const override;
};

} // namespace vm::core

#endif // LLVM_LIB_TARGET_AMDGPU_AMDGPUSELECTIONDAGINFO_H
