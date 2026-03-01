//===-- OperatingSystem.cpp -----------------------------------------------===//
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

#include "lldb/Target/OperatingSystem.h"
#include "lldb/Core/PluginManager.h"
#include "lldb/Target/Thread.h"

using namespace lldb;
using namespace lldb_private;

OperatingSystem *OperatingSystem::FindPlugin(Process *process,
                                             const char *plugin_name) {
  OperatingSystemCreateInstance create_callback = nullptr;
  if (plugin_name) {
    create_callback =
        PluginManager::GetOperatingSystemCreateCallbackForPluginName(
            plugin_name);
    if (create_callback) {
      std::unique_ptr<OperatingSystem> instance_up(
          create_callback(process, true));
      if (instance_up)
        return instance_up.release();
    }
  } else {
    for (uint32_t idx = 0;
         (create_callback =
              PluginManager::GetOperatingSystemCreateCallbackAtIndex(idx)) !=
         nullptr;
         ++idx) {
      std::unique_ptr<OperatingSystem> instance_up(
          create_callback(process, false));
      if (instance_up)
        return instance_up.release();
    }
  }
  return nullptr;
}

OperatingSystem::OperatingSystem(Process *process) : m_process(process) {}

bool OperatingSystem::IsOperatingSystemPluginThread(
    const lldb::ThreadSP &thread_sp) {
  if (thread_sp)
    return thread_sp->IsOperatingSystemPluginThread();
  return false;
}
