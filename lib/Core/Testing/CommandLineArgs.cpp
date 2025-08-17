//===--- CommandLineArgs.cpp ----------------------------------------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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

#include "language/Core/Testing/CommandLineArgs.h"
#include "toolchain/MC/TargetRegistry.h"
#include "toolchain/Support/ErrorHandling.h"

namespace language::Core {
std::vector<TestLanguage> getCOrLater(const int MinimumStd) {
  std::vector<TestLanguage> Result{};

#define TESTLANGUAGE_C(lang, version, std_flag, version_index)                 \
  if (version >= MinimumStd)                                                   \
    Result.push_back(Lang_##lang##version);
#include "language/Core/Testing/TestLanguage.def"

  return Result;
}
std::vector<TestLanguage> getCXXOrLater(const int MinimumStd) {
  std::vector<TestLanguage> Result{};

#define TESTLANGUAGE_CXX(lang, version, std_flag, version_index)               \
  if (version >= MinimumStd)                                                   \
    Result.push_back(Lang_##lang##version);
#include "language/Core/Testing/TestLanguage.def"

  return Result;
}

std::vector<std::string> getCommandLineArgsForTesting(TestLanguage Lang) {
  // Test with basic arguments.
  switch (Lang) {
#define TESTLANGUAGE_C(lang, version, std_flag, version_index)                 \
  case Lang_##lang##version:                                                   \
    return { "-x", "c", "-std=" #std_flag };
#define TESTLANGUAGE_CXX(lang, version, std_flag, version_index)               \
  case Lang_##lang##version:                                                   \
    return { "-std=" #std_flag, "-frtti" };
#include "language/Core/Testing/TestLanguage.def"

  case Lang_OBJC:
    return {"-x", "objective-c", "-frtti", "-fobjc-nonfragile-abi"};
  case Lang_OBJCXX:
    return {"-x", "objective-c++", "-frtti"};
  case Lang_OpenCL:
    toolchain_unreachable("Unhandled TestLanguage enum");
  }
  toolchain_unreachable("Unhandled TestLanguage enum");
}

std::vector<std::string> getCC1ArgsForTesting(TestLanguage Lang) {
  switch (Lang) {
#define TESTLANGUAGE_C(lang, version, std_flag, version_index)                 \
  case Lang_##lang##version:                                                   \
    return { "-xc", "-std=" #std_flag };
#define TESTLANGUAGE_CXX(lang, version, std_flag, version_index)               \
  case Lang_##lang##version:                                                   \
    return { "-std=" #std_flag };
#include "language/Core/Testing/TestLanguage.def"

  case Lang_OBJC:
    return {"-xobjective-c"};
    break;
  case Lang_OBJCXX:
    return {"-xobjective-c++"};
    break;
  case Lang_OpenCL:
    toolchain_unreachable("Unhandled TestLanguage enum");
  }
  toolchain_unreachable("Unhandled TestLanguage enum");
}

StringRef getFilenameForTesting(TestLanguage Lang) {
  switch (Lang) {
#define TESTLANGUAGE_C(lang, version, std_flag, version_index)                 \
  case Lang_##lang##version:                                                   \
    return "input.c";
#define TESTLANGUAGE_CXX(lang, version, std_flag, version_index)               \
  case Lang_##lang##version:                                                   \
    return "input.cc";
#include "language/Core/Testing/TestLanguage.def"

  case Lang_OpenCL:
    return "input.cl";

  case Lang_OBJC:
    return "input.m";

  case Lang_OBJCXX:
    return "input.mm";
  }
  toolchain_unreachable("Unhandled TestLanguage enum");
}

std::string getAnyTargetForTesting() {
  for (const auto &Target : toolchain::TargetRegistry::targets()) {
    std::string Error;
    StringRef TargetName(Target.getName());
    if (TargetName == "x86-64")
      TargetName = "x86_64";
    if (toolchain::TargetRegistry::lookupTarget(TargetName, Error) == &Target) {
      return std::string(TargetName);
    }
  }
  return "";
}

} // end namespace language::Core
