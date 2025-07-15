//===--- ThreadSafeRefCntPointerTest.cpp ----------------------------------===//
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

#include "language/Basic/ThreadSafeRefCounted.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "gtest/gtest.h"

using toolchain::IntrusiveRefCntPtr;

struct TestRelease : toolchain::ThreadSafeRefCountedBase<TestRelease> {
  bool &destroy;
  TestRelease(bool &destroy) : destroy(destroy) {}
  ~TestRelease() { destroy = true; }
};

TEST(ThreadSafeRefCountedBase, ReleaseSimple) {
  bool destroyed = false;
  {
    IntrusiveRefCntPtr<TestRelease> ref = new TestRelease(destroyed);
  }
  EXPECT_TRUE(destroyed);
}
TEST(ThreadSafeRefCountedBase, Release) {
  bool destroyed = false;
  {
    IntrusiveRefCntPtr<TestRelease> ref = new TestRelease(destroyed);
    ref->Retain();
    ref->Release();
  }
  EXPECT_TRUE(destroyed);
}

struct TestReleaseVPTR : language::ThreadSafeRefCountedBaseVPTR {
  bool &destroy;
  TestReleaseVPTR(bool &destroy) : destroy(destroy) {}
  ~TestReleaseVPTR() override { destroy = true; }
};

TEST(ThreadSafeRefCountedBaseVPTR, ReleaseSimple) {
  bool destroyed = false;
  {
    IntrusiveRefCntPtr<TestReleaseVPTR> ref = new TestReleaseVPTR(destroyed);
  }
  EXPECT_TRUE(destroyed);
}
TEST(ThreadSafeRefCountedBaseVPTR, Release) {
  bool destroyed = false;
  {
    IntrusiveRefCntPtr<TestReleaseVPTR> ref = new TestReleaseVPTR(destroyed);
    ref->Retain();
    ref->Release();
  }
  EXPECT_TRUE(destroyed);
}
