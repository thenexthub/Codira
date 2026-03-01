//==- NativeEnumTypes.cpp - Native Type Enumerator impl ----------*- C++ -*-==//
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

#include "vm/core/DebugInfo/PDB/Native/NativeEnumTypes.h"

#include "vm/core/ADT/STLExtras.h"
#include "vm/core/DebugInfo/CodeView/CVRecord.h"
#include "vm/core/DebugInfo/CodeView/LazyRandomTypeCollection.h"
#include "vm/core/DebugInfo/CodeView/TypeRecordHelpers.h"
#include "vm/core/DebugInfo/PDB/Native/NativeSession.h"
#include "vm/core/DebugInfo/PDB/Native/SymbolCache.h"
#include "vm/core/DebugInfo/PDB/PDBSymbol.h"
#include "vm/core/DebugInfo/PDB/PDBTypes.h"

using namespace vm::core;
using namespace vm::core::codeview;
using namespace vm::core::pdb;

NativeEnumTypes::NativeEnumTypes(NativeSession &PDBSession,
                                 LazyRandomTypeCollection &Types,
                                 std::vector<codeview::TypeLeafKind> Kinds)
    : Index(0), Session(PDBSession) {
  std::optional<TypeIndex> TI = Types.getFirst();
  while (TI) {
    CVType CVT = Types.getType(*TI);
    TypeLeafKind K = CVT.kind();
    if (toolchain::is_contained(Kinds, K)) {
      // Don't add forward refs, we'll find those later while enumerating.
      if (!isUdtForwardRef(CVT))
        Matches.push_back(*TI);
    } else if (K == TypeLeafKind::LF_MODIFIER) {
      TypeIndex ModifiedTI = getModifiedType(CVT);
      if (!ModifiedTI.isSimple()) {
        CVType UnmodifiedCVT = Types.getType(ModifiedTI);
        // LF_MODIFIERs point to forward refs, but don't worry about that
        // here.  We're pushing the TypeIndex of the LF_MODIFIER itself,
        // so we'll worry about resolving forward refs later.
        if (toolchain::is_contained(Kinds, UnmodifiedCVT.kind()))
          Matches.push_back(*TI);
      }
    }
    TI = Types.getNext(*TI);
  }
}

NativeEnumTypes::NativeEnumTypes(NativeSession &PDBSession,
                                 std::vector<codeview::TypeIndex> Indices)
    : Matches(std::move(Indices)), Index(0), Session(PDBSession) {}

uint32_t NativeEnumTypes::getChildCount() const {
  return static_cast<uint32_t>(Matches.size());
}

std::unique_ptr<PDBSymbol> NativeEnumTypes::getChildAtIndex(uint32_t N) const {
  if (N < Matches.size()) {
    SymIndexId Id = Session.getSymbolCache().findSymbolByTypeIndex(Matches[N]);
    return Session.getSymbolCache().getSymbolById(Id);
  }
  return nullptr;
}

std::unique_ptr<PDBSymbol> NativeEnumTypes::getNext() {
  return getChildAtIndex(Index++);
}

void NativeEnumTypes::reset() { Index = 0; }
