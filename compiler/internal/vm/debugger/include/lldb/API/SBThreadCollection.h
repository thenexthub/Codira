//===-- SBThreadCollection.h ------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBTHREADCOLLECTION_H
#define LLDB_API_SBTHREADCOLLECTION_H

#include "lldb/API/SBDefines.h"

namespace lldb {

class LLDB_API SBThreadCollection {
public:
  SBThreadCollection();

  SBThreadCollection(const SBThreadCollection &rhs);

  const SBThreadCollection &operator=(const SBThreadCollection &rhs);

  ~SBThreadCollection();

  explicit operator bool() const;

  bool IsValid() const;

  size_t GetSize();

  lldb::SBThread GetThreadAtIndex(size_t idx);

protected:
  // Mimic shared pointer...
  lldb_private::ThreadCollection *get() const;

  lldb_private::ThreadCollection *operator->() const;

  lldb::ThreadCollectionSP &operator*();

  const lldb::ThreadCollectionSP &operator*() const;

  SBThreadCollection(const lldb::ThreadCollectionSP &threads);

  void SetOpaque(const lldb::ThreadCollectionSP &threads);

private:
  friend class SBTarget;
  friend class SBProcess;
  friend class SBThread;
  friend class SBSaveCoreOptions;
  lldb::ThreadCollectionSP m_opaque_sp;
};

} // namespace lldb

#endif // LLDB_API_SBTHREADCOLLECTION_H
