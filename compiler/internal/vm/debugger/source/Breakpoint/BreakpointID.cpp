//===-- BreakpointID.cpp --------------------------------------------------===//
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

#include <cstdio>
#include <optional>

#include "lldb/Breakpoint/Breakpoint.h"
#include "lldb/Breakpoint/BreakpointID.h"
#include "lldb/Utility/Status.h"
#include "lldb/Utility/Stream.h"

using namespace lldb;
using namespace lldb_private;

BreakpointID::BreakpointID(break_id_t bp_id, break_id_t loc_id)
    : m_break_id(bp_id), m_location_id(loc_id) {}

BreakpointID::~BreakpointID() = default;

static llvm::StringRef g_range_specifiers[] = {"-", "to", "To", "TO"};

// Tells whether or not STR is valid to use between two strings representing
// breakpoint IDs, to indicate a range of breakpoint IDs.  This is broken out
// into a separate function so that we can easily change or add to the format
// for specifying ID ranges at a later date.

bool BreakpointID::IsRangeIdentifier(llvm::StringRef str) {
  return llvm::is_contained(g_range_specifiers, str);
}

bool BreakpointID::IsValidIDExpression(llvm::StringRef str) {
  return BreakpointID::ParseCanonicalReference(str).has_value();
}

llvm::ArrayRef<llvm::StringRef> BreakpointID::GetRangeSpecifiers() {
  return llvm::ArrayRef(g_range_specifiers);
}

void BreakpointID::GetDescription(Stream *s, lldb::DescriptionLevel level) {
  if (level == eDescriptionLevelVerbose)
    s->Printf("%p BreakpointID:", static_cast<void *>(this));

  if (m_break_id == LLDB_INVALID_BREAK_ID)
    s->PutCString("<invalid>");
  else if (m_location_id == LLDB_INVALID_BREAK_ID)
    s->Printf("%i", m_break_id);
  else
    s->Printf("%i.%i", m_break_id, m_location_id);
}

void BreakpointID::GetCanonicalReference(Stream *s, break_id_t bp_id,
                                         break_id_t loc_id) {
  if (bp_id == LLDB_INVALID_BREAK_ID)
    s->PutCString("<invalid>");
  else if (loc_id == LLDB_INVALID_BREAK_ID)
    s->Printf("%i", bp_id);
  else
    s->Printf("%i.%i", bp_id, loc_id);
}

std::optional<BreakpointID>
BreakpointID::ParseCanonicalReference(llvm::StringRef input) {
  break_id_t bp_id;
  break_id_t loc_id = LLDB_INVALID_BREAK_ID;

  if (input.empty())
    return std::nullopt;

  // If it doesn't start with an integer, it's not valid.
  if (input.consumeInteger(0, bp_id))
    return std::nullopt;

  // period is optional, but if it exists, it must be followed by a number.
  if (input.consume_front(".")) {
    if (input.consumeInteger(0, loc_id))
      return std::nullopt;
  }

  // And at the end, the entire string must have been consumed.
  if (!input.empty())
    return std::nullopt;

  return BreakpointID(bp_id, loc_id);
}

bool BreakpointID::StringIsBreakpointName(llvm::StringRef str, Status &error) {
  error.Clear();
  if (str.empty())
  {
    error = Status::FromErrorString("Empty breakpoint names are not allowed");
    return false;
  }

  // First character must be a letter or _
  if (!isalpha(str[0]) && str[0] != '_')
  {
    error =
        Status::FromErrorStringWithFormatv("Breakpoint names must start with a "
                                           "character or underscore: {0}",
                                           str);
    return false;
  }

  // Cannot contain ., -, or space.
  if (str.find_first_of(".- ") != llvm::StringRef::npos) {
    error =
        Status::FromErrorStringWithFormatv("Breakpoint names cannot contain "
                                           "'.' or '-' or spaces: \"{0}\"",
                                           str);
    return false;
  }

  return true;
}
