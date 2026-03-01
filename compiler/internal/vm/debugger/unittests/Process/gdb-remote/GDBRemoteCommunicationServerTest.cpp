//===-- GDBRemoteCommunicationServerTest.cpp ------------------------------===//
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
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "GDBRemoteTestUtils.h"
#include "Plugins/Process/gdb-remote/GDBRemoteCommunicationServer.h"
#include "lldb/Utility/Connection.h"
#include "lldb/Utility/UnimplementedError.h"
#include "lldb/lldb-enumerations.h"

namespace lldb_private {
namespace process_gdb_remote {

TEST(GDBRemoteCommunicationServerTest, SendErrorResponse_ErrorNumber) {
  MockServerWithMockConnection server;
  server.SendErrorResponse(0x42);

  EXPECT_THAT(server.GetPackets(), testing::ElementsAre("$E42#ab"));
}

TEST(GDBRemoteCommunicationServerTest, SendErrorResponse_Status) {
  MockServerWithMockConnection server;
  Status status(0x42, lldb::eErrorTypePOSIX, "Test error message");
  server.SendErrorResponse(status);

  EXPECT_THAT(
      server.GetPackets(),
      testing::ElementsAre("$E42;54657374206572726f72206d657373616765#ad"));
}

TEST(GDBRemoteCommunicationServerTest, SendErrorResponse_UnimplementedError) {
  MockServerWithMockConnection server;

  auto error = llvm::make_error<UnimplementedError>();
  server.SendErrorResponse(std::move(error));

  EXPECT_THAT(server.GetPackets(), testing::ElementsAre("$#00"));
}

TEST(GDBRemoteCommunicationServerTest, SendErrorResponse_StringError) {
  MockServerWithMockConnection server;

  auto error = llvm::createStringError(llvm::inconvertibleErrorCode(),
                                       "String error test");
  server.SendErrorResponse(std::move(error));

  EXPECT_THAT(
      server.GetPackets(),
      testing::ElementsAre("$Eff;537472696e67206572726f722074657374#b0"));
}

TEST(GDBRemoteCommunicationServerTest, SendErrorResponse_ErrorList) {
  MockServerWithMockConnection server;

  auto error = llvm::joinErrors(llvm::make_error<UnimplementedError>(),
                                llvm::make_error<UnimplementedError>());

  server.SendErrorResponse(std::move(error));
  // Make sure only one packet is sent even when there are multiple errors.
  EXPECT_EQ(server.GetPackets().size(), 1UL);
}

} // namespace process_gdb_remote
} // namespace lldb_private
