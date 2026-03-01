//===-- ProcessGDBRemoteLog.h -----------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_GDB_REMOTE_PROCESSGDBREMOTELOG_H
#define LLDB_SOURCE_PLUGINS_PROCESS_GDB_REMOTE_PROCESSGDBREMOTELOG_H

#include "lldb/Utility/Log.h"
#include "llvm/ADT/BitmaskEnum.h"

namespace lldb_private {
namespace process_gdb_remote {

enum class GDBRLog : Log::MaskType {
  Async = Log::ChannelFlag<0>,
  Breakpoints = Log::ChannelFlag<1>,
  Comm = Log::ChannelFlag<2>,
  Memory = Log::ChannelFlag<3>,          // Log memory reads/writes calls
  MemoryDataLong = Log::ChannelFlag<4>,  // Log all memory reads/writes bytes
  MemoryDataShort = Log::ChannelFlag<5>, // Log short memory reads/writes bytes
  Packets = Log::ChannelFlag<6>,
  Process = Log::ChannelFlag<7>,
  Step = Log::ChannelFlag<8>,
  Thread = Log::ChannelFlag<9>,
  Watchpoints = Log::ChannelFlag<10>,
  LLVM_MARK_AS_BITMASK_ENUM(Watchpoints)
};
LLVM_ENABLE_BITMASK_ENUMS_IN_NAMESPACE();

class ProcessGDBRemoteLog {
public:
  static void Initialize();
};

} // namespace process_gdb_remote

template <> Log::Channel &LogChannelFor<process_gdb_remote::GDBRLog>();

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_PROCESS_GDB_REMOTE_PROCESSGDBREMOTELOG_H
