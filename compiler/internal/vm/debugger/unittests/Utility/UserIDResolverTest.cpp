//===-- UserIDResolverTest.cpp --------------------------------------------===//
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

#include "lldb/Utility/UserIDResolver.h"
#include "gmock/gmock.h"
#include <optional>

using namespace lldb_private;
using namespace testing;

namespace {
class TestUserIDResolver : public UserIDResolver {
public:
  MOCK_METHOD1(DoGetUserName, std::optional<std::string>(id_t uid));
  MOCK_METHOD1(DoGetGroupName, std::optional<std::string>(id_t gid));
};
} // namespace

TEST(UserIDResolver, GetUserName) {
  StrictMock<TestUserIDResolver> r;
  llvm::StringRef user47("foo");
  EXPECT_CALL(r, DoGetUserName(47)).Times(1).WillOnce(Return(user47.str()));
  EXPECT_CALL(r, DoGetUserName(42)).Times(1).WillOnce(Return(std::nullopt));

  // Call functions twice to make sure the caching works.
  EXPECT_EQ(user47, r.GetUserName(47));
  EXPECT_EQ(user47, r.GetUserName(47));
  EXPECT_EQ(std::nullopt, r.GetUserName(42));
  EXPECT_EQ(std::nullopt, r.GetUserName(42));
}

TEST(UserIDResolver, GetGroupName) {
  StrictMock<TestUserIDResolver> r;
  llvm::StringRef group47("foo");
  EXPECT_CALL(r, DoGetGroupName(47)).Times(1).WillOnce(Return(group47.str()));
  EXPECT_CALL(r, DoGetGroupName(42)).Times(1).WillOnce(Return(std::nullopt));

  // Call functions twice to make sure the caching works.
  EXPECT_EQ(group47, r.GetGroupName(47));
  EXPECT_EQ(group47, r.GetGroupName(47));
  EXPECT_EQ(std::nullopt, r.GetGroupName(42));
  EXPECT_EQ(std::nullopt, r.GetGroupName(42));
}
