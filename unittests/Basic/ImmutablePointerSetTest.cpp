//===--- ImmutablePointerSetTest.cpp --------------------------------------===//
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

#include "language/Basic/ImmutablePointerSet.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/Support/Allocator.h"
#include "gtest/gtest.h"

using namespace language;

TEST(ImmutableSortedSet, OneElementSets) {
  toolchain::BumpPtrAllocator BPA;
  ImmutablePointerSetFactory<unsigned *> F(BPA);

  unsigned *ptr1 = (unsigned *)3;
  auto *OneEltSet1 = F.get(ptr1);
  EXPECT_EQ(OneEltSet1, F.get(ptr1));
  EXPECT_EQ(OneEltSet1, F.merge(OneEltSet1, OneEltSet1));

  unsigned *ptr2 = (unsigned *)2;
  auto *OneEltSet2 = F.get(ptr2);
  EXPECT_EQ(OneEltSet2, F.get(ptr2));
  EXPECT_EQ(OneEltSet2, F.merge(OneEltSet2, OneEltSet2));
  EXPECT_NE(OneEltSet2, OneEltSet1);

  auto *Merge1 = F.merge(OneEltSet1, OneEltSet2);
  auto *Merge2 = F.merge(OneEltSet2, OneEltSet1);
  EXPECT_NE(OneEltSet1, Merge1);
  EXPECT_NE(OneEltSet2, Merge1);
  EXPECT_EQ(Merge1, Merge2);
  EXPECT_EQ(Merge1, F.merge(Merge1, Merge1));
  EXPECT_EQ(Merge2, F.merge(Merge2, Merge2));
  EXPECT_EQ(Merge1, F.merge(Merge1, OneEltSet1));
  EXPECT_EQ(Merge1, F.merge(Merge1, OneEltSet2));

  EXPECT_EQ(Merge1->size(), 2U);
  EXPECT_FALSE(Merge1->empty());
  EXPECT_EQ(*Merge1->begin(), (unsigned *)2);
  EXPECT_EQ(*std::next(Merge1->begin()), (unsigned *)3);

  EXPECT_EQ(F.getEmptySet(), F.getEmptySet());
  EXPECT_EQ(OneEltSet1, F.merge(F.getEmptySet(), OneEltSet1));
  EXPECT_EQ(OneEltSet1, F.merge(OneEltSet1, F.getEmptySet()));
}

TEST(ImmutablePointerSet, MultipleElementSets) {
  toolchain::BumpPtrAllocator BPA;
  ImmutablePointerSetFactory<unsigned *> F(BPA);

  unsigned *Ptr1 = (unsigned *)3;
  unsigned *Ptr2 = (unsigned *)4;
  unsigned *Ptr3 = (unsigned *)3;
  unsigned *Ptr4 = (unsigned *)5;
  unsigned *Ptr5 = (unsigned *)6;
  toolchain::SmallVector<unsigned *, 2> Data1 = {Ptr1, Ptr2};
  toolchain::MutableArrayRef<unsigned *> Array = Data1;
  auto *TwoEltSet = F.get(Array);
  EXPECT_FALSE(TwoEltSet->empty());
  EXPECT_EQ(TwoEltSet->size(), 2u);
  EXPECT_TRUE(TwoEltSet->count(Ptr1));
  EXPECT_TRUE(TwoEltSet->count(Ptr2));
  EXPECT_TRUE(TwoEltSet->count(Ptr3));
  EXPECT_FALSE(TwoEltSet->count(Ptr4));
  EXPECT_FALSE(TwoEltSet->count(Ptr5));

  auto *ThreeEltSet = F.merge(F.get(Ptr4), TwoEltSet);
  EXPECT_FALSE(ThreeEltSet->empty());
  EXPECT_EQ(ThreeEltSet->size(), 3u);
  EXPECT_NE(*ThreeEltSet, *TwoEltSet);
  EXPECT_TRUE(ThreeEltSet->count(Ptr1));
  EXPECT_TRUE(ThreeEltSet->count(Ptr2));
  EXPECT_TRUE(ThreeEltSet->count(Ptr3));
  EXPECT_TRUE(ThreeEltSet->count(Ptr4));
  EXPECT_FALSE(ThreeEltSet->count(Ptr5));
  EXPECT_EQ(ThreeEltSet, F.merge(TwoEltSet, ThreeEltSet));

  toolchain::SmallVector<unsigned *, 3> Data2 = {Ptr3, Ptr4, Ptr5};
  auto *PartialOverlapSet = F.get(Data2);
  EXPECT_FALSE(PartialOverlapSet->empty());
  EXPECT_EQ(PartialOverlapSet->size(), 3u);
  EXPECT_TRUE(PartialOverlapSet->count(Ptr1));
  EXPECT_FALSE(PartialOverlapSet->count(Ptr2));
  EXPECT_TRUE(PartialOverlapSet->count(Ptr3));
  EXPECT_TRUE(PartialOverlapSet->count(Ptr4));
  EXPECT_TRUE(PartialOverlapSet->count(Ptr5));
  EXPECT_NE(*PartialOverlapSet, *ThreeEltSet);
  EXPECT_NE(*PartialOverlapSet, *TwoEltSet);

  auto *MixOfThreeAndPartialOverlap = ThreeEltSet->merge(PartialOverlapSet);
  EXPECT_FALSE(MixOfThreeAndPartialOverlap->empty());
  EXPECT_EQ(MixOfThreeAndPartialOverlap->size(), 4u);
  EXPECT_TRUE(MixOfThreeAndPartialOverlap->count(Ptr1));
  EXPECT_TRUE(MixOfThreeAndPartialOverlap->count(Ptr2));
  EXPECT_TRUE(MixOfThreeAndPartialOverlap->count(Ptr3));
  EXPECT_TRUE(MixOfThreeAndPartialOverlap->count(Ptr4));
  EXPECT_TRUE(MixOfThreeAndPartialOverlap->count(Ptr5));
  EXPECT_NE(*MixOfThreeAndPartialOverlap, *PartialOverlapSet);
  EXPECT_NE(*MixOfThreeAndPartialOverlap, *ThreeEltSet);
  EXPECT_NE(*MixOfThreeAndPartialOverlap, *TwoEltSet);
}

TEST(ImmutablePointerSet, EmptyIntersectionTests) {
  toolchain::BumpPtrAllocator BPA;
  ImmutablePointerSetFactory<unsigned *> F(BPA);

  unsigned *Ptr1 = (unsigned *)3;
  unsigned *Ptr2 = (unsigned *)4;
  unsigned *Ptr3 = (unsigned *)3;
  unsigned *Ptr4 = (unsigned *)5;
  unsigned *Ptr5 = (unsigned *)6;

  toolchain::SmallVector<unsigned *, 2> Data1 = {Ptr1, Ptr2};
  toolchain::SmallVector<unsigned *, 2> Data2 = {Ptr3, Ptr2};
  toolchain::SmallVector<unsigned *, 2> Data3 = {Ptr4, Ptr5};
  toolchain::SmallVector<unsigned *, 2> Data4 = {Ptr2, Ptr4};

  EXPECT_FALSE(F.get(Data1)->hasEmptyIntersection(F.get(Data2)));
  EXPECT_TRUE(F.get(Data1)->hasEmptyIntersection(F.get(Data3)));
  EXPECT_FALSE(F.get(Data1)->hasEmptyIntersection(F.get(Data4)));
}
