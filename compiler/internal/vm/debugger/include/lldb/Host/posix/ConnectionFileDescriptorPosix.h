//===-- ConnectionFileDescriptorPosix.h -------------------------*- C++ -*-===//
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

#ifndef LLDB_HOST_POSIX_CONNECTIONFILEDESCRIPTORPOSIX_H
#define LLDB_HOST_POSIX_CONNECTIONFILEDESCRIPTORPOSIX_H

#include <atomic>
#include <memory>
#include <mutex>

#include "lldb/lldb-forward.h"

#include "lldb/Host/Pipe.h"
#include "lldb/Host/Socket.h"
#include "lldb/Utility/Connection.h"

namespace lldb_private {

class Status;

class ConnectionFileDescriptor : public Connection {
public:
  typedef llvm::function_ref<void(llvm::StringRef local_socket_id)>
      socket_id_callback_type;

  ConnectionFileDescriptor();

  ConnectionFileDescriptor(int fd, bool owns_fd);

  ConnectionFileDescriptor(std::unique_ptr<Socket> socket_up);

  ~ConnectionFileDescriptor() override;

  bool IsConnected() const override;

  lldb::ConnectionStatus Connect(llvm::StringRef url,
                                 Status *error_ptr) override;

  lldb::ConnectionStatus Connect(llvm::StringRef url,
                                 socket_id_callback_type socket_id_callback,
                                 Status *error_ptr);

  lldb::ConnectionStatus Disconnect(Status *error_ptr) override;

  size_t Read(void *dst, size_t dst_len, const Timeout<std::micro> &timeout,
              lldb::ConnectionStatus &status, Status *error_ptr) override;

  size_t Write(const void *src, size_t src_len, lldb::ConnectionStatus &status,
               Status *error_ptr) override;

  std::string GetURI() override;

  lldb::ConnectionStatus BytesAvailable(const Timeout<std::micro> &timeout,
                                        Status *error_ptr);

  bool InterruptRead() override;

  lldb::IOObjectSP GetReadObject() override { return m_io_sp; }

protected:
  void OpenCommandPipe();

  void CloseCommandPipe();

  lldb::ConnectionStatus
  AcceptSocket(Socket::SocketProtocol socket_protocol,
               llvm::StringRef socket_name,
               llvm::function_ref<void(Socket &)> post_listen_callback,
               Status *error_ptr);

  lldb::ConnectionStatus ConnectSocket(Socket::SocketProtocol socket_protocol,
                                       llvm::StringRef socket_name,
                                       Status *error_ptr);

  lldb::ConnectionStatus AcceptTCP(llvm::StringRef host_and_port,
                                   socket_id_callback_type socket_id_callback,
                                   Status *error_ptr);

  lldb::ConnectionStatus ConnectTCP(llvm::StringRef host_and_port,
                                    socket_id_callback_type socket_id_callback,
                                    Status *error_ptr);

  lldb::ConnectionStatus ConnectUDP(llvm::StringRef args,
                                    socket_id_callback_type socket_id_callback,
                                    Status *error_ptr);

  lldb::ConnectionStatus
  ConnectNamedSocket(llvm::StringRef socket_name,
                     socket_id_callback_type socket_id_callback,
                     Status *error_ptr);

  lldb::ConnectionStatus
  AcceptNamedSocket(llvm::StringRef socket_name,
                    socket_id_callback_type socket_id_callback,
                    Status *error_ptr);

  lldb::ConnectionStatus
  AcceptAbstractSocket(llvm::StringRef socket_name,
                       socket_id_callback_type socket_id_callback,
                       Status *error_ptr);

  lldb::ConnectionStatus
  ConnectAbstractSocket(llvm::StringRef socket_name,
                        socket_id_callback_type socket_id_callback,
                        Status *error_ptr);

  lldb::ConnectionStatus ConnectFD(llvm::StringRef args,
                                   socket_id_callback_type socket_id_callback,
                                   Status *error_ptr);

  lldb::ConnectionStatus ConnectFile(llvm::StringRef args,
                                     socket_id_callback_type socket_id_callback,
                                     Status *error_ptr);

  lldb::ConnectionStatus
  ConnectSerialPort(llvm::StringRef args,
                    socket_id_callback_type socket_id_callback,
                    Status *error_ptr);

  lldb::IOObjectSP m_io_sp;

  Pipe m_pipe;
  std::recursive_mutex m_mutex;
  std::atomic<bool> m_shutting_down; // This marks that we are shutting down so
                                     // if we get woken up from
  // BytesAvailable to disconnect, we won't try to read again.

  std::string m_uri;

private:
  ConnectionFileDescriptor(const ConnectionFileDescriptor &) = delete;
  const ConnectionFileDescriptor &
  operator=(const ConnectionFileDescriptor &) = delete;
};

} // namespace lldb_private

#endif // LLDB_HOST_POSIX_CONNECTIONFILEDESCRIPTORPOSIX_H
