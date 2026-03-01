//===-- SourceManagerTest.cpp ---------------------------------------------===//
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

#include "lldb/Core/SourceManager.h"
#include "lldb/Host/FileSystem.h"
#include "lldb/Utility/SupportFile.h"
#include "gtest/gtest.h"

#include "TestingSupport/MockTildeExpressionResolver.h"

using namespace lldb;
using namespace lldb_private;

class SourceFileCache : public ::testing::Test {
public:
  void SetUp() override {
    FileSystem::Initialize(std::unique_ptr<TildeExpressionResolver>(
        new MockTildeExpressionResolver("Jonas", "/jonas")));
  }
  void TearDown() override { FileSystem::Terminate(); }
};

TEST_F(SourceFileCache, FindSourceFileFound) {
  SourceManager::SourceFileCache cache;

  // Insert: foo
  FileSpec foo_file_spec("foo");
  auto foo_file_sp = std::make_shared<SourceManager::File>(
      std::make_shared<SupportFile>(foo_file_spec), lldb::DebuggerSP());
  cache.AddSourceFile(foo_file_spec, foo_file_sp);

  // Query: foo, expect found.
  FileSpec another_foo_file_spec("foo");
  ASSERT_EQ(cache.FindSourceFile(another_foo_file_spec), foo_file_sp);
}

TEST_F(SourceFileCache, FindSourceFileNotFound) {
  SourceManager::SourceFileCache cache;

  // Insert: foo
  FileSpec foo_file_spec("foo");
  auto foo_file_sp = std::make_shared<SourceManager::File>(
      std::make_shared<SupportFile>(foo_file_spec), lldb::DebuggerSP());
  cache.AddSourceFile(foo_file_spec, foo_file_sp);

  // Query: bar, expect not found.
  FileSpec bar_file_spec("bar");
  ASSERT_EQ(cache.FindSourceFile(bar_file_spec), nullptr);
}

TEST_F(SourceFileCache, FindSourceFileByUnresolvedPath) {
  SourceManager::SourceFileCache cache;

  FileSpec foo_file_spec("~/foo");

  // Mimic the resolution in SourceManager::GetFile.
  FileSpec resolved_foo_file_spec = foo_file_spec;
  FileSystem::Instance().Resolve(resolved_foo_file_spec);

  // Create the file with the resolved file spec.
  auto foo_file_sp = std::make_shared<SourceManager::File>(
      std::make_shared<SupportFile>(resolved_foo_file_spec),
      lldb::DebuggerSP());

  // Cache the result with the unresolved file spec.
  cache.AddSourceFile(foo_file_spec, foo_file_sp);

  // Query the unresolved path.
  EXPECT_EQ(cache.FindSourceFile(FileSpec("~/foo")), foo_file_sp);

  // Query the resolved path.
  EXPECT_EQ(cache.FindSourceFile(FileSpec("/jonas/foo")), foo_file_sp);
}
