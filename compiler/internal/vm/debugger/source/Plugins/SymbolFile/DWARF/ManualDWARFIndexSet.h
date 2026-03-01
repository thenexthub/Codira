//===-- ManualDWARFIndexSet.h -----------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_MANUALDWARFINDEXSET_H
#define LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_MANUALDWARFINDEXSET_H

#include "Plugins/SymbolFile/DWARF/NameToDIE.h"
#include "lldb/Utility/DataEncoder.h"
#include "lldb/Utility/DataExtractor.h"
#include "llvm/ADT/STLExtras.h"
#include <optional>

namespace lldb_private::plugin::dwarf {

template <typename T> struct IndexSet {
  T function_basenames;
  T function_fullnames;
  T function_methods;
  T function_selectors;
  T objc_class_selectors;
  T globals;
  T types;
  T namespaces;

  static std::array<T(IndexSet::*), 8> Indices() {
    return {&IndexSet::function_basenames,
            &IndexSet::function_fullnames,
            &IndexSet::function_methods,
            &IndexSet::function_selectors,
            &IndexSet::objc_class_selectors,
            &IndexSet::globals,
            &IndexSet::types,
            &IndexSet::namespaces};
  }

  friend bool operator==(const IndexSet &lhs, const IndexSet &rhs) {
    return llvm::all_of(Indices(), [&lhs, &rhs](T(IndexSet::*index)) {
      return lhs.*index == rhs.*index;
    });
  }
};

std::optional<IndexSet<NameToDIE>> DecodeIndexSet(const DataExtractor &data,
                                                  lldb::offset_t *offset_ptr);
void EncodeIndexSet(const IndexSet<NameToDIE> &set, DataEncoder &encoder);

} // namespace lldb_private::plugin::dwarf

#endif // LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_MANUALDWARFINDEXSET_H
