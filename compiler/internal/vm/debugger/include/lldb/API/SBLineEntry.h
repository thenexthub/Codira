//===-- SBLineEntry.h -------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBLINEENTRY_H
#define LLDB_API_SBLINEENTRY_H

#include "lldb/API/SBAddress.h"
#include "lldb/API/SBDefines.h"
#include "lldb/API/SBFileSpec.h"

namespace lldb {

class LLDB_API SBLineEntry {
public:
  SBLineEntry();

  SBLineEntry(const lldb::SBLineEntry &rhs);

  ~SBLineEntry();

  const lldb::SBLineEntry &operator=(const lldb::SBLineEntry &rhs);

  lldb::SBAddress GetStartAddress() const;

  lldb::SBAddress GetEndAddress() const;

  lldb::SBAddress
  GetSameLineContiguousAddressRangeEnd(bool include_inlined_functions) const;

  explicit operator bool() const;

  bool IsValid() const;

  lldb::SBFileSpec GetFileSpec() const;

  uint32_t GetLine() const;

  uint32_t GetColumn() const;

  void SetFileSpec(lldb::SBFileSpec filespec);

  void SetLine(uint32_t line);

  void SetColumn(uint32_t column);

  bool operator==(const lldb::SBLineEntry &rhs) const;

  bool operator!=(const lldb::SBLineEntry &rhs) const;

  bool GetDescription(lldb::SBStream &description);

protected:
  lldb_private::LineEntry *get();

private:
  friend class SBAddress;
  friend class SBCompileUnit;
  friend class SBFrame;
  friend class SBSymbolContext;

  const lldb_private::LineEntry *operator->() const;

  lldb_private::LineEntry &ref();

  const lldb_private::LineEntry &ref() const;

  SBLineEntry(const lldb_private::LineEntry *lldb_object_ptr);

  void SetLineEntry(const lldb_private::LineEntry &lldb_object_ref);

  std::unique_ptr<lldb_private::LineEntry> m_opaque_up;
};

} // namespace lldb

#endif // LLDB_API_SBLINEENTRY_H
