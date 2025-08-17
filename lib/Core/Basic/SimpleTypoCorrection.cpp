//===- SimpleTypoCorrection.cpp - Basic typo correction utility -----------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
// This file implements the SimpleTypoCorrection class, which performs basic
// typo correction using string similarity based on edit distance.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Basic/SimpleTypoCorrection.h"
#include "language/Core/Basic/IdentifierTable.h"
#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/StringRef.h"

using namespace language::Core;

void SimpleTypoCorrection::add(const StringRef Candidate) {
  if (Candidate.empty())
    return;

  unsigned MinPossibleEditDistance =
      abs(static_cast<int>(Candidate.size()) - static_cast<int>(Typo.size()));

  if (MinPossibleEditDistance > 0 && Typo.size() / MinPossibleEditDistance < 3)
    return;

  unsigned EditDistance = Typo.edit_distance(
      Candidate, /*AllowReplacements*/ true, MaxEditDistance);

  if (EditDistance < BestEditDistance) {
    BestCandidate = Candidate;
    BestEditDistance = EditDistance;
    BestIndex = NextIndex;
  }

  ++NextIndex;
}

void SimpleTypoCorrection::add(const char *Candidate) {
  if (Candidate)
    add(StringRef(Candidate));
}

void SimpleTypoCorrection::add(const IdentifierInfo *Candidate) {
  if (Candidate)
    add(Candidate->getName());
}

unsigned SimpleTypoCorrection::getCorrectionIndex() const { return BestIndex; }

std::optional<StringRef> SimpleTypoCorrection::getCorrection() const {
  if (hasCorrection())
    return BestCandidate;
  return std::nullopt;
}

bool SimpleTypoCorrection::hasCorrection() const {
  return BestEditDistance <= MaxEditDistance;
}
