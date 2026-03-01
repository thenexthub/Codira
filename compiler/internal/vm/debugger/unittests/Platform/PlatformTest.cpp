//===-- PlatformTest.cpp --------------------------------------------------===//
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

#include "Plugins/Platform/POSIX/PlatformPOSIX.h"
#include "TestingSupport/SubsystemRAII.h"
#include "lldb/Core/PluginManager.h"
#include "lldb/Host/FileSystem.h"
#include "lldb/Host/HostInfo.h"
#include "lldb/Target/Platform.h"

using namespace lldb;
using namespace lldb_private;

class TestPlatform : public PlatformPOSIX {
public:
  TestPlatform() : PlatformPOSIX(false) {}
};

class PlatformArm : public TestPlatform {
public:
  PlatformArm() = default;

  std::vector<ArchSpec>
  GetSupportedArchitectures(const ArchSpec &process_host_arch) override {
    return {ArchSpec("arm64-apple-ps4")};
  }

  llvm::StringRef GetPluginName() override { return "arm"; }
  llvm::StringRef GetDescription() override { return "arm"; }
};

class PlatformIntel : public TestPlatform {
public:
  PlatformIntel() = default;

  std::vector<ArchSpec>
  GetSupportedArchitectures(const ArchSpec &process_host_arch) override {
    return {ArchSpec("x86_64-apple-ps4")};
  }

  llvm::StringRef GetPluginName() override { return "intel"; }
  llvm::StringRef GetDescription() override { return "intel"; }
};

class PlatformThumb : public TestPlatform {
public:
  static void Initialize() {
    PluginManager::RegisterPlugin("thumb", "thumb",
                                  PlatformThumb::CreateInstance);
  }
  static void Terminate() {
    PluginManager::UnregisterPlugin(PlatformThumb::CreateInstance);
  }

  static PlatformSP CreateInstance(bool force, const ArchSpec *arch) {
    return std::make_shared<PlatformThumb>();
  }

  std::vector<ArchSpec>
  GetSupportedArchitectures(const ArchSpec &process_host_arch) override {
    return {ArchSpec("thumbv7-apple-ps4"), ArchSpec("thumbv7f-apple-ps4")};
  }

  llvm::StringRef GetPluginName() override { return "thumb"; }
  llvm::StringRef GetDescription() override { return "thumb"; }
};

class PlatformTest : public ::testing::Test {
  SubsystemRAII<FileSystem, HostInfo> subsystems;

protected:
  PlatformList list;

  void SetHostPlatform(const PlatformSP &platform_sp) {
    Platform::SetHostPlatform(platform_sp);
    ASSERT_EQ(Platform::GetHostPlatform(), platform_sp);
    list.Append(platform_sp, /*set_selected=*/true);
  }
};

TEST_F(PlatformTest, GetPlatformForArchitecturesHost) {
  SetHostPlatform(std::make_shared<PlatformArm>());

  const std::vector<ArchSpec> archs = {ArchSpec("arm64-apple-ps4"),
                                       ArchSpec("arm64e-apple-ps4")};
  std::vector<PlatformSP> candidates;

  // The host platform matches all architectures.
  PlatformSP platform_sp = list.GetOrCreate(archs, {}, candidates);
  ASSERT_TRUE(platform_sp);
  EXPECT_EQ(platform_sp, Platform::GetHostPlatform());
}

TEST_F(PlatformTest, GetPlatformForArchitecturesSelected) {
  SetHostPlatform(std::make_shared<PlatformIntel>());

  const std::vector<ArchSpec> archs = {ArchSpec("arm64-apple-ps4"),
                                       ArchSpec("arm64e-apple-ps4")};
  std::vector<PlatformSP> candidates;

  // The host platform matches no architectures.
  PlatformSP platform_sp = list.GetOrCreate(archs, {}, candidates);
  ASSERT_FALSE(platform_sp);

  // The selected platform matches all architectures.
  const PlatformSP selected_platform_sp = std::make_shared<PlatformArm>();
  list.Append(selected_platform_sp, /*set_selected=*/true);
  platform_sp = list.GetOrCreate(archs, {}, candidates);
  ASSERT_TRUE(platform_sp);
  EXPECT_EQ(platform_sp, selected_platform_sp);
}

TEST_F(PlatformTest, GetPlatformForArchitecturesSelectedOverHost) {
  SetHostPlatform(std::make_shared<PlatformIntel>());

  const std::vector<ArchSpec> archs = {ArchSpec("arm64-apple-ps4"),
                                       ArchSpec("x86_64-apple-ps4")};
  std::vector<PlatformSP> candidates;

  // The host platform matches one architecture.
  PlatformSP platform_sp = list.GetOrCreate(archs, {}, candidates);
  ASSERT_TRUE(platform_sp);
  EXPECT_EQ(platform_sp, Platform::GetHostPlatform());

  // The selected and host platform each match one architecture.
  // The selected platform is preferred.
  const PlatformSP selected_platform_sp = std::make_shared<PlatformArm>();
  list.Append(selected_platform_sp, /*set_selected=*/true);
  platform_sp = list.GetOrCreate(archs, {}, candidates);
  ASSERT_TRUE(platform_sp);
  EXPECT_EQ(platform_sp, selected_platform_sp);
}

TEST_F(PlatformTest, GetPlatformForArchitecturesCandidates) {
  PlatformThumb::Initialize();

  SetHostPlatform(std::make_shared<PlatformIntel>());

  const PlatformSP selected_platform_sp = std::make_shared<PlatformArm>();
  list.Append(selected_platform_sp, /*set_selected=*/true);

  const std::vector<ArchSpec> archs = {ArchSpec("thumbv7-apple-ps4"),
                                       ArchSpec("thumbv7f-apple-ps4")};
  std::vector<PlatformSP> candidates;

  // The host platform matches one architecture.
  PlatformSP platform_sp = list.GetOrCreate(archs, {}, candidates);
  ASSERT_TRUE(platform_sp);
  EXPECT_EQ(platform_sp->GetName(), "thumb");

  PlatformThumb::Terminate();
}

TEST_F(PlatformTest, CreateUnknown) {
  SetHostPlatform(std::make_shared<PlatformIntel>());
  ASSERT_EQ(list.Create("unknown-platform-name"), nullptr);
  ASSERT_EQ(list.GetOrCreate("dummy"), nullptr);
}
