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
#include <string>

#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"
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
#include "runtime/include/gc_task.h"
#include "runtime/mem/gc/g1/g1-gc.h"

#include "runtime/tests/test_utils.h"
#include "runtime/mem/object_helpers.h"

namespace ark::mem {

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,-warnings-as-errors)
class ExplicitGC : public testing::Test {
public:
    ExplicitGC() = default;

    ~ExplicitGC() override
    {
        [[maybe_unused]] bool success = Runtime::Destroy();
        ASSERT(success);
        Logger::Destroy();
    }

    NO_COPY_SEMANTIC(ExplicitGC);
    NO_MOVE_SEMANTIC(ExplicitGC);

    void SetupRuntime(const std::string &gcType, bool isExplicitFull) const
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
        options.SetGcTriggerType("debug-never");
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetExplicitConcurrentGcEnabled(isExplicitFull);
        [[maybe_unused]] bool success = Runtime::Create(options);
        ASSERT(success);
    }
};

TEST_F(ExplicitGC, TestG1GCPhases)
{
    SetupRuntime("g1-gc", true);

    uint32_t garbageRate = Runtime::GetOptions().GetG1RegionGarbageRateThreshold();
    // NOLINTNEXTLINE(readability-magic-numbers,-warnings-as-errors)
    size_t bigLen = garbageRate * DEFAULT_REGION_SIZE / 100U + sizeof(coretypes::String);

    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    ObjectAllocator allocator;
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    VMHandle<coretypes::Array> holder;
    std::string expectedLog;
    std::string log;

    holder = VMHandle<coretypes::Array>(thread, allocator.AllocArray(2U, ClassRoot::ARRAY_STRING, false));
    holder->Set(0, allocator.AllocString(bigLen));
    holder->Set(1, allocator.AllocString(bigLen));

    {
        ScopedNativeCodeThread sn(thread);
        testing::internal::CaptureStderr();
        GCTask task(GCTaskCause::EXPLICIT_CAUSE);
        task.Run(*gc);  // run young
        expectedLog = "[YOUNG (Explicit)]";
        log = testing::internal::GetCapturedStderr();
        ASSERT_NE(log.find(expectedLog), std::string::npos) << "Expected:\n" << expectedLog << "\nLog:\n" << log;
    }

    ASSERT_TRUE(ObjectToRegion(holder->Get<ObjectHeader *>(1))->HasFlag(RegionFlag::IS_OLD));
    ASSERT_TRUE(ObjectToRegion(holder->Get<ObjectHeader *>(0))->HasFlag(RegionFlag::IS_OLD));

    holder->Set(0, static_cast<ObjectHeader *>(nullptr));
    holder->Set(1, static_cast<ObjectHeader *>(nullptr));

    {
        ScopedNativeCodeThread sn(thread);
        testing::internal::CaptureStderr();
        GCTask task(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
        task.Run(*gc);  // prepare for mix
        expectedLog = "[TENURED (Threshold)]";
        log = testing::internal::GetCapturedStderr();
        ASSERT_NE(log.find(expectedLog), std::string::npos) << "Expected:\n" << expectedLog << "\nLog:\n" << log;
    }

    {
        ScopedNativeCodeThread sn(thread);
        testing::internal::CaptureStderr();
        GCTask task(GCTaskCause::EXPLICIT_CAUSE);
        task.Run(*gc);  // run mixed gc
        expectedLog = "[MIXED (Explicit)]";
        log = testing::internal::GetCapturedStderr();
        ASSERT_NE(log.find(expectedLog), std::string::npos) << "Expected:\n" << expectedLog << "\nLog:\n" << log;
    }
}

TEST_F(ExplicitGC, TestG1GCWithFullExplicit)
{
    SetupRuntime("g1-gc", false);

    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    ObjectAllocator objectAllocator;

    std::string expectedLog;
    std::string log;

    VMHandle<ObjectHeader> obj;
    obj = VMHandle<ObjectHeader>(thread, objectAllocator.AllocObjectInYoung());

    {
        ScopedNativeCodeThread sn(thread);
        testing::internal::CaptureStderr();
        GCTask task(GCTaskCause::EXPLICIT_CAUSE);
        task.Run(*gc);  // run full
        expectedLog = "[FULL (Explicit)]";
        log = testing::internal::GetCapturedStderr();
        ASSERT_NE(log.find(expectedLog), std::string::npos) << "Expected:\n" << expectedLog << "\nLog:\n" << log;
    }
}

}  // namespace ark::mem
