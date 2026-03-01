//===-- ExpressionParser.cpp ----------------------------------------------===//
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

#include "lldb/Expression/ExpressionParser.h"
#include "lldb/Expression/DiagnosticManager.h"
#include "lldb/Expression/IRExecutionUnit.h"
#include "lldb/Target/ExecutionContext.h"
#include "lldb/Target/ThreadPlanCallFunction.h"

using namespace lldb;
using namespace lldb_private;

Status ExpressionParser::PrepareForExecution(
    addr_t &func_addr, addr_t &func_end,
    std::shared_ptr<IRExecutionUnit> &execution_unit_sp,
    ExecutionContext &exe_ctx, bool &can_interpret,
    ExecutionPolicy execution_policy) {
  Status status =
      DoPrepareForExecution(func_addr, func_end, execution_unit_sp, exe_ctx,
                            can_interpret, execution_policy);
  if (status.Success() && exe_ctx.GetProcessPtr() && exe_ctx.HasThreadScope())
    status = RunStaticInitializers(execution_unit_sp, exe_ctx);

  return status;
}

Status
ExpressionParser::RunStaticInitializers(IRExecutionUnitSP &execution_unit_sp,
                                        ExecutionContext &exe_ctx) {
  Status err;

  if (!execution_unit_sp.get()) {
    err = Status::FromErrorString(
        "can't run static initializers for a NULL execution unit");
    return err;
  }

  if (!exe_ctx.HasThreadScope()) {
    err = Status::FromErrorString(
        "can't run static initializers without a thread");
    return err;
  }

  std::vector<addr_t> static_initializers;

  execution_unit_sp->GetStaticInitializers(static_initializers);

  for (addr_t static_initializer : static_initializers) {
    EvaluateExpressionOptions options;

    ThreadPlanSP call_static_initializer(new ThreadPlanCallFunction(
        exe_ctx.GetThreadRef(), Address(static_initializer), CompilerType(),
        llvm::ArrayRef<addr_t>(), options));

    DiagnosticManager execution_errors;
    ExpressionResults results =
        exe_ctx.GetThreadRef().GetProcess()->RunThreadPlan(
            exe_ctx, call_static_initializer, options, execution_errors);

    if (results != eExpressionCompleted) {
      err = Status::FromError(execution_errors.GetAsError(
          lldb::eExpressionSetupError, "couldn't run static initializer:"));
      return err;
    }
  }

  return err;
}
