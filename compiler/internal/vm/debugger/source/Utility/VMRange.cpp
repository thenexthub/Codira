//===-- VMRange.cpp -------------------------------------------------------===//
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

#include "lldb/Utility/VMRange.h"

#include "lldb/Utility/Stream.h"
#include "lldb/lldb-types.h"

#include <algorithm>
#include <iterator>
#include <vector>

#include <cstddef>
#include <cstdint>

using namespace lldb;
using namespace lldb_private;

bool VMRange::ContainsValue(const VMRange::collection &coll,
                            lldb::addr_t value) {
  return llvm::any_of(coll,
                      [&](const VMRange &r) { return r.Contains(value); });
}

bool VMRange::ContainsRange(const VMRange::collection &coll,
                            const VMRange &range) {
  return llvm::any_of(coll,
                      [&](const VMRange &r) { return r.Contains(range); });
}

void VMRange::Dump(llvm::raw_ostream &s, lldb::addr_t offset,
                   uint32_t addr_width) const {
  DumpAddressRange(s, offset + GetBaseAddress(), offset + GetEndAddress(),
                   addr_width);
}

bool lldb_private::operator==(const VMRange &lhs, const VMRange &rhs) {
  return lhs.GetBaseAddress() == rhs.GetBaseAddress() &&
         lhs.GetEndAddress() == rhs.GetEndAddress();
}

bool lldb_private::operator!=(const VMRange &lhs, const VMRange &rhs) {
  return !(lhs == rhs);
}

bool lldb_private::operator<(const VMRange &lhs, const VMRange &rhs) {
  if (lhs.GetBaseAddress() < rhs.GetBaseAddress())
    return true;
  else if (lhs.GetBaseAddress() > rhs.GetBaseAddress())
    return false;
  return lhs.GetEndAddress() < rhs.GetEndAddress();
}

bool lldb_private::operator<=(const VMRange &lhs, const VMRange &rhs) {
  return !(lhs > rhs);
}

bool lldb_private::operator>(const VMRange &lhs, const VMRange &rhs) {
  return rhs < lhs;
}

bool lldb_private::operator>=(const VMRange &lhs, const VMRange &rhs) {
  return !(lhs < rhs);
}
