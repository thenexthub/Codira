//===-- DWARFTypeUnit.h -----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFTYPEUNIT_H
#define LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFTYPEUNIT_H

#include "DWARFUnit.h"
#include "llvm/Support/Error.h"

namespace llvm {
class DWARFAbbreviationDeclarationSet;
} // namespace llvm

namespace lldb_private::plugin {
namespace dwarf {
class DWARFTypeUnit : public DWARFUnit {
public:
  void BuildAddressRangeTable(DWARFDebugAranges *debug_aranges) override {}

  void Dump(Stream *s) const override;

  uint64_t GetTypeHash() { return m_header.getTypeHash(); }

  dw_offset_t GetTypeOffset() { return GetOffset() + m_header.getTypeOffset(); }

  static bool classof(const DWARFUnit *unit) { return unit->IsTypeUnit(); }

private:
  DWARFTypeUnit(SymbolFileDWARF &dwarf, lldb::user_id_t uid,
                const llvm::DWARFUnitHeader &header,
                const llvm::DWARFAbbreviationDeclarationSet &abbrevs,
                DIERef::Section section, bool is_dwo)
      : DWARFUnit(dwarf, uid, header, abbrevs, section, is_dwo) {}

  friend class DWARFUnit;
};
} // namespace dwarf
} // namespace lldb_private::plugin

#endif // LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFTYPEUNIT_H
