//===--- MoveOnlyBorrowToDestructureUtils.h -------------------------------===//
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

#ifndef LANGUAGE_SILOPTIMIZER_MANDATORY_MOVEONLYBORROWTODESTRUCTURE_H
#define LANGUAGE_SILOPTIMIZER_MANDATORY_MOVEONLYBORROWTODESTRUCTURE_H

#include "language/Basic/FrozenMultiMap.h"
#include "language/SIL/FieldSensitivePrunedLiveness.h"
#include "language/SIL/StackList.h"
#include "language/SILOptimizer/Analysis/PostOrderAnalysis.h"

#include "toolchain/ADT/IntervalMap.h"

namespace language {
namespace siloptimizer {

class DiagnosticEmitter;

namespace borrowtodestructure {

class IntervalMapAllocator {
public:
  using Map = toolchain::IntervalMap<
      unsigned, SILValue,
      toolchain::IntervalMapImpl::NodeSizer<unsigned, SILValue>::LeafSize,
      toolchain::IntervalMapHalfOpenInfo<unsigned>>;

  using Allocator = Map::Allocator;

private:
  /// Lazily initialized allocator.
  std::optional<Allocator> allocator;

public:
  Allocator &get() {
    if (!allocator)
      allocator.emplace();
    return *allocator;
  }
};

struct Implementation;

} // namespace borrowtodestructure

class BorrowToDestructureTransform {
  friend borrowtodestructure::Implementation;

  using IntervalMapAllocator = borrowtodestructure::IntervalMapAllocator;

  IntervalMapAllocator &allocator;
  MarkUnresolvedNonCopyableValueInst *mmci;
  SILValue rootValue;
  DiagnosticEmitter &diagnosticEmitter;
  PostOrderAnalysis *poa;
  PostOrderFunctionInfo *pofi = nullptr;
  SmallVector<SILInstruction *, 8> createdDestructures;
  SmallVector<SILPhiArgument *, 8> createdPhiArguments;

public:
  BorrowToDestructureTransform(IntervalMapAllocator &allocator,
                               MarkUnresolvedNonCopyableValueInst *mmci,
                               SILValue rootValue,
                               DiagnosticEmitter &diagnosticEmitter,
                               PostOrderAnalysis *poa)
      : allocator(allocator), mmci(mmci), rootValue(rootValue),
        diagnosticEmitter(diagnosticEmitter), poa(poa) {}

  bool transform();

private:
  PostOrderFunctionInfo *getPostOrderFunctionInfo() {
    if (!pofi)
      pofi = poa->get(mmci->getFunction());
    return pofi;
  }
};

} // namespace siloptimizer
} // namespace language

#endif
