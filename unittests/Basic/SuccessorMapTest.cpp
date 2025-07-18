//===--- SuccessorMapTest.cpp ---------------------------------------------===//
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

#include "language/Basic/SuccessorMap.h"
#include "gtest/gtest.h"

using namespace language;

int *const MissingValue = nullptr;

TEST(SuccessorMapTest, T1) {
  SuccessorMap<int, int> map;

  map.validate();
  EXPECT_EQ(MissingValue, map.findLeastUpperBound(27));

  map.insert(6, 17);
  map.validate();
  EXPECT_EQ(MissingValue, map.findLeastUpperBound(7));
  EXPECT_EQ(17, *map.findLeastUpperBound(5));
  map.validate();

  map.insert(7, 21);
  map.validate();
  EXPECT_EQ(MissingValue, map.findLeastUpperBound(8));
  EXPECT_EQ(17, *map.findLeastUpperBound(5));

  map.insert(5, 25);
  map.validate();
  EXPECT_EQ(MissingValue, map.findLeastUpperBound(8));
  EXPECT_EQ(25, *map.findLeastUpperBound(0));
  map.validate();

  map.insert(1, 3);
  map.validate();
  EXPECT_EQ(3, *map.findLeastUpperBound(0));
  EXPECT_EQ(25, *map.findLeastUpperBound(2));
  map.validate();

  map.insert(3, 15);
  map.validate();
  EXPECT_EQ(3, *map.findLeastUpperBound(0));
  EXPECT_EQ(15, *map.findLeastUpperBound(2));
  EXPECT_EQ(25, *map.findLeastUpperBound(4));
  map.validate();
}

TEST(SuccessorMapTest, T2) {
  SuccessorMap<int, int> map;
  map.insert(197838, 542428);
  map.validate();
  map.insert(668575, 820413);
  map.validate();
  map.insert(544719, 105904);
  map.validate();
  map.insert(605652, 154894);
  map.validate();
  map.insert(319462, 474093);
  EXPECT_EQ(154894, *map.findLeastUpperBound(600000));
  EXPECT_EQ(474093, *map.findLeastUpperBound(300000));
}

TEST(SuccessorMapTest, T3) {
  SuccessorMap<int, int> map;
  map.insert(864753, 848275);
  map.validate();
  map.insert(359283, 606460);
  map.validate();
  map.insert(31760, 149173);
  map.validate();
  map.insert(963845, 514568);
  map.validate();
  map.insert(34076, 768766);
  map.validate();
  map.insert(843809, 706405);
  map.validate();
  map.insert(307612, 981256);
  map.validate();
  EXPECT_EQ(848275, *map.findLeastUpperBound(859432));
}
