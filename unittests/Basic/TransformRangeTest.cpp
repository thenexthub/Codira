//===--- TransformRangeTest.cpp -------------------------------------------===//
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

#include "language/Basic/STLExtras.h"
#include "toolchain/ADT/ArrayRef.h"
#include "gtest/gtest.h"

using namespace language;

TEST(TransformRangeTest, Empty) {
  auto transform = [](int i) -> float { return float(i); };
  std::function<float (int)> f(transform);
  std::vector<int> v1;
  auto EmptyArray = makeTransformRange(toolchain::ArrayRef<int>(v1), f);
  EXPECT_EQ(EmptyArray.empty(), v1.empty());
}

TEST(TransformRangeTest, Subscript) {
  auto transform = [](int i) -> float { return float(i); };
  std::function<float (int)> f(transform);
  std::vector<int> v1;

  v1.push_back(0);
  v1.push_back(2);
  v1.push_back(3);
  v1.push_back(100);
  v1.push_back(-5);
  v1.push_back(-30);

  auto Array = makeTransformRange(toolchain::ArrayRef<int>(v1), f);

  EXPECT_EQ(Array.size(), v1.size());
  for (unsigned i = 0, e = Array.size(); i != e; ++i) {
    EXPECT_EQ(Array[i], transform(v1[i]));
  }
}

TEST(TransformRangeTest, Iteration) {
  auto transform = [](int i) -> float { return float(i); };
  std::function<float (int)> f(transform);
  std::vector<int> v1;

  v1.push_back(0);
  v1.push_back(2);
  v1.push_back(3);
  v1.push_back(100);
  v1.push_back(-5);
  v1.push_back(-30);

  auto Array = makeTransformRange(toolchain::ArrayRef<int>(v1), f);

  auto VBegin = v1.begin();
  auto VIter = v1.begin();
  auto VEnd = v1.end();
  auto TBegin = Array.begin();
  auto TIter = Array.begin();
  auto TEnd = Array.end();

  // Forwards.
  while (VIter != VEnd) {
    EXPECT_NE(TIter, TEnd);
    EXPECT_EQ(transform(*VIter), *TIter);
    ++VIter;
    ++TIter;
  }

  // Backwards.
  while (VIter != VBegin) {
    EXPECT_NE(TIter, TBegin);

    --VIter;
    --TIter;

    EXPECT_EQ(transform(*VIter), *TIter);
  }
}

TEST(TransformRangeTest, IterationWithSizelessSubscriptlessRange) {
  auto transform = [](int i) -> float { return float(i); };
  std::function<float (int)> f(transform);
  std::vector<int> v1;

  v1.push_back(0);
  v1.push_back(2);
  v1.push_back(3);
  v1.push_back(100);
  v1.push_back(-5);
  v1.push_back(-30);

  auto Array = makeTransformRange(toolchain::make_range(v1.begin(), v1.end()), f);

  auto VBegin = v1.begin();
  auto VIter = v1.begin();
  auto VEnd = v1.end();
  auto TBegin = Array.begin();
  auto TIter = Array.begin();
  auto TEnd = Array.end();

  // Forwards.
  while (VIter != VEnd) {
    EXPECT_NE(TIter, TEnd);
    EXPECT_EQ(transform(*VIter), *TIter);
    ++VIter;
    ++TIter;
  }

  // Backwards.
  while (VIter != VBegin) {
    EXPECT_NE(TIter, TBegin);

    --VIter;
    --TIter;

    EXPECT_EQ(transform(*VIter), *TIter);
  }
}
