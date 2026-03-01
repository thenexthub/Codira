//===-- ThreadSafeDenseSet.h ------------------------------------------*- C++
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

#ifndef liblldb_ThreadSafeDenseSet_h_
#define liblldb_ThreadSafeDenseSet_h_

#include <mutex>

#include "llvm/ADT/DenseSet.h"


namespace lldb_private {

template <typename _ElementType, typename _MutexType = std::mutex>
class ThreadSafeDenseSet {
public:
  typedef llvm::DenseSet<_ElementType> LLVMSetType;

  ThreadSafeDenseSet(unsigned set_initial_capacity = 0)
      : m_set(set_initial_capacity), m_mutex() {}

  void Insert(_ElementType e) {
    std::lock_guard<_MutexType> guard(m_mutex);
    m_set.insert(e);
  }

  void Erase(_ElementType e) {
    std::lock_guard<_MutexType> guard(m_mutex);
    m_set.erase(e);
  }

  bool Lookup(_ElementType e) {
    std::lock_guard<_MutexType> guard(m_mutex);
    return (m_set.count(e) > 0);
  }

  void Clear() {
    std::lock_guard<_MutexType> guard(m_mutex);
    m_set.clear();
  }

protected:
  LLVMSetType m_set;
  _MutexType m_mutex;
};

} // namespace lldb_private

#endif // liblldb_ThreadSafeDenseSet_h_
