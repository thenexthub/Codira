//===--------------------- SocketTestUtilities.h ----------------*- C++ -*-===//
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

#ifndef LLDB_UNITTESTS_TESTINGSUPPORT_HOST_SOCKETTESTUTILITIES_H
#define LLDB_UNITTESTS_TESTINGSUPPORT_HOST_SOCKETTESTUTILITIES_H

#include <cstdio>
#include <functional>
#include <thread>

#include "lldb/Host/Config.h"
#include "lldb/Host/Socket.h"
#include "lldb/Host/common/TCPSocket.h"
#include "lldb/Host/common/UDPSocket.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Testing/Support/Error.h"

#if LLDB_ENABLE_POSIX
#include "lldb/Host/posix/DomainSocket.h"
#endif

namespace lldb_private {
template <typename SocketType>
void CreateConnectedSockets(
    llvm::StringRef listen_remote_address,
    const std::function<std::string(const SocketType &)> &get_connect_addr,
    std::unique_ptr<SocketType> *a_up, std::unique_ptr<SocketType> *b_up);
bool CreateTCPConnectedSockets(std::string listen_remote_ip,
                               std::unique_ptr<TCPSocket> *a_up,
                               std::unique_ptr<TCPSocket> *b_up);
#if LLDB_ENABLE_POSIX
void CreateDomainConnectedSockets(llvm::StringRef path,
                                  std::unique_ptr<DomainSocket> *a_up,
                                  std::unique_ptr<DomainSocket> *b_up);
#endif

bool HostSupportsIPv6();
bool HostSupportsIPv4();

/// Returns true if the name `localhost` maps to a loopback IPv4 address.
bool HostSupportsLocalhostToIPv4();
/// Returns true if the name `localhost` maps to a loopback IPv6 address.
bool HostSupportsLocalhostToIPv6();

/// Return an IP for localhost based on host support.
///
/// This will return either "127.0.0.1" if IPv4 is detected, or "[::1]" if IPv6
/// is detected. If neither are detected, return an error.
llvm::Expected<std::string> GetLocalhostIP();

} // namespace lldb_private

#endif
