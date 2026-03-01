//==- XtensaMachineFunctionInfo.h - Xtensa machine function info --*- C++ -*-=//
//
//                     The LLVM Compiler Infrastructure
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
// This file declares Xtensa-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_XTENSA_XTENSAMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_XTENSA_XTENSAMACHINEFUNCTIONINFO_H

#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/Target/TargetMachine.h"

namespace vm::core {

class XtensaMachineFunctionInfo : public MachineFunctionInfo {
  /// FrameIndex of the spill slot for the scratch register in BranchRelaxation.
  int BranchRelaxationScratchFrameIndex = -1;
  unsigned VarArgsFirstGPR;
  int VarArgsOnStackFrameIndex;
  int VarArgsInRegsFrameIndex;
  bool SaveFrameRegister = false;
  unsigned CPLabelId = 0;

public:
  explicit XtensaMachineFunctionInfo(const Function &F,
                                     const TargetSubtargetInfo *STI)
      : VarArgsFirstGPR(0), VarArgsOnStackFrameIndex(0),
        VarArgsInRegsFrameIndex(0) {}

  int getBranchRelaxationScratchFrameIndex() const {
    return BranchRelaxationScratchFrameIndex;
  }
  void setBranchRelaxationScratchFrameIndex(int Index) {
    BranchRelaxationScratchFrameIndex = Index;
  }

  unsigned getVarArgsFirstGPR() const { return VarArgsFirstGPR; }
  void setVarArgsFirstGPR(unsigned GPR) { VarArgsFirstGPR = GPR; }

  int getVarArgsOnStackFrameIndex() const { return VarArgsOnStackFrameIndex; }
  void setVarArgsOnStackFrameIndex(int FI) { VarArgsOnStackFrameIndex = FI; }

  // Get and set the frame index of the first stack vararg.
  int getVarArgsInRegsFrameIndex() const { return VarArgsInRegsFrameIndex; }
  void setVarArgsInRegsFrameIndex(int FI) { VarArgsInRegsFrameIndex = FI; }

  bool isSaveFrameRegister() const { return SaveFrameRegister; }
  void setSaveFrameRegister() { SaveFrameRegister = true; }

  unsigned createCPLabelId() { return CPLabelId++; }
};

} // namespace vm::core

#endif /* LLVM_LIB_TARGET_XTENSA_XTENSAMACHINEFUNCTIONINFO_H */
