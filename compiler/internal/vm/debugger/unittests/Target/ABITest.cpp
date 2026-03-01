//===-- ABITest.cpp -------------------------------------------------------===//
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

#include "lldb/Target/ABI.h"
#include "gtest/gtest.h"

using namespace lldb_private;

TEST(MCBasedABI, MapRegisterName) {
  auto map = [](std::string name) {
    MCBasedABI::MapRegisterName(name, "foo", "bar");
    return name;
  };
  EXPECT_EQ("bar", map("foo"));
  EXPECT_EQ("bar0", map("foo0"));
  EXPECT_EQ("bar47", map("foo47"));
  EXPECT_EQ("foo47x", map("foo47x"));
  EXPECT_EQ("fooo47", map("fooo47"));
  EXPECT_EQ("bar47", map("bar47"));
}

