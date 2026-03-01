//===-- SharedClusterTest.cpp ---------------------------------------------===//
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

#include "lldb/Utility/SharedCluster.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lldb_private;

namespace {
class DestructNotifier {
public:
  DestructNotifier(std::vector<int> &Queue, int Key) : Queue(Queue), Key(Key) {}
  ~DestructNotifier() { Queue.push_back(Key); }

  std::vector<int> &Queue;
  const int Key;
};
} // namespace

TEST(SharedCluster, ClusterManager) {
  std::vector<int> Queue;
  {
    auto CM = ClusterManager<DestructNotifier>::Create();
    auto *One = new DestructNotifier(Queue, 1);
    auto *Two = new DestructNotifier(Queue, 2);
    CM->ManageObject(One);
    CM->ManageObject(Two);

    ASSERT_THAT(Queue, testing::IsEmpty());
    {
      std::shared_ptr<DestructNotifier> OnePtr = CM->GetSharedPointer(One);
      ASSERT_EQ(OnePtr->Key, 1);
      ASSERT_THAT(Queue, testing::IsEmpty());

      {
        std::shared_ptr<DestructNotifier> OnePtrCopy = OnePtr;
        ASSERT_EQ(OnePtrCopy->Key, 1);
        ASSERT_THAT(Queue, testing::IsEmpty());
      }

      {
        std::shared_ptr<DestructNotifier> TwoPtr = CM->GetSharedPointer(Two);
        ASSERT_EQ(TwoPtr->Key, 2);
        ASSERT_THAT(Queue, testing::IsEmpty());
      }

      ASSERT_THAT(Queue, testing::IsEmpty());
    }
    ASSERT_THAT(Queue, testing::IsEmpty());
  }
  ASSERT_THAT(Queue, testing::ElementsAre(1, 2));
}
