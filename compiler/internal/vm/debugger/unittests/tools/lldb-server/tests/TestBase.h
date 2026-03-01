//===-- TestBase.h ----------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_UNITTESTS_TOOLS_LLDB_SERVER_TESTS_TESTBASE_H
#define LLDB_UNITTESTS_TOOLS_LLDB_SERVER_TESTS_TESTBASE_H

#include "TestClient.h"
#include "lldb/Host/FileSystem.h"
#include "lldb/Host/HostInfo.h"
#include "lldb/Host/Socket.h"
#include "llvm/Support/Path.h"
#include "llvm/Testing/Support/Error.h"
#include "gtest/gtest.h"

namespace llgs_tests {

class TestBase: public ::testing::Test {
public:
  static void SetUpTestCase() {
    lldb_private::FileSystem::Initialize();
    lldb_private::HostInfo::Initialize();
    ASSERT_THAT_ERROR(lldb_private::Socket::Initialize(), llvm::Succeeded());
  }

  static void TearDownTestCase() {
    lldb_private::Socket::Terminate();
    lldb_private::HostInfo::Terminate();
    lldb_private::FileSystem::Terminate();
  }

  static std::string getInferiorPath(llvm::StringRef Name) {
    llvm::SmallString<64> Path(LLDB_TEST_INFERIOR_PATH);
    llvm::sys::path::append(Path, Name + LLDB_TEST_INFERIOR_SUFFIX);
    return std::string(Path.str());
  }

  static std::string getLogFileName();
};

class StandardStartupTest: public TestBase {
public:
  void SetUp() override {
    auto ClientOr = TestClient::launch(getLogFileName());
    ASSERT_THAT_EXPECTED(ClientOr, llvm::Succeeded());
    Client = std::move(*ClientOr);
  }

protected:
  std::unique_ptr<TestClient> Client;
};

} // namespace llgs_tests

#endif // LLDB_UNITTESTS_TOOLS_LLDB_SERVER_TESTS_TESTBASE_H
