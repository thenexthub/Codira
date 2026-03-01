//===- PDBSymbolData.cpp - PDB data (e.g. variable) accessors ---*- C++ -*-===//
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

#include "vm/core/DebugInfo/PDB/PDBSymbolData.h"
#include "vm/core/DebugInfo/PDB/IPDBLineNumber.h"
#include "vm/core/DebugInfo/PDB/IPDBSectionContrib.h"
#include "vm/core/DebugInfo/PDB/IPDBSession.h"
#include "vm/core/DebugInfo/PDB/PDBSymDumper.h"

using namespace vm::core;
using namespace vm::core::pdb;

void PDBSymbolData::dump(PDBSymDumper &Dumper) const { Dumper.dump(*this); }

std::unique_ptr<IPDBEnumLineNumbers> PDBSymbolData::getLineNumbers() const {
  auto Len = RawSymbol->getLength();
  Len = Len ? Len : 1;
  if (auto RVA = RawSymbol->getRelativeVirtualAddress())
    return Session.findLineNumbersByRVA(RVA, Len);

  if (auto Section = RawSymbol->getAddressSection())
    return Session.findLineNumbersBySectOffset(
        Section, RawSymbol->getAddressOffset(), Len);

  return nullptr;
}

uint32_t PDBSymbolData::getCompilandId() const {
  if (auto Lines = getLineNumbers()) {
    if (auto FirstLine = Lines->getNext())
      return FirstLine->getCompilandId();
  }

  uint32_t DataSection = RawSymbol->getAddressSection();
  uint32_t DataOffset = RawSymbol->getAddressOffset();
  if (DataSection == 0) {
    if (auto RVA = RawSymbol->getRelativeVirtualAddress())
      Session.addressForRVA(RVA, DataSection, DataOffset);
  }

  if (DataSection) {
    if (auto SecContribs = Session.getSectionContribs()) {
      while (auto Section = SecContribs->getNext()) {
        if (Section->getAddressSection() == DataSection &&
            Section->getAddressOffset() <= DataOffset &&
            (Section->getAddressOffset() + Section->getLength()) > DataOffset)
          return Section->getCompilandId();
      }
    }
  } else {
    auto LexParentId = RawSymbol->getLexicalParentId();
    while (auto LexParent = Session.getSymbolById(LexParentId)) {
      if (LexParent->getSymTag() == PDB_SymType::Exe)
        break;
      if (LexParent->getSymTag() == PDB_SymType::Compiland)
        return LexParentId;
      LexParentId = LexParent->getRawSymbol().getLexicalParentId();
    }
  }

  return 0;
}
