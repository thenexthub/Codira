//===-- ThreadPlanStepThrough.h ---------------------------------*- C++ -*-===//
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

#ifndef LLDB_TARGET_THREADPLANSTEPTHROUGH_H
#define LLDB_TARGET_THREADPLANSTEPTHROUGH_H

#include "lldb/Target/Thread.h"
#include "lldb/Target/ThreadPlan.h"

namespace lldb_private {

class ThreadPlanStepThrough : public ThreadPlan {
public:
  ~ThreadPlanStepThrough() override;

  void GetDescription(Stream *s, lldb::DescriptionLevel level) override;
  bool ValidatePlan(Stream *error) override;
  bool ShouldStop(Event *event_ptr) override;
  bool StopOthers() override;
  lldb::StateType GetPlanRunState() override;
  bool WillStop() override;
  bool MischiefManaged() override;
  void DidPush() override;

protected:
  bool DoPlanExplainsStop(Event *event_ptr) override;
  bool DoWillResume(lldb::StateType resume_state, bool current_plan) override;

  ThreadPlanStepThrough(Thread &thread, StackID &return_stack_id,
                        bool stop_others);

  void LookForPlanToStepThroughFromCurrentPC();

  bool HitOurBackstopBreakpoint();

private:
  friend lldb::ThreadPlanSP
  Thread::QueueThreadPlanForStepThrough(StackID &return_stack_id,
                                        bool abort_other_plans,
                                        bool stop_others, Status &status);

  void ClearBackstopBreakpoint();

  lldb::ThreadPlanSP m_sub_plan_sp;
  lldb::addr_t m_start_address;
  lldb::break_id_t m_backstop_bkpt_id;
  lldb::addr_t m_backstop_addr;
  StackID m_return_stack_id;
  bool m_stop_others;

  ThreadPlanStepThrough(const ThreadPlanStepThrough &) = delete;
  const ThreadPlanStepThrough &
  operator=(const ThreadPlanStepThrough &) = delete;
};

} // namespace lldb_private

#endif // LLDB_TARGET_THREADPLANSTEPTHROUGH_H
