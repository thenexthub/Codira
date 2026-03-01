//===-- ThreadCollection.h --------------------------------------*- C++ -*-===//
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

#ifndef LLDB_TARGET_THREADCOLLECTION_H
#define LLDB_TARGET_THREADCOLLECTION_H

#include <mutex>
#include <vector>

#include "lldb/Utility/Iterable.h"
#include "lldb/lldb-private.h"

namespace lldb_private {

class ThreadCollection {
public:
  typedef std::vector<lldb::ThreadSP> collection;
  typedef LockingAdaptedIterable<std::recursive_mutex, collection>
      ThreadIterable;

  ThreadCollection();

  ThreadCollection(collection threads);

  virtual ~ThreadCollection() = default;

  uint32_t GetSize();

  void AddThread(const lldb::ThreadSP &thread_sp);

  void AddThreadSortedByIndexID(const lldb::ThreadSP &thread_sp);

  void InsertThread(const lldb::ThreadSP &thread_sp, uint32_t idx);

  // Note that "idx" is not the same as the "thread_index". It is a zero based
  // index to accessing the current threads, whereas "thread_index" is a unique
  // index assigned
  lldb::ThreadSP GetThreadAtIndex(uint32_t idx);

  virtual ThreadIterable Threads() {
    return ThreadIterable(m_threads, GetMutex());
  }

  virtual std::recursive_mutex &GetMutex() const { return m_mutex; }

protected:
  collection m_threads;
  mutable std::recursive_mutex m_mutex;
};

} // namespace lldb_private

#endif // LLDB_TARGET_THREADCOLLECTION_H
