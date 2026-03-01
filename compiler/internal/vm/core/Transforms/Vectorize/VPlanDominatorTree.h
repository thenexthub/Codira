//===-- VPlanDominatorTree.h ------------------------------------*- C++ -*-===//
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
/// This file implements dominator tree analysis for a single level of a VPlan's
/// H-CFG.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_VPLANDOMINATORTREE_H
#define LLVM_TRANSFORMS_VECTORIZE_VPLANDOMINATORTREE_H

#include "VPlan.h"
#include "VPlanCFG.h"
#include "vm/core/ADT/GraphTraits.h"
#include "vm/core/IR/Dominators.h"
#include "vm/core/Support/GenericDomTree.h"
#include "vm/core/Support/GenericDomTreeConstruction.h"

namespace vm::core {

template <> struct DomTreeNodeTraits<VPBlockBase> {
  using NodeType = VPBlockBase;
  using NodePtr = VPBlockBase *;
  using ParentPtr = VPlan *;

  static NodePtr getEntryNode(ParentPtr Parent) { return Parent->getEntry(); }
  static ParentPtr getParent(NodePtr B) { return B->getPlan(); }
};

/// Template specialization of the standard LLVM dominator tree utility for
/// VPBlockBases.
class VPDominatorTree : public DominatorTreeBase<VPBlockBase, false> {
  using Base = DominatorTreeBase<VPBlockBase, false>;

public:
  explicit VPDominatorTree(VPlan &Plan) { recalculate(Plan); }

  /// Returns true if \p A properly dominates \p B.
  using Base::properlyDominates;
  bool properlyDominates(const VPRecipeBase *A, const VPRecipeBase *B);
};

using VPDomTreeNode = DomTreeNodeBase<VPBlockBase>;

/// Template specializations of GraphTraits for VPDomTreeNode.
template <>
struct GraphTraits<VPDomTreeNode *>
    : public DomTreeGraphTraitsBase<VPDomTreeNode,
                                    VPDomTreeNode::const_iterator> {};

template <>
struct GraphTraits<const VPDomTreeNode *>
    : public DomTreeGraphTraitsBase<const VPDomTreeNode,
                                    VPDomTreeNode::const_iterator> {};
} // namespace vm::core
#endif // LLVM_TRANSFORMS_VECTORIZE_VPLANDOMINATORTREE_H
