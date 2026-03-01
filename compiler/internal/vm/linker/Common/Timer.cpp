//===- Timer.cpp ----------------------------------------------------------===//
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

#include "lld/Common/Timer.h"
#include "lld/Common/ErrorHandler.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/Format.h"
#include <ratio>

using namespace lld;
using namespace llvm;

ScopedTimer::ScopedTimer(Timer &t) : t(&t) {
  startTime = std::chrono::high_resolution_clock::now();
}

void ScopedTimer::stop() {
  if (!t)
    return;
  t->addToTotal(std::chrono::high_resolution_clock::now() - startTime);
  t = nullptr;
}

ScopedTimer::~ScopedTimer() { stop(); }

Timer::Timer(llvm::StringRef name) : total(0), name(std::string(name)) {}
Timer::Timer(llvm::StringRef name, Timer &parent)
    : total(0), name(std::string(name)) {
  parent.children.push_back(this);
}

void Timer::print() {
  double totalDuration = static_cast<double>(millis());

  // We want to print the grand total under all the intermediate phases, so we
  // print all children first, then print the total under that.
  for (const auto &child : children)
    if (child->total > 0)
      child->print(1, totalDuration);

  message(std::string(50, '-'));

  print(0, millis(), false);
}

double Timer::millis() const {
  return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
             std::chrono::nanoseconds(total))
      .count();
}

void Timer::print(int depth, double totalDuration, bool recurse) const {
  double p = 100.0 * millis() / totalDuration;

  SmallString<32> str;
  llvm::raw_svector_ostream stream(str);
  std::string s = std::string(depth * 2, ' ') + name + std::string(":");
  stream << format("%-30s%7d ms (%5.1f%%)", s.c_str(), (int)millis(), p);

  message(str);

  if (recurse) {
    for (const auto &child : children)
      if (child->total > 0)
        child->print(depth + 1, totalDuration);
  }
}
