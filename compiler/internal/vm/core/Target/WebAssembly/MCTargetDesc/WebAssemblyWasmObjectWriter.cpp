//===-- WebAssemblyWasmObjectWriter.cpp - WebAssembly Wasm Writer ---------===//
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
/// This file handles Wasm-specific object emission, converting LLVM's
/// internal fixups into the appropriate relocations.
///
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/WebAssemblyFixupKinds.h"
#include "MCTargetDesc/WebAssemblyMCAsmInfo.h"
#include "MCTargetDesc/WebAssemblyMCTargetDesc.h"
#include "vm/core/BinaryFormat/Wasm.h"
#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCFixup.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/MC/MCSectionWasm.h"
#include "vm/core/MC/MCSymbolWasm.h"
#include "vm/core/MC/MCValue.h"
#include "vm/core/MC/MCWasmObjectWriter.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/ErrorHandling.h"

using namespace vm::core;

namespace {
class WebAssemblyWasmObjectWriter final : public MCWasmObjectTargetWriter {
public:
  explicit WebAssemblyWasmObjectWriter(bool Is64Bit, bool IsEmscripten);

private:
  unsigned getRelocType(const MCValue &Target, const MCFixup &Fixup,
                        const MCSectionWasm &FixupSection,
                        bool IsLocRel) const override;
};
} // end anonymous namespace

WebAssemblyWasmObjectWriter::WebAssemblyWasmObjectWriter(bool Is64Bit,
                                                         bool IsEmscripten)
    : MCWasmObjectTargetWriter(Is64Bit, IsEmscripten) {}

static const MCSection *getTargetSection(const MCExpr *Expr) {
  if (auto SyExp = dyn_cast<MCSymbolRefExpr>(Expr)) {
    if (SyExp->getSymbol().isInSection())
      return &SyExp->getSymbol().getSection();
    return nullptr;
  }

  if (auto BinOp = dyn_cast<MCBinaryExpr>(Expr)) {
    auto SectionLHS = getTargetSection(BinOp->getLHS());
    auto SectionRHS = getTargetSection(BinOp->getRHS());
    return SectionLHS == SectionRHS ? nullptr : SectionLHS;
  }

  if (auto UnOp = dyn_cast<MCUnaryExpr>(Expr))
    return getTargetSection(UnOp->getSubExpr());

  return nullptr;
}

unsigned WebAssemblyWasmObjectWriter::getRelocType(
    const MCValue &Target, const MCFixup &Fixup,
    const MCSectionWasm &FixupSection, bool IsLocRel) const {
  auto &SymA = static_cast<const MCSymbolWasm &>(*Target.getAddSym());
  auto Spec = WebAssembly::Specifier(Target.getSpecifier());
  switch (Spec) {
  case WebAssembly::S_GOT:
    SymA.setUsedInGOT();
    return wasm::R_WASM_GLOBAL_INDEX_LEB;
  case WebAssembly::S_GOT_TLS:
    SymA.setUsedInGOT();
    SymA.setTLS();
    return wasm::R_WASM_GLOBAL_INDEX_LEB;
  case WebAssembly::S_TBREL:
    assert(SymA.isFunction());
    return is64Bit() ? wasm::R_WASM_TABLE_INDEX_REL_SLEB64
                     : wasm::R_WASM_TABLE_INDEX_REL_SLEB;
  case WebAssembly::S_TLSREL:
    SymA.setTLS();
    return is64Bit() ? wasm::R_WASM_MEMORY_ADDR_TLS_SLEB64
                     : wasm::R_WASM_MEMORY_ADDR_TLS_SLEB;
  case WebAssembly::S_MBREL:
    assert(SymA.isData());
    return is64Bit() ? wasm::R_WASM_MEMORY_ADDR_REL_SLEB64
                     : wasm::R_WASM_MEMORY_ADDR_REL_SLEB;
  case WebAssembly::S_TYPEINDEX:
    return wasm::R_WASM_TYPE_INDEX_LEB;
  case WebAssembly::S_None:
    break;
  case WebAssembly::S_FUNCINDEX:
    return wasm::R_WASM_FUNCTION_INDEX_I32;
  }

  switch (unsigned(Fixup.getKind())) {
  case WebAssembly::fixup_sleb128_i32:
    if (SymA.isFunction())
      return wasm::R_WASM_TABLE_INDEX_SLEB;
    return wasm::R_WASM_MEMORY_ADDR_SLEB;
  case WebAssembly::fixup_sleb128_i64:
    if (SymA.isFunction())
      return wasm::R_WASM_TABLE_INDEX_SLEB64;
    return wasm::R_WASM_MEMORY_ADDR_SLEB64;
  case WebAssembly::fixup_uleb128_i32:
    if (SymA.isGlobal())
      return wasm::R_WASM_GLOBAL_INDEX_LEB;
    if (SymA.isFunction())
      return wasm::R_WASM_FUNCTION_INDEX_LEB;
    if (SymA.isTag())
      return wasm::R_WASM_TAG_INDEX_LEB;
    if (SymA.isTable())
      return wasm::R_WASM_TABLE_NUMBER_LEB;
    return wasm::R_WASM_MEMORY_ADDR_LEB;
  case WebAssembly::fixup_uleb128_i64:
    assert(SymA.isData());
    return wasm::R_WASM_MEMORY_ADDR_LEB64;
  case FK_Data_4:
    if (SymA.isFunction()) {
      if (FixupSection.isMetadata())
        return wasm::R_WASM_FUNCTION_OFFSET_I32;
      assert(FixupSection.isWasmData());
      return wasm::R_WASM_TABLE_INDEX_I32;
    }
    if (SymA.isGlobal())
      return wasm::R_WASM_GLOBAL_INDEX_I32;
    if (auto Section = static_cast<const MCSectionWasm *>(
            getTargetSection(Fixup.getValue()))) {
      if (Section->isText())
        return wasm::R_WASM_FUNCTION_OFFSET_I32;
      else if (!Section->isWasmData())
        return wasm::R_WASM_SECTION_OFFSET_I32;
    }
    return IsLocRel ? wasm::R_WASM_MEMORY_ADDR_LOCREL_I32
                    : wasm::R_WASM_MEMORY_ADDR_I32;
  case FK_Data_8:
    if (SymA.isFunction()) {
      if (FixupSection.isMetadata())
        return wasm::R_WASM_FUNCTION_OFFSET_I64;
      return wasm::R_WASM_TABLE_INDEX_I64;
    }
    if (SymA.isGlobal())
      llvm_unreachable("unimplemented R_WASM_GLOBAL_INDEX_I64");
    if (auto Section = static_cast<const MCSectionWasm *>(
            getTargetSection(Fixup.getValue()))) {
      if (Section->isText())
        return wasm::R_WASM_FUNCTION_OFFSET_I64;
      else if (!Section->isWasmData())
        llvm_unreachable("unimplemented R_WASM_SECTION_OFFSET_I64");
    }
    assert(SymA.isData());
    return wasm::R_WASM_MEMORY_ADDR_I64;
  default:
    llvm_unreachable("unimplemented fixup kind");
  }
}

std::unique_ptr<MCObjectTargetWriter>
toolchain::createWebAssemblyWasmObjectWriter(bool Is64Bit, bool IsEmscripten) {
  return std::make_unique<WebAssemblyWasmObjectWriter>(Is64Bit, IsEmscripten);
}
