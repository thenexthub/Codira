//===-- GDBRemoteCommunicationTest.cpp ------------------------------------===//
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

#include "GDBRemoteTestUtils.h"
#include "lldb/Host/ConnectionFileDescriptor.h"
#include "llvm/Testing/Support/Error.h"

using namespace lldb_private::process_gdb_remote;
using namespace lldb_private;
using namespace lldb;
typedef GDBRemoteCommunication::PacketResult PacketResult;

namespace {

class TestClient : public GDBRemoteCommunication {
public:
  TestClient() : GDBRemoteCommunication() {}

  PacketResult ReadPacket(StringExtractorGDBRemote &response) {
    return GDBRemoteCommunication::ReadPacket(response, std::chrono::seconds(1),
                                              /*sync_on_timeout*/ false);
  }
};

class GDBRemoteCommunicationTest : public GDBRemoteTest {
public:
  void SetUp() override {
    llvm::Expected<Socket::Pair> pair = Socket::CreatePair();
    ASSERT_THAT_EXPECTED(pair, llvm::Succeeded());
    client.SetConnection(
        std::make_unique<ConnectionFileDescriptor>(std::move(pair->first)));
    server.SetConnection(
        std::make_unique<ConnectionFileDescriptor>(std::move(pair->second)));
  }

protected:
  TestClient client;
  MockServer server;

  bool Write(llvm::StringRef packet) {
    ConnectionStatus status;
    return server.WriteAll(packet.data(), packet.size(), status, nullptr) ==
           packet.size();
  }
};
} // end anonymous namespace

// Test that we can decode packets correctly. In particular, verify that
// checksum calculation works.
TEST_F(GDBRemoteCommunicationTest, ReadPacket) {
  struct TestCase {
    llvm::StringLiteral Packet;
    llvm::StringLiteral Payload;
  };
  static constexpr TestCase Tests[] = {
      {{"$#00"}, {""}},
      {{"$foobar#79"}, {"foobar"}},
      {{"$}]#da"}, {"}"}},          // Escaped }
      {{"$x*%#c7"}, {"xxxxxxxxx"}}, // RLE
      {{"+$#00"}, {""}},            // Spurious ACK
      {{"-$#00"}, {""}},            // Spurious NAK
  };
  for (const auto &Test : Tests) {
    SCOPED_TRACE(Test.Packet + " -> " + Test.Payload);
    StringExtractorGDBRemote response;
    ASSERT_TRUE(Write(Test.Packet));
    ASSERT_EQ(PacketResult::Success, client.ReadPacket(response));
    ASSERT_EQ(Test.Payload, response.GetStringRef());
    ASSERT_EQ(PacketResult::Success, server.GetAck());
  }
}

// Test that packets with incorrect RLE sequences do not cause a crash and
// reported as invalid.
TEST_F(GDBRemoteCommunicationTest, CheckForPacket) {
  using PacketType = GDBRemoteCommunication::PacketType;
  struct TestCase {
    llvm::StringLiteral Packet;
    PacketType Result;
  };
  static constexpr TestCase Tests[] = {
      {{"$#00"}, PacketType::Standard},
      {{"$xx*#00"}, PacketType::Invalid}, // '*' without a count
      {{"$*#00"}, PacketType::Invalid},   // '*' without a preceding character
      {{"$xx}#00"}, PacketType::Invalid}, // bare escape character '}'
      {{"%#00"}, PacketType::Notify},     // a correct packet after an invalid
  };
  for (const auto &Test : Tests) {
    SCOPED_TRACE(Test.Packet);
    StringExtractorGDBRemote response;
    EXPECT_EQ(Test.Result, client.CheckForPacket(Test.Packet.bytes_begin(),
                                                 Test.Packet.size(), response));
  }
}
