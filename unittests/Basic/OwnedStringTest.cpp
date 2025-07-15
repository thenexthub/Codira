//===--- OwnedStringTest.cpp ----------------------------------------------===//
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

#include "language/Basic/OwnedString.h"
#include "gtest/gtest.h"

using namespace language;

TEST(OwnedStringTest, char_pointer_empty) {
  const char *data = "";
  const size_t length = strlen(data);
  OwnedString ownedString = OwnedString::makeUnowned(data);

  EXPECT_EQ(length, ownedString.size());
  EXPECT_TRUE(ownedString.empty());
  EXPECT_EQ(data, ownedString.str().data());
}

TEST(OwnedStringTest, char_pointer_non_empty) {
  const char *data = "string";
  const size_t length = strlen(data);
  OwnedString ownedString = OwnedString::makeUnowned(data);

  EXPECT_EQ(length, ownedString.size());
  EXPECT_FALSE(ownedString.empty());
  EXPECT_EQ(data, ownedString.str().data());
}

TEST(OwnedStringTest, ref_counted_copies_buffer) {
  char *data = static_cast<char *>(malloc(6));
  memcpy(data, "hello", 6);
  size_t length = strlen(data);

  OwnedString ownedString =
      OwnedString::makeRefCounted(StringRef(data, length));

  EXPECT_EQ(ownedString.str(), "hello");
  EXPECT_NE(ownedString.str().data(), data);

  memcpy(data, "world", 6);

  // Even if the original buffer changes, the string should stay the same
  EXPECT_EQ(ownedString.str(), "hello");
}

TEST(OwnedStringTest, ref_counted_assignment) {
  OwnedString str = OwnedString::makeRefCounted("hello");
  OwnedString copy = str;

  EXPECT_EQ(str.str().data(), copy.str().data());
}
