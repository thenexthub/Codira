//===-- ContinueTest.cpp --------------------------------------------------===//
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

#include "DAP.h"
#include "Handler/RequestHandler.h"
#include "Protocol/ProtocolRequests.h"
#include "TestBase.h"
#include "llvm/Testing/Support/Error.h"
#include "gtest/gtest.h"

using namespace llvm;
using namespace lldb;
using namespace lldb_dap;
using namespace lldb_dap_tests;
using namespace lldb_dap::protocol;

class ContinueRequestHandlerTest : public DAPTestBase {};

TEST_F(ContinueRequestHandlerTest, NotStopped) {
  SBTarget target;
  dap->debugger.SetSelectedTarget(target);

  ContinueRequestHandler handler(*dap);

  ContinueArguments args_all_threads;
  args_all_threads.singleThread = false;
  args_all_threads.threadId = 0;

  auto result_all_threads = handler.Run(args_all_threads);
  EXPECT_THAT_EXPECTED(result_all_threads,
                       llvm::FailedWithMessage("not stopped"));

  ContinueArguments args_single_thread;
  args_single_thread.singleThread = true;
  args_single_thread.threadId = 1234;

  auto result_single_thread = handler.Run(args_single_thread);
  EXPECT_THAT_EXPECTED(result_single_thread,
                       llvm::FailedWithMessage("not stopped"));
}
