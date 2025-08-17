//===- FunctionSummary.h - Stores summaries of functions. -------*- C++ -*-===//
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
// This file defines a summary of a function gathered/used by static analysis.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_FUNCTIONSUMMARY_H
#define LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_FUNCTIONSUMMARY_H

#include "language/Core/AST/Decl.h"
#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/DenseSet.h"
#include "toolchain/ADT/SmallBitVector.h"
#include <cassert>
#include <deque>
#include <optional>
#include <utility>

namespace language::Core {
namespace ento {

using SetOfDecls = std::deque<Decl *>;
using SetOfConstDecls = toolchain::DenseSet<const Decl *>;

class FunctionSummariesTy {
  class FunctionSummary {
  public:
    /// Marks the IDs of the basic blocks visited during the analyzes.
    toolchain::SmallBitVector VisitedBasicBlocks;

    /// Total number of blocks in the function.
    unsigned TotalBasicBlocks : 30;

    /// True if this function has been checked against the rules for which
    /// functions may be inlined.
    unsigned InlineChecked : 1;

    /// True if this function may be inlined.
    unsigned MayInline : 1;

    /// The number of times the function has been inlined.
    unsigned TimesInlined : 32;

    FunctionSummary()
        : TotalBasicBlocks(0), InlineChecked(0), MayInline(0),
          TimesInlined(0) {}
  };

  using MapTy = toolchain::DenseMap<const Decl *, FunctionSummary>;
  MapTy Map;

public:
  MapTy::iterator findOrInsertSummary(const Decl *D) {
    MapTy::iterator I = Map.find(D);
    if (I != Map.end())
      return I;

    using KVPair = std::pair<const Decl *, FunctionSummary>;

    I = Map.insert(KVPair(D, FunctionSummary())).first;
    assert(I != Map.end());
    return I;
  }

  void markMayInline(const Decl *D) {
    MapTy::iterator I = findOrInsertSummary(D);
    I->second.InlineChecked = 1;
    I->second.MayInline = 1;
  }

  void markShouldNotInline(const Decl *D) {
    MapTy::iterator I = findOrInsertSummary(D);
    I->second.InlineChecked = 1;
    I->second.MayInline = 0;
  }

  std::optional<bool> mayInline(const Decl *D) {
    MapTy::const_iterator I = Map.find(D);
    if (I != Map.end() && I->second.InlineChecked)
      return I->second.MayInline;
    return std::nullopt;
  }

  void markVisitedBasicBlock(unsigned ID, const Decl* D, unsigned TotalIDs) {
    MapTy::iterator I = findOrInsertSummary(D);
    toolchain::SmallBitVector &Blocks = I->second.VisitedBasicBlocks;
    assert(ID < TotalIDs);
    if (TotalIDs > Blocks.size()) {
      Blocks.resize(TotalIDs);
      I->second.TotalBasicBlocks = TotalIDs;
    }
    Blocks.set(ID);
  }

  unsigned getNumVisitedBasicBlocks(const Decl* D) {
    MapTy::const_iterator I = Map.find(D);
    if (I != Map.end())
      return I->second.VisitedBasicBlocks.count();
    return 0;
  }

  unsigned getNumTimesInlined(const Decl* D) {
    MapTy::const_iterator I = Map.find(D);
    if (I != Map.end())
      return I->second.TimesInlined;
    return 0;
  }

  void bumpNumTimesInlined(const Decl* D) {
    MapTy::iterator I = findOrInsertSummary(D);
    I->second.TimesInlined++;
  }

  /// Get the percentage of the reachable blocks.
  unsigned getPercentBlocksReachable(const Decl *D) {
    MapTy::const_iterator I = Map.find(D);
      if (I != Map.end())
        return ((I->second.VisitedBasicBlocks.count() * 100) /
                 I->second.TotalBasicBlocks);
    return 0;
  }

  unsigned getTotalNumBasicBlocks();
  unsigned getTotalNumVisitedBasicBlocks();
};

} // namespace ento
} // namespace language::Core

#endif // LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_FUNCTIONSUMMARY_H
