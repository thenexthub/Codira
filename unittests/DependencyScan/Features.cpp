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
#include "language/Basic/Toolchain.h"
#include "language/Option/Options.h"
#include "toolchain/ADT/StringRef.h"
#include <vector>
#include <unordered_set>
#include "gtest/gtest.h"

using namespace language;
using namespace language::unittest;

static void
testHasOption(toolchain::opt::OptTable &table, options::ID id,
              const std::unordered_set<std::string> &optionSet) {
  if (table.getOption(id).hasFlag(language::options::FrontendOption)) {
    auto name = table.getOptionName(id);
    if (!name.empty() && name[0] != '\0') {
      auto nameStr = std::string(name);
      bool setContainsOption = optionSet.find(nameStr) != optionSet.end();
      EXPECT_EQ(setContainsOption, true) << "Missing Option: " << nameStr;
    }
  }
}

TEST_F(ScanTest, TestHasArgumentQuery) {
  std::unique_ptr<toolchain::opt::OptTable> table = language::createCodiraOptTable();
  auto supported_args_set = languagescan_compiler_supported_arguments_query();
  std::unordered_set<std::string> optionSet;
  for (size_t i = 0; i < supported_args_set->count; ++i) {
    languagescan_string_ref_t option = supported_args_set->strings[i];
    const char* data = static_cast<const char*>(option.data);
    optionSet.insert(std::string(data, option.length));
  }
  languagescan_string_set_dispose(supported_args_set);
#define OPTION(...)                                                            \
  testHasOption(*table, language::options::TOOLCHAIN_MAKE_OPT_ID(__VA_ARGS__),         \
                optionSet);
#include "language/Option/Options.inc"
#undef OPTION
}

TEST_F(ScanTest, TestDoesNotHaveArgumentQuery) {
  auto supported_args_set = languagescan_compiler_supported_arguments_query();
  std::unordered_set<std::string> optionSet;
  for (size_t i = 0; i < supported_args_set->count; ++i) {
    languagescan_string_ref_t option = supported_args_set->strings[i];
    const char* data = static_cast<const char*>(option.data);
    optionSet.insert(std::string(data, option.length));
  }
  languagescan_string_set_dispose(supported_args_set);
  bool hasOption;
  hasOption = optionSet.find("-clearly-not-a-compiler-flag") != optionSet.end();
  EXPECT_EQ(hasOption, false);
  hasOption = optionSet.find("-emit-modul") != optionSet.end();
  EXPECT_EQ(hasOption, false);
}
