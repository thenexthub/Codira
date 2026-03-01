//===-- LLDBLog.h -----------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_UTILITY_LLDBLOG_H
#define LLDB_UTILITY_LLDBLOG_H

#include "lldb/Utility/Log.h"
#include "llvm/ADT/BitmaskEnum.h"
#include <cstdint>

namespace lldb_private {

enum class LLDBLog : Log::MaskType {
  API = Log::ChannelFlag<0>,
  AST = Log::ChannelFlag<1>,
  Breakpoints = Log::ChannelFlag<2>,
  Commands = Log::ChannelFlag<3>,
  Communication = Log::ChannelFlag<4>,
  Connection = Log::ChannelFlag<5>,
  DataFormatters = Log::ChannelFlag<6>,
  Demangle = Log::ChannelFlag<7>,
  DynamicLoader = Log::ChannelFlag<8>,
  Events = Log::ChannelFlag<9>,
  Expressions = Log::ChannelFlag<10>,
  Host = Log::ChannelFlag<11>,
  JITLoader = Log::ChannelFlag<12>,
  Language = Log::ChannelFlag<13>,
  MMap = Log::ChannelFlag<14>,
  Modules = Log::ChannelFlag<15>,
  Object = Log::ChannelFlag<16>,
  OS = Log::ChannelFlag<17>,
  Platform = Log::ChannelFlag<18>,
  Process = Log::ChannelFlag<19>,
  Script = Log::ChannelFlag<20>,
  State = Log::ChannelFlag<21>,
  Step = Log::ChannelFlag<22>,
  Symbols = Log::ChannelFlag<23>,
  SystemRuntime = Log::ChannelFlag<24>,
  Target = Log::ChannelFlag<25>,
  Temporary = Log::ChannelFlag<26>,
  Thread = Log::ChannelFlag<27>,
  Types = Log::ChannelFlag<28>,
  Unwind = Log::ChannelFlag<29>,
  Watchpoints = Log::ChannelFlag<30>,
  OnDemand = Log::ChannelFlag<31>,
  Source = Log::ChannelFlag<32>,
  Disassembler = Log::ChannelFlag<33>,
  InstrumentationRuntime = Log::ChannelFlag<34>,
  LLVM_MARK_AS_BITMASK_ENUM(InstrumentationRuntime),
};

LLVM_ENABLE_BITMASK_ENUMS_IN_NAMESPACE();

void InitializeLldbChannel();

template <> Log::Channel &LogChannelFor<LLDBLog>();
} // namespace lldb_private

#endif // LLDB_UTILITY_LLDBLOG_H
