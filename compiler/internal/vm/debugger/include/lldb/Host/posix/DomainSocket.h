//===-- DomainSocket.h ------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_HOST_POSIX_DOMAINSOCKET_H
#define LLDB_HOST_POSIX_DOMAINSOCKET_H

#include "lldb/Host/Socket.h"
#include <string>
#include <vector>

namespace lldb_private {
class DomainSocket : public Socket {
public:
  DomainSocket(NativeSocket socket, bool should_close);
  explicit DomainSocket(bool should_close);

  using Pair =
      std::pair<std::unique_ptr<DomainSocket>, std::unique_ptr<DomainSocket>>;
  static llvm::Expected<Pair> CreatePair();

  Status Connect(llvm::StringRef name) override;
  Status Listen(llvm::StringRef name, int backlog) override;

  using Socket::Accept;
  llvm::Expected<std::vector<MainLoopBase::ReadHandleUP>>
  Accept(MainLoopBase &loop,
         std::function<void(std::unique_ptr<Socket> socket)> sock_cb) override;

  std::string GetRemoteConnectionURI() const override;

  std::vector<std::string> GetListeningConnectionURI() const override;

  static llvm::Expected<std::unique_ptr<DomainSocket>>
  FromBoundNativeSocket(NativeSocket sockfd, bool should_close);

protected:
  DomainSocket(SocketProtocol protocol);
  DomainSocket(SocketProtocol protocol, NativeSocket socket, bool should_close);

  virtual size_t GetNameOffset() const;
  virtual void DeleteSocketFile(llvm::StringRef name);
  std::string GetSocketName() const;

private:
  DomainSocket(NativeSocket socket, const DomainSocket &listen_socket);
};
}

#endif // LLDB_HOST_POSIX_DOMAINSOCKET_H
