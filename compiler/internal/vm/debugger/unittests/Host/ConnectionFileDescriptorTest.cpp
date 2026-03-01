//===-- ConnectionFileDescriptorTest.cpp ----------------------------------===//
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

#include "TestingSupport/Host/SocketTestUtilities.h"
#include "gtest/gtest.h"
#include "TestingSupport/SubsystemRAII.h"
#include "lldb/Host/posix/ConnectionFileDescriptorPosix.h"
#include "lldb/Utility/UriParser.h"

using namespace lldb_private;

class ConnectionFileDescriptorTest : public testing::Test {
public:
  SubsystemRAII<Socket> subsystems;

  void TestGetURI(std::string ip) {
    std::unique_ptr<TCPSocket> socket_a_up;
    std::unique_ptr<TCPSocket> socket_b_up;
    CreateTCPConnectedSockets(ip, &socket_a_up, &socket_b_up);
    uint16_t socket_a_remote_port = socket_a_up->GetRemotePortNumber();
    ConnectionFileDescriptor connection_file_descriptor(std::move(socket_a_up));

    std::string uri(connection_file_descriptor.GetURI());
    EXPECT_EQ((URI{"connect", ip, socket_a_remote_port, "/"}),
              *URI::Parse(uri));
  }
};

TEST_F(ConnectionFileDescriptorTest, TCPGetURIv4) {
  if (!HostSupportsIPv4())
    return;
  TestGetURI("127.0.0.1");
}

TEST_F(ConnectionFileDescriptorTest, TCPGetURIv6) {
  if (!HostSupportsIPv6())
    return;
  TestGetURI("::1");
}
