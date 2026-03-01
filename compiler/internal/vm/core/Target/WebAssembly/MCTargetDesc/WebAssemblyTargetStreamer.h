//==-- WebAssemblyTargetStreamer.h - WebAssembly Target Streamer -*- C++ -*-==//
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
/// This file declares WebAssembly-specific target streamer classes.
/// These are for implementing support for target-specific assembly directives.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_WEBASSEMBLY_MCTARGETDESC_WEBASSEMBLYTARGETSTREAMER_H
#define LLVM_LIB_TARGET_WEBASSEMBLY_MCTARGETDESC_WEBASSEMBLYTARGETSTREAMER_H

#include "vm/core/BinaryFormat/Wasm.h"
#include "vm/core/CodeGenTypes/MachineValueType.h"
#include "vm/core/MC/MCStreamer.h"

namespace vm::core {

class MCSymbolWasm;
class formatted_raw_ostream;

/// WebAssembly-specific streamer interface, to implement support
/// WebAssembly-specific assembly directives.
class WebAssemblyTargetStreamer : public MCTargetStreamer {
public:
  explicit WebAssemblyTargetStreamer(MCStreamer &S);

  /// .local
  virtual void emitLocal(ArrayRef<wasm::ValType> Types) = 0;
  /// .functype
  virtual void emitFunctionType(const MCSymbolWasm *Sym) = 0;
  /// .indidx
  virtual void emitIndIdx(const MCExpr *Value) = 0;
  /// .globaltype
  virtual void emitGlobalType(const MCSymbolWasm *Sym) = 0;
  /// .tabletype
  virtual void emitTableType(const MCSymbolWasm *Sym) = 0;
  /// .tagtype
  virtual void emitTagType(const MCSymbolWasm *Sym) = 0;
  /// .import_module
  virtual void emitImportModule(const MCSymbolWasm *Sym,
                                StringRef ImportModule) = 0;
  /// .import_name
  virtual void emitImportName(const MCSymbolWasm *Sym,
                              StringRef ImportName) = 0;
  /// .export_name
  virtual void emitExportName(const MCSymbolWasm *Sym,
                              StringRef ExportName) = 0;

protected:
  void emitValueType(wasm::ValType Type);
};

/// This part is for ascii assembly output
class WebAssemblyTargetAsmStreamer final : public WebAssemblyTargetStreamer {
  formatted_raw_ostream &OS;

public:
  WebAssemblyTargetAsmStreamer(MCStreamer &S, formatted_raw_ostream &OS);

  void emitLocal(ArrayRef<wasm::ValType> Types) override;
  void emitFunctionType(const MCSymbolWasm *Sym) override;
  void emitIndIdx(const MCExpr *Value) override;
  void emitGlobalType(const MCSymbolWasm *Sym) override;
  void emitTableType(const MCSymbolWasm *Sym) override;
  void emitTagType(const MCSymbolWasm *Sym) override;
  void emitImportModule(const MCSymbolWasm *Sym, StringRef ImportModule) override;
  void emitImportName(const MCSymbolWasm *Sym, StringRef ImportName) override;
  void emitExportName(const MCSymbolWasm *Sym, StringRef ExportName) override;
};

/// This part is for Wasm object output
class WebAssemblyTargetWasmStreamer final : public WebAssemblyTargetStreamer {
public:
  explicit WebAssemblyTargetWasmStreamer(MCStreamer &S);

  void emitLocal(ArrayRef<wasm::ValType> Types) override;
  void emitFunctionType(const MCSymbolWasm *Sym) override {}
  void emitIndIdx(const MCExpr *Value) override;
  void emitGlobalType(const MCSymbolWasm *Sym) override {}
  void emitTableType(const MCSymbolWasm *Sym) override {}
  void emitTagType(const MCSymbolWasm *Sym) override {}
  void emitImportModule(const MCSymbolWasm *Sym,
                        StringRef ImportModule) override {}
  void emitImportName(const MCSymbolWasm *Sym,
                      StringRef ImportName) override {}
  void emitExportName(const MCSymbolWasm *Sym,
                      StringRef ExportName) override {}
};

/// This part is for null output
class WebAssemblyTargetNullStreamer final : public WebAssemblyTargetStreamer {
public:
  explicit WebAssemblyTargetNullStreamer(MCStreamer &S)
      : WebAssemblyTargetStreamer(S) {}

  void emitLocal(ArrayRef<wasm::ValType>) override {}
  void emitFunctionType(const MCSymbolWasm *) override {}
  void emitIndIdx(const MCExpr *) override {}
  void emitGlobalType(const MCSymbolWasm *) override {}
  void emitTableType(const MCSymbolWasm *) override {}
  void emitTagType(const MCSymbolWasm *) override {}
  void emitImportModule(const MCSymbolWasm *, StringRef) override {}
  void emitImportName(const MCSymbolWasm *, StringRef) override {}
  void emitExportName(const MCSymbolWasm *, StringRef) override {}
};

} // end namespace vm::core

#endif
