//===-- RegularExpressionTest.cpp -----------------------------------------===//
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

#include "lldb/Utility/RegularExpression.h"
#include "llvm/ADT/SmallVector.h"
#include "gtest/gtest.h"

using namespace lldb_private;
using namespace llvm;

TEST(RegularExpression, Valid) {
  RegularExpression r1("^[0-9]+$");
  cantFail(r1.GetError());
  EXPECT_TRUE(r1.IsValid());
  EXPECT_EQ("^[0-9]+$", r1.GetText());
  EXPECT_TRUE(r1.Execute("916"));
}

TEST(RegularExpression, CopyAssignment) {
  RegularExpression r1("^[0-9]+$");
  RegularExpression r2 = r1;
  cantFail(r2.GetError());
  EXPECT_TRUE(r2.IsValid());
  EXPECT_EQ("^[0-9]+$", r2.GetText());
  EXPECT_TRUE(r2.Execute("916"));
}

TEST(RegularExpression, Empty) {
  RegularExpression r1("");
  Error err = r1.GetError();
  EXPECT_TRUE(static_cast<bool>(err));
  consumeError(std::move(err));
  EXPECT_FALSE(r1.IsValid());
  EXPECT_EQ("", r1.GetText());
  EXPECT_FALSE(r1.Execute("916"));
}

TEST(RegularExpression, Invalid) {
  RegularExpression r1("a[b-");
  Error err = r1.GetError();
  EXPECT_TRUE(static_cast<bool>(err));
  consumeError(std::move(err));
  EXPECT_FALSE(r1.IsValid());
  EXPECT_EQ("a[b-", r1.GetText());
  EXPECT_FALSE(r1.Execute("ab"));
}

TEST(RegularExpression, Match) {
  RegularExpression r1("[0-9]+([a-f])?:([0-9]+)");
  cantFail(r1.GetError());
  EXPECT_TRUE(r1.IsValid());
  EXPECT_EQ("[0-9]+([a-f])?:([0-9]+)", r1.GetText());

  SmallVector<StringRef, 3> matches;
  EXPECT_TRUE(r1.Execute("9a:513b", &matches));
  EXPECT_EQ(3u, matches.size());
  EXPECT_EQ("9a:513", matches[0].str());
  EXPECT_EQ("a", matches[1].str());
  EXPECT_EQ("513", matches[2].str());
}
