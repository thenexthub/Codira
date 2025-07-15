//===--- RangeTest.cpp ----------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#include "language/Basic/Range.h"
#include "language/Basic/OptionSet.h"
#include "gtest/gtest.h"

using namespace language;

TEST(Range, basic) {
  unsigned start = 0;
  unsigned end = 50;
  unsigned expected_i = start;
  bool sawEndMinusOne = false;
  for (unsigned i : range(start, end)) {
    EXPECT_GE(i, start);
    EXPECT_LT(i, end);
    EXPECT_EQ(expected_i, i);
    ++expected_i;

    sawEndMinusOne |= (i == (end - 1));
  }
  EXPECT_TRUE(sawEndMinusOne);
}

TEST(ReverseRange, basic) {
  unsigned start = 0;
  unsigned end = 50;
  unsigned expected_i = end;
  bool sawStartPlusOne = false;
  for (unsigned i : reverse_range(start, end)) {
    EXPECT_GT(i, start);
    EXPECT_LE(i, end);
    EXPECT_EQ(expected_i, i);
    --expected_i;

    sawStartPlusOne |= (i == start + 1);
  }
  EXPECT_TRUE(sawStartPlusOne);
}
