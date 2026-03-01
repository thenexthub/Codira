//===--- SyntheticCountsUtils.cpp - synthetic counts propagation utils ---===//
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
// This file defines utilities for propagating synthetic counts.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/SyntheticCountsUtils.h"
#include "vm/core/ADT/DenseSet.h"
#include "vm/core/ADT/SCCIterator.h"
#include "vm/core/Analysis/CallGraph.h"
#include "vm/core/IR/ModuleSummaryIndex.h"

using namespace vm::core;

// Given an SCC, propagate entry counts along the edge of the SCC nodes.
template <typename CallGraphType>
void SyntheticCountsUtils<CallGraphType>::propagateFromSCC(
    const SccTy &SCC, GetProfCountTy GetProfCount, AddCountTy AddCount) {

  DenseSet<NodeRef> SCCNodes(toolchain::from_range, SCC);
  SmallVector<std::pair<NodeRef, EdgeRef>, 8> SCCEdges, NonSCCEdges;

  // Partition the edges coming out of the SCC into those whose destination is
  // in the SCC and the rest.
  for (const auto &Node : SCCNodes) {
    for (auto &E : children_edges<CallGraphType>(Node)) {
      if (SCCNodes.count(CGT::edge_dest(E)))
        SCCEdges.emplace_back(Node, E);
      else
        NonSCCEdges.emplace_back(Node, E);
    }
  }

  // For nodes in the same SCC, update the counts in two steps:
  // 1. Compute the additional count for each node by propagating the counts
  // along all incoming edges to the node that originate from within the same
  // SCC and summing them up.
  // 2. Add the additional counts to the nodes in the SCC.
  // This ensures that the order of
  // traversal of nodes within the SCC doesn't affect the final result.

  DenseMap<NodeRef, Scaled64> AdditionalCounts;
  for (auto &E : SCCEdges) {
    auto OptProfCount = GetProfCount(E.first, E.second);
    if (!OptProfCount)
      continue;
    auto Callee = CGT::edge_dest(E.second);
    AdditionalCounts[Callee] += *OptProfCount;
  }

  // Update the counts for the nodes in the SCC.
  for (auto &Entry : AdditionalCounts)
    AddCount(Entry.first, Entry.second);

  // Now update the counts for nodes outside the SCC.
  for (auto &E : NonSCCEdges) {
    auto OptProfCount = GetProfCount(E.first, E.second);
    if (!OptProfCount)
      continue;
    auto Callee = CGT::edge_dest(E.second);
    AddCount(Callee, *OptProfCount);
  }
}

/// Propgate synthetic entry counts on a callgraph \p CG.
///
/// This performs a reverse post-order traversal of the callgraph SCC. For each
/// SCC, it first propagates the entry counts to the nodes within the SCC
/// through call edges and updates them in one shot. Then the entry counts are
/// propagated to nodes outside the SCC. This requires \p GraphTraits
/// to have a specialization for \p CallGraphType.

template <typename CallGraphType>
void SyntheticCountsUtils<CallGraphType>::propagate(const CallGraphType &CG,
                                                    GetProfCountTy GetProfCount,
                                                    AddCountTy AddCount) {
  std::vector<SccTy> SCCs;

  // Collect all the SCCs.
  for (auto I = scc_begin(CG); !I.isAtEnd(); ++I)
    SCCs.push_back(*I);

  // The callgraph-scc needs to be visited in top-down order for propagation.
  // The scc iterator returns the scc in bottom-up order, so reverse the SCCs
  // and call propagateFromSCC.
  for (auto &SCC : reverse(SCCs))
    propagateFromSCC(SCC, GetProfCount, AddCount);
}

template class toolchain::SyntheticCountsUtils<const CallGraph *>;
template class toolchain::SyntheticCountsUtils<ModuleSummaryIndex *>;
