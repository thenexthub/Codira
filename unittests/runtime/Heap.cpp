//===--- Heap.cpp - Heap tests --------------------------------------------===//
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

#include "language/Runtime/Heap.h"

#include "gtest/gtest.h"

void shouldAlloc(size_t size, size_t alignMask) {
  void *ptr = language::language_slowAlloc(size, alignMask);
  EXPECT_NE(ptr, (void *)NULL)
    << "Allocation failed for size " << size << " and alignment mask "
    << alignMask << ".";
  language::language_slowDealloc(ptr, size, alignMask);
}

void shouldAlloc(size_t size) {
  shouldAlloc(size, 0);
  shouldAlloc(size, 1);
  shouldAlloc(size, 3);
  shouldAlloc(size, 7);
  shouldAlloc(size, 15);
  shouldAlloc(size, 31);
  shouldAlloc(size, 63);
  shouldAlloc(size, 4095);
}

TEST(HeapTest, slowAlloc) {
  shouldAlloc(1);
  shouldAlloc(8);
  shouldAlloc(32);
  shouldAlloc(1093);
}

void shouldAllocTyped(size_t size, size_t alignMask, language::MallocTypeId typeId) {
  void *ptr = language::language_slowAllocTyped(size, alignMask, typeId);
  EXPECT_NE(ptr, (void *)NULL)
    << "Typed allocation failed for size " << size << " and alignment mask "
    << alignMask << ".";
  language::language_slowDealloc(ptr, size, alignMask);
}

void shouldAllocTyped(size_t size, language::MallocTypeId typeId) {
  shouldAlloc(size, 0);
  shouldAlloc(size, 1);
  shouldAlloc(size, 3);
  shouldAlloc(size, 7);
  shouldAlloc(size, 15);
  shouldAlloc(size, 31);
  shouldAlloc(size, 63);
  shouldAlloc(size, 4095);
}

void shouldAllocTyped(size_t size) {
  shouldAllocTyped(size, 42);
}

TEST(HeapTest, slowAllocTyped) {
  shouldAllocTyped(1);
  shouldAllocTyped(8);
  shouldAllocTyped(32);
  shouldAllocTyped(1093);
}

