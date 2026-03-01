//===- Timer.h ----------------------------------------------*- C++ -*-===//
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

#ifndef LLD_COMMON_TIMER_H
#define LLD_COMMON_TIMER_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringRef.h"
#include <assert.h>
#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <vector>

namespace lld {

class Timer;

struct ScopedTimer {
  explicit ScopedTimer(Timer &t);

  ~ScopedTimer();

  void stop();

  std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

  Timer *t = nullptr;
};

class Timer {
public:
  Timer(llvm::StringRef name, Timer &parent);

  // Creates the root timer.
  explicit Timer(llvm::StringRef name);

  void addToTotal(std::chrono::nanoseconds time) { total += time.count(); }
  void print();

  double millis() const;

private:
  void print(int depth, double totalDuration, bool recurse = true) const;

  std::atomic<std::chrono::nanoseconds::rep> total;
  std::vector<Timer *> children;
  std::string name;
};

} // namespace lld

#endif
