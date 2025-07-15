//===--- IndexSubset.cpp - Fixed-size subset of indices -------------------===//
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

#include "language/AST/IndexSubset.h"
#include "language/Basic/Assertions.h"

using namespace language;

IndexSubset *
IndexSubset::getFromString(ASTContext &ctx, StringRef string) {
  unsigned capacity = string.size();
  toolchain::SmallBitVector indices(capacity);
  for (unsigned i : range(capacity)) {
    if (string[i] == 'S')
      indices.set(i);
    else if (string[i] != 'U')
      return nullptr;
  }
  return get(ctx, indices);
}

std::string IndexSubset::getString() const {
  std::string result;
  result.reserve(capacity);
  for (unsigned i : range(capacity))
    result += contains(i) ? 'S' : 'U';
  return result;
}

bool IndexSubset::isSubsetOf(IndexSubset *other) const {
  assert(capacity == other->capacity);
  for (auto index : range(numBitWords))
    if (getBitWord(index) & ~other->getBitWord(index))
      return false;
  return true;
}

bool IndexSubset::isSupersetOf(IndexSubset *other) const {
  assert(capacity == other->capacity);
  for (auto index : range(numBitWords))
    if (~getBitWord(index) & other->getBitWord(index))
      return false;
  return true;
}

bool IndexSubset::isDisjointWith(IndexSubset *other) const {
  assert(capacity == other->capacity);
  for (auto index : range(numBitWords))
    if (getBitWord(index) & other->getBitWord(index))
      return false;
  return true;
}

IndexSubset *IndexSubset::adding(unsigned index, ASTContext &ctx) const {
  assert(index < getCapacity());
  if (contains(index))
    return const_cast<IndexSubset *>(this);
  SmallBitVector newIndices(capacity);
  bool inserted = false;
  for (auto curIndex : getIndices()) {
    if (!inserted && curIndex > index) {
      newIndices.set(index);
      inserted = true;
    }
    newIndices.set(curIndex);
  }
  return get(ctx, newIndices);
}

IndexSubset *IndexSubset::extendingCapacity(
    ASTContext &ctx, unsigned newCapacity) const {
  assert(newCapacity >= capacity);
  if (newCapacity == capacity)
    return const_cast<IndexSubset *>(this);
  SmallBitVector indices(newCapacity);
  for (auto index : getIndices())
    indices.set(index);
  return IndexSubset::get(ctx, indices);
}

void IndexSubset::print(toolchain::raw_ostream &s) const {
  s << '{';
  toolchain::interleave(
      range(capacity), [this, &s](unsigned i) { s << contains(i); },
      [&s] { s << ", "; });
  s << '}';
}

void IndexSubset::dump() const {
  auto &s = toolchain::errs();
  s << "(index_subset capacity=" << capacity << " indices=(";
  interleave(getIndices(), [&s](unsigned i) { s << i; },
             [&s] { s << ", "; });
  s << "))\n";
}

int IndexSubset::findNext(int startIndex) const {
  assert(startIndex < (int)capacity && "Start index cannot be past the end");
  unsigned bitWordIndex = 0, offset = 0;
  if (startIndex >= 0) {
    auto indexAndOffset = getBitWordIndexAndOffset(startIndex);
    bitWordIndex = indexAndOffset.first;
    offset = indexAndOffset.second + 1;
  }
  for (; bitWordIndex < numBitWords; ++bitWordIndex, offset = 0) {
    for (; offset < numBitsPerBitWord; ++offset) {
      auto index = bitWordIndex * numBitsPerBitWord + offset;
      auto bitWord = getBitWord(bitWordIndex);
      if (!bitWord)
        break;
      if (index >= capacity)
        return capacity;
      if (bitWord & ((BitWord)1 << offset))
        return index;
    }
  }
  return capacity;
}

int IndexSubset::findPrevious(int endIndex) const {
  assert(endIndex >= 0 && "End index cannot be before the start");
  int bitWordIndex = numBitWords - 1, offset = numBitsPerBitWord - 1;
  if (endIndex < (int)capacity) {
    auto indexAndOffset = getBitWordIndexAndOffset(endIndex);
    bitWordIndex = (int)indexAndOffset.first;
    offset = (int)indexAndOffset.second - 1;
  }
  for (; bitWordIndex >= 0; --bitWordIndex, offset = numBitsPerBitWord - 1) {
    for (; offset >= 0; --offset) {
      auto index = bitWordIndex * (int)numBitsPerBitWord + offset;
      auto bitWord = getBitWord(bitWordIndex);
      if (!bitWord)
        break;
      if (index < 0)
        return -1;
      if (bitWord & ((BitWord)1 << offset))
        return index;
    }
  }
  return -1;
}
