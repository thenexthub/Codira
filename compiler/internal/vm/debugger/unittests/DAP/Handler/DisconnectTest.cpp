//===-- DisconnectTest.cpp ------------------------------------------------===//
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
#include "Protocol/ProtocolBase.h"
#include "TestBase.h"
#include "lldb/API/SBDefines.h"
#include "lldb/lldb-enumerations.h"
#include "llvm/Testing/Support/Error.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <memory>
#include <optional>

using namespace llvm;
using namespace lldb;
using namespace lldb_dap;
using namespace lldb_dap_tests;
using namespace lldb_dap::protocol;
using testing::_;

class DisconnectRequestHandlerTest : public DAPTestBase {};

TEST_F(DisconnectRequestHandlerTest, DisconnectTriggersTerminated) {
  DisconnectRequestHandler handler(*dap);
  ASSERT_THAT_ERROR(handler.Run(std::nullopt), Succeeded());
  EXPECT_CALL(client, Received(IsEvent("terminated", _)));
  Run();
}

// Is flaky on Linux, see https://github.com/llvm/llvm-project/issues/154763.
#ifndef __linux__
TEST_F(DisconnectRequestHandlerTest, DisconnectTriggersTerminateCommands) {
  CreateDebugger();

  if (!GetDebuggerSupportsTarget("X86"))
    GTEST_SKIP() << "Unsupported platform";

  LoadCore();

  DisconnectRequestHandler handler(*dap);

  dap->configuration.terminateCommands = {"?script print(1)",
                                          "script print(2)"};
  EXPECT_EQ(dap->target.GetProcess().GetState(), lldb::eStateStopped);
  ASSERT_THAT_ERROR(handler.Run(std::nullopt), Succeeded());
  EXPECT_CALL(client, Received(Output("1\n")));
  EXPECT_CALL(client, Received(Output("2\n"))).Times(2);
  EXPECT_CALL(client, Received(Output("(lldb) script print(2)\n")));
  EXPECT_CALL(client, Received(Output("Running terminateCommands:\n")));
  EXPECT_CALL(client, Received(IsEvent("terminated", _)));
  Run();
}
#endif
