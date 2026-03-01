//===-- ThreadSafeValue.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_CORE_THREADSAFEVALUE_H
#define LLDB_CORE_THREADSAFEVALUE_H

#include <mutex>

#include "lldb/lldb-defines.h"

namespace lldb_private {

template <class T> class ThreadSafeValue {
public:
  ThreadSafeValue() = default;
  ThreadSafeValue(const T &value) : m_value(value) {}

  ~ThreadSafeValue() = default;

  T GetValue() const {
    T value;
    {
      std::lock_guard<std::recursive_mutex> guard(m_mutex);
      value = m_value;
    }
    return value;
  }

  // Call this if you have already manually locked the mutex using the
  // GetMutex() accessor
  const T &GetValueNoLock() const { return m_value; }

  void SetValue(const T &value) {
    std::lock_guard<std::recursive_mutex> guard(m_mutex);
    m_value = value;
  }

  // Call this if you have already manually locked the mutex using the
  // GetMutex() accessor
  // coverity[missing_lock]
  void SetValueNoLock(const T &value) { m_value = value; }

  std::recursive_mutex &GetMutex() { return m_mutex; }

private:
  T m_value;
  mutable std::recursive_mutex m_mutex;

  // For ThreadSafeValue only
  ThreadSafeValue(const ThreadSafeValue &) = delete;
  const ThreadSafeValue &operator=(const ThreadSafeValue &) = delete;
};

} // namespace lldb_private
#endif // LLDB_CORE_THREADSAFEVALUE_H
