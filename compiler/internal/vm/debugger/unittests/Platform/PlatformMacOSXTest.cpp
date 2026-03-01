//===-- PlatformMacOSXTest.cpp ------------------------------------===//
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

#include "Plugins/Platform/MacOSX/PlatformMacOSX.h"
#include "TestingSupport/SubsystemRAII.h"
#include "lldb/Host/FileSystem.h"
#include "lldb/Host/HostInfo.h"
#include "lldb/Target/Platform.h"

using namespace lldb;
using namespace lldb_private;

class PlatformMacOSXTest : public ::testing::Test {
  SubsystemRAII<FileSystem, HostInfo, PlatformMacOSX> subsystems;
};

#ifdef __APPLE__
static bool containsArch(const std::vector<ArchSpec> &archs,
                         const ArchSpec &arch) {
  return std::find_if(archs.begin(), archs.end(), [&](const ArchSpec &other) {
           return arch.IsExactMatch(other);
         }) != archs.end();
}

TEST_F(PlatformMacOSXTest, TestGetSupportedArchitectures) {
  PlatformMacOSX platform;

  const ArchSpec x86_macosx_arch("x86_64-apple-macosx");

  EXPECT_TRUE(containsArch(platform.GetSupportedArchitectures(x86_macosx_arch),
                           x86_macosx_arch));
  EXPECT_TRUE(
      containsArch(platform.GetSupportedArchitectures({}), x86_macosx_arch));

#if defined(__arm__) || defined(__arm64__) || defined(__aarch64__)
  const ArchSpec arm64_macosx_arch("arm64-apple-macosx");
  const ArchSpec arm64_ios_arch("arm64-apple-ios");

  EXPECT_TRUE(containsArch(
      platform.GetSupportedArchitectures(arm64_macosx_arch), arm64_ios_arch));
  EXPECT_TRUE(
      containsArch(platform.GetSupportedArchitectures({}), arm64_ios_arch));
  EXPECT_FALSE(containsArch(platform.GetSupportedArchitectures(arm64_ios_arch),
                            arm64_ios_arch));
#endif
}
#endif
