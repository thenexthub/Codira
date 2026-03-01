//===-- NonRelocatableStringpool.cpp --------------------------------------===//
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

#include "vm/core/CodeGen/NonRelocatableStringpool.h"
#include "vm/core/ADT/STLExtras.h"

using namespace vm::core;

DwarfStringPoolEntryRef NonRelocatableStringpool::getEntry(StringRef S) {
  auto I = Strings.try_emplace(S);
  auto &Entry = I.first->second;
  if (I.second || !Entry.isIndexed()) {
    Entry.Index = NumEntries++;
    Entry.Offset = CurrentEndOffset;
    Entry.Symbol = nullptr;
    CurrentEndOffset += S.size() + 1;
  }
  return DwarfStringPoolEntryRef(*I.first);
}

StringRef NonRelocatableStringpool::internString(StringRef S) {
  DwarfStringPoolEntry Entry{nullptr, 0, DwarfStringPoolEntry::NotIndexed};

  auto InsertResult = Strings.insert({S, Entry});
  return InsertResult.first->getKey();
}

std::vector<DwarfStringPoolEntryRef>
NonRelocatableStringpool::getEntriesForEmission() const {
  std::vector<DwarfStringPoolEntryRef> Result;
  Result.reserve(Strings.size());
  for (const auto &E : Strings)
    if (E.getValue().isIndexed())
      Result.emplace_back(E);
  toolchain::sort(Result, [](const DwarfStringPoolEntryRef A,
                        const DwarfStringPoolEntryRef B) {
    return A.getIndex() < B.getIndex();
  });
  return Result;
}
