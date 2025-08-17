//===- PostOrderCFGView.h - Post order view of CFG blocks -------*- C++ -*-===//
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
// This file implements post order view of the blocks in a CFG.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ANALYSIS_ANALYSES_POSTORDERCFGVIEW_H
#define LANGUAGE_CORE_ANALYSIS_ANALYSES_POSTORDERCFGVIEW_H

#include "language/Core/Analysis/AnalysisDeclContext.h"
#include "language/Core/Analysis/CFG.h"
#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/BitVector.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/PostOrderIterator.h"
#include <utility>
#include <vector>

namespace language::Core {

class PostOrderCFGView : public ManagedAnalysis {
  virtual void anchor();

public:
  /// Implements a set of CFGBlocks using a BitVector.
  ///
  /// This class contains a minimal interface, primarily dictated by the SetType
  /// template parameter of the toolchain::po_iterator template, as used with
  /// external storage. We also use this set to keep track of which CFGBlocks we
  /// visit during the analysis.
  class CFGBlockSet {
    toolchain::BitVector VisitedBlockIDs;

  public:
    // po_iterator requires this iterator, but the only interface needed is the
    // value_type type.
    struct iterator { using value_type = const CFGBlock *; };

    CFGBlockSet() = default;
    CFGBlockSet(const CFG *G) : VisitedBlockIDs(G->getNumBlockIDs(), false) {}

    /// Set the bit associated with a particular CFGBlock.
    /// This is the important method for the SetType template parameter.
    std::pair<std::nullopt_t, bool> insert(const CFGBlock *Block) {
      // Note that insert() is called by po_iterator, which doesn't check to
      // make sure that Block is non-null.  Moreover, the CFGBlock iterator will
      // occasionally hand out null pointers for pruned edges, so we catch those
      // here.
      if (!Block)
        return std::make_pair(std::nullopt,
                              false); // if an edge is trivially false.
      if (VisitedBlockIDs.test(Block->getBlockID()))
        return std::make_pair(std::nullopt, false);
      VisitedBlockIDs.set(Block->getBlockID());
      return std::make_pair(std::nullopt, true);
    }

    /// Check if the bit for a CFGBlock has been already set.
    /// This method is for tracking visited blocks in the main threadsafety
    /// loop. Block must not be null.
    bool alreadySet(const CFGBlock *Block) {
      return VisitedBlockIDs.test(Block->getBlockID());
    }
  };

private:
  // The CFG orders the blocks of loop bodies before those of loop successors
  // (both numerically, and in the successor order of the loop condition
  // block). So, RPO necessarily reverses that order, placing the loop successor
  // *before* the loop body. For many analyses, particularly those that converge
  // to a fixpoint, this results in potentially significant extra work because
  // loop successors will necessarily need to be reconsidered once the algorithm
  // has reached a fixpoint on the loop body.
  //
  // This definition of CFG graph traits reverses the order of children, so that
  // loop bodies will come first in an RPO.
  struct CFGLoopBodyFirstTraits {
    using NodeRef = const ::language::Core::CFGBlock *;
    using ChildIteratorType = ::language::Core::CFGBlock::const_succ_reverse_iterator;

    static ChildIteratorType child_begin(NodeRef N) { return N->succ_rbegin(); }
    static ChildIteratorType child_end(NodeRef N) { return N->succ_rend(); }

    using nodes_iterator = ::language::Core::CFG::const_iterator;

    static NodeRef getEntryNode(const ::language::Core::CFG *F) {
      return &F->getEntry();
    }

    static nodes_iterator nodes_begin(const ::language::Core::CFG *F) {
      return F->nodes_begin();
    }

    static nodes_iterator nodes_end(const ::language::Core::CFG *F) {
      return F->nodes_end();
    }

    static unsigned size(const ::language::Core::CFG *F) { return F->size(); }
  };
  using po_iterator =
      toolchain::po_iterator<const CFG *, CFGBlockSet, true, CFGLoopBodyFirstTraits>;
  std::vector<const CFGBlock *> Blocks;

  using BlockOrderTy = toolchain::DenseMap<const CFGBlock *, unsigned>;
  BlockOrderTy BlockOrder;

public:
  friend struct BlockOrderCompare;

  using iterator = std::vector<const CFGBlock *>::reverse_iterator;
  using const_iterator = std::vector<const CFGBlock *>::const_reverse_iterator;

  PostOrderCFGView(const CFG *cfg);

  iterator begin() { return Blocks.rbegin(); }
  iterator end() { return Blocks.rend(); }

  const_iterator begin() const { return Blocks.rbegin(); }
  const_iterator end() const { return Blocks.rend(); }

  bool empty() const { return begin() == end(); }

  struct BlockOrderCompare {
    const PostOrderCFGView &POV;

  public:
    BlockOrderCompare(const PostOrderCFGView &pov) : POV(pov) {}

    bool operator()(const CFGBlock *b1, const CFGBlock *b2) const;
  };

  BlockOrderCompare getComparator() const {
    return BlockOrderCompare(*this);
  }

  // Used by AnalyisContext to construct this object.
  static const void *getTag();

  static std::unique_ptr<PostOrderCFGView>
  create(AnalysisDeclContext &analysisContext);
};

} // namespace language::Core

#endif // LANGUAGE_CORE_ANALYSIS_ANALYSES_POSTORDERCFGVIEW_H
