//===-- FindFileTest.cpp -------------------------------------------------===//
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

#include "TestingSupport/TestUtilities.h"
#include "lldb/Host/FileSystem.h"
#include "lldb/Host/HostInfo.h"
#include "lldb/Target/PathMappingList.h"
#include "lldb/Utility/FileSpec.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/FileUtilities.h"
#include "gtest/gtest.h"
#include <optional>
#include <utility>

using namespace llvm;
using namespace llvm::sys::fs;
using namespace lldb_private;

namespace {
struct Matches {
  FileSpec original;
  llvm::StringRef remapped;
  Matches(const char *o, const char *r) : original(o), remapped(r) {}
  Matches(const char *o, llvm::sys::path::Style style, const char *r)
      : original(o, style), remapped(r) {}
};

class FindFileTest : public testing::Test {
public:
  void SetUp() override {
    FileSystem::Initialize();
    HostInfo::Initialize();
  }
  void TearDown() override {
    HostInfo::Terminate();
    FileSystem::Terminate();
  }
};
} // namespace

static void TestFileFindings(const PathMappingList &map,
                             llvm::ArrayRef<Matches> matches,
                             llvm::ArrayRef<FileSpec> fails) {
  for (const auto &fail : fails) {
    SCOPED_TRACE(fail.GetPath().c_str());
    EXPECT_FALSE(map.FindFile(fail));
  }

  for (const auto &match : matches) {
    SCOPED_TRACE(match.original.GetPath() + " -> " + match.remapped);
    std::optional<FileSpec> remapped;

    EXPECT_TRUE(bool(remapped = map.FindFile(match.original)));
    EXPECT_TRUE(FileSpec(*remapped).GetPath() ==
                ConstString(match.remapped).GetStringRef());
  }
}

TEST_F(FindFileTest, FindFileTests) {
  const auto *Info = testing::UnitTest::GetInstance()->current_test_info();
  llvm::SmallString<128> DirName, FileName;
  int fd;

  ASSERT_NO_ERROR(createUniqueDirectory(Info->name(), DirName));

  sys::path::append(FileName, Twine(DirName), Twine("test"));
  ASSERT_NO_ERROR(openFile(FileName, fd, CD_CreateAlways, FA_Read, OF_None));

  llvm::FileRemover dir_remover(DirName);
  llvm::FileRemover file_remover(FileName);
  PathMappingList map;

  map.Append("/old", DirName.str(), false);
  map.Append(R"(C:\foo)", DirName.str(), false);

  Matches matches[] = {
      {"/old", llvm::sys::path::Style::posix, DirName.c_str()},
      {"/old/test", llvm::sys::path::Style::posix, FileName.c_str()},
      {R"(C:\foo)", llvm::sys::path::Style::windows, DirName.c_str()},
      {R"(C:\foo\test)", llvm::sys::path::Style::windows, FileName.c_str()}};

  std::vector<FileSpec> fails{
      // path not mapped
      FileSpec("/foo", llvm::sys::path::Style::posix),
      FileSpec("/new", llvm::sys::path::Style::posix),
      FileSpec(R"(C:\new)", llvm::sys::path::Style::windows),
      // path mapped, but file not exist
      FileSpec("/old/test1", llvm::sys::path::Style::posix),
      FileSpec(R"(C:\foo\test2)", llvm::sys::path::Style::windows)};

  TestFileFindings(map, matches, fails);
}
