//===-- SBStatisticsOptions.cpp -------------------------------------------===//
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

#include "lldb/API/SBStatisticsOptions.h"
#include "lldb/Target/Statistics.h"
#include "lldb/Utility/Instrumentation.h"

#include "Utils.h"

using namespace lldb;
using namespace lldb_private;

SBStatisticsOptions::SBStatisticsOptions()
    : m_opaque_up(new StatisticsOptions()) {
  LLDB_INSTRUMENT_VA(this);
}

SBStatisticsOptions::SBStatisticsOptions(const SBStatisticsOptions &rhs) {
  LLDB_INSTRUMENT_VA(this, rhs);

  m_opaque_up = clone(rhs.m_opaque_up);
}

SBStatisticsOptions::~SBStatisticsOptions() = default;

const SBStatisticsOptions &
SBStatisticsOptions::operator=(const SBStatisticsOptions &rhs) {
  LLDB_INSTRUMENT_VA(this, rhs);

  if (this != &rhs)
    m_opaque_up = clone(rhs.m_opaque_up);
  return *this;
}

void SBStatisticsOptions::SetSummaryOnly(bool b) {
  m_opaque_up->SetSummaryOnly(b);
}

bool SBStatisticsOptions::GetSummaryOnly() {
  return m_opaque_up->GetSummaryOnly();
}

void SBStatisticsOptions::SetIncludeTargets(bool b) {
  m_opaque_up->SetIncludeTargets(b);
}

bool SBStatisticsOptions::GetIncludeTargets() const {
  return m_opaque_up->GetIncludeTargets();
}

void SBStatisticsOptions::SetIncludeModules(bool b) {
  m_opaque_up->SetIncludeModules(b);
}

bool SBStatisticsOptions::GetIncludeModules() const {
  return m_opaque_up->GetIncludeModules();
}

void SBStatisticsOptions::SetIncludeTranscript(bool b) {
  m_opaque_up->SetIncludeTranscript(b);
}

bool SBStatisticsOptions::GetIncludeTranscript() const {
  return m_opaque_up->GetIncludeTranscript();
}

void SBStatisticsOptions::SetReportAllAvailableDebugInfo(bool b) {
  m_opaque_up->SetLoadAllDebugInfo(b);
}

bool SBStatisticsOptions::GetReportAllAvailableDebugInfo() {
  return m_opaque_up->GetLoadAllDebugInfo();
}

const lldb_private::StatisticsOptions &SBStatisticsOptions::ref() const {
  return *m_opaque_up;
}
