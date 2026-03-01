//===-- ThreadPlanStepOverRange.h -------------------------------*- C++ -*-===//
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

#ifndef LLDB_TARGET_THREADPLANSTEPOVERRANGE_H
#define LLDB_TARGET_THREADPLANSTEPOVERRANGE_H

#include "lldb/Core/AddressRange.h"
#include "lldb/Target/StackID.h"
#include "lldb/Target/Thread.h"
#include "lldb/Target/ThreadPlanStepRange.h"
#include "lldb/Target/TimeoutResumeAll.h"

namespace lldb_private {

class ThreadPlanStepOverRange : public ThreadPlanStepRange,
                                ThreadPlanShouldStopHere,
                                TimeoutResumeAll {
public:
  ThreadPlanStepOverRange(Thread &thread, const AddressRange &range,
                          const SymbolContext &addr_context,
                          lldb::RunMode stop_others,
                          LazyBool step_out_avoids_no_debug);

  ~ThreadPlanStepOverRange() override;

  void GetDescription(Stream *s, lldb::DescriptionLevel level) override;
  void SetStopOthers(bool new_value) override;
  bool ShouldStop(Event *event_ptr) override;
  void DidPush() override;

protected:
  bool DoPlanExplainsStop(Event *event_ptr) override;
  bool DoWillResume(lldb::StateType resume_state, bool current_plan) override;

  void SetFlagsToDefault() override {
    GetFlags().Set(ThreadPlanStepOverRange::s_default_flag_values);
  }

private:
  static uint32_t s_default_flag_values;

  void SetupAvoidNoDebug(LazyBool step_out_avoids_code_without_debug_info);
  bool IsEquivalentContext(const SymbolContext &context);

  bool m_first_resume;
  lldb::RunMode m_run_mode;

  ThreadPlanStepOverRange(const ThreadPlanStepOverRange &) = delete;
  const ThreadPlanStepOverRange &
  operator=(const ThreadPlanStepOverRange &) = delete;
};

} // namespace lldb_private

#endif // LLDB_TARGET_THREADPLANSTEPOVERRANGE_H
