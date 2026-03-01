//===-- ThreadSafeDenseMap.h ------------------------------------*- C++ -*-===//
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

#ifndef LLDB_UTILITY_THREADSAFEDENSEMAP_H
#define LLDB_UTILITY_THREADSAFEDENSEMAP_H

#include <mutex>

#include "llvm/ADT/DenseMap.h"

namespace lldb_private {

template <typename _KeyType, typename _ValueType> class ThreadSafeDenseMap {
public:
  typedef llvm::DenseMap<_KeyType, _ValueType> LLVMMapType;

  ThreadSafeDenseMap(unsigned map_initial_capacity = 0)
      : m_map(map_initial_capacity), m_mutex() {}

  void Insert(_KeyType k, _ValueType v) {
    std::lock_guard<std::mutex> guard(m_mutex);
    m_map.insert(std::make_pair(k, v));
  }

  void Erase(_KeyType k) {
    std::lock_guard<std::mutex> guard(m_mutex);
    m_map.erase(k);
  }

  _ValueType Lookup(_KeyType k) {
    std::lock_guard<std::mutex> guard(m_mutex);
    return m_map.lookup(k);
  }

  bool Lookup(_KeyType k, _ValueType &v) {
    std::lock_guard<std::mutex> guard(m_mutex);
    auto iter = m_map.find(k), end = m_map.end();
    if (iter == end)
      return false;
    v = iter->second;
    return true;
  }

  void Clear() {
    std::lock_guard<std::mutex> guard(m_mutex);
    m_map.clear();
  }

protected:
  LLVMMapType m_map;
  std::mutex m_mutex;
};

} // namespace lldb_private

#endif // LLDB_UTILITY_THREADSAFEDENSEMAP_H
