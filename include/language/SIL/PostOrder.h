//===--- PostOrder.h --------------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_SIL_POSTORDER_H
#define LANGUAGE_SIL_POSTORDER_H

#include "language/Basic/Range.h"
#include "language/SIL/CFG.h"
#include "language/SIL/SILBasicBlock.h"
#include "language/SIL/SILFunction.h"
#include "language/SILOptimizer/Analysis/Analysis.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/PostOrderIterator.h"
#include "toolchain/ADT/iterator_range.h"
#include <vector>

namespace language {

class PostOrderFunctionInfo {
  std::vector<SILBasicBlock *> PostOrder;
  toolchain::DenseMap<SILBasicBlock *, unsigned> BBToPOMap;

public:
  PostOrderFunctionInfo(SILFunction *F) {
    for (auto *BB : make_range(po_begin(F), po_end(F))) {
      BBToPOMap[BB] = PostOrder.size();
      PostOrder.push_back(BB);
    }
  }

  using iterator = decltype(PostOrder)::iterator;
  using const_iterator = decltype(PostOrder)::const_iterator;
  using reverse_iterator = decltype(PostOrder)::reverse_iterator;
  using const_reverse_iterator = decltype(PostOrder)::const_reverse_iterator;

  using range = iterator_range<iterator>;
  using const_range = iterator_range<const_iterator>;
  using reverse_range = iterator_range<reverse_iterator>;
  using const_reverse_range = iterator_range<const_reverse_iterator>;

  range getPostOrder() {
    return make_range(PostOrder.begin(), PostOrder.end());
  }
  const_range getPostOrder() const {
    return make_range(PostOrder.begin(), PostOrder.end());
  }
  reverse_range getReversePostOrder() {
    return make_range(PostOrder.rbegin(), PostOrder.rend());
  }
  const_reverse_range getReversePostOrder() const {
    return make_range(PostOrder.rbegin(), PostOrder.rend());
  }

  const_reverse_range getReversePostOrder(SILBasicBlock *StartBlock) const {
    unsigned RPONumber = getRPONumber(StartBlock).value();
    return getReversePostOrder(RPONumber);
  }

  const_reverse_range getReversePostOrder(unsigned RPONumber) const {
    return make_range(std::next(PostOrder.rbegin(), RPONumber),
                      PostOrder.rend());
  }

  unsigned size() const { return PostOrder.size(); }

  std::optional<unsigned> getPONumber(SILBasicBlock *BB) const {
    auto Iter = BBToPOMap.find(BB);
    if (Iter != BBToPOMap.end())
      return Iter->second;
    return std::nullopt;
  }

  std::optional<unsigned> getRPONumber(SILBasicBlock *BB) const {
    auto Iter = BBToPOMap.find(BB);
    if (Iter != BBToPOMap.end())
      return PostOrder.size() - Iter->second - 1;
    return std::nullopt;
  }
};

} // end language namespace

#endif
