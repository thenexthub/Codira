//===-- ThreadPlanCallUserExpression.cpp ----------------------------------===//
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

#include "lldb/Target/ThreadPlanCallUserExpression.h"

#include "lldb/Breakpoint/Breakpoint.h"
#include "lldb/Breakpoint/BreakpointLocation.h"
#include "lldb/Core/Address.h"
#include "lldb/Expression/DiagnosticManager.h"
#include "lldb/Expression/DynamicCheckerFunctions.h"
#include "lldb/Expression/UserExpression.h"
#include "lldb/Host/HostInfo.h"
#include "lldb/Target/LanguageRuntime.h"
#include "lldb/Target/Process.h"
#include "lldb/Target/RegisterContext.h"
#include "lldb/Target/StopInfo.h"
#include "lldb/Target/Target.h"
#include "lldb/Target/Thread.h"
#include "lldb/Target/ThreadPlanRunToAddress.h"
#include "lldb/Utility/LLDBLog.h"
#include "lldb/Utility/Log.h"
#include "lldb/Utility/Stream.h"

using namespace lldb;
using namespace lldb_private;

// ThreadPlanCallUserExpression: Plan to call a single function

ThreadPlanCallUserExpression::ThreadPlanCallUserExpression(
    Thread &thread, Address &function, llvm::ArrayRef<lldb::addr_t> args,
    const EvaluateExpressionOptions &options,
    lldb::UserExpressionSP &user_expression_sp)
    : ThreadPlanCallFunction(thread, function, CompilerType(), args, options),
      m_user_expression_sp(user_expression_sp) {
  // User expressions are generally "User generated" so we should set them up
  // to stop when done.
  SetIsControllingPlan(true);
  SetOkayToDiscard(false);
}

ThreadPlanCallUserExpression::~ThreadPlanCallUserExpression() = default;

void ThreadPlanCallUserExpression::GetDescription(
    Stream *s, lldb::DescriptionLevel level) {
  if (level == eDescriptionLevelBrief)
    s->Printf("User Expression thread plan");
  else
    ThreadPlanCallFunction::GetDescription(s, level);
}

void ThreadPlanCallUserExpression::DidPush() {
  ThreadPlanCallFunction::DidPush();
  if (m_user_expression_sp)
    m_user_expression_sp->WillStartExecuting();
}

void ThreadPlanCallUserExpression::DidPop() {
  ThreadPlanCallFunction::DidPop();
  if (m_user_expression_sp)
    m_user_expression_sp.reset();
}

bool ThreadPlanCallUserExpression::MischiefManaged() {
  Log *log = GetLog(LLDBLog::Step);

  if (IsPlanComplete()) {
    LLDB_LOGF(log, "ThreadPlanCallFunction(%p): Completed call function plan.",
              static_cast<void *>(this));

    if (m_manage_materialization && PlanSucceeded() && m_user_expression_sp) {
      lldb::addr_t function_stack_top;
      lldb::addr_t function_stack_bottom;
      lldb::addr_t function_stack_pointer = GetFunctionStackPointer();

      function_stack_bottom = function_stack_pointer - HostInfo::GetPageSize();
      function_stack_top = function_stack_pointer;

      DiagnosticManager diagnostics;

      ExecutionContext exe_ctx(GetThread());

      m_user_expression_sp->FinalizeJITExecution(
          diagnostics, exe_ctx, m_result_var_sp, function_stack_bottom,
          function_stack_top);
    }

    ThreadPlan::MischiefManaged();
    return true;
  } else {
    return false;
  }
}

StopInfoSP ThreadPlanCallUserExpression::GetRealStopInfo() {
  StopInfoSP stop_info_sp = ThreadPlanCallFunction::GetRealStopInfo();

  if (stop_info_sp) {
    lldb::addr_t addr = GetStopAddress();
    DynamicCheckerFunctions *checkers = m_process.GetDynamicCheckers();
    StreamString s;

    if (checkers && checkers->DoCheckersExplainStop(addr, s))
      stop_info_sp->SetDescription(s.GetData());
  }

  return stop_info_sp;
}

void ThreadPlanCallUserExpression::DoTakedown(bool success) {
  ThreadPlanCallFunction::DoTakedown(success);
  m_user_expression_sp->DidFinishExecuting();
}
