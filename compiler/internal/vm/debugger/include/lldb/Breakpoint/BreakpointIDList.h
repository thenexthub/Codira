//===-- BreakpointIDList.h --------------------------------------*- C++ -*-===//
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

#ifndef LLDB_BREAKPOINT_BREAKPOINTIDLIST_H
#define LLDB_BREAKPOINT_BREAKPOINTIDLIST_H

#include <utility>
#include <vector>

#include "lldb/Breakpoint/BreakpointID.h"
#include "lldb/Breakpoint/BreakpointName.h"
#include "lldb/lldb-enumerations.h"
#include "lldb/lldb-private.h"

#include "llvm/Support/Error.h"

namespace lldb_private {

// class BreakpointIDList

class BreakpointIDList {
public:
  // TODO: Convert this class to StringRef.
  typedef std::vector<BreakpointID> BreakpointIDArray;

  BreakpointIDList();

  virtual ~BreakpointIDList();

  size_t GetSize() const;

  BreakpointID GetBreakpointIDAtIndex(size_t index) const;

  bool RemoveBreakpointIDAtIndex(size_t index);

  void Clear();

  bool AddBreakpointID(BreakpointID bp_id);

  bool Contains(BreakpointID bp_id) const;

  // Returns a pair consisting of the beginning and end of a breakpoint
  // ID range expression.  If the input string is not a valid specification,
  // returns an empty pair.
  static std::pair<llvm::StringRef, llvm::StringRef>
  SplitIDRangeExpression(llvm::StringRef in_string);

  static llvm::Error
  FindAndReplaceIDRanges(Args &old_args, Target *target, bool allow_locations,
                         BreakpointName::Permissions ::PermissionKinds purpose,
                         Args &new_args);

private:
  BreakpointIDArray m_breakpoint_ids;

  BreakpointIDList(const BreakpointIDList &) = delete;
  const BreakpointIDList &operator=(const BreakpointIDList &) = delete;
};

} // namespace lldb_private

#endif // LLDB_BREAKPOINT_BREAKPOINTIDLIST_H
