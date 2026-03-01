//===-- GDBRemoteCommunicationServer.h --------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_GDB_REMOTE_GDBREMOTECOMMUNICATIONSERVER_H
#define LLDB_SOURCE_PLUGINS_PROCESS_GDB_REMOTE_GDBREMOTECOMMUNICATIONSERVER_H

#include <functional>
#include <map>

#include "GDBRemoteCommunication.h"
#include "lldb/lldb-private-forward.h"

#include "llvm/Support/Errc.h"
#include "llvm/Support/Error.h"

class StringExtractorGDBRemote;

namespace lldb_private {
namespace process_gdb_remote {

class ProcessGDBRemote;

class GDBRemoteCommunicationServer : public GDBRemoteCommunication {
public:
  using PacketHandler =
      std::function<PacketResult(StringExtractorGDBRemote &packet,
                                 Status &error, bool &interrupt, bool &quit)>;

  GDBRemoteCommunicationServer();

  ~GDBRemoteCommunicationServer() override;

  void
  RegisterPacketHandler(StringExtractorGDBRemote::ServerPacketType packet_type,
                        PacketHandler handler);

  PacketResult GetPacketAndSendResponse(Timeout<std::micro> timeout,
                                        Status &error, bool &interrupt,
                                        bool &quit);

protected:
  std::map<StringExtractorGDBRemote::ServerPacketType, PacketHandler>
      m_packet_handlers;
  bool m_exit_now; // use in asynchronous handling to indicate process should
                   // exit.

  bool m_send_error_strings = false; // If the client enables this then
                                     // we will send error strings as well.

  PacketResult Handle_QErrorStringEnable(StringExtractorGDBRemote &packet);

  PacketResult SendErrorResponse(const Status &error);

  PacketResult SendErrorResponse(llvm::Error error);

  PacketResult SendUnimplementedResponse(const char *packet);

  PacketResult SendErrorResponse(uint8_t error);

  PacketResult SendIllFormedResponse(const StringExtractorGDBRemote &packet,
                                     const char *error_message);

  PacketResult SendOKResponse();

  /// Serialize and send a JSON object response.
  PacketResult SendJSONResponse(const llvm::json::Value &value);

  /// Serialize and send a JSON object response, or respond with an error if the
  /// input object is an \a llvm::Error.
  PacketResult SendJSONResponse(llvm::Expected<llvm::json::Value> value);

private:
  GDBRemoteCommunicationServer(const GDBRemoteCommunicationServer &) = delete;
  const GDBRemoteCommunicationServer &
  operator=(const GDBRemoteCommunicationServer &) = delete;
};

} // namespace process_gdb_remote
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_PROCESS_GDB_REMOTE_GDBREMOTECOMMUNICATIONSERVER_H
