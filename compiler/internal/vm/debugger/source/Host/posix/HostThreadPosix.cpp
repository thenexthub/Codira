//===-- HostThreadPosix.cpp -----------------------------------------------===//
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

#include "lldb/Host/posix/HostThreadPosix.h"
#include "lldb/Utility/Status.h"

#include <cerrno>
#include <pthread.h>

using namespace lldb;
using namespace lldb_private;

HostThreadPosix::HostThreadPosix() = default;

HostThreadPosix::HostThreadPosix(lldb::thread_t thread)
    : HostNativeThreadBase(thread) {}

HostThreadPosix::~HostThreadPosix() = default;

Status HostThreadPosix::Join(lldb::thread_result_t *result) {
  Status error;
  if (IsJoinable()) {
    int err = ::pthread_join(m_thread, result);
    error = Status(err, lldb::eErrorTypePOSIX);
  } else {
    if (result)
      *result = nullptr;
    error = Status(EINVAL, eErrorTypePOSIX);
  }

  Reset();
  return error;
}

Status HostThreadPosix::Cancel() {
  Status error;
  if (IsJoinable()) {
#ifndef __FreeBSD__
    llvm_unreachable("someone is calling HostThread::Cancel()");
#else
    int err = ::pthread_cancel(m_thread);
    error = Status(err, eErrorTypePOSIX);
#endif
  }
  return error;
}

Status HostThreadPosix::Detach() {
  Status error;
  if (IsJoinable()) {
    int err = ::pthread_detach(m_thread);
    error = Status(err, eErrorTypePOSIX);
  }
  Reset();
  return error;
}
