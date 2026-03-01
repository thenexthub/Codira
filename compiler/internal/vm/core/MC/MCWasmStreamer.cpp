//===- lib/MC/MCWasmStreamer.cpp - Wasm Object Output ---------------------===//
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
// This file assembles .s files and emits Wasm .o object files.
//
//===----------------------------------------------------------------------===//

#include "vm/core/MC/MCWasmStreamer.h"
#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCAssembler.h"
#include "vm/core/MC/MCCodeEmitter.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCFixup.h"
#include "vm/core/MC/MCObjectStreamer.h"
#include "vm/core/MC/MCSection.h"
#include "vm/core/MC/MCSectionWasm.h"
#include "vm/core/MC/MCSymbol.h"
#include "vm/core/MC/MCSymbolWasm.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/ErrorHandling.h"

namespace vm::core {
class MCContext;
class MCStreamer;
class MCSubtargetInfo;
} // namespace vm::core

using namespace vm::core;

MCWasmStreamer::~MCWasmStreamer() = default; // anchor.

void MCWasmStreamer::emitLabel(MCSymbol *S, SMLoc Loc) {
  auto *Symbol = static_cast<MCSymbolWasm *>(S);
  MCObjectStreamer::emitLabel(Symbol, Loc);

  const MCSectionWasm &Section =
      static_cast<const MCSectionWasm &>(*getCurrentSectionOnly());
  if (Section.getSegmentFlags() & wasm::WASM_SEG_FLAG_TLS)
    Symbol->setTLS();
}

void MCWasmStreamer::emitLabelAtPos(MCSymbol *S, SMLoc Loc, MCFragment &F,
                                    uint64_t Offset) {
  auto *Symbol = static_cast<MCSymbolWasm *>(S);
  MCObjectStreamer::emitLabelAtPos(Symbol, Loc, F, Offset);

  const MCSectionWasm &Section =
      static_cast<const MCSectionWasm &>(*getCurrentSectionOnly());
  if (Section.getSegmentFlags() & wasm::WASM_SEG_FLAG_TLS)
    Symbol->setTLS();
}

void MCWasmStreamer::changeSection(MCSection *Section, uint32_t Subsection) {
  MCAssembler &Asm = getAssembler();
  auto *SectionWasm = static_cast<const MCSectionWasm *>(Section);
  const MCSymbol *Grp = SectionWasm->getGroup();
  if (Grp)
    Asm.registerSymbol(*Grp);

  this->MCObjectStreamer::changeSection(Section, Subsection);
  Asm.registerSymbol(*Section->getBeginSymbol());
}

bool MCWasmStreamer::emitSymbolAttribute(MCSymbol *S, MCSymbolAttr Attribute) {
  assert(Attribute != MCSA_IndirectSymbol && "indirect symbols not supported");
  auto *Symbol = static_cast<MCSymbolWasm *>(S);

  // Adding a symbol attribute always introduces the symbol; note that an
  // important side effect of calling registerSymbol here is to register the
  // symbol with the assembler.
  getAssembler().registerSymbol(*Symbol);

  switch (Attribute) {
  case MCSA_LazyReference:
  case MCSA_Reference:
  case MCSA_SymbolResolver:
  case MCSA_PrivateExtern:
  case MCSA_WeakDefinition:
  case MCSA_WeakDefAutoPrivate:
  case MCSA_Invalid:
  case MCSA_IndirectSymbol:
  case MCSA_Protected:
  case MCSA_Exported:
    return false;

  case MCSA_Hidden:
    Symbol->setHidden(true);
    break;

  case MCSA_Weak:
  case MCSA_WeakReference:
    Symbol->setWeak(true);
    Symbol->setExternal(true);
    break;

  case MCSA_Global:
    Symbol->setExternal(true);
    break;

  case MCSA_ELF_TypeFunction:
    Symbol->setType(wasm::WASM_SYMBOL_TYPE_FUNCTION);
    break;

  case MCSA_ELF_TypeTLS:
    Symbol->setTLS();
    break;

  case MCSA_ELF_TypeObject:
  case MCSA_Cold:
    break;

  case MCSA_NoDeadStrip:
    Symbol->setNoStrip();
    break;

  default:
    // unrecognized directive
    llvm_unreachable("unexpected MCSymbolAttr");
    return false;
  }

  return true;
}

void MCWasmStreamer::emitCommonSymbol(MCSymbol *S, uint64_t Size,
                                      Align ByteAlignment) {
  llvm_unreachable("Common symbols are not yet implemented for Wasm");
}

void MCWasmStreamer::emitELFSize(MCSymbol *Symbol, const MCExpr *Value) {
  static_cast<MCSymbolWasm *>(Symbol)->setSize(Value);
}

void MCWasmStreamer::emitLocalCommonSymbol(MCSymbol *S, uint64_t Size,
                                           Align ByteAlignment) {
  llvm_unreachable("Local common symbols are not yet implemented for Wasm");
}

void MCWasmStreamer::emitIdent(StringRef IdentString) {
  // TODO(sbc): Add the ident section once we support mergable strings
  // sections in the object format
}

void MCWasmStreamer::finishImpl() {
  emitFrames();

  this->MCObjectStreamer::finishImpl();
}

MCStreamer *toolchain::createWasmStreamer(MCContext &Context,
                                     std::unique_ptr<MCAsmBackend> &&MAB,
                                     std::unique_ptr<MCObjectWriter> &&OW,
                                     std::unique_ptr<MCCodeEmitter> &&CE) {
  MCWasmStreamer *S =
      new MCWasmStreamer(Context, std::move(MAB), std::move(OW), std::move(CE));
  return S;
}
