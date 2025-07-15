//===--- ClangImporterOptionsTest.cpp -------------------------------------===//
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
#include "language/Basic/LangOptions.h"
#include "toolchain/ADT/StringRef.h"
#include "gtest/gtest.h"

static std::string remap(toolchain::StringRef path) { return "remapped"; }

TEST(ClangImporterOptions, nonPathsSkipped) {
  std::vector<std::string> args = {"-unmapped", "-another=unmapped"};
  language::ClangImporterOptions options;
  options.ExtraArgs = args;

  EXPECT_EQ(options.getRemappedExtraArgs(remap), args);
}

TEST(ClangImporterOptions, optionPairs) {
  std::vector<std::string> args = {"-unmapped",    "-another=unmapped",
                                   "-I",           "some/path",
                                   "-ivfsoverlay", "another/path"};
  language::ClangImporterOptions options;
  options.ExtraArgs = args;

  std::vector<std::string> expected = {"-unmapped",    "-another=unmapped",
                                       "-I",           "remapped",
                                       "-ivfsoverlay", "remapped"};
  EXPECT_EQ(options.getRemappedExtraArgs(remap), expected);
}

TEST(ClangImporterOptions, joinedPaths) {
  std::vector<std::string> args = {"-unmapped", "-another=unmapped",
                                   "-Isome/path",
                                   "-working-directory=another/path"};
  language::ClangImporterOptions options;
  options.ExtraArgs = args;

  std::vector<std::string> expected = {"-unmapped", "-another=unmapped",
                                       "-Iremapped",
                                       "-working-directory=remapped"};
  EXPECT_EQ(options.getRemappedExtraArgs(remap), expected);
}
