//===-- MemoryHistory.h -----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_TARGET_MEMORYHISTORY_H
#define LLDB_TARGET_MEMORYHISTORY_H

#include <vector>

#include "lldb/Core/PluginInterface.h"
#include "lldb/lldb-private.h"
#include "lldb/lldb-types.h"

namespace lldb_private {

typedef std::vector<lldb::ThreadSP> HistoryThreads;

class MemoryHistory : public std::enable_shared_from_this<MemoryHistory>,
                      public PluginInterface {
public:
  static lldb::MemoryHistorySP FindPlugin(const lldb::ProcessSP process);

  virtual HistoryThreads GetHistoryThreads(lldb::addr_t address) = 0;
};

} // namespace lldb_private

#endif // LLDB_TARGET_MEMORYHISTORY_H
