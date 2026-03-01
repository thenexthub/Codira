//===-- VASprintfTest.cpp -------------------------------------------------===//
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

#include "lldb/Utility/VASPrintf.h"
#include "llvm/ADT/SmallString.h"

#include "gtest/gtest.h"

#include <locale.h>

#if defined (_WIN32)
#define TEST_ENCODING ".932"  // On Windows, test codepage 932
#else
#define TEST_ENCODING "C"     // ...otherwise, any widely available uni-byte LC
#endif

using namespace lldb_private;
using namespace llvm;

static bool Sprintf(llvm::SmallVectorImpl<char> &Buffer, const char *Fmt, ...) {
  va_list args;
  va_start(args, Fmt);
  bool Result = VASprintf(Buffer, Fmt, args);
  va_end(args);
  return Result;
}

TEST(VASprintfTest, NoBufferResize) {
  std::string TestStr("small");

  llvm::SmallString<32> BigBuffer;
  ASSERT_TRUE(Sprintf(BigBuffer, "%s", TestStr.c_str()));
  EXPECT_STREQ(TestStr.c_str(), BigBuffer.c_str());
  EXPECT_EQ(TestStr.size(), BigBuffer.size());
}

TEST(VASprintfTest, BufferResize) {
  std::string TestStr("bigger");
  llvm::SmallString<4> SmallBuffer;
  ASSERT_TRUE(Sprintf(SmallBuffer, "%s", TestStr.c_str()));
  EXPECT_STREQ(TestStr.c_str(), SmallBuffer.c_str());
  EXPECT_EQ(TestStr.size(), SmallBuffer.size());
}

TEST(VASprintfTest, EncodingError) {
  // Save the current locale first.
  std::string Current(::setlocale(LC_ALL, nullptr));

  // Ensure tested locale is successfully set
  ASSERT_TRUE(setlocale(LC_ALL, TEST_ENCODING));

  wchar_t Invalid[2];
  Invalid[0] = 0x100;
  Invalid[1] = 0;
  llvm::SmallString<32> Buffer;
  EXPECT_FALSE(Sprintf(Buffer, "%ls", Invalid));
  EXPECT_EQ("<Encoding error>", Buffer);

  // Ensure we've restored the original locale once tested
  ASSERT_TRUE(setlocale(LC_ALL, Current.c_str()));
}
