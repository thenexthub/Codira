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

#include "gtest/gtest.h"
#include "iostream"
#include "runtime/class_linker_context.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/vm_handle.h"
#include "runtime/handle_base-inl.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/mem/mem_stats.h"
#include "runtime/mem/mem_stats_default.h"

namespace ark::mem::test {

class MemStatsGCTest : public testing::Test {
public:
    void SetupRuntime(const std::string &gcType)
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetUseTlabForAllocations(false);
        options.SetGcType(gcType);
        options.SetRunGcInPlace(true);
        options.SetExplicitConcurrentGcEnabled(false);
        bool success = Runtime::Create(options);
        ASSERT_TRUE(success) << "Cannot create Runtime";
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    template <uint64_t OBJECT_COUNT>
    void MemStatsTest(uint64_t tries, size_t objectSize);

    void TearDown() override
    {
        thread_->ManagedCodeEnd();
        bool success = Runtime::Destroy();
        ASSERT_TRUE(success) << "Cannot destroy Runtime";
    }

private:
    void SetAligment(size_t allocSize, size_t objectMaxSize, size_t &aligmentSize, size_t &aligmentDiff);
    void SetAllocatedStats(uint64_t &allocatedObjects, uint64_t &allocatedBytes, size_t allocSize,
                           uint64_t objectCount);

    ark::MTManagedThread *thread_ {};
};

void MemStatsGCTest::SetAligment(size_t allocSize, size_t objectMaxSize, size_t &aligmentSize, size_t &aligmentDiff)
{
    if (allocSize < objectMaxSize) {
        aligmentSize = 1UL << RunSlots<>::ConvertToPowerOfTwoUnsafe(allocSize);
        aligmentDiff = aligmentSize - allocSize;
    } else {
        aligmentSize = AlignUp(allocSize, GetAlignmentInBytes(FREELIST_DEFAULT_ALIGNMENT));
        aligmentDiff = 2U * (aligmentSize - allocSize);
    }
}

void MemStatsGCTest::SetAllocatedStats(uint64_t &allocatedObjects, uint64_t &allocatedBytes, size_t allocSize,
                                       uint64_t objectCount)
{
    allocatedObjects += objectCount;
    allocatedBytes += objectCount * allocSize;
}

template <uint64_t OBJECT_COUNT>
void MemStatsGCTest::MemStatsTest(uint64_t tries, size_t objectSize)
{
    ASSERT(objectSize >= sizeof(coretypes::String));
    mem::MemStatsType *stats = thread_->GetVM()->GetMemStats();
    ASSERT_NE(stats, nullptr);

    auto classLinker = Runtime::GetCurrent()->GetClassLinker();
    ASSERT_NE(classLinker, nullptr);
    auto allocator = classLinker->GetAllocator();

    std::string simpleString;
    for (size_t j = 0; j < objectSize - sizeof(coretypes::String); j++) {
        simpleString.append("x");
    }
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto objectAllocator = thread_->GetVM()->GetGC()->GetObjectAllocator();
    thread_->GetVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));

    size_t allocSize = simpleString.size() + sizeof(coretypes::String);
    size_t aligmentSize = 0;
    size_t aligmentDiff = 0;
    SetAligment(allocSize, objectAllocator->GetRegularObjectMaxSize(), aligmentSize, aligmentDiff);

    uint64_t allocatedObjects = stats->GetTotalObjectsAllocated();
    uint64_t allocatedBytes = stats->GetAllocated(SpaceType::SPACE_TYPE_OBJECT);
    uint64_t freedObjects = stats->GetTotalObjectsFreed();
    uint64_t freedBytes = stats->GetFreed(SpaceType::SPACE_TYPE_OBJECT);
    uint64_t diffTotal = 0;
    std::array<VMHandle<coretypes::String> *, OBJECT_COUNT> handlers {};
    for (size_t i = 0; i < tries; i++) {
        [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread_);
        for (uint64_t j = 0; j < OBJECT_COUNT; j++) {
            coretypes::String *stringObj =
                coretypes::String::CreateFromMUtf8(reinterpret_cast<const uint8_t *>(&simpleString[0]),
                                                   simpleString.length(), ctx, Runtime::GetCurrent()->GetPandaVM());
            ASSERT_NE(stringObj, nullptr);
            handlers[j] = allocator->New<VMHandle<coretypes::String>>(thread_, stringObj);
        }

        SetAllocatedStats(allocatedObjects, allocatedBytes, allocSize, OBJECT_COUNT);
        diffTotal += OBJECT_COUNT * aligmentDiff;
        ASSERT_EQ(allocatedObjects, stats->GetTotalObjectsAllocated());
        ASSERT_LE(allocatedBytes, stats->GetAllocated(SpaceType::SPACE_TYPE_OBJECT));
        ASSERT_GE(allocatedBytes + diffTotal, stats->GetAllocated(SpaceType::SPACE_TYPE_OBJECT));

        // run GC
        thread_->GetVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
        ASSERT_EQ(allocatedObjects, stats->GetTotalObjectsAllocated());
        ASSERT_LE(allocatedBytes, stats->GetAllocated(SpaceType::SPACE_TYPE_OBJECT));
        ASSERT_GE(allocatedBytes + diffTotal, stats->GetAllocated(SpaceType::SPACE_TYPE_OBJECT));
        ASSERT_EQ(freedObjects, stats->GetTotalObjectsFreed());
        ASSERT_LE(freedBytes, stats->GetFreed(SpaceType::SPACE_TYPE_OBJECT));
        ASSERT_GE(freedBytes + diffTotal, stats->GetFreed(SpaceType::SPACE_TYPE_OBJECT));

        for (uint64_t j = 0; j < OBJECT_COUNT; j++) {
            allocator->Delete(handlers[j]);
        }
        freedObjects += OBJECT_COUNT;
        freedBytes += OBJECT_COUNT * allocSize;
    }
}

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
constexpr size_t OBJECTS_SIZE[] = {
    32,   // RunSlots: aligned & object_size = RunSlot size
    72,   // RunSlots: aligned & object_size != RunSlot size
    129,  // RunSlots: not aligned
    512,  // FreeList: aligned
    1025  // FreeList: not aligned
};

TEST_F(MemStatsGCTest, StwGcTest)
{
    constexpr uint64_t OBJECTS_COUNT = 500;
    constexpr uint64_t TRIES = 10;

    SetupRuntime("stw");
    for (size_t objectSize : OBJECTS_SIZE) {
        MemStatsTest<OBJECTS_COUNT>(TRIES, objectSize);
    }
}

}  // namespace ark::mem::test
