//====--------------- lib/Support/BlockFrequency.cpp -----------*- C++ -*-====//
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
// This file implements Block Frequency class.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Support/BlockFrequency.h"
#include "vm/core/Support/BranchProbability.h"
#include "vm/core/Support/MathExtras.h"
#include "vm/core/Support/ScaledNumber.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

BlockFrequency &BlockFrequency::operator*=(BranchProbability Prob) {
  Frequency = Prob.scale(Frequency);
  return *this;
}

BlockFrequency BlockFrequency::operator*(BranchProbability Prob) const {
  BlockFrequency Freq(Frequency);
  Freq *= Prob;
  return Freq;
}

BlockFrequency &BlockFrequency::operator/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  return *this;
}

BlockFrequency BlockFrequency::operator/(BranchProbability Prob) const {
  BlockFrequency Freq(Frequency);
  Freq /= Prob;
  return Freq;
}

std::optional<BlockFrequency> BlockFrequency::mul(uint64_t Factor) const {
  bool Overflow;
  uint64_t ResultFrequency = SaturatingMultiply(Frequency, Factor, &Overflow);
  if (Overflow)
    return {};
  return BlockFrequency(ResultFrequency);
}

void toolchain::printRelativeBlockFreq(raw_ostream &OS, BlockFrequency EntryFreq,
                                  BlockFrequency Freq) {
  if (Freq == BlockFrequency(0)) {
    OS << "0";
    return;
  }
  if (EntryFreq == BlockFrequency(0)) {
    OS << "<invalid BFI>";
    return;
  }
  ScaledNumber<uint64_t> Block(Freq.getFrequency(), 0);
  ScaledNumber<uint64_t> Entry(EntryFreq.getFrequency(), 0);
  OS << Block / Entry;
}
