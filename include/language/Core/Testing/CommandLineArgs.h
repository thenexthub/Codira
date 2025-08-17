//===--- CommandLineArgs.h ------------------------------------------------===//
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
//
//  This file defines language options for Clang unittests.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TESTING_COMMANDLINEARGS_H
#define LANGUAGE_CORE_TESTING_COMMANDLINEARGS_H

#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/StringRef.h"
#include <string>
#include <vector>

namespace language::Core {

enum TestLanguage {
#define TESTLANGUAGE(lang, version, std_flag, version_index)                   \
  Lang_##lang##version,
#include "language/Core/Testing/TestLanguage.def"

  Lang_OpenCL,
  Lang_OBJC,
  Lang_OBJCXX,
};

std::vector<TestLanguage> getCOrLater(int MinimumStd);
std::vector<TestLanguage> getCXXOrLater(int MinimumStd);

std::vector<std::string> getCommandLineArgsForTesting(TestLanguage Lang);
std::vector<std::string> getCC1ArgsForTesting(TestLanguage Lang);

StringRef getFilenameForTesting(TestLanguage Lang);

/// Find a target name such that looking for it in TargetRegistry by that name
/// returns the same target. We expect that there is at least one target
/// configured with this property.
std::string getAnyTargetForTesting();

} // end namespace language::Core

#endif
