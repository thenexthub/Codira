//===-- InstrumentationRuntime.cpp ----------------------------------------===//
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
//===---------------------------------------------------------------------===//

#include "lldb/Target/InstrumentationRuntime.h"
#include "lldb/Core/Module.h"
#include "lldb/Core/ModuleList.h"
#include "lldb/Core/PluginManager.h"
#include "lldb/Target/Process.h"
#include "lldb/Utility/RegularExpression.h"
#include "lldb/lldb-private.h"

using namespace lldb;
using namespace lldb_private;

void InstrumentationRuntime::ModulesDidLoad(
    lldb_private::ModuleList &module_list, lldb_private::Process *process,
    InstrumentationRuntimeCollection &runtimes) {
  InstrumentationRuntimeCreateInstance create_callback = nullptr;
  InstrumentationRuntimeGetType get_type_callback;
  for (uint32_t idx = 0;; ++idx) {
    create_callback =
        PluginManager::GetInstrumentationRuntimeCreateCallbackAtIndex(idx);
    if (create_callback == nullptr)
      break;
    get_type_callback =
        PluginManager::GetInstrumentationRuntimeGetTypeCallbackAtIndex(idx);
    InstrumentationRuntimeType type = get_type_callback();

    InstrumentationRuntimeCollection::iterator pos;
    pos = runtimes.find(type);
    if (pos == runtimes.end()) {
      runtimes[type] = create_callback(process->shared_from_this());
    }
  }
}

void InstrumentationRuntime::ModulesDidLoad(
    lldb_private::ModuleList &module_list) {
  if (IsActive())
    return;

  if (GetRuntimeModuleSP()) {
    Activate();
    return;
  }

  module_list.ForEach([this](const lldb::ModuleSP module_sp) {
    const FileSpec &file_spec = module_sp->GetFileSpec();
    if (!file_spec)
      return IterationAction::Continue;

    const RegularExpression &runtime_regex = GetPatternForRuntimeLibrary();
    if (MatchAllModules() ||
        runtime_regex.Execute(file_spec.GetFilename().GetCString()) ||
        module_sp->IsExecutable()) {
      if (CheckIfRuntimeIsValid(module_sp)) {
        SetRuntimeModuleSP(module_sp);
        Activate();
        if (!IsActive())
          SetRuntimeModuleSP({}); // Don't cache module if activation failed.
        return IterationAction::Stop;
      }
    }

    return IterationAction::Continue;
  });
}

lldb::ThreadCollectionSP
InstrumentationRuntime::GetBacktracesFromExtendedStopInfo(
    StructuredData::ObjectSP info) {
  return std::make_shared<ThreadCollection>();
}
