//===--- SourceManagerTest.cpp --------------------------------------------===//
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

#include "language/Basic/SourceManager.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "gtest/gtest.h"
#include <vector>

using namespace language;
using namespace toolchain;

static std::vector<SourceLoc> tokenize(SourceManager &SM, StringRef Source) {
  unsigned ID = SM.addMemBufferCopy(Source);
  const MemoryBuffer *Buf = SM.getLLVMSourceMgr().getMemoryBuffer(ID);

  SourceLoc BeginLoc(SMLoc::getFromPointer(Buf->getBuffer().begin()));
  std::vector<SourceLoc> Result;
  Result.push_back(BeginLoc);
  for (unsigned i = 1, e = Source.size(); i != e; ++i) {
    if (Source[i - 1] == ' ')
      Result.push_back(BeginLoc.getAdvancedLoc(i));
  }
  return Result;
}

TEST(SourceManager, IsBeforeInBuffer) {
  SourceManager SM;
  auto Locs = tokenize(SM, "aaa bbb ccc ddd");

  EXPECT_TRUE(SM.isBeforeInBuffer(Locs[0], Locs[1]));
  EXPECT_TRUE(SM.isBeforeInBuffer(Locs[1], Locs[2]));
  EXPECT_TRUE(SM.isBeforeInBuffer(Locs[2], Locs[3]));
  EXPECT_TRUE(SM.isBeforeInBuffer(Locs[0], Locs[3]));

  EXPECT_TRUE(SM.isBeforeInBuffer(Locs[0], Locs[0].getAdvancedLoc(1)));
  EXPECT_TRUE(SM.isBeforeInBuffer(Locs[0].getAdvancedLoc(1), Locs[1]));
}

TEST(SourceManager, RangeContainsTokenLoc) {
  SourceManager SM;
  auto Locs = tokenize(SM, "aaa bbb ccc ddd");

  SourceRange R_aa(Locs[0], Locs[0]);
  SourceRange R_ab(Locs[0], Locs[1]);
  SourceRange R_ac(Locs[0], Locs[2]);

  SourceRange R_bc(Locs[1], Locs[2]);

  EXPECT_TRUE(SM.rangeContainsTokenLoc(R_aa, Locs[0]));
  EXPECT_FALSE(SM.rangeContainsTokenLoc(R_aa, Locs[0].getAdvancedLoc(1)));

  EXPECT_TRUE(SM.rangeContainsTokenLoc(R_ab, Locs[0]));
  EXPECT_TRUE(SM.rangeContainsTokenLoc(R_ab, Locs[0].getAdvancedLoc(1)));
  EXPECT_TRUE(SM.rangeContainsTokenLoc(R_ab, Locs[1]));
  EXPECT_FALSE(SM.rangeContainsTokenLoc(R_ab, Locs[1].getAdvancedLoc(1)));

  EXPECT_TRUE(SM.rangeContainsTokenLoc(R_ac, Locs[0]));
  EXPECT_TRUE(SM.rangeContainsTokenLoc(R_ac, Locs[0].getAdvancedLoc(1)));
  EXPECT_TRUE(SM.rangeContainsTokenLoc(R_ac, Locs[1]));
  EXPECT_TRUE(SM.rangeContainsTokenLoc(R_ac, Locs[1].getAdvancedLoc(1)));
  EXPECT_TRUE(SM.rangeContainsTokenLoc(R_ac, Locs[2]));
  EXPECT_FALSE(SM.rangeContainsTokenLoc(R_ac, Locs[2].getAdvancedLoc(1)));

  EXPECT_FALSE(SM.rangeContainsTokenLoc(R_bc, Locs[0]));
  EXPECT_FALSE(SM.rangeContainsTokenLoc(R_bc, Locs[0].getAdvancedLoc(1)));
}

TEST(SourceManager, RangeContains) {
  SourceManager SM;
  auto Locs = tokenize(SM, "aaa bbb ccc ddd");

  SourceRange R_aa(Locs[0], Locs[0]);
  SourceRange R_ab(Locs[0], Locs[1]);
  SourceRange R_ac(Locs[0], Locs[2]);
  SourceRange R_ad(Locs[0], Locs[3]);

  SourceRange R_bc(Locs[1], Locs[2]);

  EXPECT_TRUE(SM.rangeContains(R_ab, R_aa));
  EXPECT_TRUE(SM.rangeContains(R_ac, R_aa));

  EXPECT_TRUE(SM.rangeContains(R_ac, R_ab));
  EXPECT_TRUE(SM.rangeContains(R_ad, R_ab));

  EXPECT_TRUE(SM.rangeContains(R_ad, R_ac));

  EXPECT_TRUE(SM.rangeContains(R_ac, R_bc));
  EXPECT_TRUE(SM.rangeContains(R_ad, R_bc));
}

