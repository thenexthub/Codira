//===-- TestBase.cpp ------------------------------------------------------===//
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

#include "TestBase.h"
#include <cstdlib>

using namespace llgs_tests;
using namespace llvm;

std::string TestBase::getLogFileName() {
  const auto *test_info =
      ::testing::UnitTest::GetInstance()->current_test_info();
  assert(test_info);

  const char *Dir = getenv("LOG_FILE_DIRECTORY");
  if (!Dir)
    return "";

  if (!llvm::sys::fs::is_directory(Dir)) {
    GTEST_LOG_(WARNING) << "Cannot access log directory: " << Dir;
    return "";
  }

  SmallString<64> DirStr(Dir);
  sys::path::append(DirStr, std::string("server-") +
                                test_info->test_case_name() + "-" +
                                test_info->name() + ".log");
  return std::string(DirStr.str());
}

