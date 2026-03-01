//===-- DWARFDebugAranges.h -------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFDEBUGARANGES_H
#define LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFDEBUGARANGES_H

#include "lldb/Core/dwarf.h"
#include "lldb/Utility/RangeMap.h"
#include "llvm/Support/Error.h"

namespace lldb_private::plugin {
namespace dwarf {
class DWARFDebugAranges {
protected:
  typedef RangeDataVector<dw_addr_t, uint32_t, dw_offset_t> RangeToDIE;

public:
  typedef RangeToDIE::Entry Range;
  typedef std::vector<RangeToDIE::Entry> RangeColl;

  DWARFDebugAranges();

  void Clear() { m_aranges.Clear(); }

  void extract(const DWARFDataExtractor &debug_aranges_data);

  // Use append range multiple times and then call sort
  void AppendRange(dw_offset_t cu_offset, dw_addr_t low_pc, dw_addr_t high_pc);

  void Sort(bool minimize);

  void Dump(Log *log) const;

  dw_offset_t FindAddress(dw_addr_t address) const;

  bool IsEmpty() const { return m_aranges.IsEmpty(); }
  size_t GetNumRanges() const { return m_aranges.GetSize(); }

  dw_offset_t OffsetAtIndex(uint32_t idx) const {
    const Range *range = m_aranges.GetEntryAtIndex(idx);
    if (range)
      return range->data;
    return DW_INVALID_OFFSET;
  }

protected:
  RangeToDIE m_aranges;
};
} // namespace dwarf
} // namespace lldb_private::plugin

#endif // LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFDEBUGARANGES_H
