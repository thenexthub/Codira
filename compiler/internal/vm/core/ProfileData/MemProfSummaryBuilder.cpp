//=-- MemProfSummaryBuilder.cpp - MemProf summary building ---------------=//
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
//
// This file contains MemProf summary builder.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ProfileData/MemProfSummaryBuilder.h"
#include "vm/core/ProfileData/MemProfCommon.h"

using namespace vm::core;
using namespace vm::core::memprof;

std::unique_ptr<MemProfSummary> MemProfSummaryBuilder::getSummary() {
  return std::make_unique<MemProfSummary>(NumContexts, NumColdContexts,
                                          NumHotContexts, MaxColdTotalSize,
                                          MaxWarmTotalSize, MaxHotTotalSize);
}

void MemProfSummaryBuilder::addRecord(uint64_t CSId,
                                      const PortableMemInfoBlock &Info) {
  auto I = Contexts.insert(CSId);
  if (!I.second)
    return;
  NumContexts++;
  auto AllocType = getAllocType(Info.getTotalLifetimeAccessDensity(),
                                Info.getAllocCount(), Info.getTotalLifetime());
  auto TotalSize = Info.getTotalSize();
  switch (AllocType) {
  case AllocationType::Cold:
    NumColdContexts++;
    if (TotalSize > MaxColdTotalSize)
      MaxColdTotalSize = TotalSize;
    break;
  case AllocationType::NotCold:
    if (TotalSize > MaxWarmTotalSize)
      MaxWarmTotalSize = TotalSize;
    break;
  case AllocationType::Hot:
    NumHotContexts++;
    if (TotalSize > MaxHotTotalSize)
      MaxHotTotalSize = TotalSize;
    break;
  default:
    assert(false);
  }
}

void MemProfSummaryBuilder::addRecord(const IndexedMemProfRecord &Record) {
  for (auto &Alloc : Record.AllocSites)
    addRecord(Alloc.CSId, Alloc.Info);
}

void MemProfSummaryBuilder::addRecord(const MemProfRecord &Record) {
  for (auto &Alloc : Record.AllocSites)
    addRecord(computeFullStackId(Alloc.CallStack), Alloc.Info);
}
