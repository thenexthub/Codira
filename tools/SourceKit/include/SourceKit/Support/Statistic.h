//===--- Statistic.h - ------------------------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SOURCEKIT_SUPPORT_STATISTIC_H
#define LLVM_SOURCEKIT_SUPPORT_STATISTIC_H

#include "SourceKit/Support/UIdent.h"
#include <atomic>
#include <string>

namespace SourceKit {

struct Statistic {
  const UIdent name;
  const std::string description;
  std::atomic<int64_t> value = {0};

  Statistic(UIdent name, std::string description)
      : name(name), description(std::move(description)) {}

  int64_t operator++() {
    return 1 + value.fetch_add(1, std::memory_order_relaxed);
  }
  int64_t operator--() {
    return value.fetch_sub(1, std::memory_order_relaxed) - 1;
  }

  void updateMax(int64_t newValue) {
    int64_t prev = value.load(std::memory_order_relaxed);
    // Note: compare_exchange_weak updates 'prev' if it fails.
    while (newValue > prev && !value.compare_exchange_weak(
                                  prev, newValue, std::memory_order_relaxed)) {
    }
  }
};

} // namespace SourceKit

#endif // LLVM_SOURCEKIT_SUPPORT_STATISTIC_H
