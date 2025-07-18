//===--- Dominance.h - SIL dominance analysis -------------------*- C++ -*-===//
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
//
// This file provides interfaces for computing and working with
// control-flow dominance in SIL.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_DOMINANCE_H
#define LANGUAGE_SIL_DOMINANCE_H

#include "toolchain/Support/GenericDomTree.h"
#include "language/Basic/ScopedTracking.h"
#include "language/SIL/CFG.h"

extern template class toolchain::DominatorTreeBase<language::SILBasicBlock, false>;
extern template class toolchain::DominatorTreeBase<language::SILBasicBlock, true>;
extern template class toolchain::DomTreeNodeBase<language::SILBasicBlock>;

namespace toolchain {
namespace DomTreeBuilder {
using SILDomTree = toolchain::DomTreeBase<language::SILBasicBlock>;
using SILPostDomTree = toolchain::PostDomTreeBase<language::SILBasicBlock>;

extern template void Calculate<SILDomTree>(SILDomTree &DT);
extern template void Calculate<SILPostDomTree>(SILPostDomTree &DT);
} // namespace DomTreeBuilder
} // namespace toolchain

namespace language {

using DominatorTreeBase = toolchain::DominatorTreeBase<language::SILBasicBlock, false>;
using PostDominatorTreeBase = toolchain::DominatorTreeBase<language::SILBasicBlock, true>;
using DominanceInfoNode = toolchain::DomTreeNodeBase<SILBasicBlock>;

/// A class for computing basic dominance information.
class DominanceInfo : public DominatorTreeBase {
  using super = DominatorTreeBase;
public:
  DominanceInfo(SILFunction *F);

  ~DominanceInfo();

  /// Does instruction A properly dominate instruction B?
  bool properlyDominates(SILInstruction *a, SILInstruction *b);

  /// Does instruction A dominate instruction B?
  bool dominates(SILInstruction *a, SILInstruction *b) {
    return a == b || properlyDominates(a, b);
  }

  /// Does value A properly dominate instruction B?
  bool properlyDominates(SILValue a, SILInstruction *b);

  /// The nearest block which dominates all the uses of \p value.
  SILBasicBlock *getLeastCommonAncestorOfUses(SILValue value);

  void verify() const;

  /// Return true if the other dominator tree does not match this dominator
  /// tree.
  inline bool errorOccurredOnComparison(const DominanceInfo &Other) const {
    const auto *R = getRootNode();
    const auto *OtherR = Other.getRootNode();

    if (!R || !OtherR || R->getBlock() != OtherR->getBlock())
      return true;

    // Returns *false* if they match.
    if (compare(Other))
      return true;

    return false;
  }

  using DominatorTreeBase::properlyDominates;
  using DominatorTreeBase::dominates;

  bool isValid(SILFunction *F) const {
    return getNode(&F->front()) != nullptr;
  }
  void reset() {
    super::reset();
  }

#ifndef NDEBUG
  void dump() TOOLCHAIN_ATTRIBUTE_USED { print(toolchain::errs()); }
#endif
};

/// Compute a single block's dominance frontier.
///
/// Precondition: no critical edges
///
/// Postcondition: each block in \p boundary is dominated by \p root and either
/// exits the function or has a single successor which has a predecessor that is
/// not dominated by \p root.

void computeDominatedBoundaryBlocks(SILBasicBlock *root, DominanceInfo *domTree,
                                    SmallVectorImpl<SILBasicBlock *> &boundary);

/// Helper class for visiting basic blocks in dominance order, based on a
/// worklist algorithm. Example usage:
/// \code
///   DominanceOrder DomOrder(Function->front(), DominanceInfo);
///   while (SILBasicBlock *block = DomOrder.getNext()) {
///     doSomething(block);
///     domOrder.pushChildren(block);
///   }
/// \endcode
class DominanceOrder {
  
  SmallVector<SILBasicBlock *, 16> buffer;
  DominanceInfo *DT;
  size_t srcIdx = 0;
  
public:
  
  /// Constructor.
  /// \p entry The root of the dominator (sub-)tree.
  /// \p DT The dominance info of the function.
  /// \p capacity Should be the number of basic blocks in the dominator tree to
  ///             reduce memory allocation.
  DominanceOrder(SILBasicBlock *root, DominanceInfo *DT, int capacity = 0) :
            DT(DT) {
     buffer.reserve(capacity);
     buffer.push_back(root);
  }

  /// Gets the next block from the worklist.
  ///
  SILBasicBlock *getNext() {
    if (srcIdx == buffer.size())
      return nullptr;
    return buffer[srcIdx++];
  }

  /// Pushes the dominator children of a block onto the worklist.
  void pushChildren(SILBasicBlock *block) {
    pushChildrenIf(block, [] (SILBasicBlock *) { return true; });
  }

  /// Conditionally pushes the dominator children of a block onto the worklist.
  /// \p pred Takes a block (= a dominator child) as argument and returns true
  ///         if it should be added to the worklist.
  ///
  template <typename Pred> void pushChildrenIf(SILBasicBlock *block, Pred pred) {
    DominanceInfoNode *DINode = DT->getNode(block);
    for (auto *DIChild : *DINode) {
      SILBasicBlock *child = DIChild->getBlock();
      if (pred(child))
        buffer.push_back(DIChild->getBlock());
    }
  }
};

/// A class for computing basic post-dominance information.
class PostDominanceInfo : public PostDominatorTreeBase {
  using super = PostDominatorTreeBase;
public:
  PostDominanceInfo(SILFunction *F);

  bool properlyDominates(SILInstruction *A, SILInstruction *B);
  bool properlyDominates(SILValue A, SILInstruction *B);

  void verify() const;

  /// Return true if the other dominator tree does not match this dominator
  /// tree.
  inline bool errorOccurredOnComparison(const PostDominanceInfo &Other) const {
    const auto *R = getRootNode();
    const auto *OtherR = Other.getRootNode();

    if (!R || !OtherR || R->getBlock() != OtherR->getBlock())
      return true;

    if (!R->getBlock()) {
      // The post dom-tree has multiple roots. The compare() function can not
      // cope with multiple roots if at least one of the roots is caused by
      // an infinite loop in the CFG (it crashes because no nodes are allocated
      // for the blocks in the infinite loop).
      // So we return a conservative false in this case.
      // TODO: eventually fix the DominatorTreeBase::compare() function.
      return false;
    }

    // Returns *false* if they match.
    if (compare(Other))
      return true;

    return false;
  }

  bool isValid(SILFunction *F) const { return getNode(&F->front()) != nullptr; }

  using super::properlyDominates;
};

/// Invoke the given callback for all the reachable blocks
/// in a function.  It will be called in a depth-first,
/// dominance-consistent order.
///
/// Furthermore, prior to running each block, a tracking scope will
/// be entered for each of the trackers passed in, as if by:
///
///   typename ScopedTrackingTraits<Tracker>::scope_type scope(tracker);
///
/// This allows state to be saved and restored for each of the trackers,
/// such that each tracker will only represent state that was computed
/// in a dominating block.
template <class... Trackers, class Fn>
void runInDominanceOrderWithScopes(DominanceInfo *dominance, Fn &&fn,
                                   Trackers &...trackers) {
  using TrackingStackNode = TrackingScopes<Trackers...>;
  toolchain::SmallVector<std::unique_ptr<TrackingStackNode>, 8> trackingStack;

  // The stack of work to do.  A null item means to pop the top
  // entry off the tracking stack.
  toolchain::SmallVector<DominanceInfoNode *, 16> workStack;
  workStack.push_back(dominance->getRootNode());

  while (!workStack.empty()) {
    auto node = workStack.pop_back_val();

    // If the node is null, pop the top entry off the tracking stack.
    if (node == nullptr) {
      (void) trackingStack.pop_back_val();
      continue;
    }

    auto bb = node->getBlock();

    // If the node has no children, build the stack node in local
    // storage to avoid having to heap-allocate it.
    if (node->isLeaf()) {
      TrackingStackNode stackNode(trackers...);

      fn(bb);

    // Otherwise, we have to use the more general approach.
    } else {
      // Push a tracking stack node.
      trackingStack.emplace_back(new TrackingStackNode(trackers...));
      // Push a work command to pop the tracking stack node.
      workStack.push_back(nullptr);
      // Push all the child nodes as work items.
      workStack.append(node->begin(), node->end());

      fn(bb);
    }
  }

  assert(trackingStack.empty());
}

} // end namespace language

namespace toolchain {

/// DominatorTree GraphTraits specialization so the DominatorTree can be
/// iterable by generic graph iterators.
template <> struct GraphTraits<language::DominanceInfoNode *> {
  using ChildIteratorType = language::DominanceInfoNode::const_iterator;
  using NodeRef = language::DominanceInfoNode *;

  static NodeRef getEntryNode(NodeRef N) { return N; }
  static inline ChildIteratorType child_begin(NodeRef N) { return N->begin(); }
  static inline ChildIteratorType child_end(NodeRef N) { return N->end(); }
};

template <> struct GraphTraits<const language::DominanceInfoNode *> {
  using ChildIteratorType = language::DominanceInfoNode::const_iterator;
  using NodeRef = const language::DominanceInfoNode *;

  static NodeRef getEntryNode(NodeRef N) { return N; }
  static inline ChildIteratorType child_begin(NodeRef N) { return N->begin(); }
  static inline ChildIteratorType child_end(NodeRef N) { return N->end(); }
};

} // end namespace toolchain
#endif
