//===-- ThreadPlanBase.h ----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_TARGET_THREADPLANBASE_H
#define LLDB_TARGET_THREADPLANBASE_H

#include "lldb/Target/Process.h"
#include "lldb/Target/Thread.h"
#include "lldb/Target/ThreadPlan.h"

namespace lldb_private {

//  Base thread plans:
//  This is the generic version of the bottom most plan on the plan stack.  It
//  should
//  be able to handle generic breakpoint hitting, and signals and exceptions.

class ThreadPlanBase : public ThreadPlan {
  friend class Process; // RunThreadPlan manages "stopper" base plans.
public:
  ~ThreadPlanBase() override;

  void GetDescription(Stream *s, lldb::DescriptionLevel level) override;
  bool ValidatePlan(Stream *error) override;
  bool ShouldStop(Event *event_ptr) override;
  Vote ShouldReportStop(Event *event_ptr) override;
  bool StopOthers() override;
  lldb::StateType GetPlanRunState() override;
  bool WillStop() override;
  bool MischiefManaged() override;

  bool OkayToDiscard() override { return false; }

  bool IsBasePlan() override { return true; }

  lldb::RunDirection GetDirection() const override;

protected:
  bool DoWillResume(lldb::StateType resume_state, bool current_plan) override;
  bool DoPlanExplainsStop(Event *event_ptr) override;
  ThreadPlanBase(Thread &thread);

private:
  friend lldb::ThreadPlanSP Thread::QueueBasePlan(bool abort_other_plans);

  ThreadPlanBase(const ThreadPlanBase &) = delete;
  const ThreadPlanBase &operator=(const ThreadPlanBase &) = delete;
};

} // namespace lldb_private

#endif // LLDB_TARGET_THREADPLANBASE_H
