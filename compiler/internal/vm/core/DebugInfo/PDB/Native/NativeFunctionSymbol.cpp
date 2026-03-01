//===- NativeFunctionSymbol.cpp - info about function symbols----*- C++ -*-===//
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

#include "vm/core/DebugInfo/PDB/Native/NativeFunctionSymbol.h"

#include "vm/core/DebugInfo/CodeView/CVRecord.h"
#include "vm/core/DebugInfo/CodeView/CodeView.h"
#include "vm/core/DebugInfo/CodeView/SymbolDeserializer.h"
#include "vm/core/DebugInfo/CodeView/SymbolRecord.h"
#include "vm/core/DebugInfo/PDB/Native/ModuleDebugStream.h"
#include "vm/core/DebugInfo/PDB/Native/NativeEnumSymbols.h"
#include "vm/core/DebugInfo/PDB/Native/NativeSession.h"
#include "vm/core/DebugInfo/PDB/Native/SymbolCache.h"
#include "vm/core/DebugInfo/PDB/PDBExtras.h"

using namespace vm::core;
using namespace vm::core::codeview;
using namespace vm::core::pdb;

NativeFunctionSymbol::NativeFunctionSymbol(NativeSession &Session,
                                           SymIndexId Id,
                                           const codeview::ProcSym &Sym,
                                           uint32_t Offset)
    : NativeRawSymbol(Session, PDB_SymType::Function, Id), Sym(Sym),
      RecordOffset(Offset) {}

NativeFunctionSymbol::~NativeFunctionSymbol() = default;

void NativeFunctionSymbol::dump(raw_ostream &OS, int Indent,
                                PdbSymbolIdField ShowIdFields,
                                PdbSymbolIdField RecurseIdFields) const {
  NativeRawSymbol::dump(OS, Indent, ShowIdFields, RecurseIdFields);
  dumpSymbolField(OS, "name", getName(), Indent);
  dumpSymbolField(OS, "length", getLength(), Indent);
  dumpSymbolField(OS, "offset", getAddressOffset(), Indent);
  dumpSymbolField(OS, "section", getAddressSection(), Indent);
}

uint32_t NativeFunctionSymbol::getAddressOffset() const {
  return Sym.CodeOffset;
}

uint32_t NativeFunctionSymbol::getAddressSection() const { return Sym.Segment; }
std::string NativeFunctionSymbol::getName() const {
  return std::string(Sym.Name);
}

uint64_t NativeFunctionSymbol::getLength() const { return Sym.CodeSize; }

uint32_t NativeFunctionSymbol::getRelativeVirtualAddress() const {
  return Session.getRVAFromSectOffset(Sym.Segment, Sym.CodeOffset);
}

uint64_t NativeFunctionSymbol::getVirtualAddress() const {
  return Session.getVAFromSectOffset(Sym.Segment, Sym.CodeOffset);
}

static bool inlineSiteContainsAddress(InlineSiteSym &IS,
                                      uint32_t OffsetInFunc) {
  // Returns true if inline site contains the offset.
  bool Found = false;
  uint32_t CodeOffset = 0;
  for (auto &Annot : IS.annotations()) {
    switch (Annot.OpCode) {
    case BinaryAnnotationsOpCode::CodeOffset:
    case BinaryAnnotationsOpCode::ChangeCodeOffset:
    case BinaryAnnotationsOpCode::ChangeCodeOffsetAndLineOffset:
      CodeOffset += Annot.U1;
      if (OffsetInFunc >= CodeOffset)
        Found = true;
      break;
    case BinaryAnnotationsOpCode::ChangeCodeLength:
      CodeOffset += Annot.U1;
      if (Found && OffsetInFunc < CodeOffset)
        return true;
      Found = false;
      break;
    case BinaryAnnotationsOpCode::ChangeCodeLengthAndCodeOffset:
      CodeOffset += Annot.U2;
      if (OffsetInFunc >= CodeOffset && OffsetInFunc < CodeOffset + Annot.U1)
        return true;
      Found = false;
      break;
    default:
      break;
    }
  }
  return false;
}

std::unique_ptr<IPDBEnumSymbols>
NativeFunctionSymbol::findInlineFramesByVA(uint64_t VA) const {
  uint16_t Modi;
  if (!Session.moduleIndexForVA(VA, Modi))
    return nullptr;

  Expected<ModuleDebugStreamRef> ModS = Session.getModuleDebugStream(Modi);
  if (!ModS) {
    consumeError(ModS.takeError());
    return nullptr;
  }
  CVSymbolArray Syms = ModS->getSymbolArray();

  // Search for inline sites. There should be one matching top level inline
  // site. Then search in its nested inline sites.
  std::vector<SymIndexId> Frames;
  uint32_t CodeOffset = VA - getVirtualAddress();
  auto Start = Syms.at(RecordOffset);
  auto End = Syms.at(Sym.End);
  while (Start != End) {
    bool Found = false;
    // Find matching inline site within Start and End.
    for (; Start != End; ++Start) {
      if (Start->kind() != S_INLINESITE)
        continue;

      InlineSiteSym IS =
          cantFail(SymbolDeserializer::deserializeAs<InlineSiteSym>(*Start));
      if (inlineSiteContainsAddress(IS, CodeOffset)) {
        // Insert frames in reverse order.
        SymIndexId Id = Session.getSymbolCache().getOrCreateInlineSymbol(
            IS, getVirtualAddress(), Modi, Start.offset());
        Frames.insert(Frames.begin(), Id);

        // Update offsets to search within this inline site.
        ++Start;
        End = Syms.at(IS.End);
        Found = true;
        break;
      }

      Start = Syms.at(IS.End);
      if (Start == End)
        break;
    }

    if (!Found)
      break;
  }

  return std::make_unique<NativeEnumSymbols>(Session, std::move(Frames));
}
