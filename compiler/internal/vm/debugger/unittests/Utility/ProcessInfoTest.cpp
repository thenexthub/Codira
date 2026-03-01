//===-- ProcessInfoTest.cpp -----------------------------------------------===//
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

#include "lldb/Utility/ProcessInfo.h"
#include "gtest/gtest.h"

using namespace lldb_private;

TEST(ProcessInfoTest, Constructor) {
  ProcessInfo Info("foo", ArchSpec("x86_64-pc-linux"), 47);
  EXPECT_STREQ("foo", Info.GetName());
  EXPECT_EQ(ArchSpec("x86_64-pc-linux"), Info.GetArchitecture());
  EXPECT_EQ(47u, Info.GetProcessID());
}
