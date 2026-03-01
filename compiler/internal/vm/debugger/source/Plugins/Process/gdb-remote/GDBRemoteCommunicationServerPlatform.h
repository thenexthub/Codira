//===-- GDBRemoteCommunicationServerPlatform.h ------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_GDB_REMOTE_GDBREMOTECOMMUNICATIONSERVERPLATFORM_H
#define LLDB_SOURCE_PLUGINS_PROCESS_GDB_REMOTE_GDBREMOTECOMMUNICATIONSERVERPLATFORM_H

#include <map>
#include <mutex>
#include <optional>
#include <set>

#include "GDBRemoteCommunicationServerCommon.h"
#include "lldb/Host/Socket.h"

#include "llvm/Support/Error.h"

namespace lldb_private {
namespace process_gdb_remote {

class GDBRemoteCommunicationServerPlatform
    : public GDBRemoteCommunicationServerCommon {
public:
  GDBRemoteCommunicationServerPlatform(
      FileSpec debugserver_path, const Socket::SocketProtocol socket_protocol,
      uint16_t gdbserver_port);

  ~GDBRemoteCommunicationServerPlatform() override;

  Status LaunchProcess() override;

  void SetInferiorArguments(const lldb_private::Args &args);

  Status LaunchGDBServer(const lldb_private::Args &args, lldb::pid_t &pid,
                         std::string &socket_name, shared_fd_t fd);

  void SetPendingGdbServer(const std::string &socket_name);

protected:
  const FileSpec m_debugserver_path;
  const Socket::SocketProtocol m_socket_protocol;
  std::recursive_mutex m_spawned_pids_mutex;
  std::set<lldb::pid_t> m_spawned_pids;

  uint16_t m_gdbserver_port;
  std::optional<std::string> m_pending_gdb_server_socket_name;

  PacketResult Handle_qLaunchGDBServer(StringExtractorGDBRemote &packet);

  PacketResult Handle_qQueryGDBServer(StringExtractorGDBRemote &packet);

  PacketResult Handle_qKillSpawnedProcess(StringExtractorGDBRemote &packet);

  PacketResult Handle_qPathComplete(StringExtractorGDBRemote &packet);

  PacketResult Handle_qProcessInfo(StringExtractorGDBRemote &packet);

  PacketResult Handle_qGetWorkingDir(StringExtractorGDBRemote &packet);

  PacketResult Handle_QSetWorkingDir(StringExtractorGDBRemote &packet);

  PacketResult Handle_qC(StringExtractorGDBRemote &packet);

  PacketResult Handle_jSignalsInfo(StringExtractorGDBRemote &packet);

private:
  bool KillSpawnedProcess(lldb::pid_t pid);
  bool SpawnedProcessIsRunning(lldb::pid_t pid);
  void AddSpawnedProcess(lldb::pid_t pid);

  void DebugserverProcessReaped(lldb::pid_t pid);

  static const FileSpec &GetDomainSocketDir();

  static FileSpec GetDomainSocketPath(const char *prefix);

  GDBRemoteCommunicationServerPlatform(
      const GDBRemoteCommunicationServerPlatform &) = delete;
  const GDBRemoteCommunicationServerPlatform &
  operator=(const GDBRemoteCommunicationServerPlatform &) = delete;
};

} // namespace process_gdb_remote
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_PROCESS_GDB_REMOTE_GDBREMOTECOMMUNICATIONSERVERPLATFORM_H
