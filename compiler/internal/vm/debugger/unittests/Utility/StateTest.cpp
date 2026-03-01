//===-- StateTest.cpp -----------------------------------------------------===//
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

#include "lldb/Utility/State.h"
#include "llvm/Support/FormatVariadic.h"
#include "gtest/gtest.h"

using namespace lldb;
using namespace lldb_private;

TEST(StateTest, Formatv) {
  EXPECT_EQ("invalid", llvm::formatv("{0}", eStateInvalid).str());
  EXPECT_EQ("unloaded", llvm::formatv("{0}", eStateUnloaded).str());
  EXPECT_EQ("connected", llvm::formatv("{0}", eStateConnected).str());
  EXPECT_EQ("attaching", llvm::formatv("{0}", eStateAttaching).str());
  EXPECT_EQ("launching", llvm::formatv("{0}", eStateLaunching).str());
  EXPECT_EQ("stopped", llvm::formatv("{0}", eStateStopped).str());
  EXPECT_EQ("running", llvm::formatv("{0}", eStateRunning).str());
  EXPECT_EQ("stepping", llvm::formatv("{0}", eStateStepping).str());
  EXPECT_EQ("crashed", llvm::formatv("{0}", eStateCrashed).str());
  EXPECT_EQ("detached", llvm::formatv("{0}", eStateDetached).str());
  EXPECT_EQ("exited", llvm::formatv("{0}", eStateExited).str());
  EXPECT_EQ("suspended", llvm::formatv("{0}", eStateSuspended).str());
  
}
