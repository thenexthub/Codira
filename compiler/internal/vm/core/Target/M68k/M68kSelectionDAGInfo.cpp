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

#include "M68kSelectionDAGInfo.h"

#define GET_SDNODE_DESC
#include "M68kGenSDNodeInfo.inc"

using namespace vm::core;

M68kSelectionDAGInfo::M68kSelectionDAGInfo()
    : SelectionDAGGenTargetInfo(M68kGenSDNodeInfo) {}

void M68kSelectionDAGInfo::verifyTargetNode(const SelectionDAG &DAG,
                                            const SDNode *N) const {
  switch (N->getOpcode()) {
  case M68kISD::ADD:
  case M68kISD::SUBX:
    // result #1 must have type i8, but has type i32
    return;
  case M68kISD::SETCC:
    // operand #1 must have type i8, but has type i32
    return;
  }

  SelectionDAGGenTargetInfo::verifyTargetNode(DAG, N);
}

M68kSelectionDAGInfo::~M68kSelectionDAGInfo() = default;
