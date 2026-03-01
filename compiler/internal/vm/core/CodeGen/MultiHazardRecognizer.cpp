//===- MultiHazardRecognizer.cpp - Scheduler Support ----------------------===//
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
// This file implements the MultiHazardRecognizer class, which is a wrapper
// for a set of ScheduleHazardRecognizer instances
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/MultiHazardRecognizer.h"
#include "vm/core/ADT/STLExtras.h"
#include <algorithm>
#include <functional>
#include <numeric>

using namespace vm::core;

void MultiHazardRecognizer::AddHazardRecognizer(
    std::unique_ptr<ScheduleHazardRecognizer> &&R) {
  MaxLookAhead = std::max(MaxLookAhead, R->getMaxLookAhead());
  Recognizers.push_back(std::move(R));
}

bool MultiHazardRecognizer::atIssueLimit() const {
  return toolchain::any_of(Recognizers,
                      std::mem_fn(&ScheduleHazardRecognizer::atIssueLimit));
}

ScheduleHazardRecognizer::HazardType
MultiHazardRecognizer::getHazardType(SUnit *SU, int Stalls) {
  for (auto &R : Recognizers) {
    auto res = R->getHazardType(SU, Stalls);
    if (res != NoHazard)
      return res;
  }
  return NoHazard;
}

void MultiHazardRecognizer::Reset() {
  for (auto &R : Recognizers)
    R->Reset();
}

void MultiHazardRecognizer::EmitInstruction(SUnit *SU) {
  for (auto &R : Recognizers)
    R->EmitInstruction(SU);
}

void MultiHazardRecognizer::EmitInstruction(MachineInstr *MI) {
  for (auto &R : Recognizers)
    R->EmitInstruction(MI);
}

unsigned MultiHazardRecognizer::PreEmitNoops(SUnit *SU) {
  auto MN = [=](unsigned a, std::unique_ptr<ScheduleHazardRecognizer> &R) {
    return std::max(a, R->PreEmitNoops(SU));
  };
  return std::accumulate(Recognizers.begin(), Recognizers.end(), 0u, MN);
}

unsigned MultiHazardRecognizer::PreEmitNoops(MachineInstr *MI) {
  auto MN = [=](unsigned a, std::unique_ptr<ScheduleHazardRecognizer> &R) {
    return std::max(a, R->PreEmitNoops(MI));
  };
  return std::accumulate(Recognizers.begin(), Recognizers.end(), 0u, MN);
}

bool MultiHazardRecognizer::ShouldPreferAnother(SUnit *SU) {
  auto SPA = [=](std::unique_ptr<ScheduleHazardRecognizer> &R) {
    return R->ShouldPreferAnother(SU);
  };
  return toolchain::any_of(Recognizers, SPA);
}

void MultiHazardRecognizer::AdvanceCycle() {
  for (auto &R : Recognizers)
    R->AdvanceCycle();
}

void MultiHazardRecognizer::RecedeCycle() {
  for (auto &R : Recognizers)
    R->RecedeCycle();
}

void MultiHazardRecognizer::EmitNoop() {
  for (auto &R : Recognizers)
    R->EmitNoop();
}
