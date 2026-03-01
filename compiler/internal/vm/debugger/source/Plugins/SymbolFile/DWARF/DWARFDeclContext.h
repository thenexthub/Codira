//===-- DWARFDeclContext.h --------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFDECLCONTEXT_H
#define LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFDECLCONTEXT_H

#include "DWARFDefines.h"
#include "lldb/Utility/ConstString.h"
#include "llvm/ADT/StringExtras.h"

#include <cassert>
#include <string>
#include <vector>

namespace lldb_private::plugin {
namespace dwarf {
// DWARFDeclContext
//
// A class that represents a declaration context all the way down to a
// DIE. This is useful when trying to find a DIE in one DWARF to a DIE
// in another DWARF file.

class DWARFDeclContext {
public:
  struct Entry {
    Entry() = default;
    Entry(dw_tag_t t, const char *n) : tag(t), name(n) {}

    bool NameMatches(const Entry &rhs) const {
      if (name == rhs.name)
        return true;
      else if (name && rhs.name)
        return strcmp(name, rhs.name) == 0;
      return false;
    }

    /// Returns the name of this entry if it has one, or the appropriate
    /// "anonymous {namespace, class, struct, union}".
    const char *GetName() const;

    // Test operator
    explicit operator bool() const { return tag != 0; }

    dw_tag_t tag = llvm::dwarf::DW_TAG_null;
    const char *name = nullptr;
  };

  DWARFDeclContext() : m_entries() {}

  DWARFDeclContext(llvm::ArrayRef<Entry> entries) {
    llvm::append_range(m_entries, entries);
  }

  void AppendDeclContext(dw_tag_t tag, const char *name) {
    m_entries.push_back(Entry(tag, name));
  }

  bool operator==(const DWARFDeclContext &rhs) const;
  bool operator!=(const DWARFDeclContext &rhs) const { return !(*this == rhs); }

  uint32_t GetSize() const { return m_entries.size(); }

  Entry &operator[](uint32_t idx) {
    assert(idx < m_entries.size() && "invalid index");
    return m_entries[idx];
  }

  const Entry &operator[](uint32_t idx) const {
    assert(idx < m_entries.size() && "invalid index");
    return m_entries[idx];
  }

  const char *GetQualifiedName() const;

  // Same as GetQualifiedName, but the life time of the returned string will
  // be that of the LLDB session.
  ConstString GetQualifiedNameAsConstString() const {
    return ConstString(GetQualifiedName());
  }

  void Clear() {
    m_entries.clear();
    m_qualified_name.clear();
  }

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &OS,
                                       const DWARFDeclContext &ctx) {
    OS << "DWARFDeclContext{";
    llvm::ListSeparator LS;
    for (const Entry &e : ctx.m_entries) {
      OS << LS << "{" << DW_TAG_value_to_name(e.tag) << ", " << e.GetName()
         << "}";
    }
    return OS << "}";
  }

protected:
  typedef std::vector<Entry> collection;
  collection m_entries;
  mutable std::string m_qualified_name;
};
} // namespace dwarf
} // namespace lldb_private::plugin

#endif // LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFDECLCONTEXT_H
