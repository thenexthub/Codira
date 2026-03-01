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

#include <array>
#include <atomic>
#include <condition_variable>
#include <ostream>
#include <thread>

#include "gtest/gtest.h"
#include "iostream"
#include "runtime/handle_base-inl.h"
#include "runtime/include/coretypes/line_string.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/gc/gc_phase.h"
#include "runtime/mem/mem_stats.h"
#include "runtime/mem/mem_stats_additional_info.h"

namespace ark::mem::test {

class MemStatsAdditionalInfoTest : public testing::Test {
public:
    MemStatsAdditionalInfoTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetGcType("epsilon");
        Runtime::Create(options);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~MemStatsAdditionalInfoTest() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(MemStatsAdditionalInfoTest);
    NO_MOVE_SEMANTIC(MemStatsAdditionalInfoTest);

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    ark::MTManagedThread *thread_;
};

TEST_F(MemStatsAdditionalInfoTest, HeapAllocatedMaxAndTotal)
{
    static constexpr size_t BYTES_ALLOC1 = 2;
    static constexpr size_t BYTES_ALLOC2 = 5;
    static constexpr size_t RAW_ALLOC1 = 15;

    std::string simpleString = "smallData";
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    coretypes::String *stringObject = coretypes::String::CreateFromMUtf8(
        reinterpret_cast<const uint8_t *>(&simpleString[0]), simpleString.length(), ctx, thread_->GetVM());

    size_t stringSize = stringObject->ObjectSize();

    MemStatsAdditionalInfo stats;
    stats.RecordAllocateObject(BYTES_ALLOC1, SpaceType::SPACE_TYPE_OBJECT);
    stats.RecordAllocateObject(BYTES_ALLOC2, SpaceType::SPACE_TYPE_OBJECT);
    stats.RecordAllocateRaw(RAW_ALLOC1, SpaceType::SPACE_TYPE_INTERNAL);

    stats.RecordAllocateObject(stringSize, SpaceType::SPACE_TYPE_OBJECT);
    ASSERT_EQ(BYTES_ALLOC1 + BYTES_ALLOC2 + stringSize, stats.GetAllocated(SpaceType::SPACE_TYPE_OBJECT));
    stats.RecordFreeObject(stringSize, SpaceType::SPACE_TYPE_OBJECT);
    ASSERT_EQ(BYTES_ALLOC1 + BYTES_ALLOC2, stats.GetFootprint(SpaceType::SPACE_TYPE_OBJECT));
    ASSERT_EQ(BYTES_ALLOC1 + BYTES_ALLOC2 + stringSize, stats.GetAllocated(SpaceType::SPACE_TYPE_OBJECT));
    ASSERT_EQ(stringSize, stats.GetFreed(SpaceType::SPACE_TYPE_OBJECT));
}

TEST_F(MemStatsAdditionalInfoTest, AdditionalStatistic)
{
    PandaVM *vm = thread_->GetVM();
    std::string simpleString = "smallData";
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    [[maybe_unused]] coretypes::String *stringObject = coretypes::String::CreateFromMUtf8(
        reinterpret_cast<const uint8_t *>(&simpleString[0]), simpleString.length(), ctx, vm);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread_);
    [[maybe_unused]] VMHandle<ObjectHeader> handle(thread_, stringObject);
#ifndef NDEBUG
    Class *stringClass =
        Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::LINE_STRING);
    auto statistics = thread_->GetVM()->GetMemStats()->GetStatistics();
    // allocated
    ASSERT_TRUE(statistics.find(stringClass->GetName()) != std::string::npos);
    ASSERT_TRUE(statistics.find("footprint") != std::string::npos);
    ASSERT_TRUE(statistics.find('1') != std::string::npos);
#endif
}

}  // namespace ark::mem::test
