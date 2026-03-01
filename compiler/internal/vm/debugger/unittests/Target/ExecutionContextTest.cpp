//===-- ExecutionContextTest.cpp ------------------------------------------===//
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

#include "lldb/Target/ExecutionContext.h"
#include "Plugins/Platform/Linux/PlatformLinux.h"
#include "lldb/Core/Debugger.h"
#include "lldb/Host/FileSystem.h"
#include "lldb/Host/HostInfo.h"
#include "lldb/Target/Platform.h"
#include "lldb/Target/Process.h"
#include "lldb/Target/Target.h"
#include "lldb/Utility/ArchSpec.h"
#include "lldb/Utility/Endian.h"
#include "lldb/lldb-enumerations.h"
#include "lldb/lldb-forward.h"
#include "lldb/lldb-private-enumerations.h"
#include "lldb/lldb-private.h"
#include "llvm/Support/FormatVariadic.h"
#include "gtest/gtest.h"

using namespace lldb_private;
using namespace lldb;

namespace {
class ExecutionContextTest : public ::testing::Test {
public:
  void SetUp() override {
    FileSystem::Initialize();
    HostInfo::Initialize();
    platform_linux::PlatformLinux::Initialize();
  }
  void TearDown() override {
    platform_linux::PlatformLinux::Terminate();
    HostInfo::Terminate();
    FileSystem::Terminate();
  }
};

class DummyProcess : public Process {
public:
  DummyProcess(lldb::TargetSP target_sp, lldb::ListenerSP listener_sp)
      : Process(target_sp, listener_sp) {}

  bool CanDebug(lldb::TargetSP target, bool plugin_specified_by_name) override {
    return true;
  }
  Status DoDestroy() override { return {}; }
  void RefreshStateAfterStop() override {}
  size_t DoReadMemory(lldb::addr_t vm_addr, void *buf, size_t size,
                      Status &error) override {
    return 0;
  }
  bool DoUpdateThreadList(ThreadList &old_thread_list,
                          ThreadList &new_thread_list) override {
    return false;
  }
  llvm::StringRef GetPluginName() override { return "Dummy"; }
};
} // namespace

TEST_F(ExecutionContextTest, GetByteOrder) {
  ExecutionContext exe_ctx(nullptr, nullptr, nullptr);
  EXPECT_EQ(endian::InlHostByteOrder(), exe_ctx.GetByteOrder());
}

TEST_F(ExecutionContextTest, GetByteOrderTarget) {
  ArchSpec arch("powerpc64-pc-linux");

  Platform::SetHostPlatform(
      platform_linux::PlatformLinux::CreateInstance(true, &arch));

  DebuggerSP debugger_sp = Debugger::CreateInstance();
  ASSERT_TRUE(debugger_sp);

  TargetSP target_sp;
  PlatformSP platform_sp;
  Status error = debugger_sp->GetTargetList().CreateTarget(
      *debugger_sp, "", arch, eLoadDependentsNo, platform_sp, target_sp);
  ASSERT_TRUE(target_sp);
  ASSERT_TRUE(target_sp->GetArchitecture().IsValid());
  ASSERT_TRUE(platform_sp);

  ExecutionContext target_ctx(target_sp, false);
  EXPECT_EQ(target_sp->GetArchitecture().GetByteOrder(),
            target_ctx.GetByteOrder());
}

TEST_F(ExecutionContextTest, GetByteOrderProcess) {
  ArchSpec arch("powerpc64-pc-linux");

  Platform::SetHostPlatform(
      platform_linux::PlatformLinux::CreateInstance(true, &arch));

  DebuggerSP debugger_sp = Debugger::CreateInstance();
  ASSERT_TRUE(debugger_sp);

  TargetSP target_sp;
  PlatformSP platform_sp;
  Status error = debugger_sp->GetTargetList().CreateTarget(
      *debugger_sp, "", arch, eLoadDependentsNo, platform_sp, target_sp);
  ASSERT_TRUE(target_sp);
  ASSERT_TRUE(target_sp->GetArchitecture().IsValid());
  ASSERT_TRUE(platform_sp);

  ListenerSP listener_sp(Listener::MakeListener("dummy"));
  ProcessSP process_sp = std::make_shared<DummyProcess>(target_sp, listener_sp);
  ASSERT_TRUE(process_sp);

  ExecutionContext process_ctx(process_sp);
  EXPECT_EQ(process_sp->GetByteOrder(), process_ctx.GetByteOrder());
}
