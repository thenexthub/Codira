//===----------------------------------------------------------------------===//
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
// Use the umbrella header for -Wdocumentation.
#include "lldb/API/LLDB.h"

#include "TestingSupport/SubsystemRAII.h"
#include "lldb/API/SBBreakpoint.h"
#include "lldb/API/SBBreakpointLocation.h"
#include "lldb/API/SBDebugger.h"
#include "lldb/API/SBTarget.h"
#include "gtest/gtest.h"
#include <memory>
#include <mutex>

using namespace lldb_private;
using namespace lldb;

class BreakpointClearConditionTest : public ::testing::Test {
public:
  void SetUp() override {
    m_sb_debugger = SBDebugger::Create(/*source_init_files=*/false);
  };

  void TearDown() override { SBDebugger::Destroy(m_sb_debugger); }
  SBDebugger m_sb_debugger;
  SubsystemRAII<lldb::SBDebugger> subsystems;
};

template <typename T> void test_condition(T sb_object) {
  const char *in_cond_str = "Here is a condition";
  sb_object.SetCondition(in_cond_str);
  // Make sure we set the condition correctly:
  const char *out_cond_str = sb_object.GetCondition();
  EXPECT_STREQ(in_cond_str, out_cond_str);
  // Now unset it by passing in nullptr and make sure that works:
  const char *empty_tokens[2] = {nullptr, ""};
  for (auto token : empty_tokens) {
    sb_object.SetCondition(token);
    out_cond_str = sb_object.GetCondition();
    // And make sure an unset condition returns nullptr:
    EXPECT_EQ(nullptr, out_cond_str);
  }
}

TEST_F(BreakpointClearConditionTest, BreakpointClearConditionTest) {
  // Create target
  SBTarget sb_target;
  SBError error;
  sb_target =
      m_sb_debugger.CreateTarget("", "x86_64-apple-macosx-", "remote-macosx",
                                 /*add_dependent=*/false, error);

  EXPECT_EQ(sb_target.IsValid(), true);

  // Create breakpoint
  SBBreakpoint sb_breakpoint = sb_target.BreakpointCreateByAddress(0xDEADBEEF);
  test_condition(sb_breakpoint);

  // Address breakpoints always have one location, so we can also use this
  // to test the location:
  SBBreakpointLocation sb_loc = sb_breakpoint.GetLocationAtIndex(0);
  EXPECT_EQ(sb_loc.IsValid(), true);
  test_condition(sb_loc);
}
