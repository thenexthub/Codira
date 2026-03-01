//===-- PredicateTest.cpp -------------------------------------------------===//
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

#include "lldb/Utility/Predicate.h"
#include "gtest/gtest.h"
#include <thread>

using namespace lldb_private;

TEST(Predicate, WaitForValueEqualTo) {
  Predicate<int> P(0);
  EXPECT_TRUE(P.WaitForValueEqualTo(0));
  EXPECT_FALSE(P.WaitForValueEqualTo(1, std::chrono::milliseconds(10)));

  std::thread Setter([&P] {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    P.SetValue(1, eBroadcastAlways);
  });
  EXPECT_TRUE(P.WaitForValueEqualTo(1));
  Setter.join();
}

TEST(Predicate, WaitForValueNotEqualTo) {
  Predicate<int> P(0);
  EXPECT_EQ(0, P.WaitForValueNotEqualTo(1));
  EXPECT_EQ(std::nullopt,
            P.WaitForValueNotEqualTo(0, std::chrono::milliseconds(10)));
}
