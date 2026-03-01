//===----------------------------------------------------------------------===//
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

#include "lldb/Target/SyntheticFrameProvider.h"
#include "lldb/Core/PluginManager.h"
#include "lldb/Interpreter/Interfaces/ScriptedFrameProviderInterface.h"
#include "lldb/Target/Thread.h"
#include "lldb/Utility/LLDBLog.h"
#include "lldb/Utility/Log.h"
#include "lldb/Utility/Status.h"
#include "lldb/Utility/Stream.h"

using namespace lldb;
using namespace lldb_private;

SyntheticFrameProvider::SyntheticFrameProvider(StackFrameListSP input_frames)
    : m_input_frames(std::move(input_frames)) {}

SyntheticFrameProvider::~SyntheticFrameProvider() = default;

void ScriptedFrameProviderDescriptor::Dump(Stream *s) const {
  if (!s)
    return;

  s->Format("  ID: {0:x}\n", GetID());
  s->Printf("  Name: %s\n", GetName().str().c_str());

  std::string description = GetDescription();
  if (!description.empty())
    s->Printf("  Description: %s\n", description.c_str());

  // Show priority information.
  std::optional<uint32_t> priority = GetPriority();
  if (priority.has_value())
    s->Printf("  Priority: %u\n", *priority);
  else
    s->PutCString("  Priority: Default (no priority specified)\n");

  // Show thread filter information.
  if (thread_specs.empty()) {
    s->PutCString("  Thread Filter: (applies to all threads)\n");
  } else {
    s->Printf("  Thread Filter: %zu specification(s)\n", thread_specs.size());
    for (size_t i = 0; i < thread_specs.size(); ++i) {
      const ThreadSpec &spec = thread_specs[i];
      s->Printf("    [%zu] ", i);
      spec.GetDescription(s, lldb::eDescriptionLevelVerbose);
      s->PutChar('\n');
    }
  }
}

uint32_t ScriptedFrameProviderDescriptor::GetID() const {
  if (!scripted_metadata_sp)
    return 0;

  return scripted_metadata_sp->GetID();
}

std::string ScriptedFrameProviderDescriptor::GetDescription() const {
  // If we have an interface, call get_description() to fetch it.
  if (interface_sp && scripted_metadata_sp)
    return interface_sp->GetDescription(scripted_metadata_sp->GetClassName());
  return {};
}

std::optional<uint32_t> ScriptedFrameProviderDescriptor::GetPriority() const {
  // If we have an interface, call get_priority() to fetch it.
  if (interface_sp && scripted_metadata_sp)
    return interface_sp->GetPriority(scripted_metadata_sp->GetClassName());
  return std::nullopt;
}

llvm::Expected<SyntheticFrameProviderSP> SyntheticFrameProvider::CreateInstance(
    StackFrameListSP input_frames,
    const ScriptedFrameProviderDescriptor &descriptor) {
  if (!input_frames)
    return llvm::createStringError(
        "cannot create synthetic frame provider: invalid input frames");

  // Iterate through all registered ScriptedFrameProvider plugins.
  ScriptedFrameProviderCreateInstance create_callback = nullptr;
  for (uint32_t idx = 0;
       (create_callback =
            PluginManager::GetScriptedFrameProviderCreateCallbackAtIndex(
                idx)) != nullptr;
       ++idx) {
    auto provider_or_err = create_callback(input_frames, descriptor);
    if (!provider_or_err) {
      LLDB_LOG_ERROR(GetLog(LLDBLog::Target), provider_or_err.takeError(),
                     "Failed to create synthetic frame provider: {0}");
      continue;
    }

    if (auto frame_provider_up = std::move(*provider_or_err))
      return std::move(frame_provider_up);
  }

  return llvm::createStringError(
      "cannot create synthetic frame provider: no suitable plugin found");
}

llvm::Expected<SyntheticFrameProviderSP> SyntheticFrameProvider::CreateInstance(
    StackFrameListSP input_frames, llvm::StringRef plugin_name,
    const std::vector<ThreadSpec> &thread_specs) {
  if (!input_frames)
    return llvm::createStringError(
        "cannot create synthetic frame provider: invalid input frames");

  // Look up the specific C++ plugin by name.
  SyntheticFrameProviderCreateInstance create_callback =
      PluginManager::GetSyntheticFrameProviderCreateCallbackForPluginName(
          plugin_name);

  if (!create_callback)
    return llvm::createStringError(
        "cannot create synthetic frame provider: C++ plugin '%s' not found",
        plugin_name.str().c_str());

  auto provider_or_err = create_callback(input_frames, thread_specs);
  if (!provider_or_err)
    return provider_or_err.takeError();

  if (auto frame_provider_sp = std::move(*provider_or_err))
    return std::move(frame_provider_sp);

  return llvm::createStringError(
      "cannot create synthetic frame provider: C++ plugin '%s' returned null",
      plugin_name.str().c_str());
}
