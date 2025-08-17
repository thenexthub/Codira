//===- CFGReachabilityAnalysis.h - Basic reachability analysis --*- C++ -*-===//
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
// This file defines a flow-sensitive, (mostly) path-insensitive reachability
// analysis based on Clang's CFGs.  Clients can query if a given basic block
// is reachable within the CFG.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ANALYSIS_ANALYSES_CFGREACHABILITYANALYSIS_H
#define LANGUAGE_CORE_ANALYSIS_ANALYSES_CFGREACHABILITYANALYSIS_H

#include "toolchain/ADT/BitVector.h"
#include "toolchain/ADT/DenseMap.h"

namespace language::Core {

class CFG;
class CFGBlock;

// A class that performs reachability queries for CFGBlocks. Several internal
// checks in this checker require reachability information. The requests all
// tend to have a common destination, so we lazily do a predecessor search
// from the destination node and cache the results to prevent work
// duplication.
class CFGReverseBlockReachabilityAnalysis {
  using ReachableSet = toolchain::BitVector;
  using ReachableMap = toolchain::DenseMap<unsigned, ReachableSet>;

  ReachableSet analyzed;
  ReachableMap reachable;

public:
  CFGReverseBlockReachabilityAnalysis(const CFG &cfg);

  /// Returns true if the block 'Dst' can be reached from block 'Src'.
  bool isReachable(const CFGBlock *Src, const CFGBlock *Dst);

private:
  void mapReachability(const CFGBlock *Dst);
};

} // namespace language::Core

#endif // LANGUAGE_CORE_ANALYSIS_ANALYSES_CFGREACHABILITYANALYSIS_H
