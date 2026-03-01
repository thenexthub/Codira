//===-- MemoryMonitor.h ---------------------------------------------------===//
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

#ifndef LLDB_HOST_MEMORYMONITOR_H
#define LLDB_HOST_MEMORYMONITOR_H

#include <functional>
#include <memory>

namespace lldb_private {

class MemoryMonitor {
public:
  using Callback = std::function<void()>;

  MemoryMonitor(Callback callback) : m_callback(callback) {}
  virtual ~MemoryMonitor() = default;

  /// MemoryMonitor is not copyable.
  /// @{
  MemoryMonitor(const MemoryMonitor &) = delete;
  MemoryMonitor &operator=(const MemoryMonitor &) = delete;
  /// @}

  static std::unique_ptr<MemoryMonitor> Create(Callback callback);

  virtual void Start() = 0;
  virtual void Stop() = 0;

protected:
  Callback m_callback;
};

} // namespace lldb_private

#endif
