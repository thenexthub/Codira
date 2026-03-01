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

#include "AMDGPUSelectionDAGInfo.h"

#define GET_SDNODE_DESC
#include "AMDGPUGenSDNodeInfo.inc"

using namespace vm::core;

AMDGPUSelectionDAGInfo::AMDGPUSelectionDAGInfo()
    : SelectionDAGGenTargetInfo(AMDGPUGenSDNodeInfo) {}

AMDGPUSelectionDAGInfo::~AMDGPUSelectionDAGInfo() = default;

const char *AMDGPUSelectionDAGInfo::getTargetNodeName(unsigned Opcode) const {
#define NODE_NAME_CASE(node)                                                   \
  case AMDGPUISD::node:                                                        \
    return "AMDGPUISD::" #node;

  switch (static_cast<AMDGPUISD::NodeType>(Opcode)) {
    // These nodes don't have corresponding entries in *.td files yet.
    NODE_NAME_CASE(WAVE_ADDRESS)
    NODE_NAME_CASE(MAD_I64_I32)
    NODE_NAME_CASE(MAD_U64_U32)
    NODE_NAME_CASE(BUILD_VERTICAL_VECTOR)
    // These do, but only when compiling R600.td,
    // and the enum is generated from AMDGPU.td.
    NODE_NAME_CASE(DOT4)
    NODE_NAME_CASE(TEXTURE_FETCH)
    NODE_NAME_CASE(R600_EXPORT)
    NODE_NAME_CASE(CONST_ADDRESS)
    NODE_NAME_CASE(DUMMY_CHAIN)
  }

#undef NODE_NAME_CASE

  return SelectionDAGGenTargetInfo::getTargetNodeName(Opcode);
}

void AMDGPUSelectionDAGInfo::verifyTargetNode(const SelectionDAG &DAG,
                                              const SDNode *N) const {
  switch (N->getOpcode()) {
  case AMDGPUISD::IF:
    // result #0 must have type i1, but has type i32/i64
  case AMDGPUISD::ELSE:
  case AMDGPUISD::LOOP:
    // operand #1 must have type i1, but has type i32/i64
  case AMDGPUISD::LDS:
    // result #0 must have type i64 (iPTR), but has type i32
    return;
  }
  SelectionDAGGenTargetInfo::verifyTargetNode(DAG, N);
}
