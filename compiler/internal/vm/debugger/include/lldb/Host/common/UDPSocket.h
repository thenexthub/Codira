//===-- UDPSocket.h ---------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_HOST_COMMON_UDPSOCKET_H
#define LLDB_HOST_COMMON_UDPSOCKET_H

#include "lldb/Host/Socket.h"

namespace lldb_private {
class UDPSocket : public Socket {
public:
  explicit UDPSocket(bool should_close);

  static llvm::Expected<std::unique_ptr<UDPSocket>>
  CreateConnected(llvm::StringRef name);

  std::string GetRemoteConnectionURI() const override;

private:
  UDPSocket(NativeSocket socket);

  size_t Send(const void *buf, const size_t num_bytes) override;
  Status Connect(llvm::StringRef name) override;
  Status Listen(llvm::StringRef name, int backlog) override;

  llvm::Expected<std::vector<MainLoopBase::ReadHandleUP>>
  Accept(MainLoopBase &loop,
         std::function<void(std::unique_ptr<Socket> socket)> sock_cb) override {
    return llvm::errorCodeToError(
        std::make_error_code(std::errc::operation_not_supported));
  }

  SocketAddress m_sockaddr;
};
}

#endif // LLDB_HOST_COMMON_UDPSOCKET_H
