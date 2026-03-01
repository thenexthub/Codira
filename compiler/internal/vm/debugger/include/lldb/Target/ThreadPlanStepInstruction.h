//===-- ThreadPlanStepInstruction.h -----------------------------*- C++ -*-===//
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

#ifndef LLDB_TARGET_THREADPLANSTEPINSTRUCTION_H
#define LLDB_TARGET_THREADPLANSTEPINSTRUCTION_H

#include "lldb/Target/Thread.h"
#include "lldb/Target/ThreadPlan.h"
#include "lldb/lldb-private.h"

namespace lldb_private {

class ThreadPlanStepInstruction : public ThreadPlan {
public:
  ThreadPlanStepInstruction(Thread &thread, bool step_over, bool stop_others,
                            Vote report_stop_vote, Vote report_run_vote);

  ~ThreadPlanStepInstruction() override;

  void GetDescription(Stream *s, lldb::DescriptionLevel level) override;
  bool ValidatePlan(Stream *error) override;
  bool ShouldStop(Event *event_ptr) override;
  bool StopOthers() override;
  lldb::StateType GetPlanRunState() override;
  bool WillStop() override;
  bool MischiefManaged() override;
  bool IsPlanStale() override;

protected:
  bool DoPlanExplainsStop(Event *event_ptr) override;

  void SetUpState();

private:
  friend lldb::ThreadPlanSP Thread::QueueThreadPlanForStepSingleInstruction(
      bool step_over, bool abort_other_plans, bool stop_other_threads,
      Status &status);

  lldb::addr_t m_instruction_addr;
  bool m_stop_other_threads;
  bool m_step_over;
  // These two are used only for the step over case.
  bool m_start_has_symbol;
  StackID m_stack_id;
  StackID m_parent_frame_id;

  ThreadPlanStepInstruction(const ThreadPlanStepInstruction &) = delete;
  const ThreadPlanStepInstruction &
  operator=(const ThreadPlanStepInstruction &) = delete;
};

} // namespace lldb_private

#endif // LLDB_TARGET_THREADPLANSTEPINSTRUCTION_H
