//===--- StackAllocator.cpp - Unit tests for the StackAllocator -----------===//
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

#include "../../stdlib/public/runtime/StackAllocator.h"
#include "language/ABI/Metadata.h"
#include "gtest/gtest.h"

using namespace language;

static constexpr size_t slabCapacity = 256;
static constexpr size_t firstSlabBufferCapacity = 140;
static constexpr size_t fitsIntoFirstSlab = 16;
static constexpr size_t fitsIntoSlab = slabCapacity - 16;
static constexpr size_t twoFitIntoSlab = slabCapacity / 2 - 32;
static constexpr size_t exceedsSlab = slabCapacity + 16;

static Metadata SlabMetadata;

TEST(StackAllocatorTest, withPreallocatedSlab) {

  char firstSlab[firstSlabBufferCapacity];
  StackAllocator<slabCapacity, &SlabMetadata> allocator(
      firstSlab, firstSlabBufferCapacity);

  char *mem1 = (char *)allocator.alloc(fitsIntoFirstSlab);
  EXPECT_EQ(allocator.getNumAllocatedSlabs(), 0);
  char *mem1a = (char *)allocator.alloc(fitsIntoFirstSlab);
  EXPECT_EQ(allocator.getNumAllocatedSlabs(), 0);

  char *mem2 = (char *)allocator.alloc(exceedsSlab);
  EXPECT_EQ(allocator.getNumAllocatedSlabs(), 1);

  char *mem3 = (char *)allocator.alloc(fitsIntoSlab);
  EXPECT_EQ(allocator.getNumAllocatedSlabs(), 2);

  char *mem4 = (char *)allocator.alloc(fitsIntoSlab);
  EXPECT_EQ(allocator.getNumAllocatedSlabs(), 3);

  allocator.dealloc(mem4);
  allocator.dealloc(mem3);
  EXPECT_EQ(allocator.getNumAllocatedSlabs(), 3);

  char *mem5 = (char *)allocator.alloc(twoFitIntoSlab);
  EXPECT_EQ(allocator.getNumAllocatedSlabs(), 3);
  char *mem6 = (char *)allocator.alloc(twoFitIntoSlab);
  EXPECT_EQ(allocator.getNumAllocatedSlabs(), 3);
  char *mem7 = (char *)allocator.alloc(twoFitIntoSlab);
  EXPECT_EQ(allocator.getNumAllocatedSlabs(), 3);

  allocator.dealloc(mem7);
  allocator.dealloc(mem6);
  allocator.dealloc(mem5);
  EXPECT_EQ(allocator.getNumAllocatedSlabs(), 3);

  char *mem8 = (char *)allocator.alloc(exceedsSlab);
  EXPECT_EQ(allocator.getNumAllocatedSlabs(), 2);

  allocator.dealloc(mem8);
  allocator.dealloc(mem2);
  allocator.dealloc(mem1a);
  allocator.dealloc(mem1);
}

TEST(StackAllocatorTest, withoutPreallocatedSlab) {

  constexpr size_t slabCapacity = 256;

  StackAllocator<slabCapacity, &SlabMetadata> allocator;

  size_t fitsIntoSlab = slabCapacity - 16;
  size_t twoFitIntoSlab = slabCapacity / 2 - 32;
  size_t exceedsSlab = slabCapacity + 16;

  char *mem1 = (char *)allocator.alloc(twoFitIntoSlab);
  EXPECT_EQ(allocator.getNumAllocatedSlabs(), 1);
  char *mem1a = (char *)allocator.alloc(twoFitIntoSlab);
  EXPECT_EQ(allocator.getNumAllocatedSlabs(), 1);

  char *mem2 = (char *)allocator.alloc(exceedsSlab);
  EXPECT_EQ(allocator.getNumAllocatedSlabs(), 2);

  char *mem3 = (char *)allocator.alloc(fitsIntoSlab);
  EXPECT_EQ(allocator.getNumAllocatedSlabs(), 3);

  char *mem4 = (char *)allocator.alloc(fitsIntoSlab);
  EXPECT_EQ(allocator.getNumAllocatedSlabs(), 4);

  allocator.dealloc(mem4);
  allocator.dealloc(mem3);
  EXPECT_EQ(allocator.getNumAllocatedSlabs(), 4);

  char *mem5 = (char *)allocator.alloc(twoFitIntoSlab);
  EXPECT_EQ(allocator.getNumAllocatedSlabs(), 4);
  char *mem6 = (char *)allocator.alloc(twoFitIntoSlab);
  EXPECT_EQ(allocator.getNumAllocatedSlabs(), 4);
  char *mem7 = (char *)allocator.alloc(twoFitIntoSlab);
  EXPECT_EQ(allocator.getNumAllocatedSlabs(), 4);

  allocator.dealloc(mem7);
  allocator.dealloc(mem6);
  allocator.dealloc(mem5);
  EXPECT_EQ(allocator.getNumAllocatedSlabs(), 4);

  char *mem8 = (char *)allocator.alloc(exceedsSlab);
  EXPECT_EQ(allocator.getNumAllocatedSlabs(), 3);

  allocator.dealloc(mem8);
  allocator.dealloc(mem2);
  allocator.dealloc(mem1a);
  allocator.dealloc(mem1);
}
