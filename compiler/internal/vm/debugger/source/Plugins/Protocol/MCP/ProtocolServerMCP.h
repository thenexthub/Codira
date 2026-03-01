//===- ProtocolServerMCP.h ------------------------------------------------===//
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

#ifndef LLDB_PLUGINS_PROTOCOL_MCP_PROTOCOLSERVERMCP_H
#define LLDB_PLUGINS_PROTOCOL_MCP_PROTOCOLSERVERMCP_H

#include "lldb/Core/ProtocolServer.h"
#include "lldb/Host/MainLoop.h"
#include "lldb/Host/Socket.h"
#include "lldb/Protocol/MCP/Server.h"
#include "lldb/Protocol/MCP/Transport.h"
#include <map>
#include <memory>
#include <thread>
#include <tuple>
#include <vector>

namespace lldb_private::mcp {

class ProtocolServerMCP : public ProtocolServer {

  using ServerUP = std::unique_ptr<lldb_protocol::mcp::Server>;

  using ReadHandleUP = MainLoop::ReadHandleUP;

public:
  ProtocolServerMCP();
  ~ProtocolServerMCP() override;

  llvm::Error Start(ProtocolServer::Connection connection) override;
  llvm::Error Stop() override;

  static void Initialize();
  static void Terminate();

  static llvm::StringRef GetPluginNameStatic() { return "MCP"; }
  static llvm::StringRef GetPluginDescriptionStatic();

  static lldb::ProtocolServerUP CreateInstance();

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

  Socket *GetSocket() const override { return m_listener.get(); }

protected:
  // This adds tools and resource providers that
  // are specific to this server. Overridable by the unit tests.
  virtual void Extend(lldb_protocol::mcp::Server &server) const;

private:
  void AcceptCallback(std::unique_ptr<Socket> socket);

  bool m_running = false;

  lldb_private::MainLoop m_loop;
  std::thread m_loop_thread;
  std::mutex m_mutex;
  size_t m_client_count = 0;

  std::unique_ptr<Socket> m_listener;
  std::vector<ReadHandleUP> m_accept_handles;

  ServerUP m_server;
  lldb_protocol::mcp::ServerInfoHandle m_server_info_handle;
};

} // namespace lldb_private::mcp

#endif
