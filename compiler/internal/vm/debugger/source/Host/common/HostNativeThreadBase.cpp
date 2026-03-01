//===-- HostNativeThreadBase.cpp ------------------------------------------===//
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

#include "lldb/Host/HostNativeThreadBase.h"
#include "lldb/Host/HostInfo.h"
#include "lldb/Host/ThreadLauncher.h"
#include "lldb/Utility/LLDBLog.h"
#include "lldb/Utility/Log.h"

#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/Threading.h"

using namespace lldb;
using namespace lldb_private;

HostNativeThreadBase::HostNativeThreadBase(thread_t thread)
    : m_thread(thread) {}

lldb::thread_t HostNativeThreadBase::GetSystemHandle() const {
  return m_thread;
}

lldb::thread_result_t HostNativeThreadBase::GetResult() const {
  return m_result;
}

bool HostNativeThreadBase::IsJoinable() const {
  return m_thread != LLDB_INVALID_HOST_THREAD;
}

void HostNativeThreadBase::Reset() {
  m_thread = LLDB_INVALID_HOST_THREAD;
  m_result = 0; // NOLINT(modernize-use-nullptr)
}

bool HostNativeThreadBase::EqualsThread(lldb::thread_t thread) const {
  return m_thread == thread;
}

lldb::thread_t HostNativeThreadBase::Release() {
  lldb::thread_t result = m_thread;
  m_thread = LLDB_INVALID_HOST_THREAD;
  m_result = 0; // NOLINT(modernize-use-nullptr)

  return result;
}

lldb::thread_result_t
HostNativeThreadBase::ThreadCreateTrampoline(lldb::thread_arg_t arg) {
  std::unique_ptr<ThreadLauncher::HostThreadCreateInfo> info_up(
      (ThreadLauncher::HostThreadCreateInfo *)arg);
  llvm::set_thread_name(info_up->thread_name);

  Log *log = GetLog(LLDBLog::Thread);
  LLDB_LOGF(log, "thread created");

  return info_up->impl();
}
