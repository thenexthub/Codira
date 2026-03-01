//===-- HostNativeThreadBase.h ----------------------------------*- C++ -*-===//
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

#ifndef LLDB_HOST_HOSTNATIVETHREADBASE_H
#define LLDB_HOST_HOSTNATIVETHREADBASE_H

#include "lldb/Utility/Status.h"
#include "lldb/lldb-defines.h"
#include "lldb/lldb-types.h"

namespace lldb_private {

#if defined(_WIN32)
#define THREAD_ROUTINE __stdcall
#else
#define THREAD_ROUTINE
#endif

class HostNativeThreadBase {
  friend class ThreadLauncher;
  HostNativeThreadBase(const HostNativeThreadBase &) = delete;
  const HostNativeThreadBase &operator=(const HostNativeThreadBase &) = delete;

public:
  HostNativeThreadBase() = default;
  explicit HostNativeThreadBase(lldb::thread_t thread);
  virtual ~HostNativeThreadBase() = default;

  virtual Status Join(lldb::thread_result_t *result) = 0;
  virtual Status Cancel() = 0;
  virtual bool IsJoinable() const;
  virtual void Reset();
  virtual bool EqualsThread(lldb::thread_t thread) const;
  lldb::thread_t Release();

  lldb::thread_t GetSystemHandle() const;
  lldb::thread_result_t GetResult() const;

protected:
  static lldb::thread_result_t THREAD_ROUTINE
  ThreadCreateTrampoline(lldb::thread_arg_t arg);

  lldb::thread_t m_thread = LLDB_INVALID_HOST_THREAD;
  lldb::thread_result_t m_result = 0; // NOLINT(modernize-use-nullptr)
};
}

#endif
