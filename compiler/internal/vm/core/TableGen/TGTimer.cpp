//===- TGTimer.cpp - TableGen Timer implementation --------------*- C++ -*-===//
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
// Implement the tablegen timer class.
//
//===----------------------------------------------------------------------===//

#include "vm/core/TableGen/TGTimer.h"
using namespace vm::core;

// These functions implement the phase timing facility. Starting a timer
// when one is already running stops the running one.
void TGTimer::startTimer(StringRef Name) {
  if (!TimingGroup)
    return;
  if (LastTimer && LastTimer->isRunning()) {
    LastTimer->stopTimer();
    if (BackendTimer) {
      LastTimer->clear();
      BackendTimer = false;
    }
  }

  LastTimer = std::make_unique<Timer>("", Name, *TimingGroup);
  LastTimer->startTimer();
}

void TGTimer::stopTimer() {
  if (!TimingGroup)
    return;

  assert(LastTimer && "No phase timer was started");
  LastTimer->stopTimer();
}

void TGTimer::startBackendTimer(StringRef Name) {
  if (!TimingGroup)
    return;

  startTimer(Name);
  BackendTimer = true;
}

void TGTimer::stopBackendTimer() {
  if (!TimingGroup || !BackendTimer)
    return;
  stopTimer();
  BackendTimer = false;
}
