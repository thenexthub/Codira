//===-- DebuggerTest.cpp --------------------------------------------------===//
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

#include "lldb/Core/Debugger.h"
#include "Plugins/Platform/MacOSX/PlatformMacOSX.h"
#include "Plugins/Platform/MacOSX/PlatformRemoteMacOSX.h"
#include "TestingSupport/TestUtilities.h"
#include "lldb/Host/FileSystem.h"
#include "lldb/Host/HostInfo.h"
#include "gtest/gtest.h"

using namespace lldb;
using namespace lldb_private;

namespace {
class DebuggerTest : public ::testing::Test {
public:
  void SetUp() override {
    FileSystem::Initialize();
    HostInfo::Initialize();
    PlatformMacOSX::Initialize();
    std::call_once(TestUtilities::g_debugger_initialize_flag,
                   []() { Debugger::Initialize(nullptr); });
    ArchSpec arch("x86_64-apple-macosx-");
    Platform::SetHostPlatform(
        PlatformRemoteMacOSX::CreateInstance(true, &arch));
  }
  void TearDown() override {
    PlatformMacOSX::Terminate();
    HostInfo::Terminate();
    FileSystem::Terminate();
  }
};
} // namespace

TEST_F(DebuggerTest, TestSettings) {
  DebuggerSP debugger_sp = Debugger::CreateInstance();

  EXPECT_TRUE(debugger_sp->SetUseColor(true));
  EXPECT_TRUE(debugger_sp->GetUseColor());

  FormatEntity::Entry format("foo");
  EXPECT_TRUE(debugger_sp->SetStatuslineFormat(format));
  EXPECT_EQ(debugger_sp->GetStatuslineFormat().string, "foo");

  Debugger::Destroy(debugger_sp);
}
