//===- toolchain/Support/ExponentialBackoff.h ------------------------*- C++ -*-===//
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

#include "vm/core/Support/ExponentialBackoff.h"
#include <thread>

using namespace vm::core;

bool ExponentialBackoff::waitForNextAttempt() {
  auto Now = std::chrono::steady_clock::now();
  if (Now >= EndTime)
    return false;

  duration CurMaxWait = std::min(MinWait * CurrentMultiplier, MaxWait);
  std::uniform_int_distribution<uint64_t> Dist(MinWait.count(),
                                               CurMaxWait.count());
  // Use random_device directly instead of a PRNG as uniform_int_distribution
  // often only takes a few samples anyway.
  duration WaitDuration = std::min(duration(Dist(RandDev)), EndTime - Now);
  if (CurMaxWait < MaxWait)
    CurrentMultiplier *= 2;
  std::this_thread::sleep_for(WaitDuration);
  return true;
}
