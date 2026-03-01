//===-- ThreadPlanStepOverBreakpoint.h --------------------------*- C++ -*-===//
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

#ifndef LLDB_TARGET_THREADPLANSTEPOVERBREAKPOINT_H
#define LLDB_TARGET_THREADPLANSTEPOVERBREAKPOINT_H

#include "lldb/Target/Thread.h"
#include "lldb/Target/ThreadPlan.h"

namespace lldb_private {

class ThreadPlanStepOverBreakpoint : public ThreadPlan {
public:
  ThreadPlanStepOverBreakpoint(Thread &thread);

  ~ThreadPlanStepOverBreakpoint() override;

  void GetDescription(Stream *s, lldb::DescriptionLevel level) override;
  bool ValidatePlan(Stream *error) override;
  bool ShouldStop(Event *event_ptr) override;
  bool SupportsResumeOthers() override;
  bool StopOthers() override;
  lldb::StateType GetPlanRunState() override;
  bool WillStop() override;
  void DidPop() override;
  bool MischiefManaged() override;
  void ThreadDestroyed() override;
  void SetAutoContinue(bool do_it);
  bool ShouldAutoContinue(Event *event_ptr) override;
  bool IsPlanStale() override;

  lldb::addr_t GetBreakpointLoadAddress() const { return m_breakpoint_addr; }

protected:
  bool DoPlanExplainsStop(Event *event_ptr) override;
  bool DoWillResume(lldb::StateType resume_state, bool current_plan) override;

  void ReenableBreakpointSite();

private:
  lldb::addr_t m_breakpoint_addr;
  lldb::user_id_t m_breakpoint_site_id;
  bool m_auto_continue;
  bool m_reenabled_breakpoint_site;

  ThreadPlanStepOverBreakpoint(const ThreadPlanStepOverBreakpoint &) = delete;
  const ThreadPlanStepOverBreakpoint &
  operator=(const ThreadPlanStepOverBreakpoint &) = delete;
};

} // namespace lldb_private

#endif // LLDB_TARGET_THREADPLANSTEPOVERBREAKPOINT_H
