//===-- ThreadLauncherTest.cpp --------------------------------------------===//
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

#include "lldb/Host/ThreadLauncher.h"
#include "llvm/Testing/Support/Error.h"
#include "gtest/gtest.h"
#include <future>

using namespace lldb_private;

TEST(ThreadLauncherTest, LaunchThread) {
  std::promise<int> promise;
  std::future<int> future = promise.get_future();
  llvm::Expected<HostThread> thread =
      ThreadLauncher::LaunchThread("test", [&promise] {
        promise.set_value(47);
        return (lldb::thread_result_t)47;
      });
  ASSERT_THAT_EXPECTED(thread, llvm::Succeeded());
  EXPECT_EQ(future.get(), 47);
  lldb::thread_result_t result;
  thread->Join(&result);
  EXPECT_EQ(result, (lldb::thread_result_t)47);
}
