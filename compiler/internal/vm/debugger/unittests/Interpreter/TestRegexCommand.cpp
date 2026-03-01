//===-- TestRegexCommand.cpp ----------------------------------------------===//
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

#include "Commands/CommandObjectRegexCommand.h"
#include "llvm/Testing/Support/Error.h"
#include "gtest/gtest.h"

using namespace lldb_private;
using namespace lldb;

namespace {
class TestRegexCommand : public CommandObjectRegexCommand {
public:
  using CommandObjectRegexCommand::SubstituteVariables;

  static std::string
  Substitute(llvm::StringRef input,
             const llvm::SmallVectorImpl<llvm::StringRef> &replacements) {
    llvm::Expected<std::string> str = SubstituteVariables(input, replacements);
    if (!str)
      return llvm::toString(str.takeError());
    return *str;
  }
};
} // namespace

TEST(RegexCommandTest, SubstituteVariablesSuccess) {
  const llvm::SmallVector<llvm::StringRef, 4> substitutions = {"all", "foo",
                                                               "bar", "baz"};

  EXPECT_EQ(TestRegexCommand::Substitute("%0", substitutions), "all");
  EXPECT_EQ(TestRegexCommand::Substitute("%1", substitutions), "foo");
  EXPECT_EQ(TestRegexCommand::Substitute("%2", substitutions), "bar");
  EXPECT_EQ(TestRegexCommand::Substitute("%3", substitutions), "baz");
  EXPECT_EQ(TestRegexCommand::Substitute("%1%2%3", substitutions), "foobarbaz");
  EXPECT_EQ(TestRegexCommand::Substitute("#%1#%2#%3#", substitutions),
            "#foo#bar#baz#");
}

TEST(RegexCommandTest, SubstituteVariablesFailed) {
  const llvm::SmallVector<llvm::StringRef, 4> substitutions = {"all", "foo",
                                                               "bar", "baz"};

  ASSERT_THAT_EXPECTED(
      TestRegexCommand::SubstituteVariables("%1%2%3%4", substitutions),
      llvm::Failed());
  ASSERT_THAT_EXPECTED(
      TestRegexCommand::SubstituteVariables("%5", substitutions),
      llvm::Failed());
  ASSERT_THAT_EXPECTED(
      TestRegexCommand::SubstituteVariables("%11", substitutions),
      llvm::Failed());
}

TEST(RegexCommandTest, SubstituteVariablesNoRecursion) {
  const llvm::SmallVector<llvm::StringRef, 4> substitutions = {"all", "%2",
                                                               "%3", "%4"};
  EXPECT_EQ(TestRegexCommand::Substitute("%0", substitutions), "all");
  EXPECT_EQ(TestRegexCommand::Substitute("%1", substitutions), "%2");
  EXPECT_EQ(TestRegexCommand::Substitute("%2", substitutions), "%3");
  EXPECT_EQ(TestRegexCommand::Substitute("%3", substitutions), "%4");
  EXPECT_EQ(TestRegexCommand::Substitute("%1%2%3", substitutions), "%2%3%4");
}
