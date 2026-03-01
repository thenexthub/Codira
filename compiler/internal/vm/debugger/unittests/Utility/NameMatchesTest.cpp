//===-- NameMatchesTest.cpp -----------------------------------------------===//
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

#include "lldb/Utility/NameMatches.h"
#include "gtest/gtest.h"

using namespace lldb_private;

TEST(NameMatchesTest, Ignore) {
  EXPECT_TRUE(NameMatches("foo", NameMatch::Ignore, "bar"));
}

TEST(NameMatchesTest, Equals) {
  EXPECT_TRUE(NameMatches("foo", NameMatch::Equals, "foo"));
  EXPECT_FALSE(NameMatches("foo", NameMatch::Equals, "bar"));
}

TEST(NameMatchesTest, Contains) {
  EXPECT_TRUE(NameMatches("foobar", NameMatch::Contains, "foo"));
  EXPECT_TRUE(NameMatches("foobar", NameMatch::Contains, "oob"));
  EXPECT_TRUE(NameMatches("foobar", NameMatch::Contains, "bar"));
  EXPECT_TRUE(NameMatches("foobar", NameMatch::Contains, "foobar"));
  EXPECT_TRUE(NameMatches("", NameMatch::Contains, ""));
  EXPECT_FALSE(NameMatches("", NameMatch::Contains, "foo"));
  EXPECT_FALSE(NameMatches("foobar", NameMatch::Contains, "baz"));
}

TEST(NameMatchesTest, StartsWith) {
  EXPECT_TRUE(NameMatches("foo", NameMatch::StartsWith, "f"));
  EXPECT_TRUE(NameMatches("foo", NameMatch::StartsWith, ""));
  EXPECT_TRUE(NameMatches("", NameMatch::StartsWith, ""));
  EXPECT_FALSE(NameMatches("foo", NameMatch::StartsWith, "b"));
  EXPECT_FALSE(NameMatches("", NameMatch::StartsWith, "b"));
}

TEST(NameMatchesTest, EndsWith) {
  EXPECT_TRUE(NameMatches("foo", NameMatch::EndsWith, "o"));
  EXPECT_TRUE(NameMatches("foo", NameMatch::EndsWith, ""));
  EXPECT_TRUE(NameMatches("", NameMatch::EndsWith, ""));
  EXPECT_FALSE(NameMatches("foo", NameMatch::EndsWith, "b"));
  EXPECT_FALSE(NameMatches("", NameMatch::EndsWith, "b"));
}

TEST(NameMatchesTest, RegularExpression) {
  EXPECT_TRUE(NameMatches("foobar", NameMatch::RegularExpression, "foo"));
  EXPECT_TRUE(NameMatches("foobar", NameMatch::RegularExpression, "f[oa]o"));
  EXPECT_FALSE(NameMatches("foo", NameMatch::RegularExpression, ""));
  EXPECT_FALSE(NameMatches("", NameMatch::RegularExpression, ""));
  EXPECT_FALSE(NameMatches("foo", NameMatch::RegularExpression, "b"));
  EXPECT_FALSE(NameMatches("", NameMatch::RegularExpression, "b"));
  EXPECT_FALSE(NameMatches("^a", NameMatch::RegularExpression, "^a"));
}
