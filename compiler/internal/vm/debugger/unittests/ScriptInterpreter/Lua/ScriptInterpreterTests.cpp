//===-- LuaTests.cpp ------------------------------------------------------===//
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

#include "Plugins/Platform/Linux/PlatformLinux.h"
#include "Plugins/ScriptInterpreter/Lua/ScriptInterpreterLua.h"
#include "lldb/Core/Debugger.h"
#include "lldb/Host/FileSystem.h"
#include "lldb/Host/HostInfo.h"
#include "lldb/Interpreter/CommandReturnObject.h"
#include "lldb/Target/Platform.h"
#include "gtest/gtest.h"

using namespace lldb_private;
using namespace lldb;

namespace {
class ScriptInterpreterTest : public ::testing::Test {
public:
  void SetUp() override {
    FileSystem::Initialize();
    HostInfo::Initialize();

    // Pretend Linux is the host platform.
    platform_linux::PlatformLinux::Initialize();
    ArchSpec arch("powerpc64-pc-linux");
    Platform::SetHostPlatform(
        platform_linux::PlatformLinux::CreateInstance(true, &arch));
  }
  void TearDown() override {
    platform_linux::PlatformLinux::Terminate();
    HostInfo::Terminate();
    FileSystem::Terminate();
  }
};
} // namespace

TEST_F(ScriptInterpreterTest, ExecuteOneLine) {
  DebuggerSP debugger_sp = Debugger::CreateInstance();
  ASSERT_TRUE(debugger_sp);

  ScriptInterpreterLua script_interpreter(*debugger_sp);
  CommandReturnObject result(/*colors*/ false);
  EXPECT_TRUE(script_interpreter.ExecuteOneLine("foo = 1", &result));
  EXPECT_FALSE(script_interpreter.ExecuteOneLine("nil = foo", &result));
  EXPECT_EQ(result.GetErrorString().find(
                "error: lua failed attempting to evaluate 'nil = foo'"),
            0U);
}
