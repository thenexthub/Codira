//===-- TimeoutResumeAll.h -------------------------*- C++ -*-===//
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

#ifndef LLDB_TARGET_TIMEOUTRESUMEALL_H
#define LLDB_TARGET_TIMEOUTRESUMEALL_H

#include "lldb/Target/ThreadPlanSingleThreadTimeout.h"

namespace lldb_private {

// Mixin class that provides the capability for ThreadPlan to support single
// thread execution that resumes all threads after a timeout.
// Opt-in thread plan should call PushNewTimeout() in its DidPush() and
// ResumeWithTimeout() during DoWillResume().
class TimeoutResumeAll {
public:
  TimeoutResumeAll(Thread &thread)
      : m_thread(thread),
        m_timeout_info(
            std::make_shared<ThreadPlanSingleThreadTimeout::TimeoutInfo>()) {}

  void PushNewTimeout() {
    ThreadPlanSingleThreadTimeout::PushNewWithTimeout(m_thread, m_timeout_info);
  }

  void ResumeWithTimeout() {
    ThreadPlanSingleThreadTimeout::ResumeFromPrevState(m_thread,
                                                       m_timeout_info);
  }

private:
  Thread &m_thread;
  ThreadPlanSingleThreadTimeout::TimeoutInfoSP m_timeout_info;
};

} // namespace lldb_private

#endif // LLDB_TARGET_TIMEOUTRESUMEALL_H
