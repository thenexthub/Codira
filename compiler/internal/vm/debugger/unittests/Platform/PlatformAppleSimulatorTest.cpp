//===-- PlatformAppleSimulatorTest.cpp ------------------------------------===//
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

#include "gtest/gtest.h"

#include "Plugins/Platform/MacOSX/PlatformAppleSimulator.h"
#include "Plugins/Platform/MacOSX/PlatformRemoteAppleTV.h"
#include "Plugins/Platform/MacOSX/PlatformRemoteAppleWatch.h"
#include "Plugins/Platform/MacOSX/PlatformRemoteiOS.h"
#include "TestingSupport/SubsystemRAII.h"
#include "lldb/Host/FileSystem.h"
#include "lldb/Host/HostInfo.h"
#include "lldb/Target/Platform.h"

using namespace lldb;
using namespace lldb_private;

class PlatformAppleSimulatorTest : public ::testing::Test {
  SubsystemRAII<FileSystem, HostInfo, PlatformAppleSimulator, PlatformRemoteiOS,
                PlatformRemoteAppleTV, PlatformRemoteAppleWatch>
      subsystems;
};

#ifdef __APPLE__

static void testSimPlatformArchHasSimEnvironment(llvm::StringRef name) {
  auto platform_sp = Platform::Create(name);
  ASSERT_TRUE(platform_sp);
  int num_arches = 0;

  for (auto arch : platform_sp->GetSupportedArchitectures({})) {
    EXPECT_EQ(arch.GetTriple().getEnvironment(), llvm::Triple::Simulator);
    num_arches++;
  }

  EXPECT_GT(num_arches, 0);
}

TEST_F(PlatformAppleSimulatorTest, TestSimHasSimEnvionament) {
  testSimPlatformArchHasSimEnvironment("ios-simulator");
  testSimPlatformArchHasSimEnvironment("tvos-simulator");
  testSimPlatformArchHasSimEnvironment("watchos-simulator");
}

TEST_F(PlatformAppleSimulatorTest, TestHostPlatformToSim) {
  static const ArchSpec platform_arch(
      HostInfo::GetArchitecture(HostInfo::eArchKindDefault));

  const llvm::Triple::OSType sim_platforms[] = {
      llvm::Triple::IOS,
      llvm::Triple::TvOS,
      llvm::Triple::WatchOS,
  };

  for (auto sim : sim_platforms) {
    PlatformList list;
    ArchSpec arch = platform_arch;
    arch.GetTriple().setOS(sim);
    arch.GetTriple().setEnvironment(llvm::Triple::Simulator);

    auto platform_sp = list.GetOrCreate(arch, {}, nullptr);
    EXPECT_TRUE(platform_sp);
  }
}

TEST_F(PlatformAppleSimulatorTest, TestPlatformSelectionOrder) {
  static const ArchSpec platform_arch(
      HostInfo::GetArchitecture(HostInfo::eArchKindDefault));

  const llvm::Triple::OSType sim_platforms[] = {
      llvm::Triple::IOS,
      llvm::Triple::TvOS,
      llvm::Triple::WatchOS,
  };

  PlatformList list;
  list.GetOrCreate("remote-ios");
  list.GetOrCreate("remote-tvos");
  list.GetOrCreate("remote-watchos");

  for (auto sim : sim_platforms) {
    ArchSpec arch = platform_arch;
    arch.GetTriple().setOS(sim);
    arch.GetTriple().setEnvironment(llvm::Triple::Simulator);

    Status error;
    auto platform_sp = list.GetOrCreate(arch, {}, nullptr, error);
    EXPECT_TRUE(platform_sp);
    EXPECT_TRUE(platform_sp->GetName().contains("simulator"));
  }
}

#endif
