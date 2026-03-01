//===-- LogChannelDWARF.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_LOGCHANNELDWARF_H
#define LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_LOGCHANNELDWARF_H

#include "lldb/Utility/Log.h"
#include "llvm/ADT/BitmaskEnum.h"

namespace lldb_private {

enum class DWARFLog : Log::MaskType {
  DebugInfo = Log::ChannelFlag<0>,
  DebugLine = Log::ChannelFlag<1>,
  DebugMap = Log::ChannelFlag<2>,
  Lookups = Log::ChannelFlag<3>,
  TypeCompletion = Log::ChannelFlag<4>,
  SplitDwarf = Log::ChannelFlag<5>,
  LLVM_MARK_AS_BITMASK_ENUM(TypeCompletion)
};
LLVM_ENABLE_BITMASK_ENUMS_IN_NAMESPACE();

class LogChannelDWARF {
public:
  static void Initialize();
  static void Terminate();
};

template <> Log::Channel &LogChannelFor<DWARFLog>();
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_LOGCHANNELDWARF_H
