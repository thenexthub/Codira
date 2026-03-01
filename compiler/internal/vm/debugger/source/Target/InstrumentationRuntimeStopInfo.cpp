//===-- InstrumentationRuntimeStopInfo.cpp --------------------------------===//
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

#include "lldb/Target/InstrumentationRuntimeStopInfo.h"

#include "lldb/Core/Module.h"
#include "lldb/Target/InstrumentationRuntime.h"
#include "lldb/Target/Process.h"
#include "lldb/lldb-enumerations.h"
#include "lldb/lldb-private.h"

using namespace lldb;
using namespace lldb_private;

static bool IsStoppedInDarwinSanitizer(Thread &thread, Module &module) {
  return module.GetFileSpec().GetFilename().GetStringRef().starts_with(
      "libclang_rt.");
}

InstrumentationRuntimeStopInfo::InstrumentationRuntimeStopInfo(
    Thread &thread, std::string description,
    StructuredData::ObjectSP additional_data)
    : StopInfo(thread, 0) {
  m_extended_info = additional_data;
  m_description = description;
}

const char *InstrumentationRuntimeStopInfo::GetDescription() {
  return m_description.c_str();
}

StopInfoSP
InstrumentationRuntimeStopInfo::CreateStopReasonWithInstrumentationData(
    Thread &thread, std::string description,
    StructuredData::ObjectSP additionalData) {
  return StopInfoSP(
      new InstrumentationRuntimeStopInfo(thread, description, additionalData));
}

std::optional<uint32_t>
InstrumentationRuntimeStopInfo::GetSuggestedStackFrameIndex(
    bool inlined_stack) {
  ThreadSP thread_sp = GetThread();
  if (!thread_sp)
    return std::nullopt;

  // Defensive upper-bound of when we stop walking up the frames in
  // case we somehow ended up looking at an infinite recursion.
  constexpr size_t max_stack_depth = 128;

  // Start at parent frame.
  size_t stack_idx = 1;
  StackFrameSP most_relevant_frame_sp =
      thread_sp->GetStackFrameAtIndex(stack_idx);

  while (most_relevant_frame_sp && stack_idx <= max_stack_depth) {
    auto const &sc =
        most_relevant_frame_sp->GetSymbolContext(lldb::eSymbolContextModule);

    if (!sc.module_sp)
      return std::nullopt;

    // Found a frame outside of the sanitizer runtime libraries.
    // That's the one we want to display.
    if (!IsStoppedInDarwinSanitizer(*thread_sp, *sc.module_sp))
      return stack_idx;

    ++stack_idx;
    most_relevant_frame_sp = thread_sp->GetStackFrameAtIndex(stack_idx);
  }

  return stack_idx;
}
