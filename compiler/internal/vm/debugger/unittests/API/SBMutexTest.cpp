//===-- SBMutexTest.cpp ---------------------------------------------------===//
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

// Use the umbrella header for -Wdocumentation.
#include "lldb/API/LLDB.h"

#include "TestingSupport/SubsystemRAII.h"
#include "lldb/API/SBDebugger.h"
#include "lldb/API/SBTarget.h"
#include "gtest/gtest.h"
#include <atomic>
#include <chrono>
#include <future>
#include <mutex>

using namespace lldb;
using namespace lldb_private;

class SBMutexTest : public testing::Test {
protected:
  void SetUp() override { debugger = SBDebugger::Create(); }
  void TearDown() override { SBDebugger::Destroy(debugger); }

  SubsystemRAII<lldb::SBDebugger> subsystems;
  SBDebugger debugger;
};

TEST_F(SBMutexTest, LockTest) {
  lldb::SBTarget target = debugger.GetDummyTarget();
  std::atomic<bool> locked = false;
  std::future<void> f;
  {
    lldb::SBMutex lock = target.GetAPIMutex();

    ASSERT_TRUE(lock.try_lock());
    lock.unlock();

    std::lock_guard<lldb::SBMutex> lock_guard(lock);
    ASSERT_FALSE(locked.exchange(true));

    f = std::async(std::launch::async, [&]() {
      ASSERT_TRUE(locked);
      EXPECT_FALSE(lock.try_lock());
      target.BreakpointCreateByName("foo", "bar");
      ASSERT_FALSE(locked);
    });
    ASSERT_TRUE(f.valid());

    // Wait 500ms to confirm the thread is blocked.
    auto status = f.wait_for(std::chrono::milliseconds(500));
    ASSERT_EQ(status, std::future_status::timeout);

    ASSERT_TRUE(locked.exchange(false));
  }
  f.wait();
}
