//===-- FileActionTest.cpp ------------------------------------------------===//
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

#include <fcntl.h>

#include "lldb/Host/FileAction.h"
#include "gtest/gtest.h"
#if defined(_WIN32)
#include "lldb/Host/windows/PosixApi.h"
#endif

using namespace lldb_private;

TEST(FileActionTest, Open) {
  FileAction Action;
  Action.Open(47, FileSpec("/tmp"), /*read*/ true, /*write*/ false);
  EXPECT_EQ(Action.GetAction(), FileAction::eFileActionOpen);
  EXPECT_EQ(Action.GetFileSpec(), FileSpec("/tmp"));
}

TEST(FileActionTest, OpenReadWrite) {
  FileAction Action;
  Action.Open(48, FileSpec("/tmp_0"), /*read*/ true, /*write*/ true);
  EXPECT_TRUE(Action.GetActionArgument() & (O_NOCTTY | O_CREAT | O_RDWR));
  EXPECT_FALSE(Action.GetActionArgument() & O_RDONLY);
  EXPECT_FALSE(Action.GetActionArgument() & O_WRONLY);
}

TEST(FileActionTest, OpenReadOnly) {
  FileAction Action;
  Action.Open(49, FileSpec("/tmp_1"), /*read*/ true, /*write*/ false);
#ifndef _WIN32
  EXPECT_TRUE(Action.GetActionArgument() & (O_NOCTTY | O_RDONLY));
#endif
  EXPECT_FALSE(Action.GetActionArgument() & O_WRONLY);
}

TEST(FileActionTest, OpenWriteOnly) {
  FileAction Action;
  Action.Open(50, FileSpec("/tmp_2"), /*read*/ false, /*write*/ true);
  EXPECT_TRUE(Action.GetActionArgument() &
              (O_NOCTTY | O_CREAT | O_WRONLY | O_TRUNC));
  EXPECT_FALSE(Action.GetActionArgument() & O_RDONLY);
}
