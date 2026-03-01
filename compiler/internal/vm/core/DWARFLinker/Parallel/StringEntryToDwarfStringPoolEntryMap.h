//===- StringEntryToDwarfStringPoolEntryMap.h -------------------*- C++ -*-===//
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

#ifndef LLVM_LIB_DWARFLINKER_PARALLEL_STRINGENTRYTODWARFSTRINGPOOLENTRYMAP_H
#define LLVM_LIB_DWARFLINKER_PARALLEL_STRINGENTRYTODWARFSTRINGPOOLENTRYMAP_H

#include "DWARFLinkerGlobalData.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/DWARFLinker/StringPool.h"

namespace vm::core {
namespace dwarf_linker {
namespace parallel {

/// This class creates a DwarfStringPoolEntry for the corresponding StringEntry.
class StringEntryToDwarfStringPoolEntryMap {
public:
  StringEntryToDwarfStringPoolEntryMap(LinkingGlobalData &GlobalData)
      : GlobalData(GlobalData) {}
  ~StringEntryToDwarfStringPoolEntryMap() = default;

  /// Create DwarfStringPoolEntry for specified StringEntry if necessary.
  /// Initialize DwarfStringPoolEntry with initial values.
  DwarfStringPoolEntryWithExtString *add(const StringEntry *String) {
    DwarfStringPoolEntriesTy::iterator it = DwarfStringPoolEntries.find(String);

    if (it == DwarfStringPoolEntries.end()) {
      DwarfStringPoolEntryWithExtString *DataPtr =
          GlobalData.getAllocator()
              .Allocate<DwarfStringPoolEntryWithExtString>();
      DataPtr->String = String->getKey();
      DataPtr->Index = DwarfStringPoolEntry::NotIndexed;
      DataPtr->Offset = 0;
      DataPtr->Symbol = nullptr;
      it = DwarfStringPoolEntries.insert(std::make_pair(String, DataPtr)).first;
    }

    assert(it->second != nullptr);
    return it->second;
  }

  /// Returns already existed DwarfStringPoolEntry for the specified
  /// StringEntry.
  DwarfStringPoolEntryWithExtString *
  getExistingEntry(const StringEntry *String) const {
    DwarfStringPoolEntriesTy::const_iterator it =
        DwarfStringPoolEntries.find(String);

    assert(it != DwarfStringPoolEntries.end());
    assert(it->second != nullptr);
    return it->second;
  }

  /// Erase contents of StringsForEmission.
  void clear() { DwarfStringPoolEntries.clear(); }

protected:
  using DwarfStringPoolEntriesTy =
      DenseMap<const StringEntry *, DwarfStringPoolEntryWithExtString *>;
  DwarfStringPoolEntriesTy DwarfStringPoolEntries;

  LinkingGlobalData &GlobalData;
};

} // end of namespace parallel
} // end of namespace dwarf_linker
} // end of namespace vm::core

#endif // LLVM_LIB_DWARFLINKER_PARALLEL_STRINGENTRYTODWARFSTRINGPOOLENTRYMAP_H
