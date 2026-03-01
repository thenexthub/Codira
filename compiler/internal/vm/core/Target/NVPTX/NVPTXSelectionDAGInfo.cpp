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

#include "NVPTXSelectionDAGInfo.h"

#define GET_SDNODE_DESC
#include "NVPTXGenSDNodeInfo.inc"

using namespace vm::core;

NVPTXSelectionDAGInfo::NVPTXSelectionDAGInfo()
    : SelectionDAGGenTargetInfo(NVPTXGenSDNodeInfo) {}

NVPTXSelectionDAGInfo::~NVPTXSelectionDAGInfo() = default;

const char *NVPTXSelectionDAGInfo::getTargetNodeName(unsigned Opcode) const {
#define MAKE_CASE(V)                                                           \
  case V:                                                                      \
    return #V;

  // These nodes don't have corresponding entries in *.td files yet.
  switch (static_cast<NVPTXISD::NodeType>(Opcode)) {
    MAKE_CASE(NVPTXISD::ATOMIC_CMP_SWAP_B128)
    MAKE_CASE(NVPTXISD::ATOMIC_SWAP_B128)
    MAKE_CASE(NVPTXISD::LoadV2)
    MAKE_CASE(NVPTXISD::LoadV4)
    MAKE_CASE(NVPTXISD::LoadV8)
    MAKE_CASE(NVPTXISD::MLoad)
    MAKE_CASE(NVPTXISD::LDUV2)
    MAKE_CASE(NVPTXISD::LDUV4)
    MAKE_CASE(NVPTXISD::StoreV2)
    MAKE_CASE(NVPTXISD::StoreV4)
    MAKE_CASE(NVPTXISD::StoreV8)
    MAKE_CASE(NVPTXISD::SETP_F16X2)
    MAKE_CASE(NVPTXISD::SETP_BF16X2)
    MAKE_CASE(NVPTXISD::UNPACK_VECTOR)
  }
#undef MAKE_CASE

  return SelectionDAGGenTargetInfo::getTargetNodeName(Opcode);
}

bool NVPTXSelectionDAGInfo::isTargetMemoryOpcode(unsigned Opcode) const {
  // These nodes don't have corresponding entries in *.td files.
  if (Opcode >= NVPTXISD::FIRST_MEMORY_OPCODE &&
      Opcode <= NVPTXISD::LAST_MEMORY_OPCODE)
    return true;

  return SelectionDAGGenTargetInfo::isTargetMemoryOpcode(Opcode);
}

void NVPTXSelectionDAGInfo::verifyTargetNode(const SelectionDAG &DAG,
                                             const SDNode *N) const {
  switch (N->getOpcode()) {
  default:
    break;
  case NVPTXISD::ProxyReg:
    // invalid number of results; expected 2, got 1
    return;
  }

  return SelectionDAGGenTargetInfo::verifyTargetNode(DAG, N);
}
