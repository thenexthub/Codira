//===-- ProcessWindowsLog.h -------------------------------------*- C++ -*-===//
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

#ifndef liblldb_ProcessWindowsLog_h_
#define liblldb_ProcessWindowsLog_h_

#include "lldb/Utility/Log.h"
#include "llvm/ADT/BitmaskEnum.h"

namespace lldb_private {

enum class WindowsLog : Log::MaskType {
  Breakpoints = Log::ChannelFlag<0>, // Log breakpoint operations
  Event = Log::ChannelFlag<1>,       // Low level debug events
  Exception = Log::ChannelFlag<2>,   // Log exceptions
  Memory = Log::ChannelFlag<3>,      // Log memory reads/writes calls
  Process = Log::ChannelFlag<4>,     // Log process operations
  Registers = Log::ChannelFlag<5>,   // Log register operations
  Step = Log::ChannelFlag<6>,        // Log step operations
  Thread = Log::ChannelFlag<7>,      // Log thread operations
  LLVM_MARK_AS_BITMASK_ENUM(Thread)
};
LLVM_ENABLE_BITMASK_ENUMS_IN_NAMESPACE();

class ProcessWindowsLog {
public:
  static void Initialize();
  static void Terminate();
};

template <> Log::Channel &LogChannelFor<WindowsLog>();
}

#endif // liblldb_ProcessWindowsLog_h_
