//===-------------------- IncrementalSourceMgr.cpp ------------------------===//
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
///
/// \file
/// This file defines some implementations for IncrementalSourceMgr.
///
//===----------------------------------------------------------------------===//

#include "vm/core/MCA/IncrementalSourceMgr.h"
#ifndef NDEBUG
#include "vm/core/Support/Format.h"
#endif

using namespace vm::core;
using namespace vm::core::mca;

void IncrementalSourceMgr::clear() {
  Staging.clear();
  InstStorage.clear();
  TotalCounter = 0U;
  EOS = false;
}

void IncrementalSourceMgr::updateNext() {
  ++TotalCounter;
  Instruction *I = Staging.front();
  Staging.pop_front();
  I->reset();

  if (InstFreedCB)
    InstFreedCB(I);
}

#ifndef NDEBUG
void IncrementalSourceMgr::printStatistic(raw_ostream &OS) {
  unsigned MaxInstStorageSize = InstStorage.size();
  if (MaxInstStorageSize <= TotalCounter) {
    auto Ratio = double(MaxInstStorageSize) / double(TotalCounter);
    OS << "Cache ratio = " << MaxInstStorageSize << " / " << TotalCounter
       << toolchain::format(" (%.2f%%)", (1.0 - Ratio) * 100.0) << "\n";
  } else {
    OS << "Error: Number of created instructions "
       << "are larger than the number of issued instructions\n";
  }
}
#endif
