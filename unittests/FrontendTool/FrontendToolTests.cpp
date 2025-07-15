//===--- FrontendTests.cpp ------------------------------------------------===//
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

#include "gtest/gtest.h"
#include "language/FrontendTool/FrontendTool.h"
#include "toolchain/ADT/SmallString.h"

using namespace language;

namespace {

TEST(FrontendTests, MakefileQuotingTest) {
  using namespace frontend::utils;

  toolchain::SmallString<256> buffer;
  EXPECT_EQ(escapeForMake("S:\\b\\toolchain", buffer), "S:\\b\\toolchain");
  EXPECT_EQ(escapeForMake("S:\\b\\language\\$sBoWV", buffer),
            "S:\\b\\language\\$$sBoWV");
  EXPECT_EQ(escapeForMake("S:\\b\\language\\#hashtag", buffer),
            "S:\\b\\language\\\\#hashtag");

}

} // end anonymous namespace

