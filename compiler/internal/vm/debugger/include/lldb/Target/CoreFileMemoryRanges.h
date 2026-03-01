//===-- CoreFileMemoryRanges.h ----------------------------------*- C++ -*-===//
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

#include "lldb/Utility/RangeMap.h"
#include "lldb/Utility/Status.h"
#include "lldb/Utility/StreamString.h"

#include "llvm/ADT/AddressRanges.h"

#ifndef LLDB_TARGET_COREFILEMEMORYRANGES_H
#define LLDB_TARGET_COREFILEMEMORYRANGES_H

namespace lldb_private {

struct CoreFileMemoryRange {
  llvm::AddressRange range;  /// The address range to save into the core file.
  uint32_t lldb_permissions; /// A bit set of lldb::Permissions bits.

  bool operator==(const CoreFileMemoryRange &rhs) const {
    return range == rhs.range && lldb_permissions == rhs.lldb_permissions;
  }

  bool operator!=(const CoreFileMemoryRange &rhs) const {
    return !(*this == rhs);
  }

  bool operator<(const CoreFileMemoryRange &rhs) const {
    return std::tie(range, lldb_permissions) <
           std::tie(rhs.range, rhs.lldb_permissions);
  }

  std::string Dump() const {
    lldb_private::StreamString stream;
    stream << "[";
    stream.PutHex64(range.start());
    stream << '-';
    stream.PutHex64(range.end());
    stream << ")";
    return stream.GetString().str();
  }
};

class CoreFileMemoryRanges
    : public lldb_private::RangeDataVector<lldb::addr_t, lldb::addr_t,
                                           CoreFileMemoryRange> {
public:
  /// Finalize and merge all overlapping ranges in this collection. Ranges
  /// will be separated based on permissions.
  Status FinalizeCoreFileSaveRanges();
};
} // namespace lldb_private

#endif // LLDB_TARGET_COREFILEMEMORYRANGES_H
