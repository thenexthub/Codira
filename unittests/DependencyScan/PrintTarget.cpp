//===----------------------------------------------------------------------===//
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

#include "ScanFixture.h"
#include "language-c/DependencyScan/DependencyScan.h"
#include "language/DependencyScan/StringUtils.h"
#include "language/Basic/Toolchain.h"
#include "language/Option/Options.h"
#include "toolchain/ADT/StringRef.h"
#include <vector>
#include <unordered_set>
#include "gtest/gtest.h"

using namespace language;
using namespace language::unittest;

TEST_F(ScanTest, TestTargetInfoQuery) {
  std::vector<std::string> CommandStrArr = {
    std::string("-print-target-info"),
    std::string("-target"), std::string("x86_64-apple-macosx12.0")};
  // On Windows we need to add an extra escape for path separator characters because otherwise
  // the command line tokenizer will treat them as escape characters.
  for (size_t i = 0; i < CommandStrArr.size(); ++i) {
    std::replace(CommandStrArr[i].begin(), CommandStrArr[i].end(), '\\', '/');
  }
  std::vector<const char *> Compilation;
  for (auto &str : CommandStrArr)
    Compilation.push_back(str.c_str());

  SmallString<128> pathRoot("base");
  SmallString<128> compilerPath(pathRoot);
  toolchain::sys::path::append(compilerPath, "foo", "bar", "bin", "language-frontend");
  SmallString<128> relativeLibPath(pathRoot);
  toolchain::sys::path::append(relativeLibPath, "foo", "bar", "lib", "language");;

  auto targetInfo = language::dependencies::getTargetInfo(Compilation, compilerPath.c_str());
  if (targetInfo.getError()) {
    toolchain::errs() << "For compilation: ";
    for (auto &str : Compilation)
      toolchain::errs() << str << " ";
    toolchain::errs() << "\nERROR:" << targetInfo.getError().message() << "\n";
  }

  auto targetInfoStr = std::string(language::c_string_utils::get_C_string(targetInfo.get()));
  EXPECT_NE(targetInfoStr.find("\"triple\": \"x86_64-apple-macosx12.0\""), std::string::npos);
  EXPECT_NE(targetInfoStr.find("\"librariesRequireRPath\": false"), std::string::npos);

  std::string expectedRuntimeResourcePath = "\"runtimeResourcePath\": \"" + relativeLibPath.str().str() + "\"";
  // On windows, need to normalize the path back to "\\" separators
  size_t start_pos = 0;
  while((start_pos = expectedRuntimeResourcePath.find("\\", start_pos)) != std::string::npos) {
    expectedRuntimeResourcePath.replace(start_pos, 1, "\\\\");
    start_pos += 2;
  }
  toolchain::dbgs() << "Expected Runtime Resource Path: \n" << expectedRuntimeResourcePath << "\n";
  toolchain::dbgs() << "Result Target Info: \n" << targetInfoStr << "\n";
  EXPECT_NE(targetInfoStr.find(expectedRuntimeResourcePath.c_str()), std::string::npos);
}
