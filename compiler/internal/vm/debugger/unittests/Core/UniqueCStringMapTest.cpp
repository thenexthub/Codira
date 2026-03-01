//===-- UniqueCStringMapTest.cpp ------------------------------------------===//
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

#include "lldb/Core/UniqueCStringMap.h"
#include "gmock/gmock.h"

using namespace lldb_private;

namespace {
struct NoDefault {
  int x;

  NoDefault(int x) : x(x) {}
  NoDefault() = delete;

  friend bool operator==(NoDefault lhs, NoDefault rhs) {
    return lhs.x == rhs.x;
  }

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &OS,
                                       NoDefault x) {
    return OS << "NoDefault{" << x.x << "}";
  }
};
} // namespace

TEST(UniqueCStringMap, NoDefaultConstructor) {
  using MapT = UniqueCStringMap<NoDefault>;
  using EntryT = MapT::Entry;

  MapT Map;
  ConstString Foo("foo"), Bar("bar");

  Map.Append(Foo, NoDefault(42));
  EXPECT_THAT(Map.Find(Foo, NoDefault(47)), NoDefault(42));
  EXPECT_THAT(Map.Find(Bar, NoDefault(47)), NoDefault(47));
  EXPECT_THAT(Map.FindFirstValueForName(Foo),
              testing::Pointee(testing::Field(&EntryT::value, NoDefault(42))));
  EXPECT_THAT(Map.FindFirstValueForName(Bar), nullptr);

  std::vector<NoDefault> Values;
  EXPECT_THAT(Map.GetValues(Foo, Values), 1);
  EXPECT_THAT(Values, testing::ElementsAre(NoDefault(42)));

  Values.clear();
  EXPECT_THAT(Map.GetValues(Bar, Values), 0);
  EXPECT_THAT(Values, testing::IsEmpty());
}

TEST(UniqueCStringMap, ValueCompare) {
  UniqueCStringMap<int> Map;

  ConstString Foo("foo");

  Map.Append(Foo, 0);
  Map.Append(Foo, 5);
  Map.Append(Foo, -5);

  Map.Sort(std::less<int>());
  std::vector<int> Values;
  EXPECT_THAT(Map.GetValues(Foo, Values), 3);
  EXPECT_THAT(Values, testing::ElementsAre(-5, 0, 5));
}
