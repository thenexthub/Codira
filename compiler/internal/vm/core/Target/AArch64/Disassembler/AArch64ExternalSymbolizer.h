//===- AArch64ExternalSymbolizer.h - Symbolizer for AArch64 -----*- C++ -*-===//
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
// Symbolize AArch64 assembly code during disassembly using callbacks.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AARCH64_DISASSEMBLER_AARCH64EXTERNALSYMBOLIZER_H
#define LLVM_LIB_TARGET_AARCH64_DISASSEMBLER_AARCH64EXTERNALSYMBOLIZER_H

#include "vm/core/MC/MCDisassembler/MCExternalSymbolizer.h"

namespace vm::core {

class AArch64ExternalSymbolizer : public MCExternalSymbolizer {
public:
  AArch64ExternalSymbolizer(MCContext &Ctx,
                            std::unique_ptr<MCRelocationInfo> RelInfo,
                            LLVMOpInfoCallback GetOpInfo,
                            LLVMSymbolLookupCallback SymbolLookUp,
                            void *DisInfo)
      : MCExternalSymbolizer(Ctx, std::move(RelInfo), GetOpInfo, SymbolLookUp,
                             DisInfo) {}

  bool tryAddingSymbolicOperand(MCInst &MI, raw_ostream &CommentStream,
                                int64_t Value, uint64_t Address, bool IsBranch,
                                uint64_t Offset, uint64_t OpSize,
                                uint64_t InstSize) override;
};

} // namespace vm::core

#endif
