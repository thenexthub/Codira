//===-- LogChannelDWARF.cpp -----------------------------------------------===//
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

#include "LogChannelDWARF.h"

using namespace lldb_private;

static constexpr Log::Category g_categories[] = {
    {{"comp"},
     {"log struct/union/class type completions"},
     DWARFLog::TypeCompletion},
    {{"info"}, {"log the parsing of .debug_info"}, DWARFLog::DebugInfo},
    {{"line"}, {"log the parsing of .debug_line"}, DWARFLog::DebugLine},
    {{"lookups"},
     {"log any lookups that happen by name, regex, or address"},
     DWARFLog::Lookups},
    {{"map"},
     {"log insertions of object files into DWARF debug maps"},
     DWARFLog::DebugMap},
    {{"split"}, {"log split DWARF related activities"}, DWARFLog::SplitDwarf},
};

static Log::Channel g_channel(g_categories, DWARFLog::DebugInfo);

template <> Log::Channel &lldb_private::LogChannelFor<DWARFLog>() {
  return g_channel;
}

void LogChannelDWARF::Initialize() {
  Log::Register("dwarf", g_channel);
}

void LogChannelDWARF::Terminate() { Log::Unregister("dwarf"); }
