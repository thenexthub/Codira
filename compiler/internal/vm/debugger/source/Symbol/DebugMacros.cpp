//===-- DebugMacros.cpp ---------------------------------------------------===//
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

#include "lldb/Symbol/DebugMacros.h"

#include "lldb/Symbol/CompileUnit.h"

using namespace lldb_private;

DebugMacroEntry::DebugMacroEntry(EntryType type, uint32_t line,
                                 uint32_t debug_line_file_idx, const char *str)
    : m_type(type), m_line(line), m_debug_line_file_idx(debug_line_file_idx),
      m_str(str) {}

DebugMacroEntry::DebugMacroEntry(EntryType type,
                                 const DebugMacrosSP &debug_macros_sp)
    : m_type(type), m_line(0), m_debug_line_file_idx(0),
      m_debug_macros_sp(debug_macros_sp) {}

const FileSpec &DebugMacroEntry::GetFileSpec(CompileUnit *comp_unit) const {
  return comp_unit->GetSupportFiles().GetFileSpecAtIndex(m_debug_line_file_idx);
}

DebugMacroEntry DebugMacroEntry::CreateDefineEntry(uint32_t line,
                                                   const char *str) {
  return DebugMacroEntry(DebugMacroEntry::DEFINE, line, 0, str);
}

DebugMacroEntry DebugMacroEntry::CreateUndefEntry(uint32_t line,
                                                  const char *str) {
  return DebugMacroEntry(DebugMacroEntry::UNDEF, line, 0, str);
}

DebugMacroEntry
DebugMacroEntry::CreateStartFileEntry(uint32_t line,
                                      uint32_t debug_line_file_idx) {
  return DebugMacroEntry(DebugMacroEntry::START_FILE, line, debug_line_file_idx,
                         nullptr);
}

DebugMacroEntry DebugMacroEntry::CreateEndFileEntry() {
  return DebugMacroEntry(DebugMacroEntry::END_FILE, 0, 0, nullptr);
}

DebugMacroEntry
DebugMacroEntry::CreateIndirectEntry(const DebugMacrosSP &debug_macros_sp) {
  return DebugMacroEntry(DebugMacroEntry::INDIRECT, debug_macros_sp);
}
