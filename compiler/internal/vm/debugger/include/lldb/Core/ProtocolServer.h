//===-- ProtocolServer.h --------------------------------------------------===//
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

#ifndef LLDB_CORE_PROTOCOLSERVER_H
#define LLDB_CORE_PROTOCOLSERVER_H

#include "lldb/Core/PluginInterface.h"
#include "lldb/Host/Socket.h"
#include "lldb/lldb-private-interfaces.h"

namespace lldb_private {

class ProtocolServer : public PluginInterface {
public:
  ProtocolServer() = default;
  virtual ~ProtocolServer() = default;

  static ProtocolServer *GetOrCreate(llvm::StringRef name);

  static llvm::Error Terminate();

  static std::vector<llvm::StringRef> GetSupportedProtocols();

  struct Connection {
    Socket::SocketProtocol protocol;
    std::string name;
  };

  virtual llvm::Error Start(Connection connection) = 0;
  virtual llvm::Error Stop() = 0;

  virtual Socket *GetSocket() const = 0;
};

} // namespace lldb_private

#endif
