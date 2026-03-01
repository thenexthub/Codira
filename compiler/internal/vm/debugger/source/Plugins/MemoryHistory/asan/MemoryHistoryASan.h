//===-- MemoryHistoryASan.h -------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_MEMORYHISTORY_ASAN_MEMORYHISTORYASAN_H
#define LLDB_SOURCE_PLUGINS_MEMORYHISTORY_ASAN_MEMORYHISTORYASAN_H

#include "lldb/Target/ABI.h"
#include "lldb/Target/MemoryHistory.h"
#include "lldb/Target/Process.h"
#include "lldb/lldb-private.h"

namespace lldb_private {

class MemoryHistoryASan : public lldb_private::MemoryHistory {
public:
  ~MemoryHistoryASan() override = default;

  static lldb::MemoryHistorySP
  CreateInstance(const lldb::ProcessSP &process_sp);

  static void Initialize();

  static void Terminate();

  static llvm::StringRef GetPluginNameStatic() { return "asan"; }

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

  lldb_private::HistoryThreads GetHistoryThreads(lldb::addr_t address) override;

private:
  MemoryHistoryASan(const lldb::ProcessSP &process_sp);

  lldb::ProcessWP m_process_wp;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_MEMORYHISTORY_ASAN_MEMORYHISTORYASAN_H
