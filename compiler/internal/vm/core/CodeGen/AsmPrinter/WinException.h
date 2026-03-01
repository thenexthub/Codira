//===-- WinException.h - Windows Exception Handling ----------*- C++ -*--===//
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
// This file contains support for writing windows exception info into asm files.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_CODEGEN_ASMPRINTER_WIN64EXCEPTION_H
#define LLVM_LIB_CODEGEN_ASMPRINTER_WIN64EXCEPTION_H

#include "EHStreamer.h"
#include <vector>

namespace vm::core {
class GlobalValue;
class MachineFunction;
class MCExpr;
class MCSection;
struct WinEHFuncInfo;

class LLVM_LIBRARY_VISIBILITY WinException : public EHStreamer {
  /// Per-function flag to indicate if personality info should be emitted.
  bool shouldEmitPersonality = false;

  /// Per-function flag to indicate if the LSDA should be emitted.
  bool shouldEmitLSDA = false;

  /// Per-function flag to indicate if frame moves info should be emitted.
  bool shouldEmitMoves = false;

  /// True if this is a 64-bit target and we should use image relative offsets.
  bool useImageRel32 = false;

  /// True if we are generating exception handling on Windows for ARM64.
  bool isAArch64 = false;

  /// True if we are generating exception handling on Windows for ARM (Thumb).
  bool isThumb = false;

  /// Pointer to the current funclet entry BB.
  const MachineBasicBlock *CurrentFuncletEntry = nullptr;

  /// The section of the last funclet start.
  MCSection *CurrentFuncletTextSection = nullptr;

  /// The list of symbols to add to the ehcont section
  std::vector<const MCSymbol *> EHContTargets;

  void emitCSpecificHandlerTable(const MachineFunction *MF);

  void emitSEHActionsForRange(const WinEHFuncInfo &FuncInfo,
                              const MCSymbol *BeginLabel,
                              const MCSymbol *EndLabel, int State);

  /// Emit the EH table data for 32-bit and 64-bit functions using
  /// the __CxxFrameHandler3 personality.
  void emitCXXFrameHandler3Table(const MachineFunction *MF);

  /// Emit the EH table data for _except_handler3 and _except_handler4
  /// personality functions. These are only used on 32-bit and do not use CFI
  /// tables.
  void emitExceptHandlerTable(const MachineFunction *MF);

  void emitCLRExceptionTable(const MachineFunction *MF);

  void computeIP2StateTable(
      const MachineFunction *MF, const WinEHFuncInfo &FuncInfo,
      SmallVectorImpl<std::pair<const MCExpr *, int>> &IPToStateTable);

  /// Emits the label used with toolchain.eh.recoverfp, which is used by
  /// outlined funclets.
  void emitEHRegistrationOffsetLabel(const WinEHFuncInfo &FuncInfo,
                                     StringRef FLinkageName);

  const MCExpr *create32bitRef(const MCSymbol *Value);
  const MCExpr *create32bitRef(const GlobalValue *GV);
  const MCExpr *getLabel(const MCSymbol *Label);
  const MCExpr *getOffset(const MCSymbol *OffsetOf, const MCSymbol *OffsetFrom);
  const MCExpr *getOffsetPlusOne(const MCSymbol *OffsetOf,
                                 const MCSymbol *OffsetFrom);

  /// Gets the offset that we should use in a table for a stack object with the
  /// given index. For targets using CFI (Win64, etc), this is relative to the
  /// established SP at the end of the prologue. For targets without CFI (Win32
  /// only), it is relative to the frame pointer.
  int getFrameIndexOffset(int FrameIndex, const WinEHFuncInfo &FuncInfo);

  void endFuncletImpl();
public:
  //===--------------------------------------------------------------------===//
  // Main entry points.
  //
  WinException(AsmPrinter *A);
  ~WinException() override;

  /// Emit all exception information that should come after the content.
  void endModule() override;

  /// Gather pre-function exception information.  Assumes being emitted
  /// immediately after the function entry point.
  void beginFunction(const MachineFunction *MF) override;

  void markFunctionEnd() override;

  /// Gather and emit post-function exception information.
  void endFunction(const MachineFunction *) override;

  /// Emit target-specific EH funclet machinery.
  void beginFunclet(const MachineBasicBlock &MBB, MCSymbol *Sym) override;
  void endFunclet() override;
};
}

#endif

