//===--- UnicodeTest.cpp --------------------------------------------------===//
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

#include "language/Basic/Unicode.h"
#include "gtest/gtest.h"

using namespace language::unicode;

TEST(ExtractExtendedGraphemeCluster, Test1) {
  EXPECT_EQ("", extractFirstExtendedGraphemeCluster(""));
  EXPECT_EQ("a", extractFirstExtendedGraphemeCluster("a"));
  EXPECT_EQ("a", extractFirstExtendedGraphemeCluster("abc"));
}

TEST(IsSingleExtendedGraphemeCluster, Test1) {
  EXPECT_EQ(false, isSingleExtendedGraphemeCluster(""));
  EXPECT_EQ(true, isSingleExtendedGraphemeCluster("a"));
}
