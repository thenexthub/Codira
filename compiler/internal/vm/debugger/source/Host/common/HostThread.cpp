//===-- HostThread.cpp ----------------------------------------------------===//
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

#include "lldb/Host/HostThread.h"
#include "lldb/Host/HostNativeThread.h"

using namespace lldb;
using namespace lldb_private;

HostThread::HostThread() : m_native_thread(new HostNativeThread) {}

HostThread::HostThread(lldb::thread_t thread)
    : m_native_thread(new HostNativeThread(thread)) {}

Status HostThread::Join(lldb::thread_result_t *result) {
  return m_native_thread->Join(result);
}

Status HostThread::Cancel() { return m_native_thread->Cancel(); }

void HostThread::Reset() { return m_native_thread->Reset(); }

lldb::thread_t HostThread::Release() { return m_native_thread->Release(); }

bool HostThread::IsJoinable() const { return m_native_thread->IsJoinable(); }

HostNativeThread &HostThread::GetNativeThread() {
  return static_cast<HostNativeThread &>(*m_native_thread);
}

const HostNativeThread &HostThread::GetNativeThread() const {
  return static_cast<const HostNativeThread &>(*m_native_thread);
}

lldb::thread_result_t HostThread::GetResult() const {
  return m_native_thread->GetResult();
}

bool HostThread::EqualsThread(lldb::thread_t thread) const {
  return m_native_thread->EqualsThread(thread);
}

bool HostThread::HasThread() const {
  if (!m_native_thread)
    return false;
  return m_native_thread->GetSystemHandle() != LLDB_INVALID_HOST_THREAD;
}
