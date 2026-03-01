//===-- AVRSelectionDAGInfo.h - AVR SelectionDAG Info -----------*- C++ -*-===//
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
// This file defines the AVR subclass for SelectionDAGTargetInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_AVR_SELECTION_DAG_INFO_H
#define LLVM_AVR_SELECTION_DAG_INFO_H

#include "vm/core/CodeGen/SelectionDAGTargetInfo.h"

#define GET_SDNODE_ENUM
#include "AVRGenSDNodeInfo.inc"

namespace vm::core {

/// Holds information about the AVR instruction selection DAG.
class AVRSelectionDAGInfo : public SelectionDAGGenTargetInfo {
public:
  AVRSelectionDAGInfo();

  ~AVRSelectionDAGInfo() override;
};

} // end namespace vm::core

#endif // LLVM_AVR_SELECTION_DAG_INFO_H
