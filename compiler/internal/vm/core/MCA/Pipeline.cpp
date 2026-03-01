//===--------------------- Pipeline.cpp -------------------------*- C++ -*-===//
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
/// \file
///
/// This file implements an ordered container of stages that simulate the
/// pipeline of a hardware backend.
///
//===----------------------------------------------------------------------===//

#include "vm/core/MCA/Pipeline.h"
#include "vm/core/MCA/HWEventListener.h"
#include "vm/core/Support/Debug.h"

namespace vm::core {
namespace mca {

#define DEBUG_TYPE "toolchain-mca"

void Pipeline::addEventListener(HWEventListener *Listener) {
  if (Listener)
    Listeners.insert(Listener);
  for (auto &S : Stages)
    S->addListener(Listener);
}

bool Pipeline::hasWorkToProcess() {
  return any_of(Stages, [](const std::unique_ptr<Stage> &S) {
    return S->hasWorkToComplete();
  });
}

Expected<unsigned> Pipeline::run() {
  assert(!Stages.empty() && "Unexpected empty pipeline found!");

  do {
    if (!isPaused())
      notifyCycleBegin();
    if (Error Err = runCycle())
      return std::move(Err);
    notifyCycleEnd();
    ++Cycles;
  } while (hasWorkToProcess());

  return Cycles;
}

Error Pipeline::runCycle() {
  Error Err = ErrorSuccess();
  // Update stages before we start processing new instructions.
  for (auto I = Stages.rbegin(), E = Stages.rend(); I != E && !Err; ++I) {
    const std::unique_ptr<Stage> &S = *I;
    if (isPaused())
      Err = S->cycleResume();
    else
      Err = S->cycleStart();
  }

  CurrentState = State::Started;

  // Now fetch and execute new instructions.
  InstRef IR;
  Stage &FirstStage = *Stages[0];
  while (!Err && FirstStage.isAvailable(IR))
    Err = FirstStage.execute(IR);

  if (Err.isA<InstStreamPause>()) {
    CurrentState = State::Paused;
    return Err;
  }

  // Update stages in preparation for a new cycle.
  for (const std::unique_ptr<Stage> &S : Stages) {
    Err = S->cycleEnd();
    if (Err)
      break;
  }

  return Err;
}

void Pipeline::appendStage(std::unique_ptr<Stage> S) {
  assert(S && "Invalid null stage in input!");
  if (!Stages.empty()) {
    Stage *Last = Stages.back().get();
    Last->setNextInSequence(S.get());
  }

  Stages.push_back(std::move(S));
}

void Pipeline::notifyCycleBegin() {
  LLVM_DEBUG(dbgs() << "\n[E] Cycle begin: " << Cycles << '\n');
  for (HWEventListener *Listener : Listeners)
    Listener->onCycleBegin();
}

void Pipeline::notifyCycleEnd() {
  LLVM_DEBUG(dbgs() << "[E] Cycle end: " << Cycles << "\n");
  for (HWEventListener *Listener : Listeners)
    Listener->onCycleEnd();
}
} // namespace mca.
} // namespace vm::core
