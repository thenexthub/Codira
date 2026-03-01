//===-- ProcessGDBRemoteLog.cpp -------------------------------------------===//
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

#include "ProcessGDBRemoteLog.h"
#include "ProcessGDBRemote.h"
#include "llvm/Support/Threading.h"

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::process_gdb_remote;

static constexpr Log::Category g_categories[] = {
    {{"async"}, {"log asynchronous activity"}, GDBRLog::Async},
    {{"break"}, {"log breakpoints"}, GDBRLog::Breakpoints},
    {{"comm"}, {"log communication activity"}, GDBRLog::Comm},
    {{"packets"}, {"log gdb remote packets"}, GDBRLog::Packets},
    {{"memory"}, {"log memory reads and writes"}, GDBRLog::Memory},
    {{"data-short"},
     {"log memory bytes for memory reads and writes for short transactions "
      "only"},
     GDBRLog::MemoryDataShort},
    {{"data-long"},
     {"log memory bytes for memory reads and writes for all transactions"},
     GDBRLog::MemoryDataLong},
    {{"process"}, {"log process events and activities"}, GDBRLog::Process},
    {{"step"}, {"log step related activities"}, GDBRLog::Step},
    {{"thread"}, {"log thread events and activities"}, GDBRLog::Thread},
    {{"watch"}, {"log watchpoint related activities"}, GDBRLog::Watchpoints},
};

static Log::Channel g_channel(g_categories, GDBRLog::Packets);

template <> Log::Channel &lldb_private::LogChannelFor<GDBRLog>() {
  return g_channel;
}

void ProcessGDBRemoteLog::Initialize() {
  static llvm::once_flag g_once_flag;
  llvm::call_once(g_once_flag, []() {
    Log::Register("gdb-remote", g_channel);
  });
}
