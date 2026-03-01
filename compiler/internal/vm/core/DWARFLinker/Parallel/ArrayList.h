//===- ArrayList.h ----------------------------------------------*- C++ -*-===//
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

#ifndef LLVM_LIB_DWARFLINKER_PARALLEL_ARRAYLIST_H
#define LLVM_LIB_DWARFLINKER_PARALLEL_ARRAYLIST_H

#include "vm/core/Support/PerThreadBumpPtrAllocator.h"
#include <atomic>

namespace vm::core {
namespace dwarf_linker {
namespace parallel {

/// This class is a simple list of T structures. It keeps elements as
/// pre-allocated groups to save memory for each element's next pointer.
/// It allocates internal data using specified per-thread BumpPtrAllocator.
/// Method add() can be called asynchronously.
template <typename T, size_t ItemsGroupSize = 512> class ArrayList {
public:
  ArrayList(toolchain::parallel::PerThreadBumpPtrAllocator *Allocator)
      : Allocator(Allocator) {}

  /// Add specified \p Item to the list.
  T &add(const T &Item) {
    assert(Allocator);

    // Allocate head group if it is not allocated yet.
    while (!LastGroup) {
      if (allocateNewGroup(GroupsHead))
        LastGroup = GroupsHead.load();
    }

    ItemsGroup *CurGroup;
    size_t CurItemsCount;
    do {
      CurGroup = LastGroup;
      CurItemsCount = CurGroup->ItemsCount.fetch_add(1);

      // Check whether current group is full.
      if (CurItemsCount < ItemsGroupSize)
        break;

      // Allocate next group if necessary.
      if (!CurGroup->Next)
        allocateNewGroup(CurGroup->Next);

      LastGroup.compare_exchange_weak(CurGroup, CurGroup->Next);
    } while (true);

    // Store item into the current group.
    CurGroup->Items[CurItemsCount] = Item;
    return CurGroup->Items[CurItemsCount];
  }

  using ItemHandlerTy = function_ref<void(T &)>;

  /// Enumerate all items and apply specified \p Handler to each.
  void forEach(ItemHandlerTy Handler) {
    for (ItemsGroup *CurGroup = GroupsHead; CurGroup;
         CurGroup = CurGroup->Next) {
      for (T &Item : *CurGroup)
        Handler(Item);
    }
  }

  /// Check whether list is empty.
  bool empty() { return !GroupsHead; }

  /// Erase list.
  void erase() {
    GroupsHead = nullptr;
    LastGroup = nullptr;
  }

  void sort(function_ref<bool(const T &LHS, const T &RHS)> Comparator) {
    SmallVector<T> SortedItems;
    forEach([&](T &Item) { SortedItems.push_back(Item); });

    if (SortedItems.size()) {
      std::sort(SortedItems.begin(), SortedItems.end(), Comparator);

      size_t SortedItemIdx = 0;
      forEach([&](T &Item) { Item = SortedItems[SortedItemIdx++]; });
      assert(SortedItemIdx == SortedItems.size());
    }
  }

  size_t size() {
    size_t Result = 0;

    for (ItemsGroup *CurGroup = GroupsHead; CurGroup != nullptr;
         CurGroup = CurGroup->Next)
      Result += CurGroup->getItemsCount();

    return Result;
  }

protected:
  struct ItemsGroup {
    using ArrayTy = std::array<T, ItemsGroupSize>;

    // Array of items kept by this group.
    ArrayTy Items;

    // Pointer to the next items group.
    std::atomic<ItemsGroup *> Next = nullptr;

    // Number of items in this group.
    // NOTE: ItemsCount could be inaccurate as it might be incremented by
    // several threads. Use getItemsCount() method to get real number of items
    // inside ItemsGroup.
    std::atomic<size_t> ItemsCount = 0;

    size_t getItemsCount() const {
      return std::min(ItemsCount.load(), ItemsGroupSize);
    }

    typename ArrayTy::iterator begin() { return Items.begin(); }
    typename ArrayTy::iterator end() { return Items.begin() + getItemsCount(); }
  };

  // Allocate new group. Put allocated group into the \p AtomicGroup if
  // it is empty. If \p AtomicGroup is filled by another thread then
  // put allocated group into the end of groups list.
  // \returns true if allocated group is put into the \p AtomicGroup.
  bool allocateNewGroup(std::atomic<ItemsGroup *> &AtomicGroup) {
    ItemsGroup *CurGroup = nullptr;

    // Allocate new group.
    ItemsGroup *NewGroup = Allocator->Allocate<ItemsGroup>();
    NewGroup->ItemsCount = 0;
    NewGroup->Next = nullptr;

    // Try to replace current group with allocated one.
    if (AtomicGroup.compare_exchange_strong(CurGroup, NewGroup))
      return true;

    // Put allocated group as last group.
    while (CurGroup) {
      ItemsGroup *NextGroup = CurGroup->Next;

      if (!NextGroup) {
        if (CurGroup->Next.compare_exchange_weak(NextGroup, NewGroup))
          break;
      }

      CurGroup = NextGroup;
    }

    return false;
  }

  std::atomic<ItemsGroup *> GroupsHead = nullptr;
  std::atomic<ItemsGroup *> LastGroup = nullptr;
  toolchain::parallel::PerThreadBumpPtrAllocator *Allocator = nullptr;
};

} // end of namespace parallel
} // end of namespace dwarf_linker
} // end of namespace vm::core

#endif // LLVM_LIB_DWARFLINKER_PARALLEL_ARRAYLIST_H
