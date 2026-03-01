//===-- ZipFileResolverTest.cpp -------------------------------------------===//
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

#include "lldb/Host/common/ZipFileResolver.h"
#include "TestingSupport/SubsystemRAII.h"
#include "TestingSupport/TestUtilities.h"
#include "gtest/gtest.h"

using namespace lldb_private;
using namespace llvm;

namespace {
class ZipFileResolverTest : public ::testing::Test {
  SubsystemRAII<FileSystem> subsystems;
};

std::string TestZipPath() {
  FileSpec zip_spec(GetInputFilePath("zip-test.zip"));
  FileSystem::Instance().Resolve(zip_spec);
  return zip_spec.GetPath();
}
} // namespace

TEST_F(ZipFileResolverTest, ResolveSharedLibraryPathWithNormalFile) {
  const FileSpec file_spec("/system/lib64/libtest.so");

  ZipFileResolver::FileKind file_kind;
  std::string file_path;
  lldb::offset_t file_offset;
  lldb::offset_t file_size;
  ASSERT_TRUE(ZipFileResolver::ResolveSharedLibraryPath(
      file_spec, file_kind, file_path, file_offset, file_size));

  EXPECT_EQ(file_kind, ZipFileResolver::FileKind::eFileKindNormal);
  EXPECT_EQ(file_path, file_spec.GetPath());
  EXPECT_EQ(file_offset, 0UL);
  EXPECT_EQ(file_size, 0UL);
}

TEST_F(ZipFileResolverTest, ResolveSharedLibraryPathWithZipMissing) {
  const std::string zip_path = TestZipPath();
  const FileSpec file_spec(zip_path + "!/lib/arm64-v8a/libmissing.so");

  ZipFileResolver::FileKind file_kind;
  std::string file_path;
  lldb::offset_t file_offset;
  lldb::offset_t file_size;
  ASSERT_FALSE(ZipFileResolver::ResolveSharedLibraryPath(
      file_spec, file_kind, file_path, file_offset, file_size));
}

TEST_F(ZipFileResolverTest, ResolveSharedLibraryPathWithZipExisting) {
  const std::string zip_path = TestZipPath();
  const FileSpec file_spec(zip_path + "!/lib/arm64-v8a/libzip-test.so");

  ZipFileResolver::FileKind file_kind;
  std::string file_path;
  lldb::offset_t file_offset;
  lldb::offset_t file_size;
  ASSERT_TRUE(ZipFileResolver::ResolveSharedLibraryPath(
      file_spec, file_kind, file_path, file_offset, file_size));

  EXPECT_EQ(file_kind, ZipFileResolver::FileKind::eFileKindZip);
  EXPECT_EQ(file_path, zip_path);
  EXPECT_EQ(file_offset, 4096UL);
  EXPECT_EQ(file_size, 3600UL);
}
