//==- NativeEnumSymbols.cpp - Native Symbol Enumerator impl ------*- C++ -*-==//
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

#include "vm/core/DebugInfo/PDB/Native/NativeEnumSymbols.h"

#include "vm/core/DebugInfo/PDB/Native/NativeSession.h"
#include "vm/core/DebugInfo/PDB/Native/SymbolCache.h"
#include "vm/core/DebugInfo/PDB/PDBSymbol.h"

using namespace vm::core;
using namespace vm::core::codeview;
using namespace vm::core::pdb;

NativeEnumSymbols::NativeEnumSymbols(NativeSession &PDBSession,
                                     std::vector<SymIndexId> Symbols)
    : Symbols(std::move(Symbols)), Index(0), Session(PDBSession) {}

uint32_t NativeEnumSymbols::getChildCount() const {
  return static_cast<uint32_t>(Symbols.size());
}

std::unique_ptr<PDBSymbol>
NativeEnumSymbols::getChildAtIndex(uint32_t N) const {
  if (N < Symbols.size()) {
    return Session.getSymbolCache().getSymbolById(Symbols[N]);
  }
  return nullptr;
}

std::unique_ptr<PDBSymbol> NativeEnumSymbols::getNext() {
  return getChildAtIndex(Index++);
}

void NativeEnumSymbols::reset() { Index = 0; }
