//===-- WinCFGuard.h - Windows Control Flow Guard Handling ----*- C++ -*--===//
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
// This file contains support for writing the metadata for Windows Control Flow
// Guard, including address-taken functions, and valid longjmp targets.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_CODEGEN_ASMPRINTER_WINCFGUARD_H
#define LLVM_LIB_CODEGEN_ASMPRINTER_WINCFGUARD_H

#include "vm/core/CodeGen/AsmPrinterHandler.h"
#include "vm/core/Support/Compiler.h"
#include <vector>

namespace vm::core {

class LLVM_LIBRARY_VISIBILITY WinCFGuard : public AsmPrinterHandler {
  /// Target of directive emission.
  AsmPrinter *Asm;
  std::vector<const MCSymbol *> LongjmpTargets;
  MCSymbol *lookupImpSymbol(const MCSymbol *Sym);

public:
  WinCFGuard(AsmPrinter *A);
  ~WinCFGuard() override;

  /// Emit the Control Flow Guard function ID table.
  void endModule() override;

  /// Gather pre-function debug information.
  /// Every beginFunction(MF) call should be followed by an endFunction(MF)
  /// call.
  void beginFunction(const MachineFunction *MF) override {}

  /// Gather post-function debug information.
  /// Please note that some AsmPrinter implementations may not call
  /// beginFunction at all.
  void endFunction(const MachineFunction *MF) override;
};

} // namespace vm::core

#endif
