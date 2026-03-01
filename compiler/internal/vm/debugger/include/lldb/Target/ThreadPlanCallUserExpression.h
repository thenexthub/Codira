//===-- ThreadPlanCallUserExpression.h --------------------------------*- C++
//-*-===//
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

#ifndef LLDB_TARGET_THREADPLANCALLUSEREXPRESSION_H
#define LLDB_TARGET_THREADPLANCALLUSEREXPRESSION_H

#include "lldb/Target/Thread.h"
#include "lldb/Target/ThreadPlan.h"
#include "lldb/Target/ThreadPlanCallFunction.h"
#include "lldb/lldb-private.h"

#include "llvm/ADT/ArrayRef.h"

namespace lldb_private {

class ThreadPlanCallUserExpression : public ThreadPlanCallFunction {
public:
  ThreadPlanCallUserExpression(Thread &thread, Address &function,
                               llvm::ArrayRef<lldb::addr_t> args,
                               const EvaluateExpressionOptions &options,
                               lldb::UserExpressionSP &user_expression_sp);

  ~ThreadPlanCallUserExpression() override;

  void GetDescription(Stream *s, lldb::DescriptionLevel level) override;

  void DidPush() override;

  void DidPop() override;

  lldb::StopInfoSP GetRealStopInfo() override;

  bool MischiefManaged() override;

  void TransferExpressionOwnership() { m_manage_materialization = true; }

  lldb::ExpressionVariableSP GetExpressionVariable() override {
    return m_result_var_sp;
  }

protected:
  void DoTakedown(bool success) override;
private:
  lldb::UserExpressionSP
      m_user_expression_sp; // This is currently just used to ensure the
                            // User expression the initiated this ThreadPlan
                            // lives as long as the thread plan does.
  bool m_manage_materialization = false;
  lldb::ExpressionVariableSP
      m_result_var_sp; // If we are left to manage the materialization,
                       // then stuff the result expression variable here.

  ThreadPlanCallUserExpression(const ThreadPlanCallUserExpression &) = delete;
  const ThreadPlanCallUserExpression &
  operator=(const ThreadPlanCallUserExpression &) = delete;
};

} // namespace lldb_private

#endif // LLDB_TARGET_THREADPLANCALLUSEREXPRESSION_H
