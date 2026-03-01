//===-- ProcessKDPLog.cpp -------------------------------------------------===//
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

#include "ProcessKDPLog.h"

using namespace lldb_private;

static constexpr Log::Category g_categories[] = {
    {{"async"}, {"log asynchronous activity"}, KDPLog::Async},
    {{"break"}, {"log breakpoints"}, KDPLog::Breakpoints},
    {{"comm"}, {"log communication activity"}, KDPLog::Comm},
    {{"data-long"},
     {"log memory bytes for memory reads and writes for all transactions"},
     KDPLog::MemoryDataLong},
    {{"data-short"},
     {"log memory bytes for memory reads and writes for short transactions "
      "only"},
     KDPLog::MemoryDataShort},
    {{"memory"}, {"log memory reads and writes"}, KDPLog::Memory},
    {{"packets"}, {"log gdb remote packets"}, KDPLog::Packets},
    {{"process"}, {"log process events and activities"}, KDPLog::Process},
    {{"step"}, {"log step related activities"}, KDPLog::Step},
    {{"thread"}, {"log thread events and activities"}, KDPLog::Thread},
    {{"watch"}, {"log watchpoint related activities"}, KDPLog::Watchpoints},
};

static Log::Channel g_channel(g_categories, KDPLog::Packets);

template <> Log::Channel &lldb_private::LogChannelFor<KDPLog>() {
  return g_channel;
}

void ProcessKDPLog::Initialize() { Log::Register("kdp-remote", g_channel); }
