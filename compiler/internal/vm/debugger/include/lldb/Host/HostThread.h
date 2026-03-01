//===-- HostThread.h --------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_HOST_HOSTTHREAD_H
#define LLDB_HOST_HOSTTHREAD_H

#include "lldb/Host/HostNativeThreadForward.h"
#include "lldb/Utility/Status.h"
#include "lldb/lldb-types.h"

#include <memory>

namespace lldb_private {

class HostNativeThreadBase;

/// \class HostInfo HostInfo.h "lldb/Host/HostThread.h"
/// A class that represents a thread running inside of a process on the
///        local machine.
///
/// HostThread allows querying and manipulation of threads running on the host
/// machine.
///
class HostThread {
public:
  HostThread();
  HostThread(lldb::thread_t thread);

  Status Join(lldb::thread_result_t *result);
  Status Cancel();
  void Reset();
  lldb::thread_t Release();

  bool IsJoinable() const;
  HostNativeThread &GetNativeThread();
  const HostNativeThread &GetNativeThread() const;
  lldb::thread_result_t GetResult() const;

  bool EqualsThread(lldb::thread_t thread) const;

  bool HasThread() const;

private:
  std::shared_ptr<HostNativeThreadBase> m_native_thread;
};
}

#endif
