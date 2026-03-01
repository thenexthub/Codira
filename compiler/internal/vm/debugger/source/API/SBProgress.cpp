//===-- SBProgress.cpp --------------------------------------------------*-===//
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

#include "lldb/API/SBProgress.h"
#include "lldb/Core/Progress.h"
#include "lldb/Utility/Instrumentation.h"

using namespace lldb;

SBProgress::SBProgress(const char *title, const char *details,
                       SBDebugger &debugger) {
  LLDB_INSTRUMENT_VA(this, title, details, debugger);

  m_opaque_up = std::make_unique<lldb_private::Progress>(
      title, details, /*total=*/std::nullopt, debugger.get(),
      /*minimum_report_time=*/std::nullopt,
      lldb_private::Progress::Origin::eExternal);
}

SBProgress::SBProgress(const char *title, const char *details,
                       uint64_t total_units, SBDebugger &debugger) {
  LLDB_INSTRUMENT_VA(this, title, details, total_units, debugger);

  m_opaque_up = std::make_unique<lldb_private::Progress>(
      title, details, total_units, debugger.get(),
      /*minimum_report_time=*/std::nullopt,
      lldb_private::Progress::Origin::eExternal);
}

SBProgress::SBProgress(SBProgress &&rhs)
    : m_opaque_up(std::move(rhs.m_opaque_up)) {}

SBProgress::~SBProgress() = default;

void SBProgress::Increment(uint64_t amount, const char *description) {
  LLDB_INSTRUMENT_VA(amount, description);

  if (!m_opaque_up)
    return;

  std::optional<std::string> description_opt;
  if (description && description[0])
    description_opt = description;
  m_opaque_up->Increment(amount, std::move(description_opt));
}

void SBProgress::Finalize() {
  // The lldb_private::Progress object is designed to be RAII and send the end
  // progress event when it gets destroyed. So force our contained object to be
  // destroyed and send the progress end event. Clearing this object also allows
  // all other methods to quickly return without doing any work if they are
  // called after this method.
  m_opaque_up.reset();
}

lldb_private::Progress &SBProgress::ref() const { return *m_opaque_up; }
