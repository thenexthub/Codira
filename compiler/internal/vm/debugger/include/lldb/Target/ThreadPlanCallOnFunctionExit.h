//===-- ThreadPlanCallOnFunctionExit.h --------------------------*- C++ -*-===//
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

#ifndef LLDB_TARGET_THREADPLANCALLONFUNCTIONEXIT_H
#define LLDB_TARGET_THREADPLANCALLONFUNCTIONEXIT_H

#include "lldb/Target/ThreadPlan.h"

#include <functional>

namespace lldb_private {

// =============================================================================
/// This thread plan calls a function object when the current function exits.
// =============================================================================

class ThreadPlanCallOnFunctionExit : public ThreadPlan {
public:
  /// Definition for the callback made when the currently executing thread
  /// finishes executing its function.
  using Callback = std::function<void()>;

  ThreadPlanCallOnFunctionExit(Thread &thread, const Callback &callback);

  void DidPush() override;

  // ThreadPlan API

  void GetDescription(Stream *s, lldb::DescriptionLevel level) override;

  bool ValidatePlan(Stream *error) override;

  bool ShouldStop(Event *event_ptr) override;

  bool WillStop() override;

protected:
  bool DoPlanExplainsStop(Event *event_ptr) override;

  lldb::StateType GetPlanRunState() override;

private:
  Callback m_callback;
  lldb::ThreadPlanSP m_step_out_threadplan_sp;
};
}

#endif // LLDB_TARGET_THREADPLANCALLONFUNCTIONEXIT_H
