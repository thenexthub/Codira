//===--- TestClangConfig.h ------------------------------------------------===//
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

#ifndef LANGUAGE_CORE_TESTING_TESTCLANGCONFIG_H
#define LANGUAGE_CORE_TESTING_TESTCLANGCONFIG_H

#include "language/Core/Testing/CommandLineArgs.h"
#include "toolchain/Support/raw_ostream.h"
#include <string>
#include <vector>

namespace language::Core {

/// A Clang configuration for end-to-end tests that can be converted to
/// command line arguments for the driver.
///
/// The configuration is represented as typed, named values, making it easier
/// and safer to work with compared to an array of string command line flags.
struct TestClangConfig {
  TestLanguage Language;

  /// The argument of the `-target` command line flag.
  std::string Target;

  bool isC() const {
    return false
#define TESTLANGUAGE_C(lang, version, std_flag, version_index)                 \
  || Language == Lang_##lang##version
#include "language/Core/Testing/TestLanguage.def"
        ;
  }

  bool isC(int Version) const {
    return false
#define TESTLANGUAGE_C(lang, version, std_flag, version_index)                 \
  || (Version == version && Language == Lang_##lang##version)
#include "language/Core/Testing/TestLanguage.def"
        ;
  }

  bool isCOrLater(int MinimumStdVersion) const {
    const auto MinimumStdVersionIndex = 0
#define TESTLANGUAGE_C(lang, version, std_flag, version_index)                 \
  +(MinimumStdVersion == version ? version_index : 0)
#include "language/Core/Testing/TestLanguage.def"
        ;
    switch (Language) {
#define TESTLANGUAGE_C(lang, version, std_flag, version_index)                 \
  case Lang_##lang##version:                                                   \
    return MinimumStdVersionIndex <= version_index;
#include "language/Core/Testing/TestLanguage.def"
    default:
      return false;
    }
  }

  bool isC99OrLater() const { return isCOrLater(99); }

  bool isCOrEarlier(int MaximumStdVersion) const {
    return isC() && (isC(MaximumStdVersion) || !isCOrLater(MaximumStdVersion));
  }

  bool isCXX() const {
    return false
#define TESTLANGUAGE_CXX(lang, version, std_flag, version_index)               \
  || Language == Lang_##lang##version
#include "language/Core/Testing/TestLanguage.def"
        ;
  }

  bool isCXX(int Version) const {
    return false
#define TESTLANGUAGE_CXX(lang, version, std_flag, version_index)               \
  || (Version == version && Language == Lang_##lang##version)
#include "language/Core/Testing/TestLanguage.def"
        ;
  }

  bool isCXXOrLater(int MinimumStdVersion) const {
    const auto MinimumStdVersionIndex = 0
#define TESTLANGUAGE_CXX(lang, version, std_flag, version_index)               \
  +(MinimumStdVersion == version ? version_index : 0)
#include "language/Core/Testing/TestLanguage.def"
        ;
    switch (Language) {
#define TESTLANGUAGE_CXX(lang, version, std_flag, version_index)               \
  case Lang_##lang##version:                                                   \
    return MinimumStdVersionIndex <= version_index;
#include "language/Core/Testing/TestLanguage.def"
    default:
      return false;
    }
  }

  bool isCXX11OrLater() const { return isCXXOrLater(11); }

  bool isCXX14OrLater() const { return isCXXOrLater(14); }

  bool isCXX17OrLater() const { return isCXXOrLater(17); }

  bool isCXX20OrLater() const { return isCXXOrLater(20); }

  bool isCXX23OrLater() const { return isCXXOrLater(23); }

  bool isCXXOrEarlier(int MaximumStdVersion) const {
    return isCXX() &&
           (isCXX(MaximumStdVersion) || !isCXXOrLater(MaximumStdVersion));
  }

  bool supportsCXXDynamicExceptionSpecification() const {
    return Language == Lang_CXX03 || Language == Lang_CXX11 ||
           Language == Lang_CXX14;
  }

  bool hasDelayedTemplateParsing() const {
    return Target == "x86_64-pc-win32-msvc";
  }

  std::vector<std::string> getCommandLineArgs() const {
    std::vector<std::string> Result = getCommandLineArgsForTesting(Language);
    Result.push_back("-target");
    Result.push_back(Target);
    return Result;
  }

  std::string toShortString() const {
    std::string Result;
    toolchain::raw_string_ostream OS(Result);
    switch (Language) {
#define TESTLANGUAGE(lang, version, std_flag, version_index)                   \
  case Lang_##lang##version:                                                   \
    OS << (#lang #version);                                                    \
    break;
#include "language/Core/Testing/TestLanguage.def"
    case Lang_OpenCL:
      OS << "OpenCL";
      break;
    case Lang_OBJC:
      OS << "OBJC";
      break;
    case Lang_OBJCXX:
      OS << "OBJCXX";
      break;
    }

    OS << (Target.find("win") != std::string::npos ? "_win" : "");
    return Result;
  }

  std::string toString() const {
    std::string Result;
    toolchain::raw_string_ostream OS(Result);
    OS << "{ Language=" << Language << ", Target=" << Target << " }";
    return Result;
  }

  friend std::ostream &operator<<(std::ostream &OS,
                                  const TestClangConfig &ClangConfig) {
    return OS << ClangConfig.toString();
  }
};

} // end namespace language::Core

#endif
