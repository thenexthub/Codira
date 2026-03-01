//===-- SBFileSpecList.h --------------------------------------------*- C++
//-*-===//
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

#ifndef LLDB_API_SBFILESPECLIST_H
#define LLDB_API_SBFILESPECLIST_H

#include "lldb/API/SBDefines.h"

namespace lldb {

class LLDB_API SBFileSpecList {
public:
  SBFileSpecList();

  SBFileSpecList(const lldb::SBFileSpecList &rhs);

  ~SBFileSpecList();

  const SBFileSpecList &operator=(const lldb::SBFileSpecList &rhs);

  uint32_t GetSize() const;

  bool GetDescription(SBStream &description) const;

  void Append(const SBFileSpec &sb_file);

  bool AppendIfUnique(const SBFileSpec &sb_file);

  void Clear();

  uint32_t FindFileIndex(uint32_t idx, const SBFileSpec &sb_file, bool full);

  const SBFileSpec GetFileSpecAtIndex(uint32_t idx) const;

private:
  friend class SBTarget;

  const lldb_private::FileSpecList *operator->() const;

  const lldb_private::FileSpecList *get() const;

  const lldb_private::FileSpecList &operator*() const;

  const lldb_private::FileSpecList &ref() const;

  std::unique_ptr<lldb_private::FileSpecList> m_opaque_up;
};

} // namespace lldb

#endif // LLDB_API_SBFILESPECLIST_H
