//===-- NameToDIE.h ---------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_NAMETODIE_H
#define LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_NAMETODIE_H

#include <functional>

#include "DIERef.h"
#include "lldb/Core/UniqueCStringMap.h"
#include "lldb/Core/dwarf.h"
#include "lldb/lldb-defines.h"
#include "lldb/lldb-private-enumerations.h"

namespace lldb_private::plugin {
namespace dwarf {
class DWARFUnit;

class NameToDIE {
public:
  NameToDIE() : m_map() {}

  ~NameToDIE() = default;

  void Dump(Stream *s);

  void Insert(ConstString name, const DIERef &die_ref);

  void Append(const NameToDIE &other);

  void Finalize();

  bool Find(ConstString name,
            llvm::function_ref<IterationAction(DIERef ref)> callback) const;

  bool Find(const RegularExpression &regex,
            llvm::function_ref<IterationAction(DIERef ref)> callback) const;

  /// \a unit must be the skeleton unit if possible, not GetNonSkeletonUnit().
  void FindAllEntriesForUnit(
      DWARFUnit &unit,
      llvm::function_ref<IterationAction(DIERef ref)> callback) const;

  void
  ForEach(std::function<bool(ConstString name, const DIERef &die_ref)> const
              &callback) const;

  /// Decode a serialized version of this object from data.
  ///
  /// \param data
  ///   The decoder object that references the serialized data.
  ///
  /// \param offset_ptr
  ///   A pointer that contains the offset from which the data will be decoded
  ///   from that gets updated as data gets decoded.
  ///
  /// \param strtab
  ///   All strings in cache files are put into string tables for efficiency
  ///   and cache file size reduction. Strings are stored as uint32_t string
  ///   table offsets in the cache data.
  bool Decode(const DataExtractor &data, lldb::offset_t *offset_ptr,
              const StringTableReader &strtab);

  /// Encode this object into a data encoder object.
  ///
  /// This allows this object to be serialized to disk.
  ///
  /// \param encoder
  ///   A data encoder object that serialized bytes will be encoded into.
  ///
  /// \param strtab
  ///   All strings in cache files are put into string tables for efficiency
  ///   and cache file size reduction. Strings are stored as uint32_t string
  ///   table offsets in the cache data.
  void Encode(DataEncoder &encoder, ConstStringTable &strtab) const;

  /// Used for unit testing the encoding and decoding.
  bool operator==(const NameToDIE &rhs) const;

  bool IsEmpty() const { return m_map.IsEmpty(); }

  void Clear() { m_map.Clear(); }

protected:
  UniqueCStringMap<DIERef> m_map;
};
} // namespace dwarf
} // namespace lldb_private::plugin

#endif // LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_NAMETODIE_H
