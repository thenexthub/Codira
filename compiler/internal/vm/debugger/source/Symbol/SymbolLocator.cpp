//===-- symbolLocator.cpp -------------------------------------------------===//
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

#include "lldb/Symbol/SymbolLocator.h"

#include "lldb/Core/Debugger.h"
#include "lldb/Core/PluginManager.h"
#include "lldb/Host/Host.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/Support/ThreadPool.h"

using namespace lldb;
using namespace lldb_private;

void SymbolLocator::DownloadSymbolFileAsync(const UUID &uuid) {
  static llvm::SmallSet<UUID, 8> g_seen_uuids;
  static std::mutex g_mutex;

  auto lookup = [=]() {
    {
      std::lock_guard<std::mutex> guard(g_mutex);
      if (g_seen_uuids.count(uuid))
        return;
      g_seen_uuids.insert(uuid);
    }

    Status error;
    ModuleSpec module_spec;
    module_spec.GetUUID() = uuid;
    if (!PluginManager::DownloadObjectAndSymbolFile(module_spec, error,
                                                    /*force_lookup=*/true,
                                                    /*copy_executable=*/true))
      return;

    if (error.Fail())
      return;

    Debugger::ReportSymbolChange(module_spec);
  };

  switch (ModuleList::GetGlobalModuleListProperties().GetSymbolAutoDownload()) {
  case eSymbolDownloadOff:
    break;
  case eSymbolDownloadBackground:
    Debugger::GetThreadPool().async(lookup);
    break;
  case eSymbolDownloadForeground:
    lookup();
    break;
  };
}
