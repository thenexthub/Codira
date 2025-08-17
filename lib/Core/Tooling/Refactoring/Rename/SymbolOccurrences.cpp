//===--- SymbolOccurrences.cpp - Clang refactoring library ----------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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

#include "language/Core/Tooling/Refactoring/Rename/SymbolOccurrences.h"
#include "language/Core/Tooling/Refactoring/Rename/SymbolName.h"
#include "toolchain/ADT/STLExtras.h"

using namespace language::Core;
using namespace tooling;

SymbolOccurrence::SymbolOccurrence(const SymbolName &Name, OccurrenceKind Kind,
                                   ArrayRef<SourceLocation> Locations)
    : Kind(Kind) {
  ArrayRef<std::string> NamePieces = Name.getNamePieces();
  assert(Locations.size() == NamePieces.size() &&
         "mismatching number of locations and lengths");
  assert(!Locations.empty() && "no locations");
  if (Locations.size() == 1) {
    new (&SingleRange) SourceRange(
        Locations[0], Locations[0].getLocWithOffset(NamePieces[0].size()));
    return;
  }
  MultipleRanges = std::make_unique<SourceRange[]>(Locations.size());
  NumRanges = Locations.size();
  for (const auto &Loc : toolchain::enumerate(Locations)) {
    MultipleRanges[Loc.index()] = SourceRange(
        Loc.value(),
        Loc.value().getLocWithOffset(NamePieces[Loc.index()].size()));
  }
}
