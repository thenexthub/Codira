//===-- ThreadCollection.cpp ----------------------------------------------===//
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
#include <cstdlib>

#include <algorithm>
#include <mutex>

#include "lldb/Target/Thread.h"
#include "lldb/Target/ThreadCollection.h"

using namespace lldb;
using namespace lldb_private;

ThreadCollection::ThreadCollection() : m_threads(), m_mutex() {}

ThreadCollection::ThreadCollection(collection threads)
    : m_threads(threads), m_mutex() {}

void ThreadCollection::AddThread(const ThreadSP &thread_sp) {
  std::lock_guard<std::recursive_mutex> guard(GetMutex());
  m_threads.push_back(thread_sp);
}

void ThreadCollection::AddThreadSortedByIndexID(const ThreadSP &thread_sp) {
  std::lock_guard<std::recursive_mutex> guard(GetMutex());
  // Make sure we always keep the threads sorted by thread index ID
  const uint32_t thread_index_id = thread_sp->GetIndexID();
  if (m_threads.empty() || m_threads.back()->GetIndexID() < thread_index_id)
    m_threads.push_back(thread_sp);
  else {
    m_threads.insert(
        llvm::upper_bound(m_threads, thread_sp,
                          [](const ThreadSP &lhs, const ThreadSP &rhs) -> bool {
                            return lhs->GetIndexID() < rhs->GetIndexID();
                          }),
        thread_sp);
  }
}

void ThreadCollection::InsertThread(const lldb::ThreadSP &thread_sp,
                                    uint32_t idx) {
  std::lock_guard<std::recursive_mutex> guard(GetMutex());
  if (idx < m_threads.size())
    m_threads.insert(m_threads.begin() + idx, thread_sp);
  else
    m_threads.push_back(thread_sp);
}

uint32_t ThreadCollection::GetSize() {
  std::lock_guard<std::recursive_mutex> guard(GetMutex());
  return m_threads.size();
}

ThreadSP ThreadCollection::GetThreadAtIndex(uint32_t idx) {
  std::lock_guard<std::recursive_mutex> guard(GetMutex());
  ThreadSP thread_sp;
  if (idx < m_threads.size())
    thread_sp = m_threads[idx];
  return thread_sp;
}
