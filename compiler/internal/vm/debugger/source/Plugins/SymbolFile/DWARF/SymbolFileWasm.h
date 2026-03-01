//===----------------------------------------------------------------------===//
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

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_SYMBOLFILEWASM_H
#define LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_SYMBOLFILEWASM_H

#include "SymbolFileDWARF.h"

namespace lldb_private::plugin {
namespace dwarf {
class SymbolFileWasm : public SymbolFileDWARF {
public:
  SymbolFileWasm(lldb::ObjectFileSP objfile_sp, SectionList *dwo_section_list);

  ~SymbolFileWasm() override;

  lldb::offset_t GetVendorDWARFOpcodeSize(const DataExtractor &data,
                                          const lldb::offset_t data_offset,
                                          const uint8_t op) const override;

  bool ParseVendorDWARFOpcode(uint8_t op, const DataExtractor &opcodes,
                              lldb::offset_t &offset, RegisterContext *reg_ctx,
                              lldb::RegisterKind reg_kind,
                              std::vector<Value> &stack) const override;
};
} // namespace dwarf
} // namespace lldb_private::plugin

#endif
