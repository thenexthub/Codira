//===-- DwarfException.h - Dwarf Exception Framework -----------*- C++ -*--===//
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
// This file contains support for writing dwarf exception info into asm files.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_CODEGEN_ASMPRINTER_DWARFEXCEPTION_H
#define LLVM_LIB_CODEGEN_ASMPRINTER_DWARFEXCEPTION_H

#include "EHStreamer.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/MC/MCDwarf.h"

namespace vm::core {
class MachineFunction;
class ARMTargetStreamer;

class LLVM_LIBRARY_VISIBILITY DwarfCFIException : public EHStreamer {
  /// Per-function flag to indicate if .cfi_personality should be emitted.
  bool shouldEmitPersonality = false;

  /// Per-function flag to indicate if .cfi_personality must be emitted.
  bool forceEmitPersonality = false;

  /// Per-function flag to indicate if .cfi_lsda should be emitted.
  bool shouldEmitLSDA = false;

  /// Per-function flag to indicate if frame CFI info should be emitted.
  bool shouldEmitCFI = false;

  /// Per-module flag to indicate if .cfi_section has beeen emitted.
  bool hasEmittedCFISections = false;

  /// Vector of all personality functions seen so far in the module.
  std::vector<const GlobalValue *> Personalities;

  void addPersonality(const GlobalValue *Personality);

public:
  //===--------------------------------------------------------------------===//
  // Main entry points.
  //
  DwarfCFIException(AsmPrinter *A);
  ~DwarfCFIException() override;

  /// Emit all exception information that should come after the content.
  void endModule() override;

  /// Gather pre-function exception information.  Assumes being emitted
  /// immediately after the function entry point.
  void beginFunction(const MachineFunction *MF) override;

  /// Gather and emit post-function exception information.
  void endFunction(const MachineFunction *) override;

  void beginBasicBlockSection(const MachineBasicBlock &MBB) override;
  void endBasicBlockSection(const MachineBasicBlock &MBB) override;
};

class LLVM_LIBRARY_VISIBILITY ARMException : public EHStreamer {
  /// Per-function flag to indicate if frame CFI info should be emitted.
  bool shouldEmitCFI = false;

  /// Per-module flag to indicate if .cfi_section has beeen emitted.
  bool hasEmittedCFISections = false;

  void emitTypeInfos(unsigned TTypeEncoding, MCSymbol *TTBaseLabel) override;
  ARMTargetStreamer &getTargetStreamer();

public:
  //===--------------------------------------------------------------------===//
  // Main entry points.
  //
  ARMException(AsmPrinter *A);
  ~ARMException() override;

  /// Emit all exception information that should come after the content.
  void endModule() override {}

  /// Gather pre-function exception information.  Assumes being emitted
  /// immediately after the function entry point.
  void beginFunction(const MachineFunction *MF) override;

  /// Gather and emit post-function exception information.
  void endFunction(const MachineFunction *) override;

  void markFunctionEnd() override;
};

class LLVM_LIBRARY_VISIBILITY AIXException : public EHStreamer {
  /// This is AIX's compat unwind section, which unwinder would use
  /// to find the location of LSDA area and personality rountine.
  void emitExceptionInfoTable(const MCSymbol *LSDA, const MCSymbol *PerSym);

public:
  AIXException(AsmPrinter *A);

  void endModule() override {}
  void beginFunction(const MachineFunction *MF) override {}
  void endFunction(const MachineFunction *MF) override;
};
} // End of namespace vm::core

#endif
