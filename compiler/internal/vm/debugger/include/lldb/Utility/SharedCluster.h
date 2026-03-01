//===------------------SharedCluster.h --------------------------*- C++ -*-===//
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

#ifndef LLDB_UTILITY_SHAREDCLUSTER_H
#define LLDB_UTILITY_SHAREDCLUSTER_H

#include "lldb/Utility/LLDBAssert.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallPtrSet.h"

#include <memory>
#include <mutex>

namespace lldb_private {

template <class T>
class ClusterManager : public std::enable_shared_from_this<ClusterManager<T>> {
public:
  static std::shared_ptr<ClusterManager> Create() {
    return std::shared_ptr<ClusterManager>(new ClusterManager());
  }

  ~ClusterManager() {
    for (T *obj : m_objects)
      delete obj;
  }

  void ManageObject(T *new_object) {
    std::lock_guard<std::mutex> guard(m_mutex);
    auto ret = m_objects.insert(new_object);
    assert(ret.second && "ManageObject called twice for the same object?");
    (void)ret;
  }

  std::shared_ptr<T> GetSharedPointer(T *desired_object) {
    std::lock_guard<std::mutex> guard(m_mutex);
    auto this_sp = this->shared_from_this();
    size_t count =  m_objects.count(desired_object);
    if (count == 0) {
      lldbassert(false && "object not found in shared cluster when expected");
      desired_object = nullptr;
    }
    return {std::move(this_sp), desired_object};
  }

private:
  ClusterManager() : m_objects() {}
  // The cluster manager is used primarily to manage the
  // children of root ValueObjects. So it will always have
  // one element - the root.  Pointers will often have dynamic
  // values, so having 2 entries is pretty common.  It's also
  // pretty common to have small (2,3) structs, so setting the
  // static size to 4 will cover those cases with no allocations
  // w/o wasting too much space.
  llvm::SmallPtrSet<T *, 4> m_objects;
  std::mutex m_mutex;
};

} // namespace lldb_private

#endif // LLDB_UTILITY_SHAREDCLUSTER_H
