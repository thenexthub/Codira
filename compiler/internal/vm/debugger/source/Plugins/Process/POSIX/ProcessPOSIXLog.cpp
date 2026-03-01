//===-- ProcessPOSIXLog.cpp -----------------------------------------------===//
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

#include "ProcessPOSIXLog.h"

#include "llvm/Support/Threading.h"

using namespace lldb_private;

static constexpr Log::Category g_categories[] = {
    {{"break"}, {"log breakpoints"}, POSIXLog::Breakpoints},
    {{"memory"}, {"log memory reads and writes"}, POSIXLog::Memory},
    {{"process"}, {"log process events and activities"}, POSIXLog::Process},
    {{"ptrace"}, {"log all calls to ptrace"}, POSIXLog::Ptrace},
    {{"registers"}, {"log register read/writes"}, POSIXLog::Registers},
    {{"thread"}, {"log thread events and activities"}, POSIXLog::Thread},
    {{"watch"}, {"log watchpoint related activities"}, POSIXLog::Watchpoints},
};

static Log::Channel g_channel(g_categories, POSIXLog::Process);

template <> Log::Channel &lldb_private::LogChannelFor<POSIXLog>() {
  return g_channel;
}

void ProcessPOSIXLog::Initialize() {
  static llvm::once_flag g_once_flag;
  llvm::call_once(g_once_flag, []() { Log::Register("posix", g_channel); });
}
