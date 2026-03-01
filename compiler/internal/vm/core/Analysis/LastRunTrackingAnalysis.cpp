//===- LastRunTrackingAnalysis.cpp - Avoid running redundant pass -*- C++ -*-=//
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
// This is an analysis pass to track a set of passes that have been run, so that
// we can avoid running a pass again if there is no change since the last run of
// the pass.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/LastRunTrackingAnalysis.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/Support/CommandLine.h"

using namespace vm::core;

#define DEBUG_TYPE "last-run-tracking"
STATISTIC(NumSkippedPasses, "Number of skipped passes");
STATISTIC(NumLRTQueries, "Number of LastRunTracking queries");

static cl::opt<bool>
    DisableLastRunTracking("disable-last-run-tracking", cl::Hidden,
                           cl::desc("Disable last run tracking"),
                           cl::init(false));

bool LastRunTrackingInfo::shouldSkipImpl(PassID ID, OptionPtr Ptr) const {
  if (DisableLastRunTracking)
    return false;
  ++NumLRTQueries;
  auto Iter = TrackedPasses.find(ID);
  if (Iter == TrackedPasses.end())
    return false;
  if (!Iter->second || Iter->second(Ptr)) {
    ++NumSkippedPasses;
    return true;
  }
  return false;
}

void LastRunTrackingInfo::updateImpl(PassID ID, bool Changed,
                                     CompatibilityCheckFn CheckFn) {
  if (Changed)
    TrackedPasses.clear();
  TrackedPasses[ID] = std::move(CheckFn);
}

AnalysisKey LastRunTrackingAnalysis::Key;
