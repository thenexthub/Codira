//===-- SubsystemRAIITest.cpp ---------------------------------------------===//
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

#include "gtest/gtest-spi.h"
#include "gtest/gtest.h"

#include "TestingSupport/SubsystemRAII.h"

using namespace lldb_private;

namespace {

enum class SystemState {
  /// Start state of the subsystem.
  Start,
  /// Initialize has been called but Terminate hasn't been called yet.
  Initialized,
  /// Terminate has been called.
  Terminated
};

struct TestSubsystem {
  static SystemState state;
  static void Initialize() {
    assert(state == SystemState::Start);
    state = SystemState::Initialized;
  }
  static void Terminate() {
    assert(state == SystemState::Initialized);
    state = SystemState::Terminated;
  }
};
} // namespace

SystemState TestSubsystem::state = SystemState::Start;

TEST(SubsystemRAIITest, NormalSubsystem) {
  // Tests that SubsystemRAII handles Initialize functions that return void.
  EXPECT_EQ(SystemState::Start, TestSubsystem::state);
  {
    SubsystemRAII<TestSubsystem> subsystem;
    EXPECT_EQ(SystemState::Initialized, TestSubsystem::state);
  }
  EXPECT_EQ(SystemState::Terminated, TestSubsystem::state);
}

static const char *SubsystemErrorString = "Initialize failed";

namespace {
struct TestSubsystemWithError {
  static SystemState state;
  static bool will_fail;
  static llvm::Error Initialize() {
    assert(state == SystemState::Start);
    state = SystemState::Initialized;
    if (will_fail)
      return llvm::make_error<llvm::StringError>(
          SubsystemErrorString, llvm::inconvertibleErrorCode());
    return llvm::Error::success();
  }
  static void Terminate() {
    assert(state == SystemState::Initialized);
    state = SystemState::Terminated;
  }
  /// Reset the subsystem to the default state for testing.
  static void Reset() { state = SystemState::Start; }
};
} // namespace

SystemState TestSubsystemWithError::state = SystemState::Start;
bool TestSubsystemWithError::will_fail = false;

TEST(SubsystemRAIITest, SubsystemWithErrorSuccess) {
  // Tests that SubsystemRAII handles llvm::success() returned from
  // Initialize.
  TestSubsystemWithError::Reset();
  EXPECT_EQ(SystemState::Start, TestSubsystemWithError::state);
  {
    TestSubsystemWithError::will_fail = false;
    SubsystemRAII<TestSubsystemWithError> subsystem;
    EXPECT_EQ(SystemState::Initialized, TestSubsystemWithError::state);
  }
  EXPECT_EQ(SystemState::Terminated, TestSubsystemWithError::state);
}

TEST(SubsystemRAIITest, SubsystemWithErrorFailure) {
  // Tests that SubsystemRAII handles any errors returned from
  // Initialize.
  TestSubsystemWithError::Reset();
  EXPECT_EQ(SystemState::Start, TestSubsystemWithError::state);
  TestSubsystemWithError::will_fail = true;
  EXPECT_FATAL_FAILURE(SubsystemRAII<TestSubsystemWithError> subsystem,
                       SubsystemErrorString);
}
