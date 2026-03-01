//===-- SaveCoreOptions.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_OBJECTFILE_SaveCoreOPTIONS_H
#define LLDB_SOURCE_PLUGINS_OBJECTFILE_SaveCoreOPTIONS_H

#include "lldb/Target/CoreFileMemoryRanges.h"
#include "lldb/Target/ThreadCollection.h"
#include "lldb/Utility/FileSpec.h"
#include "lldb/Utility/RangeMap.h"

#include <optional>
#include <string>
#include <unordered_set>

using MemoryRanges = lldb_private::RangeVector<lldb::addr_t, lldb::addr_t>;

namespace lldb_private {

class SaveCoreOptions {
public:
  SaveCoreOptions() = default;
  ~SaveCoreOptions() = default;

  lldb_private::Status SetPluginName(const char *name);
  std::optional<std::string> GetPluginName() const;

  void SetStyle(lldb::SaveCoreStyle style);
  lldb::SaveCoreStyle GetStyle() const;

  void SetOutputFile(lldb_private::FileSpec file);
  const std::optional<lldb_private::FileSpec> GetOutputFile() const;

  Status SetProcess(lldb::ProcessSP process_sp);
  lldb::ProcessSP GetProcess() const { return m_process_sp; }

  Status AddThread(lldb::ThreadSP thread_sp);
  bool RemoveThread(lldb::ThreadSP thread_sp);
  bool ShouldThreadBeSaved(lldb::tid_t tid) const;
  bool HasSpecifiedThreads() const;

  Status EnsureValidConfiguration() const;
  const MemoryRanges &GetCoreFileMemoryRanges() const;

  void AddMemoryRegionToSave(const lldb_private::MemoryRegionInfo &region);

  llvm::Expected<lldb_private::CoreFileMemoryRanges> GetMemoryRegionsToSave();
  lldb_private::ThreadCollection::collection GetThreadsToSave() const;

  llvm::Expected<uint64_t> GetCurrentSizeInBytes();

  void Clear();

private:
  void ClearProcessSpecificData();

  std::optional<std::string> m_plugin_name;
  std::optional<lldb_private::FileSpec> m_file;
  std::optional<lldb::SaveCoreStyle> m_style;
  lldb::ProcessSP m_process_sp;
  std::unordered_set<lldb::tid_t> m_threads_to_save;
  MemoryRanges m_regions_to_save;
};
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_OBJECTFILE_SAVECOREOPTIONS_H
