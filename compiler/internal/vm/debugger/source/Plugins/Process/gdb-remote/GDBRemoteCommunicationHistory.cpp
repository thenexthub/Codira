//===-- GDBRemoteCommunicationHistory.cpp ---------------------------------===//
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

#include "GDBRemoteCommunicationHistory.h"

// Other libraries and framework includes
#include "lldb/Utility/ConstString.h"
#include "lldb/Utility/Log.h"

using namespace llvm;
using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::process_gdb_remote;

GDBRemoteCommunicationHistory::GDBRemoteCommunicationHistory(uint32_t size)
    : m_packets() {
  if (size)
    m_packets.resize(size);
}

GDBRemoteCommunicationHistory::~GDBRemoteCommunicationHistory() = default;

void GDBRemoteCommunicationHistory::AddPacket(char packet_char,
                                              GDBRemotePacket::Type type,
                                              uint32_t bytes_transmitted) {
  const size_t size = m_packets.size();
  if (size == 0)
    return;

  const uint32_t idx = GetNextIndex();
  m_packets[idx].packet.data.assign(1, packet_char);
  m_packets[idx].type = type;
  m_packets[idx].bytes_transmitted = bytes_transmitted;
  m_packets[idx].packet_idx = m_total_packet_count;
  m_packets[idx].tid = llvm::get_threadid();
}

void GDBRemoteCommunicationHistory::AddPacket(const std::string &src,
                                              uint32_t src_len,
                                              GDBRemotePacket::Type type,
                                              uint32_t bytes_transmitted) {
  const size_t size = m_packets.size();
  if (size == 0)
    return;

  const uint32_t idx = GetNextIndex();
  m_packets[idx].packet.data.assign(src, 0, src_len);
  m_packets[idx].type = type;
  m_packets[idx].bytes_transmitted = bytes_transmitted;
  m_packets[idx].packet_idx = m_total_packet_count;
  m_packets[idx].tid = llvm::get_threadid();
}

void GDBRemoteCommunicationHistory::Dump(Stream &strm) const {
  const uint32_t size = GetNumPacketsInHistory();
  const uint32_t first_idx = GetFirstSavedPacketIndex();
  const uint32_t stop_idx = m_curr_idx + size;
  for (uint32_t i = first_idx; i < stop_idx; ++i) {
    const uint32_t idx = NormalizeIndex(i);
    const GDBRemotePacket &entry = m_packets[idx];
    if (entry.type == GDBRemotePacket::ePacketTypeInvalid ||
        entry.packet.data.empty())
      break;
    strm.Printf("history[%u] ", entry.packet_idx);
    entry.Dump(strm);
  }
}

void GDBRemoteCommunicationHistory::Dump(Log *log) const {
  if (!log || m_dumped_to_log)
    return;

  m_dumped_to_log = true;
  const uint32_t size = GetNumPacketsInHistory();
  const uint32_t first_idx = GetFirstSavedPacketIndex();
  const uint32_t stop_idx = m_curr_idx + size;
  for (uint32_t i = first_idx; i < stop_idx; ++i) {
    const uint32_t idx = NormalizeIndex(i);
    const GDBRemotePacket &entry = m_packets[idx];
    if (entry.type == GDBRemotePacket::ePacketTypeInvalid ||
        entry.packet.data.empty())
      break;
    LLDB_LOGF(log, "history[%u] tid=0x%4.4" PRIx64 " <%4u> %s packet: %s",
              entry.packet_idx, entry.tid, entry.bytes_transmitted,
              (entry.type == GDBRemotePacket::ePacketTypeSend) ? "send"
                                                               : "read",
              entry.packet.data.c_str());
  }
}

