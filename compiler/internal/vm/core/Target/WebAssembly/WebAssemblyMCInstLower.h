//===-- WebAssemblyMCInstLower.h - Lower MachineInstr to MCInst -*- C++ -*-===//
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
///
/// \file
/// This file declares the class to lower WebAssembly MachineInstrs to
/// their corresponding MCInst records.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_WEBASSEMBLY_WEBASSEMBLYMCINSTLOWER_H
#define LLVM_LIB_TARGET_WEBASSEMBLY_WEBASSEMBLYMCINSTLOWER_H

#include "vm/core/BinaryFormat/Wasm.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/Support/Compiler.h"

namespace vm::core {
class WebAssemblyAsmPrinter;
class MCContext;
class MCSymbol;
class MachineInstr;
class MachineOperand;

/// This class is used to lower an MachineInstr into an MCInst.
class LLVM_LIBRARY_VISIBILITY WebAssemblyMCInstLower {
  MCContext &Ctx;
  WebAssemblyAsmPrinter &Printer;

  MCSymbol *GetGlobalAddressSymbol(const MachineOperand &MO) const;
  MCSymbol *GetExternalSymbolSymbol(const MachineOperand &MO) const;
  MCOperand lowerSymbolOperand(const MachineOperand &MO, MCSymbol *Sym) const;
  MCOperand lowerTypeIndexOperand(SmallVectorImpl<wasm::ValType> &&,
                                  SmallVectorImpl<wasm::ValType> &&) const;
  MCOperand lowerEncodedFunctionSignature(const APInt &Sig) const;

public:
  WebAssemblyMCInstLower(MCContext &ctx, WebAssemblyAsmPrinter &printer)
      : Ctx(ctx), Printer(printer) {}
  void lower(const MachineInstr *MI, MCInst &OutMI) const;
};
} // end namespace vm::core

#endif
