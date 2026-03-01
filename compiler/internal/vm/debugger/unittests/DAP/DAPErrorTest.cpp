//===-- DAPErrorTest.cpp---------------------------------------------------===//
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

#include "DAPError.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <string>
#include <system_error>

using namespace lldb_dap;
using namespace llvm;

TEST(DAPErrorTest, DefaultConstructor) {
  DAPError error("Invalid thread");

  EXPECT_EQ(error.getMessage(), "Invalid thread");
  EXPECT_EQ(error.convertToErrorCode(), llvm::inconvertibleErrorCode());
  EXPECT_TRUE(error.getShowUser());
  EXPECT_EQ(error.getURL(), std::nullopt);
  EXPECT_EQ(error.getURLLabel(), std::nullopt);
}

TEST(DAPErrorTest, FullConstructor) {
  auto timed_out = std::make_error_code(std::errc::timed_out);
  DAPError error("Timed out", timed_out, false, "URL", "URLLabel");

  EXPECT_EQ(error.getMessage(), "Timed out");
  EXPECT_EQ(error.convertToErrorCode(), timed_out);
  EXPECT_FALSE(error.getShowUser());
  EXPECT_THAT(error.getURL(), testing::Optional<std::string>("URL"));
  EXPECT_THAT(error.getURLLabel(), testing::Optional<std::string>("URLLabel"));
}
