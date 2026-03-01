//===-- GDBRemoteCommunicationServerLLGSTest.cpp --------------------------===//
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

#include "gtest/gtest.h"

#include "Plugins/Process/gdb-remote/GDBRemoteCommunicationServerLLGS.h"

using namespace lldb_private::process_gdb_remote;

TEST(GDBRemoteCommunicationServerLLGSTest, LLGSArgToURL) {
  // LLGS new-style URLs should be passed through (indepenently of
  // --reverse-connect)
  EXPECT_EQ(LLGSArgToURL("listen://127.0.0.1:1234", false),
            "listen://127.0.0.1:1234");
  EXPECT_EQ(LLGSArgToURL("listen://127.0.0.1:1234", true),
            "listen://127.0.0.1:1234");
  EXPECT_EQ(LLGSArgToURL("connect://127.0.0.1:1234", false),
            "connect://127.0.0.1:1234");
  EXPECT_EQ(LLGSArgToURL("connect://127.0.0.1:1234", true),
            "connect://127.0.0.1:1234");

  // LLGS legacy listen URLs should be converted if !reverse_connect
  EXPECT_EQ(LLGSArgToURL("tcp://127.0.0.1:1234", false),
            "listen://127.0.0.1:1234");
  EXPECT_EQ(LLGSArgToURL("unix:///tmp/foo", false), "unix-accept:///tmp/foo");
  EXPECT_EQ(LLGSArgToURL("unix-abstract://foo", false),
            "unix-abstract-accept://foo");

  // LLGS listen host:port pairs should be converted to listen://
  EXPECT_EQ(LLGSArgToURL("127.0.0.1:1234", false), "listen://127.0.0.1:1234");
  EXPECT_EQ(LLGSArgToURL("[::1]:1234", false), "listen://[::1]:1234");
  EXPECT_EQ(LLGSArgToURL("[[::1]:1234]", false), "listen://[[::1]:1234]");
  EXPECT_EQ(LLGSArgToURL("localhost:1234", false), "listen://localhost:1234");
  EXPECT_EQ(LLGSArgToURL("*:1234", false), "listen://*:1234");

  // LLGS listen :port special-case should be converted to listen://
  EXPECT_EQ(LLGSArgToURL(":1234", false), "listen://localhost:1234");

  // LLGS listen UNIX sockets should be converted to unix-accept://
  EXPECT_EQ(LLGSArgToURL("/tmp/foo", false), "unix-accept:///tmp/foo");
  EXPECT_EQ(LLGSArgToURL("127.0.0.1", false), "unix-accept://127.0.0.1");
  EXPECT_EQ(LLGSArgToURL("[::1]", false), "unix-accept://[::1]");
  EXPECT_EQ(LLGSArgToURL("localhost", false), "unix-accept://localhost");
  EXPECT_EQ(LLGSArgToURL(":frobnicate", false), "unix-accept://:frobnicate");

  // LLGS reverse connect host:port pairs should be converted to connect://
  EXPECT_EQ(LLGSArgToURL("127.0.0.1:1234", true), "connect://127.0.0.1:1234");
  EXPECT_EQ(LLGSArgToURL("[::1]:1234", true), "connect://[::1]:1234");
  EXPECT_EQ(LLGSArgToURL("[[::1]:1234]", true), "connect://[[::1]:1234]");
  EXPECT_EQ(LLGSArgToURL("localhost:1234", true), "connect://localhost:1234");

  // with LLGS reverse connect, anything else goes as unix-connect://
  EXPECT_EQ(LLGSArgToURL("/tmp/foo", true), "unix-connect:///tmp/foo");
  EXPECT_EQ(LLGSArgToURL("127.0.0.1", true), "unix-connect://127.0.0.1");
  EXPECT_EQ(LLGSArgToURL("[::1]", true), "unix-connect://[::1]");
  EXPECT_EQ(LLGSArgToURL("localhost", true), "unix-connect://localhost");
  EXPECT_EQ(LLGSArgToURL(":frobnicate", true), "unix-connect://:frobnicate");
}
