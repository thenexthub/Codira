//===-- SourceLocationSpec.cpp --------------------------------------------===//
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

#include "lldb/Core/SourceLocationSpec.h"
#include "lldb/Utility/StreamString.h"
#include "llvm/ADT/StringExtras.h"
#include <optional>

using namespace lldb;
using namespace lldb_private;

SourceLocationSpec::SourceLocationSpec(FileSpec file_spec, uint32_t line,
                                       std::optional<uint16_t> column,
                                       bool check_inlines, bool exact_match)
    : m_declaration(file_spec, line,
                    column.value_or(LLDB_INVALID_COLUMN_NUMBER)),
      m_check_inlines(check_inlines), m_exact_match(exact_match) {}

SourceLocationSpec::operator bool() const { return m_declaration.IsValid(); }

bool SourceLocationSpec::operator!() const { return !operator bool(); }

bool SourceLocationSpec::operator==(const SourceLocationSpec &rhs) const {
  return m_declaration == rhs.m_declaration &&
         m_check_inlines == rhs.GetCheckInlines() &&
         m_exact_match == rhs.GetExactMatch();
}

bool SourceLocationSpec::operator!=(const SourceLocationSpec &rhs) const {
  return !(*this == rhs);
}

bool SourceLocationSpec::operator<(const SourceLocationSpec &rhs) const {
  return SourceLocationSpec::Compare(*this, rhs) < 0;
}

Stream &lldb_private::operator<<(Stream &s, const SourceLocationSpec &loc) {
  loc.Dump(s);
  return s;
}

int SourceLocationSpec::Compare(const SourceLocationSpec &lhs,
                                const SourceLocationSpec &rhs) {
  return Declaration::Compare(lhs.m_declaration, rhs.m_declaration);
}

bool SourceLocationSpec::Equal(const SourceLocationSpec &lhs,
                               const SourceLocationSpec &rhs, bool full) {
  return full ? lhs == rhs
              : (lhs.GetFileSpec() == rhs.GetFileSpec() &&
                 lhs.GetLine() == rhs.GetLine());
}

void SourceLocationSpec::Dump(Stream &s) const {
  s << "check inlines = " << llvm::toStringRef(m_check_inlines);
  s << ", exact match = " << llvm::toStringRef(m_exact_match);
  m_declaration.Dump(&s, true);
}

std::string SourceLocationSpec::GetString() const {
  StreamString ss;
  Dump(ss);
  return ss.GetString().str();
}

std::optional<uint32_t> SourceLocationSpec::GetLine() const {
  uint32_t line = m_declaration.GetLine();
  if (line == 0 || line == LLDB_INVALID_LINE_NUMBER)
    return std::nullopt;
  return line;
}

std::optional<uint16_t> SourceLocationSpec::GetColumn() const {
  uint16_t column = m_declaration.GetColumn();
  if (column == LLDB_INVALID_COLUMN_NUMBER)
    return std::nullopt;
  return column;
}
