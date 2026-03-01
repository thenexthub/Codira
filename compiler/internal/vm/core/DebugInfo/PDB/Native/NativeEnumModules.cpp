//==- NativeEnumModules.cpp - Native Symbol Enumerator impl ------*- C++ -*-==//
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

#include "vm/core/DebugInfo/PDB/Native/NativeEnumModules.h"

#include "vm/core/DebugInfo/PDB/Native/NativeSession.h"
#include "vm/core/DebugInfo/PDB/Native/SymbolCache.h"
#include "vm/core/DebugInfo/PDB/PDBSymbol.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolCompiland.h"

namespace vm::core {
namespace pdb {

NativeEnumModules::NativeEnumModules(NativeSession &PDBSession, uint32_t Index)
    : Session(PDBSession), Index(Index) {}

uint32_t NativeEnumModules::getChildCount() const {
  return Session.getSymbolCache().getNumCompilands();
}

std::unique_ptr<PDBSymbol>
NativeEnumModules::getChildAtIndex(uint32_t N) const {
  return Session.getSymbolCache().getOrCreateCompiland(N);
}

std::unique_ptr<PDBSymbol> NativeEnumModules::getNext() {
  if (Index >= getChildCount())
    return nullptr;
  return getChildAtIndex(Index++);
}

void NativeEnumModules::reset() { Index = 0; }

}
}
