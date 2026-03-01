//===-- ThreadPlanStepUntil.h -----------------------------------*- C++ -*-===//
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

#ifndef LLDB_TARGET_THREADPLANSTEPUNTIL_H
#define LLDB_TARGET_THREADPLANSTEPUNTIL_H

#include "lldb/Target/Thread.h"
#include "lldb/Target/ThreadPlan.h"

namespace lldb_private {

class ThreadPlanStepUntil : public ThreadPlan {
public:
  ~ThreadPlanStepUntil() override;

  void GetDescription(Stream *s, lldb::DescriptionLevel level) override;
  bool ValidatePlan(Stream *error) override;
  bool ShouldStop(Event *event_ptr) override;
  bool StopOthers() override;
  lldb::StateType GetPlanRunState() override;
  bool WillStop() override;
  bool MischiefManaged() override;

protected:
  bool DoWillResume(lldb::StateType resume_state, bool current_plan) override;
  bool DoPlanExplainsStop(Event *event_ptr) override;

  ThreadPlanStepUntil(Thread &thread, lldb::addr_t *address_list,
                      size_t num_addresses, bool stop_others,
                      uint32_t frame_idx = 0);

  void AnalyzeStop();

private:
  StackID m_stack_id;
  lldb::addr_t m_step_from_insn;
  lldb::break_id_t m_return_bp_id;
  lldb::addr_t m_return_addr;
  bool m_stepped_out;
  bool m_should_stop;
  bool m_ran_analyze;
  bool m_explains_stop;

  typedef std::map<lldb::addr_t, lldb::break_id_t> until_collection;
  until_collection m_until_points;
  bool m_stop_others;

  void Clear();

  friend lldb::ThreadPlanSP Thread::QueueThreadPlanForStepUntil(
      bool abort_other_plans, lldb::addr_t *address_list, size_t num_addresses,
      bool stop_others, uint32_t frame_idx, Status &status);

  // Need an appropriate marker for the current stack so we can tell step out
  // from step in.

  ThreadPlanStepUntil(const ThreadPlanStepUntil &) = delete;
  const ThreadPlanStepUntil &operator=(const ThreadPlanStepUntil &) = delete;
};

} // namespace lldb_private

#endif // LLDB_TARGET_THREADPLANSTEPUNTIL_H
