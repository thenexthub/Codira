//===-- SBMutex.h ---------------------------------------------------------===//
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

#ifndef LLDB_API_SBMUTEX_H
#define LLDB_API_SBMUTEX_H

#include "lldb/API/SBDefines.h"
#include "lldb/lldb-forward.h"
#include <mutex>

namespace lldb {

class LLDB_API SBMutex {
public:
  SBMutex();
  SBMutex(const SBMutex &rhs);
  const SBMutex &operator=(const SBMutex &rhs);
  ~SBMutex();

  /// Returns true if this lock has ownership of the underlying mutex.
  bool IsValid() const;

  /// Blocking operation that takes ownership of this lock.
  void lock() const;

  /// Releases ownership of this lock.
  void unlock() const;

  /// Tries to lock the mutex. Returns immediately. On successful lock
  /// acquisition returns true, otherwise returns false.
  bool try_lock() const;

private:
  // Private constructor used by SBTarget to create the Target API mutex.
  // Requires a friend declaration.
  SBMutex(lldb::TargetSP target_sp);
  friend class SBTarget;

  std::shared_ptr<std::recursive_mutex> m_opaque_sp;
};

} // namespace lldb

#endif
