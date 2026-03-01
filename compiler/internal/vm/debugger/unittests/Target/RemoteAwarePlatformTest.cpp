//===-- RemoteAwarePlatformTest.cpp ---------------------------------------===//
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

#include "lldb/Target/RemoteAwarePlatform.h"
#include "lldb/Core/Debugger.h"
#include "lldb/Core/Module.h"
#include "lldb/Core/ModuleSpec.h"
#include "lldb/Host/FileSystem.h"
#include "lldb/Target/Platform.h"
#include "lldb/Target/Process.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lldb_private;
using namespace lldb;
using namespace testing;

class RemoteAwarePlatformTester : public RemoteAwarePlatform {
public:
  using RemoteAwarePlatform::RemoteAwarePlatform;

  MOCK_METHOD0(GetDescription, llvm::StringRef());
  MOCK_METHOD0(GetPluginName, llvm::StringRef());
  MOCK_METHOD1(GetSupportedArchitectures,
               std::vector<ArchSpec>(const ArchSpec &process_host_arch));
  MOCK_METHOD4(Attach,
               ProcessSP(ProcessAttachInfo &, Debugger &, Target *, Status &));
  MOCK_METHOD0(CalculateTrapHandlerSymbolNames, void());

  MOCK_METHOD1(ResolveExecutable,
               std::pair<bool, ModuleSP>(const ModuleSpec &));
  Status ResolveExecutable(const ModuleSpec &module_spec,
                           lldb::ModuleSP &exe_module_sp) /*override*/
  { // NOLINT(modernize-use-override)
    auto pair = ResolveExecutable(module_spec);
    exe_module_sp = pair.second;
    return pair.first ? Status() : Status::FromErrorString("error");
  }

  void SetRemotePlatform(lldb::PlatformSP platform) {
    m_remote_platform_sp = platform;
  }
};

class TargetPlatformTester : public Platform {
public:
  using Platform::Platform;

  MOCK_METHOD0(GetDescription, llvm::StringRef());
  MOCK_METHOD0(GetPluginName, llvm::StringRef());
  MOCK_METHOD1(GetSupportedArchitectures,
               std::vector<ArchSpec>(const ArchSpec &process_host_arch));
  MOCK_METHOD4(Attach,
               ProcessSP(ProcessAttachInfo &, Debugger &, Target *, Status &));
  MOCK_METHOD0(CalculateTrapHandlerSymbolNames, void());
  MOCK_METHOD0(GetUserIDResolver, UserIDResolver &());
};

namespace {
class RemoteAwarePlatformTest : public testing::Test {
public:
  static void SetUpTestCase() { FileSystem::Initialize(); }
  static void TearDownTestCase() { FileSystem::Terminate(); }
};
} // namespace

TEST_F(RemoteAwarePlatformTest, TestResolveExecutabelOnClientByPlatform) {
  ModuleSpec executable_spec;
  ModuleSP expected_executable(new Module(executable_spec));

  RemoteAwarePlatformTester platform(false);
  static const ArchSpec process_host_arch;
  EXPECT_CALL(platform, GetSupportedArchitectures(process_host_arch))
      .WillRepeatedly(Return(std::vector<ArchSpec>()));
  EXPECT_CALL(platform, ResolveExecutable(_))
      .WillRepeatedly(Return(std::make_pair(true, expected_executable)));

  platform.SetRemotePlatform(std::make_shared<TargetPlatformTester>(false));

  ModuleSP resolved_sp;
  lldb_private::Status status =
      platform.ResolveExecutable(executable_spec, resolved_sp);

  ASSERT_TRUE(status.Success());
  EXPECT_EQ(expected_executable.get(), resolved_sp.get());
}
