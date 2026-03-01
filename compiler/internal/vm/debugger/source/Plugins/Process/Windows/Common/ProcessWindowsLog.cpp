//===-- ProcessWindowsLog.cpp ---------------------------------------------===//
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

#include "ProcessWindowsLog.h"

using namespace lldb_private;

static constexpr Log::Category g_categories[] = {
    {{"break"}, {"log breakpoints"}, WindowsLog::Breakpoints},
    {{"event"}, {"log low level debugger events"}, WindowsLog::Event},
    {{"exception"}, {"log exception information"}, WindowsLog::Exception},
    {{"memory"}, {"log memory reads and writes"}, WindowsLog::Memory},
    {{"process"}, {"log process events and activities"}, WindowsLog::Process},
    {{"registers"}, {"log register read/writes"}, WindowsLog::Registers},
    {{"step"}, {"log step related activities"}, WindowsLog::Step},
    {{"thread"}, {"log thread events and activities"}, WindowsLog::Thread},
};

static Log::Channel g_channel(g_categories, WindowsLog::Process);

template <> Log::Channel &lldb_private::LogChannelFor<WindowsLog>() {
  return g_channel;
}

void ProcessWindowsLog::Initialize() {
  static llvm::once_flag g_once_flag;
  llvm::call_once(g_once_flag, []() { Log::Register("windows", g_channel); });
}

void ProcessWindowsLog::Terminate() {}









