//===--- IndexTrie - Trie for a sequence of integer indices ----*- C++ -*-===//
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

#ifndef LANGUAGE_SILOPTIMIZER_UTILS_INDEXTREE_H
#define LANGUAGE_SILOPTIMIZER_UTILS_INDEXTREE_H

#include "language/Basic/Toolchain.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/SmallVector.h"
#include <algorithm>

namespace language {

// Trie node representing a sequence of unsigned integer indices.
class IndexTrieNode {
public:
  static const int RootIndex = std::numeric_limits<int>::min();

private:
  int Index;
  toolchain::SmallVector<IndexTrieNode*, 8> Children;
  IndexTrieNode *Parent;

public:
  IndexTrieNode() : Index(RootIndex), Parent(nullptr) {}

  explicit IndexTrieNode(int V, IndexTrieNode *P) : Index(V), Parent(P) {}

  IndexTrieNode(IndexTrieNode &) =delete;
  IndexTrieNode &operator=(const IndexTrieNode&) =delete;

  ~IndexTrieNode() {
    for (auto *N : Children)
      delete N;
  }

  bool isRoot() const { return Index == RootIndex; }

  bool isLeaf() const { return Children.empty(); }

  int getIndex() const { return Index; }

  IndexTrieNode *getChild(int Idx) {
    assert(Idx != RootIndex);

    auto I =
        std::lower_bound(Children.begin(), Children.end(), Idx,
                         [](IndexTrieNode *a, int i) { return a->Index < i; });
    if (I != Children.end() && (*I)->Index == Idx)
      return *I;
    auto *N = new IndexTrieNode(Idx, this);
    Children.insert(I, N);
    return N;
  }

  ArrayRef<IndexTrieNode*> getChildren() const { return Children; }

  IndexTrieNode *getParent() const { return Parent; }

  /// Returns true when the sequence of indices represented by this
  /// node is a prefix of the sequence represented by the passed-in node.
  bool isPrefixOf(const IndexTrieNode *Other) const {
    const IndexTrieNode *I = Other;

    do {
      if (this == I)
        return true;

      I = I->getParent();
    } while (I);

    return false;
  }
};

} // end namespace language

#endif
