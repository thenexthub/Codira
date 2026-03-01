//===-- GDBRemote.cpp -----------------------------------------------------===//
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

#include "lldb/Utility/GDBRemote.h"

#include "lldb/Utility/Flags.h"
#include "lldb/Utility/Stream.h"

#include <cstdio>

using namespace lldb;
using namespace lldb_private;
using namespace llvm;

StreamGDBRemote::StreamGDBRemote() : StreamString() {}

StreamGDBRemote::StreamGDBRemote(uint32_t flags, uint32_t addr_size,
                                 ByteOrder byte_order)
    : StreamString(flags, addr_size, byte_order) {}

StreamGDBRemote::~StreamGDBRemote() = default;

int StreamGDBRemote::PutEscapedBytes(const void *s, size_t src_len) {
  int bytes_written = 0;
  const uint8_t *src = static_cast<const uint8_t *>(s);
  bool binary_is_set = m_flags.Test(eBinary);
  m_flags.Clear(eBinary);
  while (src_len) {
    uint8_t byte = *src;
    src++;
    src_len--;
    if (byte == 0x23 || byte == 0x24 || byte == 0x7d || byte == 0x2a) {
      bytes_written += PutChar(0x7d);
      byte ^= 0x20;
    }
    bytes_written += PutChar(byte);
  };
  if (binary_is_set)
    m_flags.Set(eBinary);
  return bytes_written;
}

llvm::StringRef GDBRemotePacket::GetTypeStr() const {
  switch (type) {
  case GDBRemotePacket::ePacketTypeSend:
    return "send";
  case GDBRemotePacket::ePacketTypeRecv:
    return "read";
  case GDBRemotePacket::ePacketTypeInvalid:
    return "invalid";
  }
  llvm_unreachable("All enum cases should be handled");
}

void GDBRemotePacket::Dump(Stream &strm) const {
  strm.Printf("tid=0x%4.4" PRIx64 " <%4u> %s packet: %s\n", tid,
              bytes_transmitted, GetTypeStr().data(), packet.data.c_str());
}
