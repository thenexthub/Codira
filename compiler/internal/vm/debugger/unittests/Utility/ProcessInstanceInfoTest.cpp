//===-- ProcessInstanceInfoTest.cpp ---------------------------------------===//
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

#include "lldb/Utility/ArchSpec.h"
#include "lldb/Utility/ProcessInfo.h"
#include "lldb/Utility/StreamString.h"
#include "lldb/Utility/UserIDResolver.h"
#include "llvm/ADT/Twine.h"
#include "gtest/gtest.h"
#include <optional>

using namespace lldb_private;

namespace {
/// A very simple resolver which fails for even ids and returns a simple string
/// for odd ones.
class DummyUserIDResolver : public UserIDResolver {
protected:
  std::optional<std::string> DoGetUserName(id_t uid) override {
    if (uid % 2)
      return ("user" + llvm::Twine(uid)).str();
    return std::nullopt;
  }

  std::optional<std::string> DoGetGroupName(id_t gid) override {
    if (gid % 2)
      return ("group" + llvm::Twine(gid)).str();
    return std::nullopt;
  }
};
} // namespace

TEST(ProcessInstanceInfo, Dump) {
  ProcessInstanceInfo info("a.out", ArchSpec("x86_64-pc-linux"), 47);
  info.SetUserID(1);
  info.SetEffectiveUserID(2);
  info.SetGroupID(3);
  info.SetEffectiveGroupID(4);

  DummyUserIDResolver resolver;
  StreamString s;
  info.Dump(s, resolver);
  EXPECT_STREQ(R"(    pid = 47
   name = a.out
   file = a.out
   arch = x86_64-pc-linux
    uid = 1     (user1)
    gid = 3     (group3)
   euid = 2     ()
   egid = 4     ()
)",
               s.GetData());
}

TEST(ProcessInstanceInfo, DumpTable) {
  ProcessInstanceInfo info("a.out", ArchSpec("x86_64-pc-linux"), 47);
  info.SetUserID(1);
  info.SetEffectiveUserID(2);
  info.SetGroupID(3);
  info.SetEffectiveGroupID(4);

  DummyUserIDResolver resolver;
  StreamString s;

  const bool show_args = false;
  const bool verbose = true;
  ProcessInstanceInfo::DumpTableHeader(s, show_args, verbose);
  info.DumpAsTableRow(s, resolver, show_args, verbose);
  EXPECT_STREQ(
      R"(PID    PARENT USER       GROUP      EFF USER   EFF GROUP  TRIPLE                         ARGUMENTS
====== ====== ========== ========== ========== ========== ============================== ============================
47     0      user1      group3     2          4          x86_64-pc-linux                
)",
      s.GetData());
}

TEST(ProcessInstanceInfo, DumpTable_invalidUID) {
  ProcessInstanceInfo info("a.out", ArchSpec("aarch64-unknown-linux-android"), 47);

  DummyUserIDResolver resolver;
  StreamString s;

  const bool show_args = false;
  const bool verbose = false;
  ProcessInstanceInfo::DumpTableHeader(s, show_args, verbose);
  info.DumpAsTableRow(s, resolver, show_args, verbose);
  EXPECT_STREQ(
      R"(PID    PARENT USER       TRIPLE                         NAME
====== ====== ========== ============================== ============================
47     0                 aarch64-unknown-linux-android  a.out
)",
      s.GetData());
}

TEST(ProcessInstanceInfoMatch, Name) {
  ProcessInstanceInfo info_bar, info_empty;
  info_bar.GetExecutableFile().SetFile("/foo/bar", FileSpec::Style::posix);

  ProcessInstanceInfoMatch match;
  match.SetNameMatchType(NameMatch::Equals);
  match.GetProcessInfo().GetExecutableFile().SetFile("bar",
                                                     FileSpec::Style::posix);

  EXPECT_TRUE(match.Matches(info_bar));
  EXPECT_FALSE(match.Matches(info_empty));

  match.GetProcessInfo().GetExecutableFile() = FileSpec();
  EXPECT_TRUE(match.Matches(info_bar));
  EXPECT_TRUE(match.Matches(info_empty));
}
