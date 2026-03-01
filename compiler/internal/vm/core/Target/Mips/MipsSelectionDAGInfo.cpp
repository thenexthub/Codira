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

#include "MipsSelectionDAGInfo.h"

#define GET_SDNODE_DESC
#include "MipsGenSDNodeInfo.inc"

using namespace vm::core;

MipsSelectionDAGInfo::MipsSelectionDAGInfo()
    : SelectionDAGGenTargetInfo(MipsGenSDNodeInfo) {}

MipsSelectionDAGInfo::~MipsSelectionDAGInfo() = default;

const char *MipsSelectionDAGInfo::getTargetNodeName(unsigned Opcode) const {
  // These nodes don't have corresponding entries in *.td files yet.
  switch (static_cast<MipsISD::NodeType>(Opcode)) {
    // clang-format off
  case MipsISD::FAbs:              return "MipsISD::FAbs";
  case MipsISD::DynAlloc:          return "MipsISD::DynAlloc";
  case MipsISD::DOUBLE_SELECT_I:   return "MipsISD::DOUBLE_SELECT_I";
  case MipsISD::DOUBLE_SELECT_I64: return "MipsISD::DOUBLE_SELECT_I64";
    // clang-format on
  }

  return SelectionDAGGenTargetInfo::getTargetNodeName(Opcode);
}

void MipsSelectionDAGInfo::verifyTargetNode(const SelectionDAG &DAG,
                                            const SDNode *N) const {
  switch (N->getOpcode()) {
  default:
    break;
  case MipsISD::ERet:
    // invalid number of operands; expected at most 2, got 3
    return;
  }

  SelectionDAGGenTargetInfo::verifyTargetNode(DAG, N);
}
