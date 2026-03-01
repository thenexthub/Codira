//===-- SBAddressRange.h ----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBADDRESSRANGE_H
#define LLDB_API_SBADDRESSRANGE_H

#include "lldb/API/SBDefines.h"

namespace lldb_private {
class AddressRange;
}

namespace lldb {

class LLDB_API SBAddressRange {
public:
  SBAddressRange();

  SBAddressRange(const lldb::SBAddressRange &rhs);

  SBAddressRange(lldb::SBAddress addr, lldb::addr_t byte_size);

  ~SBAddressRange();

  const lldb::SBAddressRange &operator=(const lldb::SBAddressRange &rhs);

  void Clear();

  /// Check the address range refers to a valid base address and has a byte
  /// size greater than zero.
  ///
  /// \return
  ///     True if the address range is valid, false otherwise.
  bool IsValid() const;

  /// Get the base address of the range.
  ///
  /// \return
  ///     Base address object.
  lldb::SBAddress GetBaseAddress() const;

  /// Get the byte size of this range.
  ///
  /// \return
  ///     The size in bytes of this address range.
  lldb::addr_t GetByteSize() const;

  bool operator==(const SBAddressRange &rhs);

  bool operator!=(const SBAddressRange &rhs);

  bool GetDescription(lldb::SBStream &description, const SBTarget target);

private:
  friend class SBAddressRangeList;
  friend class SBBlock;
  friend class SBFunction;
  friend class SBProcess;

  lldb_private::AddressRange &ref() const;

  AddressRangeUP m_opaque_up;
};

} // namespace lldb

#endif // LLDB_API_SBADDRESSRANGE_H
