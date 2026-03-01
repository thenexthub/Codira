//===-- ArgsTest.cpp ------------------------------------------------------===//
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

#include "lldb/Interpreter/OptionValueFileColonLine.h"
#include "lldb/Utility/FileSpec.h"
#include "lldb/Utility/Status.h"
#include "gtest/gtest.h"

using namespace lldb_private;

void CheckSetting(const char *input, bool success, FileSpec path = {},
                  uint32_t line_number = LLDB_INVALID_LINE_NUMBER,
                  uint32_t column_number = LLDB_INVALID_COLUMN_NUMBER) {

  OptionValueFileColonLine value;
  Status error;
  llvm::StringRef s_ref(input);
  error = value.SetValueFromString(s_ref);
  ASSERT_EQ(error.Success(), success);

  // If we were meant to fail, we don't need to do more checks:
  if (!success)
    return;

  ASSERT_EQ(value.GetLineNumber(), line_number);
  ASSERT_EQ(value.GetColumnNumber(), column_number);
  ASSERT_EQ(value.GetFileSpec(), path);
}

TEST(OptionValueFileColonLine, setFromString) {
  OptionValueFileColonLine value;
  Status error;

  // Make sure a default constructed value is invalid:
  ASSERT_EQ(value.GetLineNumber(),
            static_cast<uint32_t>(LLDB_INVALID_LINE_NUMBER));
  ASSERT_EQ(value.GetColumnNumber(),
            static_cast<uint32_t>(LLDB_INVALID_COLUMN_NUMBER));
  ASSERT_FALSE(value.GetFileSpec());

  // Make sure it is an error to pass a specifier with no line number:
  CheckSetting("foo.c", false);

  // Now try with just a file & line:
  CheckSetting("foo.c:12", true, FileSpec("foo.c"), 12);
  CheckSetting("foo.c:12:20", true, FileSpec("foo.c"), 12, 20);
  // Make sure a colon doesn't mess us up:
  CheckSetting("foo:bar.c:12", true, FileSpec("foo:bar.c"), 12);
  CheckSetting("foo:bar.c:12:20", true, FileSpec("foo:bar.c"), 12, 20);
  // Try errors in the line number:
  CheckSetting("foo.c:12c", false);
  CheckSetting("foo.c:12:20c", false);
}
