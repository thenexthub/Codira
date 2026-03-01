//===- BuryPointer.cpp - Memory Manipulation/Leak ---------------*- C++ -*-===//
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

#include "vm/core/Support/BuryPointer.h"
#include "vm/core/Support/Compiler.h"
#include <atomic>

namespace vm::core {

void BuryPointer(const void *Ptr) {
  // This function may be called only a small fixed amount of times per each
  // invocation, otherwise we do actually have a leak which we want to report.
  // If this function is called more than kGraveYardMaxSize times, the pointers
  // will not be properly buried and a leak detector will report a leak, which
  // is what we want in such case.
  static const size_t kGraveYardMaxSize = 16;
  LLVM_ATTRIBUTE_USED static const void *GraveYard[kGraveYardMaxSize];
  static std::atomic<unsigned> GraveYardSize;
  unsigned Idx = GraveYardSize++;
  if (Idx >= kGraveYardMaxSize)
    return;
  GraveYard[Idx] = Ptr;
}

} // namespace vm::core
