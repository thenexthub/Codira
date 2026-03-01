//===-- FileTest.cpp ------------------------------------------------------===//
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

#include "lldb/Host/File.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/FileUtilities.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Program.h"
#include "gtest/gtest.h"

#ifdef _WIN32
#include "lldb/Host/windows/windows.h"
#endif

using namespace lldb;
using namespace lldb_private;

TEST(File, GetWaitableHandleFileno) {
  const auto *Info = testing::UnitTest::GetInstance()->current_test_info();

  llvm::SmallString<128> name;
  int fd;
  llvm::sys::fs::createTemporaryFile(llvm::Twine(Info->test_case_name()) + "-" +
                                         Info->name(),
                                     "test", fd, name);
  llvm::FileRemover remover(name);
  ASSERT_GE(fd, 0);

  FILE *stream = fdopen(fd, "r");
  ASSERT_TRUE(stream);

  NativeFile file(stream, File::eOpenOptionReadWrite, true);
#ifdef _WIN32
  EXPECT_EQ(file.GetWaitableHandle(), (HANDLE)_get_osfhandle(fd));
#else
  EXPECT_EQ(file.GetWaitableHandle(), (file_t)fd);
#endif
}

TEST(File, GetStreamFromDescriptor) {
  const auto *Info = testing::UnitTest::GetInstance()->current_test_info();
  llvm::SmallString<128> name;
  int fd;
  llvm::sys::fs::createTemporaryFile(llvm::Twine(Info->test_case_name()) + "-" +
                                         Info->name(),
                                     "test", fd, name);

  llvm::FileRemover remover(name);
  ASSERT_GE(fd, 0);

  NativeFile file(fd, File::eOpenOptionWriteOnly, true);
  ASSERT_TRUE(file.IsValid());

  FILE *stream = file.GetStream();
  ASSERT_TRUE(stream != NULL);

  EXPECT_EQ(file.GetDescriptor(), fd);
#ifdef _WIN32
  EXPECT_EQ(file.GetWaitableHandle(), (HANDLE)_get_osfhandle(fd));
#else
  EXPECT_EQ(file.GetWaitableHandle(), (file_t)fd);
#endif
}

TEST(File, ReadOnlyModeNotWritable) {
  const auto *Info = testing::UnitTest::GetInstance()->current_test_info();
  llvm::SmallString<128> name;
  int fd;
  llvm::sys::fs::createTemporaryFile(llvm::Twine(Info->test_case_name()) + "-" +
                                         Info->name(),
                                     "test", fd, name);

  llvm::FileRemover remover(name);
  ASSERT_GE(fd, 0);

  NativeFile file(fd, File::eOpenOptionReadOnly, true);
  ASSERT_TRUE(file.IsValid());
  llvm::StringLiteral buf = "Hello World";
  size_t bytes_written = buf.size();
  Status error = file.Write(buf.data(), bytes_written);
  EXPECT_EQ(error.Fail(), true);
}
