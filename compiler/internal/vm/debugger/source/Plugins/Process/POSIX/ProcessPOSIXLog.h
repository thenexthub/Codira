//===-- ProcessPOSIXLog.h -----------------------------------------*- C++
//-*-===//
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

#ifndef liblldb_ProcessPOSIXLog_h_
#define liblldb_ProcessPOSIXLog_h_

#include "lldb/Utility/Log.h"
#include "llvm/ADT/BitmaskEnum.h"

namespace lldb_private {

enum class POSIXLog : Log::MaskType {
  Breakpoints = Log::ChannelFlag<0>,
  Memory = Log::ChannelFlag<1>,
  Process = Log::ChannelFlag<2>,
  Ptrace = Log::ChannelFlag<3>,
  Registers = Log::ChannelFlag<4>,
  Thread = Log::ChannelFlag<5>,
  Watchpoints = Log::ChannelFlag<6>,
  Trace = Log::ChannelFlag<7>,
  LLVM_MARK_AS_BITMASK_ENUM(Trace)
};
LLVM_ENABLE_BITMASK_ENUMS_IN_NAMESPACE();

class ProcessPOSIXLog {
public:
  static void Initialize();
};

template <> Log::Channel &LogChannelFor<POSIXLog>();
} // namespace lldb_private

#endif // liblldb_ProcessPOSIXLog_h_
