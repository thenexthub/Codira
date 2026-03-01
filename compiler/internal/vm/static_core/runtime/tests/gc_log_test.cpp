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

#include <gtest/gtest.h>
#include <cstring>
#include <string>

#include "libarkbase/utils/utils.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/vm_handle.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/line_string.h"
#include "runtime/mem/gc/card_table.h"
#include "runtime/mem/gc/g1/g1-allocator.h"
#include "runtime/mem/rem_set-inl.h"
#include "runtime/mem/region_space.h"
#include "runtime/tests/test_utils.h"

#include "runtime/mem/object_helpers.h"

namespace ark::mem {
class GCTestLog : public testing::TestWithParam<const char *> {
public:
    NO_MOVE_SEMANTIC(GCTestLog);
    NO_COPY_SEMANTIC(GCTestLog);

    GCTestLog() = default;

    ~GCTestLog() override
    {
        [[maybe_unused]] bool success = Runtime::Destroy();
        ASSERT(success);
        Logger::Destroy();
    }

    // NOLINTBEGIN(readability-magic-numbers)
    void SetupRuntime(const std::string &gcType, bool smallHeapAndYoungSpaces = false,
                      size_t promotionRegionAliveRate = 100) const
    {
        ark::Logger::ComponentMask componentMask;
        componentMask.set(Logger::Component::GC);

        Logger::InitializeStdLogging(Logger::Level::INFO, componentMask);
        EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::INFO, Logger::Component::GC));

        RuntimeOptions options;
        options.SetLoadRuntimes({"core"});
        options.SetGcType(gcType);
        options.SetRunGcInPlace(true);
        options.SetCompilerEnableJit(false);
        options.SetGcWorkersCount(0);
        options.SetG1PromotionRegionAliveRate(promotionRegionAliveRate);
        options.SetGcTriggerType("debug-never");
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetExplicitConcurrentGcEnabled(false);
        if (smallHeapAndYoungSpaces) {
            options.SetYoungSpaceSize(2_MB);
            options.SetHeapSizeLimit(15_MB);
        }
        [[maybe_unused]] bool success = Runtime::Create(options);
        ASSERT(success);
    }
    // NOLINTEND(readability-magic-numbers)

    size_t GetGCCounter(GC *gc)
    {
        return gc->gcCounter_;
    }

    void CounterLogTest()
    {
        SetupRuntime(GetParam());

        Runtime *runtime = Runtime::GetCurrent();
        GC *gc = runtime->GetPandaVM()->GetGC();
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        const unsigned iterations = 100;

        ASSERT(GetGCCounter(gc) == 0);

        for (size_t i = 1; i < iterations; i++) {
            testing::internal::CaptureStderr();
            task.Run(*gc);
            expectedLog_ = '[' + std::to_string(GetGCCounter(gc)) + ']';
            log_ = testing::internal::GetCapturedStderr();
            ASSERT_NE(log_.find(expectedLog_), std::string::npos) << "Expected:\n"
                                                                  << expectedLog_ << "\nLog:\n"
                                                                  << log_;
            ASSERT(GetGCCounter(gc) == i);
            task.reason = static_cast<GCTaskCause>(i % numberOfGcCauses_ == 0 ? i % numberOfGcCauses_ + 1
                                                                              : i % numberOfGcCauses_);
        }
    }

    void FullLogTest()
    {
        auto gcType = GetParam();
        bool isStw = strcmp(gcType, "stw") == 0;

        SetupRuntime(gcType);

        Runtime *runtime = Runtime::GetCurrent();
        GC *gc = runtime->GetPandaVM()->GetGC();
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);

        testing::internal::CaptureStderr();
        task.reason = GCTaskCause::YOUNG_GC_CAUSE;
        task.Run(*gc);
        expectedLog_ = isStw ? "[FULL (Young)]" : "[YOUNG (Young)]";
        log_ = testing::internal::GetCapturedStderr();
        ASSERT_NE(log_.find(expectedLog_), std::string::npos) << "Expected:\n" << expectedLog_ << "\nLog:\n" << log_;

        testing::internal::CaptureStderr();
        task.reason = GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE;
        task.Run(*gc);
        expectedLog_ = isStw ? "[FULL (Threshold)]" : "[TENURED (Threshold)]";
        log_ = testing::internal::GetCapturedStderr();
        ASSERT_NE(log_.find(expectedLog_), std::string::npos) << "Expected:\n" << expectedLog_ << "\nLog:\n" << log_;

        testing::internal::CaptureStderr();
        task.reason = GCTaskCause::OOM_CAUSE;
        task.Run(*gc);
        expectedLog_ = "[FULL (OOM)]";
        log_ = testing::internal::GetCapturedStderr();
        ASSERT_NE(log_.find(expectedLog_), std::string::npos) << "Expected:\n" << expectedLog_ << "\nLog:\n" << log_;
    }

    // GCCollectionType order is important
    static_assert(GCCollectionType::NONE < GCCollectionType::YOUNG);
    static_assert(GCCollectionType::YOUNG < GCCollectionType::MIXED);
    static_assert(GCCollectionType::MIXED < GCCollectionType::TENURED);
    static_assert(GCCollectionType::TENURED < GCCollectionType::FULL);

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::string expectedLog_;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::string log_;

private:
    const size_t numberOfGcCauses_ = 8;
};

TEST_P(GCTestLog, FullLogTest)
{
    FullLogTest();
}

TEST_F(GCTestLog, G1GCMixedCollectionLogTest)
{
    SetupRuntime("g1-gc");

    uint32_t garbageRate = Runtime::GetOptions().GetG1RegionGarbageRateThreshold();
    // NOLINTNEXTLINE(readability-magic-numbers)
    size_t bigStringLen = garbageRate * DEFAULT_REGION_SIZE / 100U + sizeof(coretypes::String);
    // NOLINTNEXTLINE(readability-magic-numbers)
    size_t bigStringLen1 = (garbageRate + 1) * DEFAULT_REGION_SIZE / 100U + sizeof(coretypes::String);
    // NOLINTNEXTLINE(readability-magic-numbers)
    size_t bigStringLen2 = (garbageRate + 2U) * DEFAULT_REGION_SIZE / 100U + sizeof(coretypes::String);
    size_t smallLen = DEFAULT_REGION_SIZE / 2U + sizeof(coretypes::String);

    GC *gc = Runtime::GetCurrent()->GetPandaVM()->GetGC();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    ObjectAllocator objectAllocator;

    testing::internal::CaptureStderr();

    VMHandle<coretypes::Array> holder;
    VMHandle<ObjectHeader> young;
    holder = VMHandle<coretypes::Array>(thread, objectAllocator.AllocArray(4U, ClassRoot::ARRAY_STRING, false));
    holder->Set(0_I, objectAllocator.AllocString(bigStringLen));
    holder->Set(1_I, objectAllocator.AllocString(bigStringLen1));
    holder->Set(2_I, objectAllocator.AllocString(bigStringLen2));
    holder->Set(3_I, objectAllocator.AllocString(smallLen));

    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }
    expectedLog_ = "[YOUNG (Young)]";
    log_ = testing::internal::GetCapturedStderr();
    ASSERT_NE(log_.find(expectedLog_), std::string::npos) << "Expected:\n" << expectedLog_ << "\nLog:\n" << log_;
    testing::internal::CaptureStderr();

    VMHandle<ObjectHeader> current;
    current = VMHandle<ObjectHeader>(thread, objectAllocator.AllocArray(smallLen, ClassRoot::ARRAY_U8, false));

    holder->Set(0_I, static_cast<ObjectHeader *>(nullptr));
    holder->Set(1_I, static_cast<ObjectHeader *>(nullptr));
    holder->Set(2_I, static_cast<ObjectHeader *>(nullptr));
    holder->Set(3_I, static_cast<ObjectHeader *>(nullptr));

    {
        ScopedNativeCodeThread sn(thread);
        GCTask task1(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
        task1.Run(*gc);
    }

    young = VMHandle<ObjectHeader>(thread, objectAllocator.AllocObjectInYoung());
    {
        ScopedNativeCodeThread sn(thread);
        GCTask task2(GCTaskCause::YOUNG_GC_CAUSE);
        task2.Run(*gc);
    }
    expectedLog_ = "[MIXED (Young)]";
    log_ = testing::internal::GetCapturedStderr();
    ASSERT_NE(log_.find(expectedLog_), std::string::npos) << "Expected:\n" << expectedLog_ << "\nLog:\n" << log_;
}

TEST_P(GCTestLog, CounterLogTest)
{
    CounterLogTest();
}

INSTANTIATE_TEST_SUITE_P(GCTestLogOnDiffGCs, GCTestLog, ::testing::ValuesIn(TESTED_GC));
}  // namespace ark::mem
