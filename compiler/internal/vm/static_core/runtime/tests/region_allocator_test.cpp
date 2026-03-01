/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#include <sys/mman.h>
#include <algorithm>
#include <thread>

#include "libarkbase/mem/mem.h"
#include "libarkbase/os/mem.h"
#include "libarkbase/utils/asan_interface.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/math_helpers.h"
#include "runtime/include/runtime.h"
#include "runtime/mem/alloc_config.h"
#include "runtime/mem/tlab.h"
#include "runtime/tests/allocator_test_base.h"
#include "runtime/mem/region_allocator-inl.h"

namespace ark::mem::test {
using NonObjectRegionAllocator = RegionAllocator<EmptyAllocConfigWithCrossingMap>;

static constexpr size_t YOUNG_SPACE_SIZE = 128_MB;

template <typename ObjectAllocator, bool REGULAR_SPACE = true>
class RegionAllocatorTestBase : public AllocatorTest<ObjectAllocator> {
public:
    RegionAllocatorTestBase()
    {
        options_.SetShouldLoadBootPandaFiles(false);
        options_.SetShouldInitializeIntrinsics(false);
        options_.SetYoungSpaceSize(YOUNG_SPACE_SIZE);
        options_.SetHeapSizeLimit(320_MB);  // NOLINT(readability-magic-numbers)
        options_.SetGcType("epsilon");
        Runtime::Create(options_);
        // For tests we don't limit spaces
        size_t spaceSize = options_.GetHeapSizeLimit();
        size_t youngSize = spaceSize;
        if constexpr (!REGULAR_SPACE) {
            // we don't need young space for non-movable or humongous allocator tests
            youngSize = 0;
        }
        spaces_.youngSpace_.Initialize(youngSize, youngSize);
        spaces_.memSpace_.Initialize(spaceSize, spaceSize);
        spaces_.InitializePercentages(0, PERCENT_100_D);
        spaces_.isInitialized_ = true;
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
        classLinker_ = Runtime::GetCurrent()->GetClassLinker();
        auto lang = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        auto *classLinkerExt = Runtime::GetCurrent()->GetClassLinker()->GetExtension(lang);
        testClass_ = classLinkerExt->CreateClass(nullptr, 0, 0, sizeof(ark::Class));
        testClass_->SetObjectSize(OBJECT_SIZE);
    }
    ~RegionAllocatorTestBase() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(RegionAllocatorTestBase);
    NO_MOVE_SEMANTIC(RegionAllocatorTestBase);

protected:
    static constexpr size_t OBJECT_SIZE = 128;
    void AddMemoryPoolToAllocator([[maybe_unused]] ObjectAllocator &allocator) final {}

    void AddMemoryPoolToAllocatorProtected([[maybe_unused]] ObjectAllocator &allocator) final {}

    bool AllocatedByThisAllocator([[maybe_unused]] ObjectAllocator &allocator, [[maybe_unused]] void *mem) override
    {
        return allocator.ContainObject(reinterpret_cast<ObjectHeader *>(mem));
    }

    void InitializeObjectAtMem(ObjectHeader *object)
    {
        object->SetClass(testClass_);
    }

    Class *testClass_;           // NOLINT(misc-non-private-member-variables-in-classes)
    GenerationalSpaces spaces_;  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    ark::MTManagedThread *thread_;
    ClassLinker *classLinker_;
    RuntimeOptions options_;
};

class RegionAllocatorTest : public RegionAllocatorTestBase<NonObjectRegionAllocator> {
public:
    static constexpr size_t TEST_REGION_SPACE_SIZE = YOUNG_SPACE_SIZE;

    size_t GetNumFreeRegions(NonObjectRegionAllocator &allocator)
    {
        return allocator.GetSpace()->GetPool()->GetFreeRegionsNumInRegionBlock();
    }

    static size_t constexpr RegionSize()
    {
        return NonObjectRegionAllocator::REGION_SIZE;
    }

    static size_t constexpr GetRegionsNumber()
    {
        return TEST_REGION_SPACE_SIZE / NonObjectRegionAllocator::REGION_SIZE;
    }

    template <RegionFlag ALLOC_TYPE>
    void *AllocateObjectWithClass(NonObjectRegionAllocator &allocator)
    {
        void *mem = allocator.Alloc<ALLOC_TYPE>(OBJECT_SIZE);
        if (mem == nullptr) {
            return nullptr;
        }
        InitializeObjectAtMem(static_cast<ObjectHeader *>(mem));
        return mem;
    }

    void AllocateRegularObject(NonObjectRegionAllocator &allocator, size_t &freeRegions, size_t &freeBytesForCurReg,
                               size_t size)
    {
        ASSERT_EQ(GetNumFreeRegions(allocator), freeRegions);
        size_t alignSize = AlignUp(size, GetAlignmentInBytes(DEFAULT_ALIGNMENT));
        if (freeBytesForCurReg >= alignSize) {
            ASSERT_TRUE(allocator.Alloc(size) != nullptr)
                << "fail allocate object with size " << alignSize << " with free size " << freeBytesForCurReg;
            freeBytesForCurReg -= alignSize;
        } else if (freeRegions > 0) {
            ASSERT_TRUE(allocator.Alloc(size) != nullptr);
            freeRegions -= 1;
            freeBytesForCurReg = NonObjectRegionAllocator::GetMaxRegularObjectSize() - alignSize;
        } else {
            ASSERT_TRUE(allocator.Alloc(alignSize) == nullptr);
            alignSize = freeBytesForCurReg;
            ASSERT(freeBytesForCurReg % GetAlignmentInBytes(DEFAULT_ALIGNMENT) == 0);
            ASSERT_TRUE(allocator.Alloc(alignSize) != nullptr);
            freeBytesForCurReg = 0;
        }
        auto reg = allocator.GetCurrentRegion<true, RegionFlag::IS_EDEN>();
        ASSERT_EQ(GetNumFreeRegions(allocator), freeRegions);
        ASSERT_EQ(reg->End() - reg->Top(), freeBytesForCurReg);
    }

    void AllocateLargeObject(NonObjectRegionAllocator &allocator, size_t &freeRegions, size_t size)
    {
        ASSERT_EQ(GetNumFreeRegions(allocator), freeRegions);
        size_t allocSize = AlignUp(size, GetAlignmentInBytes(DEFAULT_ALIGNMENT));
        if (allocSize + Region::HeadSize() > freeRegions * RegionSize()) {
            ASSERT_TRUE(allocator.Alloc(allocSize) == nullptr);
            allocSize = std::min(allocSize, freeRegions * NonObjectRegionAllocator::GetMaxRegularObjectSize());
        }
        ASSERT_TRUE(allocator.Alloc(allocSize) != nullptr);
        freeRegions -= (allocSize + Region::HeadSize() + RegionSize() - 1) / RegionSize();
        ASSERT_EQ(GetNumFreeRegions(allocator), freeRegions);
    }

    void *AllocateYoungRegular(NonObjectRegionAllocator &allocator, size_t size)
    {
        auto alignSize = AlignUp(size, GetAlignmentInBytes(DEFAULT_ALIGNMENT));
        return allocator.AllocRegular<RegionFlag::IS_EDEN>(alignSize).mem;
    }

    auto ObjectChecker(size_t &objectFound)
    {
        auto ptrObjectFound = &objectFound;
        auto objectChecker = [ptrObjectFound](ObjectHeader *object) {
            (void)object;
            (*ptrObjectFound)++;
            return ObjectStatus::ALIVE_OBJECT;
        };
        return objectChecker;
    }

    static const int LOOP_COUNT = 100;
};

TEST_F(RegionAllocatorTest, AllocateTooMuchRegularObject)
{
    auto *memStats = new mem::MemStatsType();
    NonObjectRegionAllocator allocator(memStats, &spaces_, SpaceType::SPACE_TYPE_OBJECT, TEST_REGION_SPACE_SIZE, false);
    size_t allocTimes = GetRegionsNumber();
    for (size_t i = 0; i < allocTimes; i++) {
        ASSERT_TRUE(allocator.Alloc(allocator.GetMaxRegularObjectSize() / 2UL + 1UL) != nullptr);
    }
    delete memStats;
    ASSERT_TRUE(allocator.Alloc(allocator.GetMaxRegularObjectSize() / 2UL + 1UL) == nullptr);
}

TEST_F(RegionAllocatorTest, AllocateTooMuchRandomRegularObject)
{
    auto *memStats = new mem::MemStatsType();
    for (int i = 0; i < RegionAllocatorTest::LOOP_COUNT; i++) {
        NonObjectRegionAllocator allocator(memStats, &spaces_, SpaceType::SPACE_TYPE_OBJECT, TEST_REGION_SPACE_SIZE,
                                           false);
        size_t freeRegions = GetRegionsNumber();
        size_t freeBytesForCurReg = 0;
        while (freeRegions != 0 || freeBytesForCurReg != 0) {
            size_t size = RandFromRange(1, allocator.GetMaxRegularObjectSize());
            AllocateRegularObject(allocator, freeRegions, freeBytesForCurReg, size);
        }
        ASSERT_TRUE(allocator.Alloc(1) == nullptr);
    }
    delete memStats;
}

TEST_F(RegionAllocatorTest, AllocateTooMuchLargeObject)
{
    auto *memStats = new mem::MemStatsType();
    NonObjectRegionAllocator allocator(memStats, &spaces_, SpaceType::SPACE_TYPE_OBJECT, TEST_REGION_SPACE_SIZE, false);
    ASSERT_TRUE(allocator.Alloc(allocator.GetMaxRegularObjectSize()) != nullptr);
    size_t allocTimes = (GetRegionsNumber() - 1) / 2;
    for (size_t i = 0; i < allocTimes; i++) {
        ASSERT_TRUE(allocator.Alloc(allocator.GetMaxRegularObjectSize() + 1) != nullptr);
    }
    ASSERT_TRUE(allocator.Alloc(allocator.GetMaxRegularObjectSize() + 1) == nullptr);
    allocator.Alloc(allocator.GetMaxRegularObjectSize());
    ASSERT_TRUE(allocator.Alloc(1) == nullptr);
    delete memStats;
}

TEST_F(RegionAllocatorTest, AllocateTooMuchRandomLargeObject)
{
    auto *memStats = new mem::MemStatsType();
    for (int i = 0; i < RegionAllocatorTest::LOOP_COUNT; i++) {
        NonObjectRegionAllocator allocator(memStats, &spaces_, SpaceType::SPACE_TYPE_OBJECT, TEST_REGION_SPACE_SIZE,
                                           false);
        ASSERT_TRUE(allocator.Alloc(allocator.GetMaxRegularObjectSize()) != nullptr);
        size_t freeRegions = GetRegionsNumber() - 1;
        while (freeRegions > 1) {
            size_t size =
                RandFromRange(allocator.GetMaxRegularObjectSize() + 1, 3 * allocator.GetMaxRegularObjectSize());
            AllocateLargeObject(allocator, freeRegions, size);
        }
        if (freeRegions == 1) {
            ASSERT_TRUE(allocator.Alloc(allocator.GetMaxRegularObjectSize()) != nullptr);
        }
        ASSERT_TRUE(allocator.Alloc(1) == nullptr);
    }
    delete memStats;
}

TEST_F(RegionAllocatorTest, AllocateTooMuchRandomRegularAndLargeObjectTest)
{
    auto *memStats = new mem::MemStatsType();
    for (int i = 0; i < RegionAllocatorTest::LOOP_COUNT; i++) {
        NonObjectRegionAllocator allocator(memStats, &spaces_, SpaceType::SPACE_TYPE_OBJECT, TEST_REGION_SPACE_SIZE,
                                           false);
        size_t freeRegions = GetRegionsNumber();
        size_t freeBytesForCurReg = 0;
        while (freeRegions != 0 || freeBytesForCurReg != 0) {
            ASSERT(freeBytesForCurReg % GetAlignmentInBytes(DEFAULT_ALIGNMENT) == 0);
            size_t size = RandFromRange(1, 3 * allocator.GetMaxRegularObjectSize());
            size_t alignSize = AlignUp(size, GetAlignmentInBytes(DEFAULT_ALIGNMENT));
            if (alignSize <= NonObjectRegionAllocator::GetMaxRegularObjectSize()) {
                AllocateRegularObject(allocator, freeRegions, freeBytesForCurReg, alignSize);
            } else if (freeRegions > 1) {
                AllocateLargeObject(allocator, freeRegions, alignSize);
            }
        }
        ASSERT_TRUE(allocator.Alloc(1) == nullptr);
    }
    delete memStats;
}

TEST_F(RegionAllocatorTest, AllocatedByRegionAllocatorTest)
{
    mem::MemStatsType memStats;
    NonObjectRegionAllocator allocator(&memStats, &spaces_, SpaceType::SPACE_TYPE_OBJECT, TEST_REGION_SPACE_SIZE,
                                       false);
    AllocatedByThisAllocatorTest(allocator);
}

TEST_F(RegionAllocatorTest, OneAlignmentAllocTest)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    OneAlignedAllocFreeTest<NonObjectRegionAllocator::GetMaxRegularObjectSize() - 128UL,
                            // NOLINTNEXTLINE(readability-magic-numbers)
                            NonObjectRegionAllocator::GetMaxRegularObjectSize() + 128UL, DEFAULT_ALIGNMENT>(1UL,
                                                                                                            &spaces_);
}

TEST_F(RegionAllocatorTest, AllocateFreeDifferentSizesTest)
{
    static constexpr size_t ELEMENTS_COUNT = 256;
    static constexpr size_t POOLS_COUNT = 1;
    // NOLINTNEXTLINE(readability-magic-numbers)
    AllocateFreeDifferentSizesTest<NonObjectRegionAllocator::GetMaxRegularObjectSize() - 128UL,
                                   // NOLINTNEXTLINE(readability-magic-numbers)
                                   NonObjectRegionAllocator::GetMaxRegularObjectSize() + 128UL>(ELEMENTS_COUNT,
                                                                                                POOLS_COUNT, &spaces_);
}

TEST_F(RegionAllocatorTest, RegionTLABAllocTest)
{
    static constexpr size_t TLAB_SIZE = 4_KB;
    static constexpr size_t ALLOC_SIZE = 512;
    static constexpr size_t ALLOC_COUNT = 5000000;
    auto *memStats = new mem::MemStatsType();
    NonObjectRegionAllocator allocator(memStats, &spaces_, SpaceType::SPACE_TYPE_OBJECT, TEST_REGION_SPACE_SIZE, false);
    bool isOom = false;
    TLAB *tlab = allocator.CreateTLAB(TLAB_SIZE);
    for (size_t i = 0; i < ALLOC_COUNT; i++) {
        auto oldStartPointer = tlab->GetStartAddr();
        auto mem = tlab->Alloc(ALLOC_SIZE);
        // checking new tlab address
        if (mem == nullptr) {
            auto newTlab = allocator.CreateTLAB(TLAB_SIZE);
            if (newTlab != nullptr) {
                auto newStartPointer = newTlab->GetStartAddr();
                ASSERT_NE(newStartPointer, nullptr);
                ASSERT_NE(newStartPointer, oldStartPointer);
                ASSERT_NE(newTlab, tlab);
                tlab = newTlab;
                mem = tlab->Alloc(ALLOC_SIZE);
            }
        }
        if (mem == nullptr) {
            ASSERT_EQ(GetNumFreeRegions(allocator), 0);
            isOom = true;
            break;
        }
        ASSERT_NE(mem, nullptr);
    }
    ASSERT_EQ(isOom, true) << "Increase the size of alloc_count to get OOM";
    delete memStats;
}

TEST_F(RegionAllocatorTest, RegionPoolTest)
{
    mem::MemStatsType memStats;
    NonObjectRegionAllocator allocator(&memStats, &spaces_, SpaceType::SPACE_TYPE_OBJECT, RegionSize() * 2U, true);

    // alloc two small objects in a region
    ASSERT_EQ(GetNumFreeRegions(allocator), 2U);
    auto *obj1 = reinterpret_cast<ObjectHeader *>(allocator.Alloc(1UL));  // one byte
    ASSERT_TRUE(obj1 != nullptr);
    ASSERT_EQ(GetNumFreeRegions(allocator), 1UL);
    auto *obj2 = reinterpret_cast<ObjectHeader *>(allocator.Alloc(DEFAULT_ALIGNMENT_IN_BYTES + 2U));  // two byte
    ASSERT_TRUE(obj2 != nullptr);
    ASSERT_EQ(GetNumFreeRegions(allocator), 1UL);

    // check that the two objects should be in a region
    ASSERT_EQ(ToUintPtr(obj2), ToUintPtr(obj1) + DEFAULT_ALIGNMENT_IN_BYTES);
    auto *region1 = allocator.GetRegion(obj1);
    ASSERT_TRUE(region1 != nullptr);
    auto *region2 = allocator.GetRegion(obj2);
    ASSERT_TRUE(region2 != nullptr);
    ASSERT_EQ(region1, region2);
    ASSERT_EQ(region1->Top() - region1->Begin(), 3U * DEFAULT_ALIGNMENT_IN_BYTES);

    // allocate a large object in pool(not in initial block)
    ASSERT_EQ(GetNumFreeRegions(allocator), 1);
    // NOLINTNEXTLINE(readability-magic-numbers)
    auto *obj3 = reinterpret_cast<ObjectHeader *>(allocator.Alloc(allocator.GetMaxRegularObjectSize() + 200U));
    ASSERT_TRUE(obj3 != nullptr);
    ASSERT_EQ(GetNumFreeRegions(allocator), 1UL);
    auto *region3 = allocator.GetRegion(obj3);
    ASSERT_TRUE(region3 != nullptr);
    ASSERT_NE(region2, region3);
    ASSERT_TRUE(region3->HasFlag(RegionFlag::IS_LARGE_OBJECT));

    // allocate a regular object which can't be allocated in current region
    auto *obj4 = reinterpret_cast<ObjectHeader *>(
        allocator.Alloc(allocator.GetMaxRegularObjectSize() - DEFAULT_ALIGNMENT_IN_BYTES));
    ASSERT_TRUE(obj4 != nullptr);
    ASSERT_EQ(GetNumFreeRegions(allocator), 0);
    auto *region4 = allocator.GetRegion(obj4);
    ASSERT_TRUE(region4 != nullptr);
    ASSERT_EQ(ToUintPtr(region4), ToUintPtr(region2) + RegionSize());

    auto *obj5 = reinterpret_cast<ObjectHeader *>(allocator.Alloc(DEFAULT_ALIGNMENT_IN_BYTES));
    ASSERT_TRUE(obj5 != nullptr);
    auto *region5 = allocator.GetRegion(obj5);
    ASSERT_EQ(region4, region5);
}

TEST_F(RegionAllocatorTest, IterateOverObjectsTest)
{
    mem::MemStatsType memStats;
    NonObjectRegionAllocator allocator(&memStats, &spaces_, SpaceType::SPACE_TYPE_OBJECT, 0, true);
    auto *obj1 = reinterpret_cast<ObjectHeader *>(allocator.Alloc(testClass_->GetObjectSize()));
    obj1->SetClass(testClass_);
    auto *obj2 = reinterpret_cast<ObjectHeader *>(allocator.Alloc(testClass_->GetObjectSize()));
    obj2->SetClass(testClass_);
    auto *region = allocator.GetRegion(obj1);
    size_t obj1Num = 0;
    size_t obj2Num = 0;
    region->IterateOverObjects([this, obj1, obj2, region, &obj1Num, &obj2Num, &allocator](ObjectHeader *object) {
        ASSERT_TRUE(object == obj1 || object == obj2);
        ASSERT_EQ(allocator.GetRegion(object), region);
        ASSERT_EQ(object->ClassAddr<Class>(), testClass_);
        if (object == obj1) {
            obj1Num++;
        } else if (object == obj2) {
            obj2Num++;
        }

#ifndef NDEBUG
        // can't allocator object while iterating the region
        ASSERT_DEATH(allocator.Alloc(testClass_->GetObjectSize()), "");
#endif
    });
    ASSERT_EQ(obj1Num, 1);
    ASSERT_EQ(obj2Num, 1);

#ifndef NDEBUG
    ASSERT_TRUE(region->SetAllocating(true));
    // can't iterating the region while allocating
    ASSERT_DEATH(region->IterateOverObjects([]([[maybe_unused]] ObjectHeader *object) {}), "");
    ASSERT_TRUE(region->SetAllocating(false));
#endif
}

TEST_F(RegionAllocatorTest, AllocateAndMoveYoungObjectsToTenured)
{
    static constexpr size_t ALLOCATION_COUNT = 10000;
    static constexpr size_t TENURED_OBJECTS_CREATION_RATE = 4;
    mem::MemStatsType memStats;
    NonObjectRegionAllocator allocator(&memStats, &spaces_, SpaceType::SPACE_TYPE_OBJECT, TEST_REGION_SPACE_SIZE,
                                       false);
    // Allocate some objects (young and tenured) in allocator
    for (size_t i = 0; i < ALLOCATION_COUNT; i++) {
        void *mem = nullptr;
        if (i % TENURED_OBJECTS_CREATION_RATE == 0) {
            mem = AllocateObjectWithClass<RegionFlag::IS_OLD>(allocator);
        } else {
            mem = AllocateObjectWithClass<RegionFlag::IS_EDEN>(allocator);
        }
        ASSERT_TRUE(mem != nullptr);
    }
    // Iterate over young objects and move them into tenured:
    allocator.CompactAllSpecificRegions<RegionFlag::IS_EDEN, RegionFlag::IS_OLD>(
        [&](ObjectHeader *object) {
            (void)object;
            return ObjectStatus::ALIVE_OBJECT;
        },
        []([[maybe_unused]] ObjectHeader *src, [[maybe_unused]] ObjectHeader *dst) {});
    allocator.ResetAllSpecificRegions<RegionFlag::IS_EDEN>();
    size_t objectFound = 0;
    allocator.IterateOverObjects([&objectFound](ObjectHeader *object) {
        (void)object;
        objectFound++;
    });
    ASSERT_EQ(objectFound, ALLOCATION_COUNT);
}

TEST_F(RegionAllocatorTest, AllocateAndCompactTenuredObjects)
{
    static constexpr size_t ALLOCATION_COUNT = 7000;
    static constexpr size_t YOUNG_OBJECTS_CREATION_RATE = 100;
    mem::MemStatsType memStats;
    NonObjectRegionAllocator allocator(&memStats, &spaces_, SpaceType::SPACE_TYPE_OBJECT, TEST_REGION_SPACE_SIZE,
                                       false);
    PandaVector<Region *> regionsVector;
    size_t tenuredObjectCount = 0;
    // Allocate some objects (young and tenured) in allocator
    for (size_t i = 0; i < ALLOCATION_COUNT; i++) {
        void *mem = nullptr;
        if (i % YOUNG_OBJECTS_CREATION_RATE != 0) {
            mem = AllocateObjectWithClass<RegionFlag::IS_OLD>(allocator);
            tenuredObjectCount++;
            Region *region = allocator.GetRegion(static_cast<ObjectHeader *>(mem));
            if (std::find(regionsVector.begin(), regionsVector.end(), region) == regionsVector.end()) {
                regionsVector.insert(regionsVector.begin(), region);
            }
        } else {
            mem = AllocateObjectWithClass<RegionFlag::IS_EDEN>(allocator);
        }
        ASSERT_TRUE(mem != nullptr);
    }
    ASSERT_TRUE(regionsVector.size() > 1);
    ASSERT_EQ(allocator.GetAllSpecificRegions<RegionFlag::IS_OLD>().size(), regionsVector.size());
    // Iterate over some tenured regions and compact them:
    allocator.ClearCurrentRegion<RegionFlag::IS_OLD>();
    size_t objectFound = 0;
    allocator.CompactSeveralSpecificRegions<RegionFlag::IS_OLD, RegionFlag::IS_OLD>(
        regionsVector, ObjectChecker(objectFound),
        []([[maybe_unused]] ObjectHeader *from, [[maybe_unused]] ObjectHeader *to) {
            // no need anything here
        });
    ASSERT_EQ(objectFound, tenuredObjectCount);
    objectFound = 0;
    allocator.IterateOverObjects([&objectFound](ObjectHeader *object) {
        (void)object;
        objectFound++;
    });
    ASSERT_EQ(objectFound, ALLOCATION_COUNT + tenuredObjectCount);
    allocator.ResetSeveralSpecificRegions<RegionFlag::IS_OLD, RegionSpace::ReleaseRegionsPolicy::NoRelease,
                                          OSPagesPolicy::IMMEDIATE_RETURN, false>(regionsVector);
    // Check that we have the same object amount.
    objectFound = 0;
    allocator.IterateOverObjects([&objectFound](ObjectHeader *object) {
        (void)object;
        objectFound++;
    });
    ASSERT_EQ(objectFound, ALLOCATION_COUNT);
    // Check that we can still correctly allocate smth in tenured:
    ASSERT_TRUE(AllocateObjectWithClass<RegionFlag::IS_OLD>(allocator) != nullptr);
    // Reset tenured regions:
    allocator.ResetAllSpecificRegions<RegionFlag::IS_OLD>();
    // Check that we can still correctly allocate smth in tenured:
    ASSERT_TRUE(AllocateObjectWithClass<RegionFlag::IS_OLD>(allocator) != nullptr);
}

TEST_F(RegionAllocatorTest, AllocateAndCompactTenuredObjectsViaMarkedBitmap)
{
    static constexpr size_t ALLOCATION_COUNT = 7000;
    static constexpr size_t MARKED_OBJECTS_RATE = 2;
    mem::MemStatsType memStats;
    NonObjectRegionAllocator allocator(&memStats, &spaces_, SpaceType::SPACE_TYPE_OBJECT, TEST_REGION_SPACE_SIZE,
                                       false);
    PandaVector<Region *> regionsVector;
    size_t markedTenuredObjectCount = 0;
    // Allocate some objects (young and tenured) in allocator
    for (size_t i = 0; i < ALLOCATION_COUNT; i++) {
        void *mem = AllocateObjectWithClass<RegionFlag::IS_OLD>(allocator);
        Region *region = allocator.GetRegion(static_cast<ObjectHeader *>(mem));
        if (std::find(regionsVector.begin(), regionsVector.end(), region) == regionsVector.end()) {
            regionsVector.insert(regionsVector.begin(), region);
        }
        if (i % MARKED_OBJECTS_RATE != 0) {
            region->SetMarkBit(static_cast<ObjectHeader *>(mem));
            markedTenuredObjectCount++;
        }
        ASSERT_TRUE(mem != nullptr);
    }
    ASSERT_TRUE(regionsVector.size() > 1);
    ASSERT_EQ(allocator.GetAllSpecificRegions<RegionFlag::IS_OLD>().size(), regionsVector.size());
    // Iterate over some tenured regions and compact them:
    allocator.ClearCurrentRegion<RegionFlag::IS_OLD>();
    size_t objectFound = 0;
    allocator.CompactSeveralSpecificRegions<RegionFlag::IS_OLD, RegionFlag::IS_OLD, true>(
        regionsVector, []([[maybe_unused]] ObjectHeader *object) { return ObjectStatus::ALIVE_OBJECT; },
        [&objectFound]([[maybe_unused]] ObjectHeader *from, [[maybe_unused]] ObjectHeader *to) { ++objectFound; });
    ASSERT_EQ(objectFound, markedTenuredObjectCount);
    objectFound = 0;
    allocator.IterateOverObjects([&objectFound](ObjectHeader *object) {
        (void)object;
        objectFound++;
    });
    ASSERT_EQ(objectFound, ALLOCATION_COUNT + markedTenuredObjectCount);
    allocator.ResetSeveralSpecificRegions<RegionFlag::IS_OLD, RegionSpace::ReleaseRegionsPolicy::NoRelease,
                                          OSPagesPolicy::IMMEDIATE_RETURN, false>(regionsVector);
    // Check that we have the same object amount.
    objectFound = 0;
    allocator.IterateOverObjects([&objectFound](ObjectHeader *object) {
        (void)object;
        objectFound++;
    });
    ASSERT_EQ(objectFound, markedTenuredObjectCount);
    // Check that we can still correctly allocate smth in tenured:
    ASSERT_TRUE(AllocateObjectWithClass<RegionFlag::IS_OLD>(allocator) != nullptr);
    // Reset tenured regions:
    allocator.ResetAllSpecificRegions<RegionFlag::IS_OLD>();
    // Check that we can still correctly allocate smth in tenured:
    ASSERT_TRUE(AllocateObjectWithClass<RegionFlag::IS_OLD>(allocator) != nullptr);
}

TEST_F(RegionAllocatorTest, AsanTest)
{
    static constexpr size_t ALLOCATION_COUNT = 100;
    static constexpr size_t TENURED_OBJECTS_CREATION_RATE = 4;
    mem::MemStatsType memStats;
    NonObjectRegionAllocator allocator(&memStats, &spaces_, SpaceType::SPACE_TYPE_OBJECT, TEST_REGION_SPACE_SIZE,
                                       false);
    std::vector<void *> youngObjects;
    std::vector<void *> oldObjects;
    // Allocate some objects (young and tenured) in allocator
    for (size_t i = 0; i < ALLOCATION_COUNT; i++) {
        if (i % TENURED_OBJECTS_CREATION_RATE == 0) {
            oldObjects.push_back(AllocateObjectWithClass<RegionFlag::IS_OLD>(allocator));
        } else {
            youngObjects.push_back(AllocateObjectWithClass<RegionFlag::IS_EDEN>(allocator));
        }
    }
    // Iterate over young objects and move them into tenured:
    allocator.CompactAllSpecificRegions<RegionFlag::IS_EDEN, RegionFlag::IS_OLD>(
        [&](ObjectHeader *object) {
            (void)object;
            return ObjectStatus::ALIVE_OBJECT;
        },
        []([[maybe_unused]] ObjectHeader *src, [[maybe_unused]] ObjectHeader *dst) {});
    allocator.ResetAllSpecificRegions<RegionFlag::IS_EDEN>();
    for (auto i : youngObjects) {
#ifdef PANDA_ASAN_ON
        EXPECT_DEATH(DeathWriteUint64(i), "") << "Write " << sizeof(uint64_t) << " bytes at address " << std::hex << i;
#else
        (void)i;
#endif  // PANDA_ASAN_ON
    }
    allocator.ResetAllSpecificRegions<RegionFlag::IS_OLD>();
    for (auto i : oldObjects) {
#ifdef PANDA_ASAN_ON
        EXPECT_DEATH(DeathWriteUint64(i), "") << "Write " << sizeof(uint64_t) << " bytes at address " << std::hex << i;
#else
        (void)i;
#endif  // PANDA_ASAN_ON
    }
}

TEST_F(RegionAllocatorTest, MTAllocTest)
{
#if defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_32)
    // We have an issue with QEMU during MT tests. Issue 2852
    static constexpr size_t THREADS_COUNT = 1;
#else
    static constexpr size_t THREADS_COUNT = 10;
#endif
    static constexpr size_t MIN_MT_ALLOC_SIZE = 16;
    static constexpr size_t MAX_MT_ALLOC_SIZE = 256;
    static constexpr size_t MIN_ELEMENTS_COUNT = 500;
    static constexpr size_t MAX_ELEMENTS_COUNT = 1000;
    static constexpr size_t MT_TEST_RUN_COUNT = 20;
    for (size_t i = 0; i < MT_TEST_RUN_COUNT; i++) {
        mem::MemStatsType memStats;
        // NOLINTNEXTLINE(readability-magic-numbers)
        NonObjectRegionAllocator allocator(&memStats, &spaces_, SpaceType::SPACE_TYPE_OBJECT, RegionSize() * 128U,
                                           true);
        MtAllocTest<MIN_MT_ALLOC_SIZE, MAX_MT_ALLOC_SIZE, THREADS_COUNT>(&allocator, MIN_ELEMENTS_COUNT,
                                                                         MAX_ELEMENTS_COUNT);
    }
}

TEST_F(RegionAllocatorTest, MTAllocLargeTest)
{
#if defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_32)
    // We have an issue with QEMU during MT tests. Issue 2852
    static constexpr size_t THREADS_COUNT = 1;
#else
    static constexpr size_t THREADS_COUNT = 10;
#endif
    static constexpr size_t MIN_MT_ALLOC_SIZE = 128;
    static constexpr size_t MAX_MT_ALLOC_SIZE = NonObjectRegionAllocator::GetMaxRegularObjectSize() * 3U;
    static constexpr size_t MIN_ELEMENTS_COUNT = 10;
    static constexpr size_t MAX_ELEMENTS_COUNT = 30;
    static constexpr size_t MT_TEST_RUN_COUNT = 20;
    for (size_t i = 0; i < MT_TEST_RUN_COUNT; i++) {
        mem::MemStatsType memStats;
        // NOLINTNEXTLINE(readability-magic-numbers)
        NonObjectRegionAllocator allocator(&memStats, &spaces_, SpaceType::SPACE_TYPE_OBJECT, RegionSize() * 256U,
                                           true);
        MtAllocTest<MIN_MT_ALLOC_SIZE, MAX_MT_ALLOC_SIZE, THREADS_COUNT>(&allocator, MIN_ELEMENTS_COUNT,
                                                                         MAX_ELEMENTS_COUNT);
    }
}

TEST_F(RegionAllocatorTest, ConcurrentAllocRegular)
{
    mem::MemStatsType memStats;

    constexpr size_t SPACE_SIZE = RegionSize() * 1024U;
    NonObjectRegionAllocator allocator(&memStats, &spaces_, SpaceType::SPACE_TYPE_OBJECT, SPACE_SIZE, true);

    auto allocateObjects = [&allocator, this](std::vector<void *> &vec) {
        constexpr size_t ITERATIONS = 100'500;
        constexpr size_t OBJ_SIZE = DEFAULT_REGION_SIZE - Region::HeadSize();
        vec.reserve(ITERATIONS);
        [[maybe_unused]] volatile size_t cnt = 0;
        for (size_t i = 0; i < ITERATIONS; ++i) {
            // We need this loop to make the test longer and split threads on physical cores
            // and increase the probability of the test failure
            for (size_t j = 0; j < ITERATIONS; ++j) {
                cnt += j;
            }
            void *mem = AllocateYoungRegular(allocator, OBJ_SIZE);
            if (mem == nullptr) {
                break;
            }
            vec.push_back(mem);
        }
    };

    std::vector<void *> vec1;
    std::vector<void *> vec2;

    std::thread worker([&allocateObjects, &vec1] {
        os::CpuAffinityManager::SetAffinityForCurrentThread(os::CpuPower::WEAK);
        allocateObjects(vec1);
    });

    os::CpuAffinityManager::SetAffinityForCurrentThread(os::CpuPower::BEST);
    allocateObjects(vec2);

    worker.join();

    for (auto &elem1 : vec1) {
        for (auto &elem2 : vec2) {
            ASSERT_TRUE(elem1 != elem2);
        }
    }
}

using RegionNonmovableObjectAllocator =
    RegionRunslotsAllocator<ObjectAllocConfigWithCrossingMap, RegionAllocatorLockConfig::CommonLock>;
class RegionNonmovableObjectAllocatorTest : public RegionAllocatorTestBase<RegionNonmovableObjectAllocator, false> {};

using RegionNonmovableLargeObjectAllocator =
    RegionFreeListAllocator<ObjectAllocConfigWithCrossingMap, RegionAllocatorLockConfig::CommonLock>;
class RegionNonmovableLargeObjectAllocatorTest
    : public RegionAllocatorTestBase<RegionNonmovableLargeObjectAllocator, false> {};

TEST_F(RegionNonmovableObjectAllocatorTest, AllocatorTest)
{
    mem::MemStatsType memStats;
    RegionNonmovableObjectAllocator allocator(&memStats, &spaces_, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    for (uint32_t i = 8; i <= RegionNonmovableObjectAllocator::GetMaxSize(); i++) {
        ASSERT_TRUE(allocator.Alloc(i) != nullptr);
    }
}

TEST_F(RegionNonmovableObjectAllocatorTest, MTAllocatorTest)
{
#if defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_32)
    // We have an issue with QEMU during MT tests. Issue 2852
    static constexpr size_t THREADS_COUNT = 1;
#else
    static constexpr size_t THREADS_COUNT = 10;
#endif
    static constexpr size_t MIN_MT_ALLOC_SIZE = 8;
    static constexpr size_t MAX_MT_ALLOC_SIZE = RegionNonmovableObjectAllocator::GetMaxSize();
    static constexpr size_t MIN_ELEMENTS_COUNT = 200;
    static constexpr size_t MAX_ELEMENTS_COUNT = 300;
    static constexpr size_t MT_TEST_RUN_COUNT = 20;
    for (size_t i = 0; i < MT_TEST_RUN_COUNT; i++) {
        mem::MemStatsType memStats;
        RegionNonmovableObjectAllocator allocator(&memStats, &spaces_, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
        MtAllocTest<MIN_MT_ALLOC_SIZE, MAX_MT_ALLOC_SIZE, THREADS_COUNT>(&allocator, MIN_ELEMENTS_COUNT,
                                                                         MAX_ELEMENTS_COUNT);
        // region is allocated in allocator, so don't free it explicitly
        allocator.VisitAndRemoveAllPools([]([[maybe_unused]] void *mem, [[maybe_unused]] size_t size) {});
    }
}

TEST_F(RegionNonmovableLargeObjectAllocatorTest, AllocatorTest)
{
    mem::MemStatsType memStats;
    RegionNonmovableLargeObjectAllocator allocator(&memStats, &spaces_, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    size_t startObjectSize = RegionNonmovableObjectAllocator::GetMaxSize() + 1;
    // NOLINTNEXTLINE(readability-magic-numbers)
    for (uint32_t i = startObjectSize; i <= startObjectSize + 200U; i++) {
        ASSERT_TRUE(allocator.Alloc(i) != nullptr);
    }
    ASSERT_TRUE(allocator.Alloc(RegionNonmovableLargeObjectAllocator::GetMaxSize() - 1) != nullptr);
    ASSERT_TRUE(allocator.Alloc(RegionNonmovableLargeObjectAllocator::GetMaxSize()) != nullptr);
}

TEST_F(RegionNonmovableLargeObjectAllocatorTest, MTAllocatorTest)
{
#if defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_32)
    // We have an issue with QEMU during MT tests. Issue 2852
    static constexpr size_t THREADS_COUNT = 1;
#else
    static constexpr size_t THREADS_COUNT = 10;
#endif
    static constexpr size_t MIN_MT_ALLOC_SIZE = RegionNonmovableObjectAllocator::GetMaxSize() + 1;
    static constexpr size_t MAX_MT_ALLOC_SIZE = RegionNonmovableLargeObjectAllocator::GetMaxSize();
    static constexpr size_t MIN_ELEMENTS_COUNT = 10;
    static constexpr size_t MAX_ELEMENTS_COUNT = 20;
    static constexpr size_t MT_TEST_RUN_COUNT = 20;
    for (size_t i = 0; i < MT_TEST_RUN_COUNT; i++) {
        mem::MemStatsType memStats;
        RegionNonmovableLargeObjectAllocator allocator(&memStats, &spaces_, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
        MtAllocTest<MIN_MT_ALLOC_SIZE, MAX_MT_ALLOC_SIZE, THREADS_COUNT>(&allocator, MIN_ELEMENTS_COUNT,
                                                                         MAX_ELEMENTS_COUNT);
        // region is allocated in allocator, so don't free it explicitly
        allocator.VisitAndRemoveAllPools([]([[maybe_unused]] void *mem, [[maybe_unused]] size_t size) {});
    }
}

TEST_F(RegionNonmovableLargeObjectAllocatorTest, MemStatsAllocatorTest)
{
    mem::MemStatsType memStats;
    RegionNonmovableLargeObjectAllocator allocator(&memStats, &spaces_, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    static constexpr size_t ALLOC_SIZE = 128;
    void *mem = nullptr;

    auto objectAllocatedSize = memStats.GetAllocated(SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    ASSERT_TRUE(objectAllocatedSize == 0);
    mem = allocator.Alloc(ALLOC_SIZE);
    ASSERT_TRUE(mem != nullptr);
    auto objectAllocatedSize1 = memStats.GetAllocated(SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    ASSERT_EQ(memStats.GetTotalObjectsAllocated(), 1);
    ASSERT_TRUE(objectAllocatedSize1 != 0);

    mem = allocator.Alloc(ALLOC_SIZE);
    ASSERT_TRUE(mem != nullptr);
    auto objectAllocatedSize2 = memStats.GetAllocated(SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    ASSERT_EQ(memStats.GetTotalObjectsAllocated(), 2U);
    ASSERT_EQ(objectAllocatedSize2, objectAllocatedSize1 + objectAllocatedSize1);
}

using RegionHumongousObjectAllocator =
    RegionHumongousAllocator<ObjectAllocConfig, RegionAllocatorLockConfig::CommonLock>;
class RegionHumongousObjectAllocatorTest : public RegionAllocatorTestBase<RegionHumongousObjectAllocator, false> {
protected:
    static constexpr auto REGION_VISITOR = []([[maybe_unused]] PandaVector<Region *> &regions) {};
};

TEST_F(RegionHumongousObjectAllocatorTest, AllocatorTest)
{
    static constexpr size_t MAX_ALLOC_SIZE = 5_MB;
    static constexpr size_t ALLOC_COUNT = 20;
    mem::MemStatsType memStats;
    RegionHumongousObjectAllocator allocator(&memStats, &spaces_, SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT);
    for (uint32_t i = MAX_ALLOC_SIZE / ALLOC_COUNT; i <= MAX_ALLOC_SIZE; i += MAX_ALLOC_SIZE / ALLOC_COUNT) {
        ASSERT_TRUE(allocator.Alloc(i) != nullptr);
    }
}

TEST_F(RegionHumongousObjectAllocatorTest, MTAllocatorTest)
{
#if defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_32)
    // We have an issue with QEMU during MT tests. Issue 2852
    static constexpr size_t THREADS_COUNT = 1;
#else
    static constexpr size_t THREADS_COUNT = 5;
#endif
    static constexpr size_t MIN_MT_ALLOC_SIZE = DEFAULT_REGION_SIZE;
    static constexpr size_t MAX_MT_ALLOC_SIZE = 1_MB;
    static constexpr size_t MIN_ELEMENTS_COUNT = 20;
    static constexpr size_t MAX_ELEMENTS_COUNT = 30;
    // Test with DEFAULT_REGION_SIZE
    {
        mem::MemStatsType memStats;
        RegionHumongousObjectAllocator allocator(&memStats, &spaces_, SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT);
        MtAllocTest<MIN_MT_ALLOC_SIZE, MIN_MT_ALLOC_SIZE, THREADS_COUNT>(&allocator, MIN_ELEMENTS_COUNT,
                                                                         MAX_ELEMENTS_COUNT);
        allocator.VisitAndRemoveAllPools([]([[maybe_unused]] void *mem, [[maybe_unused]] size_t size) {});
    }
    // Test with 1Mb
    {
        mem::MemStatsType memStats;
        RegionHumongousObjectAllocator allocator(&memStats, &spaces_, SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT);
        MtAllocTest<MAX_MT_ALLOC_SIZE, MAX_MT_ALLOC_SIZE, THREADS_COUNT>(&allocator, MIN_ELEMENTS_COUNT,
                                                                         MAX_ELEMENTS_COUNT);
        allocator.VisitAndRemoveAllPools([]([[maybe_unused]] void *mem, [[maybe_unused]] size_t size) {});
    }
}

TEST_F(RegionHumongousObjectAllocatorTest, CollectTest)
{
    static constexpr size_t MIN_ALLOC_SIZE = 1_MB;
    static constexpr size_t MAX_ALLOC_SIZE = 9_MB;
    static constexpr size_t ALLOCATION_COUNT = 50;
    std::vector<void *> allocatedElements;
    mem::MemStatsType memStats;
    RegionHumongousObjectAllocator allocator(&memStats, &spaces_, SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT);
    size_t currentAllocSize = MIN_ALLOC_SIZE;
    auto lang = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *classLinkerExt = Runtime::GetCurrent()->GetClassLinker()->GetExtension(lang);
    for (size_t i = 0; i < ALLOCATION_COUNT; i++) {
        auto testClass = classLinkerExt->CreateClass(nullptr, 0, 0, sizeof(ark::Class));
        testClass->SetObjectSize(currentAllocSize);
        void *mem = allocator.Alloc(currentAllocSize);
        ASSERT_TRUE(mem != nullptr);
        allocatedElements.push_back(mem);
        auto object = static_cast<ObjectHeader *>(mem);
        object->SetClass(testClass);
        currentAllocSize += ((MAX_ALLOC_SIZE - MIN_ALLOC_SIZE) / ALLOCATION_COUNT);
    }
    static std::set<void *> foundedElements;
    static auto deleteAll = [](ObjectHeader *object) {
        foundedElements.insert(object);
        return ObjectStatus::ALIVE_OBJECT;
    };
    // Collect all objects into unordered_set via allocator's method
    allocator.CollectAndRemoveFreeRegions(REGION_VISITOR, deleteAll);
    for (auto i : allocatedElements) {
        auto element = foundedElements.find(i);
        ASSERT_TRUE(element != foundedElements.end());
        foundedElements.erase(element);
    }
    ASSERT_TRUE(foundedElements.empty());
}

TEST_F(RegionHumongousObjectAllocatorTest, TestCollectAliveObject)
{
    mem::MemStatsType memStats;
    RegionHumongousObjectAllocator allocator(&memStats, &spaces_, SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT);
    auto lang = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *classLinkerExt = Runtime::GetCurrent()->GetClassLinker()->GetExtension(lang);
    auto testClass = classLinkerExt->CreateClass(nullptr, 0, 0, sizeof(ark::Class));
    size_t objectSize = DEFAULT_REGION_SIZE + 1;
    testClass->SetObjectSize(objectSize);
    void *mem = allocator.Alloc(objectSize);
    ASSERT_TRUE(mem != nullptr);
    auto object = static_cast<ObjectHeader *>(mem);
    object->SetClass(testClass);
    Region *region = ObjectToRegion(object);

    allocator.CollectAndRemoveFreeRegions(REGION_VISITOR, [](ObjectHeader *) { return ObjectStatus::ALIVE_OBJECT; });
    bool hasRegion = false;
    allocator.GetSpace()->IterateRegions([region, &hasRegion](Region *r) { hasRegion |= region == r; });
    ASSERT_TRUE(hasRegion);
    ASSERT(!region->HasFlag(RegionFlag::IS_FREE));
}

TEST_F(RegionHumongousObjectAllocatorTest, TestCollectDeadObject)
{
    mem::MemStatsType memStats;
    RegionHumongousObjectAllocator allocator(&memStats, &spaces_, SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT);
    auto lang = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *classLinkerExt = Runtime::GetCurrent()->GetClassLinker()->GetExtension(lang);
    auto testClass = classLinkerExt->CreateClass(nullptr, 0, 0, sizeof(ark::Class));
    size_t objectSize = DEFAULT_REGION_SIZE + 1;
    testClass->SetObjectSize(objectSize);
    void *mem = allocator.Alloc(objectSize);
    ASSERT_TRUE(mem != nullptr);
    auto object = static_cast<ObjectHeader *>(mem);
    object->SetClass(testClass);
    Region *region = ObjectToRegion(object);

    allocator.CollectAndRemoveFreeRegions(REGION_VISITOR, [](ObjectHeader *) { return ObjectStatus::DEAD_OBJECT; });
    bool hasRegion = false;
    allocator.GetSpace()->IterateRegions([region, &hasRegion](Region *r) { hasRegion |= region == r; });
    ASSERT_TRUE(!hasRegion);
}

}  // namespace ark::mem::test
