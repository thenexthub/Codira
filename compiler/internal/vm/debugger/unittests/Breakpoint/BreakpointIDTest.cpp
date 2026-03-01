//===-- BreakpointIDTest.cpp ----------------------------------------------===//
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

#include "lldb/Breakpoint/BreakpointID.h"
#include "lldb/Utility/Status.h"

#include "llvm/ADT/StringRef.h"

using namespace lldb;
using namespace lldb_private;

TEST(BreakpointIDTest, StringIsBreakpointName) {
  Status E;
  EXPECT_FALSE(BreakpointID::StringIsBreakpointName("1breakpoint", E));
  EXPECT_FALSE(BreakpointID::StringIsBreakpointName("-", E));
  EXPECT_FALSE(BreakpointID::StringIsBreakpointName("", E));
  EXPECT_FALSE(BreakpointID::StringIsBreakpointName("3.4", E));

  EXPECT_TRUE(BreakpointID::StringIsBreakpointName("_", E));
  EXPECT_TRUE(BreakpointID::StringIsBreakpointName("a123", E));
  EXPECT_TRUE(BreakpointID::StringIsBreakpointName("test", E));
}
