//===--- FileSystemTest.cpp - for language/Basic/FileSystem.h ----------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#include "language/Basic/FileSystem.h"
#include "language/Basic/Toolchain.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/raw_ostream.h"
#include "toolchain/Support/VirtualFileSystem.h"
#include "gtest/gtest.h"

#include <string>

#define ASSERT_NO_ERROR(x)                                                     \
  do if (std::error_code ASSERT_NO_ERROR_ec = x) {                             \
    toolchain::errs() << #x ": did not return errc::success.\n"                     \
      << "error number: " << ASSERT_NO_ERROR_ec.value() << "\n"                \
      << "error message: " << ASSERT_NO_ERROR_ec.message() << "\n";            \
    FAIL();                                                                    \
  } while (0)

using namespace toolchain::sys;
using namespace language;

namespace {

std::string getFileContents(toolchain::StringRef path) {
  auto fs = toolchain::vfs::getRealFileSystem();
  auto file = fs->openFileForRead(path);
  if (!file)
    return "";

  auto buffer = (*file)->getBuffer("");
  if (!buffer)
    return "";

  return (*buffer)->getBuffer().str();
}

TEST(FileSystem, MoveFileIfDifferentEmpty) {
  // Create unique temporary directory for these tests
  toolchain::SmallString<128> dirPath;
  ASSERT_NO_ERROR(fs::createUniqueDirectory("FileSystem-test", dirPath));

  toolchain::SmallString<128> sourceFile = dirPath;
  path::append(sourceFile, "source.txt");

  toolchain::SmallString<128> destFile = dirPath;
  path::append(destFile, "dest.txt");

  // Test 1: Move empty over nonexistent.
  {
    {
      std::error_code error;
      toolchain::raw_fd_ostream emptyOut(sourceFile, error, fs::OF_None);
      ASSERT_NO_ERROR(error);
    }
    ASSERT_FALSE(fs::exists(destFile));

    ASSERT_NO_ERROR(moveFileIfDifferent(sourceFile, destFile));
    ASSERT_TRUE(fs::exists(destFile));
    ASSERT_FALSE(fs::exists(sourceFile));

    ASSERT_NO_ERROR(fs::remove(destFile, false));
  }

  // Test 2: Move empty over empty.
  {
    {
      std::error_code error;
      toolchain::raw_fd_ostream emptySource(sourceFile, error, fs::OF_None);
      ASSERT_NO_ERROR(error);

      toolchain::raw_fd_ostream emptyDest(destFile, error, fs::OF_None);
      ASSERT_NO_ERROR(error);
    }

    ASSERT_NO_ERROR(moveFileIfDifferent(sourceFile, destFile));
    ASSERT_TRUE(fs::exists(destFile));
    ASSERT_FALSE(fs::exists(sourceFile));

    ASSERT_NO_ERROR(fs::remove(destFile, false));
  }

  // Test 3: Move empty over non-empty.
  {
    {
      std::error_code error;
      toolchain::raw_fd_ostream emptySource(sourceFile, error, fs::OF_None);
      ASSERT_NO_ERROR(error);

      toolchain::raw_fd_ostream dest(destFile, error, fs::OF_None);
      ASSERT_NO_ERROR(error);
      dest << "a";
    }

    ASSERT_NO_ERROR(moveFileIfDifferent(sourceFile, destFile));
    ASSERT_TRUE(fs::exists(destFile));
    ASSERT_FALSE(fs::exists(sourceFile));

    ASSERT_EQ(getFileContents(destFile), "");

    ASSERT_NO_ERROR(fs::remove(destFile, false));
  }

  // Test 4: Move over self.
  {
    {
      std::error_code error;
      toolchain::raw_fd_ostream emptySource(sourceFile, error, fs::OF_None);
      ASSERT_NO_ERROR(error);
    }


    ASSERT_NO_ERROR(moveFileIfDifferent(sourceFile, sourceFile));
    ASSERT_TRUE(fs::exists(sourceFile));

    ASSERT_NO_ERROR(fs::remove(sourceFile, false));
  }

  // Clean up.
  ASSERT_NO_ERROR(fs::remove(dirPath, false));
}

TEST(FileSystem, MoveFileIfDifferentNonEmpty) {
  // Create unique temporary directory for these tests
  toolchain::SmallString<128> dirPath;
  ASSERT_NO_ERROR(fs::createUniqueDirectory("FileSystem-test", dirPath));

  toolchain::SmallString<128> sourceFile = dirPath;
  path::append(sourceFile, "source.txt");

  toolchain::SmallString<128> destFile = dirPath;
  path::append(destFile, "dest.txt");

  // Test 1: Move source over nonexistent.
  {
    {
      std::error_code error;
      toolchain::raw_fd_ostream sourceOut(sourceFile, error, fs::OF_None);
      ASSERT_NO_ERROR(error);
      sourceOut << "original";
    }
    ASSERT_FALSE(fs::exists(destFile));

    ASSERT_NO_ERROR(moveFileIfDifferent(sourceFile, destFile));
    ASSERT_TRUE(fs::exists(destFile));
    ASSERT_FALSE(fs::exists(sourceFile));

    ASSERT_EQ(getFileContents(destFile), "original");

    ASSERT_NO_ERROR(fs::remove(destFile, false));
  }

  // Test 2: Move source over empty.
  {
    {
      std::error_code error;
      toolchain::raw_fd_ostream sourceOut(sourceFile, error, fs::OF_None);
      ASSERT_NO_ERROR(error);
      sourceOut << "original";

      toolchain::raw_fd_ostream destOut(destFile, error, fs::OF_None);
      ASSERT_NO_ERROR(error);
    }

    ASSERT_NO_ERROR(moveFileIfDifferent(sourceFile, destFile));
    ASSERT_TRUE(fs::exists(destFile));
    ASSERT_FALSE(fs::exists(sourceFile));

    ASSERT_EQ(getFileContents(destFile), "original");

    ASSERT_NO_ERROR(fs::remove(destFile, false));
  }

  // Test 3: Move source over non-empty-but-different.
  {
    {
      std::error_code error;
      toolchain::raw_fd_ostream sourceOut(sourceFile, error, fs::OF_None);
      ASSERT_NO_ERROR(error);
      sourceOut << "original";

      toolchain::raw_fd_ostream destOut(destFile, error, fs::OF_None);
      ASSERT_NO_ERROR(error);
      destOut << "different";
    }

    ASSERT_NO_ERROR(moveFileIfDifferent(sourceFile, destFile));
    ASSERT_TRUE(fs::exists(destFile));
    ASSERT_FALSE(fs::exists(sourceFile));

    ASSERT_EQ(getFileContents(destFile), "original");

    ASSERT_NO_ERROR(fs::remove(destFile, false));
  }

  // Test 4: Move source over identical.
  {
    {
      std::error_code error;
      toolchain::raw_fd_ostream sourceOut(sourceFile, error, fs::OF_None);
      ASSERT_NO_ERROR(error);
      sourceOut << "original";

      toolchain::raw_fd_ostream destOut(destFile, error, fs::OF_None);
      ASSERT_NO_ERROR(error);
      destOut << "original";
    }

    ASSERT_NO_ERROR(moveFileIfDifferent(sourceFile, destFile));
    ASSERT_TRUE(fs::exists(destFile));
    ASSERT_FALSE(fs::exists(sourceFile));

    ASSERT_NO_ERROR(fs::remove(destFile, false));
  }

  // Test 5: Move source over self.
  {
    {
      std::error_code error;
      toolchain::raw_fd_ostream sourceOut(sourceFile, error, fs::OF_None);
      ASSERT_NO_ERROR(error);
      sourceOut << "original";
    }

    ASSERT_NO_ERROR(moveFileIfDifferent(sourceFile, sourceFile));
    ASSERT_TRUE(fs::exists(sourceFile));

    ASSERT_NO_ERROR(fs::remove(sourceFile, false));
  }

  // Clean up.
  ASSERT_NO_ERROR(fs::remove(dirPath, false));
}

TEST(FileSystem, MoveFileIfDifferentNonExistent) {
  // Create unique temporary directory for these tests
  toolchain::SmallString<128> dirPath;
  ASSERT_NO_ERROR(fs::createUniqueDirectory("FileSystem-test", dirPath));

  toolchain::SmallString<128> sourceFile = dirPath;
  path::append(sourceFile, "source.txt");
  toolchain::SmallString<128> destFile = dirPath;
  path::append(destFile, "dest.txt");

  // Test 1: Nonexistent -> nonexistent.
  ASSERT_TRUE((bool)moveFileIfDifferent(sourceFile, destFile));

  {
    std::error_code error;
    toolchain::raw_fd_ostream emptyOut(destFile, error, fs::OF_None);
    ASSERT_NO_ERROR(error);
  }

  // Test 2: Nonexistent -> present.
  ASSERT_TRUE((bool)moveFileIfDifferent(sourceFile, destFile));

  // Clean up.
  ASSERT_NO_ERROR(fs::remove(destFile));
  ASSERT_NO_ERROR(fs::remove(dirPath));
}

TEST(FileSystem, MoveFileIfDifferentInvalid) {
  // Create unique temporary directory for these tests
  toolchain::SmallString<128> dirPath;
  ASSERT_NO_ERROR(fs::createUniqueDirectory("FileSystem-test", dirPath));

  toolchain::SmallString<128> sourceFile = dirPath;
  path::append(sourceFile, "source.txt");
  {
    std::error_code error;
    toolchain::raw_fd_ostream emptyOut(sourceFile, error, fs::OF_None);
    ASSERT_NO_ERROR(error);
  }

  // Test 1: Move a file over its parent directory.
  ASSERT_TRUE((bool)moveFileIfDifferent(sourceFile, dirPath));

  // Test 2: Move a file into a nonexistent directory.
  toolchain::SmallString<128> destFile = dirPath;
  path::append(destFile, "nonexistent", "dest.txt");
  ASSERT_TRUE((bool)moveFileIfDifferent(sourceFile, destFile));

  // Clean up.
  ASSERT_NO_ERROR(fs::remove(sourceFile, false));
  ASSERT_NO_ERROR(fs::remove(dirPath, false));
}
} // anonymous namespace
