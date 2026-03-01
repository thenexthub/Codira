//===----------------------------------------------------------------------===//
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

#include "lldb/Utility/NonNullSharedPtr.h"
#include "gtest/gtest.h"
#include <memory>

using namespace lldb_private;

namespace {
struct TestObject {
  int value;
  TestObject() : value(0) {}
  explicit TestObject(int v) : value(v) {}
};
} // namespace

TEST(NonNullSharedPtrTest, ConstructFromValidSharedPtr) {
  auto sp = std::make_shared<TestObject>(42);
  NonNullSharedPtr<TestObject> nps(sp);

  EXPECT_EQ(nps->value, 42);
  EXPECT_EQ(nps.get(), sp.get());
  EXPECT_EQ(nps.use_count(), 2);
}

TEST(NonNullSharedPtrTest, ConstructFromRValueSharedPtr) {
  auto sp = std::make_shared<TestObject>(100);
  auto *raw_ptr = sp.get();
  NonNullSharedPtr<TestObject> nps(std::move(sp));

  EXPECT_EQ(nps->value, 100);
  EXPECT_EQ(nps.get(), raw_ptr);
  EXPECT_EQ(nps.use_count(), 1);
}

TEST(NonNullSharedPtrTest, CopyConstructor) {
  auto sp = std::make_shared<TestObject>(42);
  NonNullSharedPtr<TestObject> nps1(sp);
  NonNullSharedPtr<TestObject> nps2(nps1);

  EXPECT_EQ(nps1.get(), nps2.get());
  EXPECT_EQ(nps2->value, 42);
  EXPECT_EQ(nps1.use_count(), 3);
  EXPECT_EQ(nps2.use_count(), 3);
}

TEST(NonNullSharedPtrTest, MoveConstructor) {
  auto sp = std::make_shared<TestObject>(42);
  auto *raw_ptr = sp.get();
  NonNullSharedPtr<TestObject> nps1(sp);
  NonNullSharedPtr<TestObject> nps2(std::move(nps1));

  EXPECT_EQ(nps2.get(), raw_ptr);
  EXPECT_EQ(nps2->value, 42);
  EXPECT_EQ(nps2.use_count(), 2);
}

TEST(NonNullSharedPtrTest, CopyAssignment) {
  auto sp1 = std::make_shared<TestObject>(42);
  auto sp2 = std::make_shared<TestObject>(100);

  NonNullSharedPtr<TestObject> nps1(sp1);
  NonNullSharedPtr<TestObject> nps2(sp2);

  nps2 = nps1;

  EXPECT_EQ(nps1.get(), nps2.get());
  EXPECT_EQ(nps2->value, 42);
  EXPECT_EQ(nps1.use_count(), 3);
}

TEST(NonNullSharedPtrTest, MoveAssignment) {
  auto sp1 = std::make_shared<TestObject>(42);
  auto sp2 = std::make_shared<TestObject>(100);
  auto *raw_ptr = sp1.get();

  NonNullSharedPtr<TestObject> nps1(sp1);
  NonNullSharedPtr<TestObject> nps2(sp2);

  nps2 = std::move(nps1);

  EXPECT_EQ(nps2.get(), raw_ptr);
  EXPECT_EQ(nps2->value, 42);
  EXPECT_EQ(nps2.use_count(), 2);
}

TEST(NonNullSharedPtrTest, DereferenceOperator) {
  auto sp = std::make_shared<TestObject>(42);
  NonNullSharedPtr<TestObject> nps(sp);

  TestObject &obj = *nps;
  EXPECT_EQ(obj.value, 42);

  (*nps).value = 100;
  EXPECT_EQ(nps->value, 100);
}

TEST(NonNullSharedPtrTest, ArrowOperator) {
  auto sp = std::make_shared<TestObject>(42);
  NonNullSharedPtr<TestObject> nps(sp);

  EXPECT_EQ(nps->value, 42);

  nps->value = 200;
  EXPECT_EQ(nps->value, 200);
}

TEST(NonNullSharedPtrTest, GetMethod) {
  auto sp = std::make_shared<TestObject>(42);
  auto *raw_ptr = sp.get();
  NonNullSharedPtr<TestObject> nps(sp);

  EXPECT_EQ(nps.get(), raw_ptr);
  EXPECT_NE(nps.get(), nullptr);
}

TEST(NonNullSharedPtrTest, UseCount) {
  auto sp = std::make_shared<TestObject>(42);
  EXPECT_EQ(sp.use_count(), 1);

  NonNullSharedPtr<TestObject> nps1(sp);
  EXPECT_EQ(sp.use_count(), 2);
  EXPECT_EQ(nps1.use_count(), 2);

  {
    // Copy constructor.
    NonNullSharedPtr<TestObject> nps2(nps1);
    EXPECT_EQ(sp.use_count(), 3);
    EXPECT_EQ(nps1.use_count(), 3);
    EXPECT_EQ(nps2.use_count(), 3);
  }

  {
    // Copy assignment constructor.
    NonNullSharedPtr<TestObject> nps2 = nps1;
    EXPECT_EQ(sp.use_count(), 3);
    EXPECT_EQ(nps1.use_count(), 3);
    EXPECT_EQ(nps2.use_count(), 3);
  }

  EXPECT_EQ(sp.use_count(), 2);
  EXPECT_EQ(nps1.use_count(), 2);

  sp.reset();
  EXPECT_EQ(nps1.use_count(), 1);
  EXPECT_TRUE(nps1);
}

TEST(NonNullSharedPtrTest, BoolOperator) {
  auto sp = std::make_shared<TestObject>(42);
  NonNullSharedPtr<TestObject> nps(sp);

  EXPECT_TRUE(static_cast<bool>(nps));
  EXPECT_TRUE(nps);
}

TEST(NonNullSharedPtrTest, SwapMethod) {
  auto sp1 = std::make_shared<TestObject>(42);
  auto sp2 = std::make_shared<TestObject>(100);
  auto *raw_ptr1 = sp1.get();
  auto *raw_ptr2 = sp2.get();

  NonNullSharedPtr<TestObject> nps1(sp1);
  NonNullSharedPtr<TestObject> nps2(sp2);

  nps1.swap(nps2);

  EXPECT_EQ(nps1.get(), raw_ptr2);
  EXPECT_EQ(nps2.get(), raw_ptr1);
  EXPECT_EQ(nps1->value, 100);
  EXPECT_EQ(nps2->value, 42);
}

TEST(NonNullSharedPtrTest, ADLSwap) {
  auto sp1 = std::make_shared<TestObject>(42);
  auto sp2 = std::make_shared<TestObject>(100);
  auto *raw_ptr1 = sp1.get();
  auto *raw_ptr2 = sp2.get();

  NonNullSharedPtr<TestObject> nps1(sp1);
  NonNullSharedPtr<TestObject> nps2(sp2);

  // Use ADL swap.
  swap(nps1, nps2);

  EXPECT_EQ(nps1.get(), raw_ptr2);
  EXPECT_EQ(nps2.get(), raw_ptr1);
  EXPECT_EQ(nps1->value, 100);
  EXPECT_EQ(nps2->value, 42);
}

TEST(NonNullSharedPtrTest, MultipleReferences) {
  auto sp = std::make_shared<TestObject>(42);
  NonNullSharedPtr<TestObject> nps1(sp);
  NonNullSharedPtr<TestObject> nps2(nps1);
  NonNullSharedPtr<TestObject> nps3(nps2);

  EXPECT_EQ(nps1.get(), nps2.get());
  EXPECT_EQ(nps2.get(), nps3.get());

  nps1->value = 999;
  EXPECT_EQ(nps2->value, 999);
  EXPECT_EQ(nps3->value, 999);
}
