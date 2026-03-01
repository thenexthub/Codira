//===-- ProcessLaunchInfoTest.cpp -----------------------------------------===//
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

#include "lldb/Host/ProcessLaunchInfo.h"
#include "gtest/gtest.h"

using namespace lldb_private;
using namespace lldb;

TEST(ProcessLaunchInfoTest, Constructor) {
  ProcessLaunchInfo Info(FileSpec("/stdin"), FileSpec("/stdout"),
                         FileSpec("/stderr"), FileSpec("/wd"),
                         eLaunchFlagStopAtEntry);
  EXPECT_EQ(FileSpec("/stdin"),
            Info.GetFileActionForFD(STDIN_FILENO)->GetFileSpec());
  EXPECT_EQ(FileSpec("/stdout"),
            Info.GetFileActionForFD(STDOUT_FILENO)->GetFileSpec());
  EXPECT_EQ(FileSpec("/stderr"),
            Info.GetFileActionForFD(STDERR_FILENO)->GetFileSpec());
  EXPECT_EQ(FileSpec("/wd"), Info.GetWorkingDirectory());
  EXPECT_EQ(eLaunchFlagStopAtEntry, Info.GetFlags().Get());
}
