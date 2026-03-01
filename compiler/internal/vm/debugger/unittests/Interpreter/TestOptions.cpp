//===-- TestOptions.cpp ---------------------------------------------------===//
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

#include "lldb/Interpreter/Options.h"
#include "gtest/gtest.h"

#include "llvm/Testing/Support/Error.h"

using namespace lldb_private;

TEST(OptionsTest, CreateOptionParsingError) {
  ASSERT_THAT_ERROR(
      CreateOptionParsingError("yippee", 'f', "fun",
                               "unable to convert 'yippee' to boolean"),
      llvm::FailedWithMessage("invalid value ('yippee') for -f (fun): unable "
                              "to convert 'yippee' to boolean"));

  ASSERT_THAT_ERROR(
      CreateOptionParsingError("52", 'b', "bean-count"),
      llvm::FailedWithMessage("invalid value ('52') for -b (bean-count)"));

  ASSERT_THAT_ERROR(CreateOptionParsingError("c", 'm'),
                    llvm::FailedWithMessage("invalid value ('c') for -m"));
}
