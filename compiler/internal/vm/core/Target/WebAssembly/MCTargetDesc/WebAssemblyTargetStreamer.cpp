//==-- WebAssemblyTargetStreamer.cpp - WebAssembly Target Streamer Methods --=//
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
/// This file defines WebAssembly-specific target streamer classes.
/// These are for implementing support for target-specific assembly directives.
///
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/WebAssemblyTargetStreamer.h"
#include "MCTargetDesc/WebAssemblyMCAsmInfo.h"
#include "MCTargetDesc/WebAssemblyMCTypeUtilities.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCSectionWasm.h"
#include "vm/core/MC/MCSymbolWasm.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/FormattedStream.h"
using namespace vm::core;

WebAssemblyTargetStreamer::WebAssemblyTargetStreamer(MCStreamer &S)
    : MCTargetStreamer(S) {}

void WebAssemblyTargetStreamer::emitValueType(wasm::ValType Type) {
  Streamer.emitIntValue(uint8_t(Type), 1);
}

WebAssemblyTargetAsmStreamer::WebAssemblyTargetAsmStreamer(
    MCStreamer &S, formatted_raw_ostream &OS)
    : WebAssemblyTargetStreamer(S), OS(OS) {}

WebAssemblyTargetWasmStreamer::WebAssemblyTargetWasmStreamer(MCStreamer &S)
    : WebAssemblyTargetStreamer(S) {}

static void printTypes(formatted_raw_ostream &OS,
                       ArrayRef<wasm::ValType> Types) {
  bool First = true;
  for (auto Type : Types) {
    if (First)
      First = false;
    else
      OS << ", ";
    OS << WebAssembly::typeToString(Type);
  }
  OS << '\n';
}

void WebAssemblyTargetAsmStreamer::emitLocal(ArrayRef<wasm::ValType> Types) {
  if (!Types.empty()) {
    OS << "\t.local  \t";
    printTypes(OS, Types);
  }
}

void WebAssemblyTargetAsmStreamer::emitFunctionType(const MCSymbolWasm *Sym) {
  assert(Sym->isFunction());
  OS << "\t.functype\t" << Sym->getName() << " ";
  OS << WebAssembly::signatureToString(Sym->getSignature());
  OS << "\n";
}

void WebAssemblyTargetAsmStreamer::emitGlobalType(const MCSymbolWasm *Sym) {
  assert(Sym->isGlobal());
  OS << "\t.globaltype\t" << Sym->getName() << ", "
     << WebAssembly::typeToString(
            static_cast<wasm::ValType>(Sym->getGlobalType().Type));
  if (!Sym->getGlobalType().Mutable)
    OS << ", immutable";
  OS << '\n';
}

void WebAssemblyTargetAsmStreamer::emitTableType(const MCSymbolWasm *Sym) {
  assert(Sym->isTable());
  const wasm::WasmTableType &Type = Sym->getTableType();
  OS << "\t.tabletype\t" << Sym->getName() << ", "
     << WebAssembly::typeToString(static_cast<wasm::ValType>(Type.ElemType));
  bool HasMaximum = Type.Limits.Flags & wasm::WASM_LIMITS_FLAG_HAS_MAX;
  if (Type.Limits.Minimum != 0 || HasMaximum) {
    OS << ", " << Type.Limits.Minimum;
    if (HasMaximum)
      OS << ", " << Type.Limits.Maximum;
  }
  OS << '\n';
}

void WebAssemblyTargetAsmStreamer::emitTagType(const MCSymbolWasm *Sym) {
  assert(Sym->isTag());
  OS << "\t.tagtype\t" << Sym->getName() << " ";
  OS << WebAssembly::typeListToString(Sym->getSignature()->Params);
  OS << "\n";
}

void WebAssemblyTargetAsmStreamer::emitImportModule(const MCSymbolWasm *Sym,
                                                    StringRef ImportModule) {
  OS << "\t.import_module\t" << Sym->getName() << ", "
                             << ImportModule << '\n';
}

void WebAssemblyTargetAsmStreamer::emitImportName(const MCSymbolWasm *Sym,
                                                  StringRef ImportName) {
  OS << "\t.import_name\t" << Sym->getName() << ", "
                           << ImportName << '\n';
}

void WebAssemblyTargetAsmStreamer::emitExportName(const MCSymbolWasm *Sym,
                                                  StringRef ExportName) {
  OS << "\t.export_name\t" << Sym->getName() << ", "
                           << ExportName << '\n';
}

void WebAssemblyTargetAsmStreamer::emitIndIdx(const MCExpr *Value) {
  OS << "\t.indidx\t";
  getContext().getAsmInfo()->printExpr(OS, *Value);
  OS << '\n';
}

void WebAssemblyTargetWasmStreamer::emitLocal(ArrayRef<wasm::ValType> Types) {
  SmallVector<std::pair<wasm::ValType, uint32_t>, 4> Grouped;
  for (auto Type : Types) {
    if (Grouped.empty() || Grouped.back().first != Type)
      Grouped.push_back(std::make_pair(Type, 1));
    else
      ++Grouped.back().second;
  }

  Streamer.emitULEB128IntValue(Grouped.size());
  for (auto Pair : Grouped) {
    Streamer.emitULEB128IntValue(Pair.second);
    emitValueType(Pair.first);
  }
}

void WebAssemblyTargetWasmStreamer::emitIndIdx(const MCExpr *Value) {
  llvm_unreachable(".indidx encoding not yet implemented");
}
