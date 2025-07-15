//===--- Trie.h - Trie with terms as keys ---------------------------------===//
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

#ifndef LANGUAGE_RQM_TRIE_H
#define LANGUAGE_RQM_TRIE_H

#include "toolchain/ADT/MapVector.h"
#include "Histogram.h"

namespace language {

namespace rewriting {

enum class MatchKind {
  Shortest,
  Longest
};

template<typename ValueType, MatchKind Kind>
class Trie {
public:
  struct Node;

  struct Entry {
    std::optional<ValueType> Value;
    Node *Children = nullptr;
  };

  struct Node {
    toolchain::SmallMapVector<Symbol, Entry, 1> Entries;
  };

private:
  /// We never delete nodes, except for when the entire trie is torn down.
  std::vector<Node *> Nodes;

  /// The root is stored directly.
  Node Root;

public:
  void updateHistograms(Histogram &stats, Histogram &rootStats) const {
    for (const auto &node : Nodes)
      stats.add(node->Entries.size());
    rootStats.add(Root.Entries.size());
  }

  /// Delete all entries from the trie.
  void clear() {
    Root.Entries.clear();

    for (auto iter = Nodes.rbegin(); iter != Nodes.rend(); ++iter) {
      auto *node = *iter;
      delete node;
    }

    Nodes.clear();
  }

  ~Trie() {
    clear();
  }

  /// Insert an entry with the key given by the range [begin, end).
  /// Returns the old value if the trie already had an entry for this key;
  /// this is actually an invariant violation, but we can produce a better
  /// assertion further up the stack.
  template <typename Iter>
  std::optional<ValueType> insert(Iter begin, Iter end, ValueType value) {
    DEBUG_ASSERT(begin != end);
    auto *node = &Root;

    while (true) {
      auto &entry = node->Entries[*begin];
      ++begin;

      if (begin == end) {
        auto oldValue = entry.Value;
        entry.Value = value;
        return oldValue;
      }

      if (entry.Children == nullptr) {
        entry.Children = new Node();
        Nodes.push_back(entry.Children);
      }

      node = entry.Children;
    }
  }

  /// Find the shortest or longest prefix of the range given by [begin,end),
  /// depending on whether the Kind template parameter was bound to
  /// MatchKind::Shortest or MatchKind::Longest.
  template <typename Iter>
  std::optional<ValueType> find(Iter begin, Iter end) const {
    DEBUG_ASSERT(begin != end);
    auto *node = &Root;

    std::optional<ValueType> bestMatch = std::nullopt;

    while (true) {
      auto found = node->Entries.find(*begin);
      ++begin;

      if (found == node->Entries.end())
        return bestMatch;

      const auto &entry = found->second;

      if (entry.Value) {
        if (Kind == MatchKind::Shortest)
          return entry.Value;

        bestMatch = entry.Value;
      }

      if (begin == end)
        return bestMatch;

      if (entry.Children == nullptr)
        return bestMatch;

      node = entry.Children;
    }
  }

  /// Find all keys that begin with the given symbol. Fn must take a single
  /// argument of type ValueType.
  template<typename Fn>
  void findAll(Symbol symbol, Fn fn) const {
    auto found = Root.Entries.find(symbol);
    if (found == Root.Entries.end())
      return;

    const auto &entry = found->second;

    if (entry.Value)
      fn(*entry.Value);

    if (entry.Children == nullptr)
      return;

    visitChildren(entry.Children, fn);
  }

  /// Find all keys that either match a prefix of [begin,end), or where
  /// [begin,end) matches a prefix of the key. Fn must take a single
  /// argument of type ValueType.
  template<typename Iter, typename Fn>
  void findAll(Iter begin, Iter end, Fn fn) const {
    DEBUG_ASSERT(begin != end);
    auto *node = &Root;

    while (true) {
      auto found = node->Entries.find(*begin);
      ++begin;

      if (found == node->Entries.end())
        return;

      const auto &entry = found->second;

      if (entry.Value)
        fn(*entry.Value);

      if (entry.Children == nullptr)
        return;

      node = entry.Children;

      if (begin == end) {
        visitChildren(node, fn);
        return;
      }
    }
  }

private:
  /// Depth-first traversal of all children of the given node, including
  /// the node itself. Fn must take a single argument of type ValueType.
  template<typename Fn>
  void visitChildren(const Node *node, Fn fn) const {
    for (const auto &pair : node->Entries) {
      const auto &entry = pair.second;
      if (entry.Value)
        fn(*entry.Value);

      if (entry.Children)
        visitChildren(entry.Children, fn);
    }
  }
};

}  // end namespace rewriting

}  // end namespace language

#endif
