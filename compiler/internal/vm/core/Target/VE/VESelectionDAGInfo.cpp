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

#include "VESelectionDAGInfo.h"

#define GET_SDNODE_DESC
#include "VEGenSDNodeInfo.inc"

using namespace vm::core;

VESelectionDAGInfo::VESelectionDAGInfo()
    : SelectionDAGGenTargetInfo(VEGenSDNodeInfo) {}

VESelectionDAGInfo::~VESelectionDAGInfo() = default;

const char *VESelectionDAGInfo::getTargetNodeName(unsigned Opcode) const {
#define TARGET_NODE_CASE(NAME)                                                 \
  case VEISD::NAME:                                                            \
    return "VEISD::" #NAME;

  switch (static_cast<VEISD::NodeType>(Opcode)) {
    TARGET_NODE_CASE(GLOBAL_BASE_REG)
    TARGET_NODE_CASE(LEGALAVL)
  }
#undef TARGET_NODE_CASE

  return SelectionDAGGenTargetInfo::getTargetNodeName(Opcode);
}

void VESelectionDAGInfo::verifyTargetNode(const SelectionDAG &DAG,
                                          const SDNode *N) const {
  switch (N->getOpcode()) {
  case VEISD::GETSTACKTOP:
    // result #0 has invalid type; expected ch, got i64
    return;
  }

  SelectionDAGGenTargetInfo::verifyTargetNode(DAG, N);
}
