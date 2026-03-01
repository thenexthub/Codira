//===-- TimeoutTest.cpp ---------------------------------------------------===//
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

#include "lldb/Utility/Timeout.h"
#include "llvm/Support/FormatVariadic.h"
#include "gtest/gtest.h"

using namespace lldb_private;
using namespace std::chrono;

TEST(TimeoutTest, Construction) {
  EXPECT_FALSE(Timeout<std::micro>(std::nullopt));
  EXPECT_TRUE(bool(Timeout<std::micro>(seconds(0))));
  EXPECT_EQ(seconds(0), *Timeout<std::micro>(seconds(0)));
  EXPECT_EQ(seconds(3), *Timeout<std::micro>(seconds(3)));
  EXPECT_TRUE(bool(Timeout<std::micro>(Timeout<std::milli>(seconds(0)))));
}

TEST(TimeoutTest, Format) {
  EXPECT_EQ("<infinite>",
            llvm::formatv("{0}", Timeout<std::milli>(std::nullopt)).str());
  EXPECT_EQ("1000 ms",
            llvm::formatv("{0}", Timeout<std::milli>(seconds(1))).str());
}
