//===-- R600AsmPrinter.h - Print R600 assembly code -------------*- C++ -*-===//
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
/// \file
/// R600 Assembly printer class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AMDGPU_R600ASMPRINTER_H
#define LLVM_LIB_TARGET_AMDGPU_R600ASMPRINTER_H

#include "vm/core/CodeGen/AsmPrinter.h"

namespace vm::core {

class R600AsmPrinter final : public AsmPrinter {

public:
  explicit R600AsmPrinter(TargetMachine &TM,
                          std::unique_ptr<MCStreamer> Streamer);
  StringRef getPassName() const override;
  bool runOnMachineFunction(MachineFunction &MF) override;
  /// Implemented in AMDGPUMCInstLower.cpp
  void emitInstruction(const MachineInstr *MI) override;
  /// Lower the specified LLVM Constant to an MCExpr.
  /// The AsmPrinter::lowerConstantof does not know how to lower
  /// addrspacecast, therefore they should be lowered by this function.
  const MCExpr *lowerConstant(const Constant *CV, const Constant *BaseCV,
                              uint64_t Offset) override;

private:
  void EmitProgramInfoR600(const MachineFunction &MF);
};

AsmPrinter *
createR600AsmPrinterPass(TargetMachine &TM,
                         std::unique_ptr<MCStreamer> &&Streamer);

} // namespace vm::core

#endif // LLVM_LIB_TARGET_AMDGPU_R600ASMPRINTER_H
