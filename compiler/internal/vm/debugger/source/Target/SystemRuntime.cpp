//===-- SystemRuntime.cpp -------------------------------------------------===//
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

#include "lldb/Target/SystemRuntime.h"
#include "lldb/Core/PluginManager.h"
#include "lldb/Target/Process.h"
#include "lldb/lldb-private.h"

using namespace lldb;
using namespace lldb_private;

SystemRuntime *SystemRuntime::FindPlugin(Process *process) {
  SystemRuntimeCreateInstance create_callback = nullptr;
  for (uint32_t idx = 0;
       (create_callback = PluginManager::GetSystemRuntimeCreateCallbackAtIndex(
            idx)) != nullptr;
       ++idx) {
    std::unique_ptr<SystemRuntime> instance_up(create_callback(process));
    if (instance_up)
      return instance_up.release();
  }
  return nullptr;
}

SystemRuntime::SystemRuntime(Process *process) : Runtime(process), m_types() {}

SystemRuntime::~SystemRuntime() = default;

void SystemRuntime::DidAttach() {}

void SystemRuntime::DidLaunch() {}

void SystemRuntime::Detach() {}

void SystemRuntime::ModulesDidLoad(const ModuleList &module_list) {}

const std::vector<ConstString> &SystemRuntime::GetExtendedBacktraceTypes() {
  return m_types;
}

ThreadSP SystemRuntime::GetExtendedBacktraceThread(ThreadSP thread,
                                                   ConstString type) {
  return ThreadSP();
}
