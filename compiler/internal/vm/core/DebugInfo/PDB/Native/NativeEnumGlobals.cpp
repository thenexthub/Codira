//==- NativeEnumGlobals.cpp - Native Global Enumerator impl ------*- C++ -*-==//
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

#include "vm/core/DebugInfo/PDB/Native/NativeEnumGlobals.h"

#include "vm/core/DebugInfo/CodeView/CVRecord.h"
#include "vm/core/DebugInfo/PDB/Native/GlobalsStream.h"
#include "vm/core/DebugInfo/PDB/Native/NativeSession.h"
#include "vm/core/DebugInfo/PDB/Native/PDBFile.h"
#include "vm/core/DebugInfo/PDB/Native/SymbolCache.h"
#include "vm/core/DebugInfo/PDB/Native/SymbolStream.h"
#include "vm/core/DebugInfo/PDB/PDBSymbol.h"
#include "vm/core/DebugInfo/PDB/PDBTypes.h"

using namespace vm::core;
using namespace vm::core::codeview;
using namespace vm::core::pdb;

NativeEnumGlobals::NativeEnumGlobals(NativeSession &PDBSession,
                                     std::vector<codeview::SymbolKind> Kinds)
    : Index(0), Session(PDBSession) {
  GlobalsStream &GS = cantFail(Session.getPDBFile().getPDBGlobalsStream());
  SymbolStream &SS = cantFail(Session.getPDBFile().getPDBSymbolStream());
  for (uint32_t Off : GS.getGlobalsTable()) {
    CVSymbol S = SS.readRecord(Off);
    if (!toolchain::is_contained(Kinds, S.kind()))
      continue;
    MatchOffsets.push_back(Off);
  }
}

uint32_t NativeEnumGlobals::getChildCount() const {
  return static_cast<uint32_t>(MatchOffsets.size());
}

std::unique_ptr<PDBSymbol>
NativeEnumGlobals::getChildAtIndex(uint32_t N) const {
  if (N >= MatchOffsets.size())
    return nullptr;

  SymIndexId Id =
      Session.getSymbolCache().getOrCreateGlobalSymbolByOffset(MatchOffsets[N]);
  return Session.getSymbolCache().getSymbolById(Id);
}

std::unique_ptr<PDBSymbol> NativeEnumGlobals::getNext() {
  return getChildAtIndex(Index++);
}

void NativeEnumGlobals::reset() { Index = 0; }
