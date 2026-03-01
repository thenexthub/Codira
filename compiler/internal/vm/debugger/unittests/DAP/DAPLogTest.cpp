//===----------------------------------------------------------------------===//
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

#include "DAPLog.h"
#include "llvm/Support/raw_ostream.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lldb_dap;
using namespace llvm;
using namespace testing;

static llvm::StringRef last_line(llvm::StringRef str) {
  size_t index = str.find_last_of('\n', str.size() - 1);
  if (index == llvm::StringRef::npos)
    return str;
  return str.substr(index + 1);
}

#define TIMESTAMP_PATTERN "\\[[0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] "

TEST(DAPLog, Emit) {
  Log::Mutex mux;
  std::string outs;
  raw_string_ostream os(outs);
  Log log(os, mux);
  Log inner_log = log.WithPrefix("my_prefix:");

  log.Emit("Hi");
  EXPECT_THAT(last_line(outs), MatchesRegex(TIMESTAMP_PATTERN "Hi\n"));

  inner_log.Emit("foobar");
  EXPECT_THAT(last_line(outs),
              MatchesRegex(TIMESTAMP_PATTERN "my_prefix: foobar\n"));

  log.Emit("Hello from a file/line.", "file.cpp", 42);
  EXPECT_THAT(
      last_line(outs),
      MatchesRegex(TIMESTAMP_PATTERN "file.cpp:42 Hello from a file/line.\n"));

  inner_log.Emit("Hello from a file/line.", "file.cpp", 42);
  EXPECT_THAT(last_line(outs),
              MatchesRegex(TIMESTAMP_PATTERN
                           "file.cpp:42 my_prefix: Hello from a file/line.\n"));

  log.WithPrefix("a").WithPrefix("b").WithPrefix("c").Emit("msg");
  EXPECT_THAT(last_line(outs), MatchesRegex(TIMESTAMP_PATTERN "a b c msg\n"));
}
