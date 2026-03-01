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

#include "runtime/tests/intrusive-tests/intrusive_test_suite.h"
#include "runtime/tests/test_utils.h"
#include "runtime/include/runtime.h"
#include "runtime/handle_scope.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/mem/gc/gc.h"

namespace ark::test {

class IntrusiveInterruptReleasePagesTest : public testing::Test {
public:
    IntrusiveInterruptReleasePagesTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetUseTlabForAllocations(false);
        options.SetCompilerEnableJit(false);
        options.SetGcTriggerType("debug-never");
        options.SetHeapSizeLimit(512_MB);
        options.SetYoungSpaceSize(4_MB);
        options.SetExplicitConcurrentGcEnabled(false);
        options.SetIntrusiveTest(INTERRUPT_RELEASE_PAGES_TEST);
        [[maybe_unused]] bool success = Runtime::Create(options);
        ASSERT(success);

        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~IntrusiveInterruptReleasePagesTest()
    {
        thread_->ManagedCodeEnd();
        [[maybe_unused]] bool success = Runtime::Destroy();
        ASSERT(success);
    }

    NO_COPY_SEMANTIC(IntrusiveInterruptReleasePagesTest);
    NO_MOVE_SEMANTIC(IntrusiveInterruptReleasePagesTest);

    void WarmUpMmapMemPool()
    {
        auto *gc = Runtime::GetCurrent()->GetPandaVM()->GetGC();
        for (size_t i = 0; i < 10U; ++i) {
            auto *arr = mem::ObjectAllocator::AllocArray(100_MB, ClassRoot::ARRAY_U1, false);
            ASSERT(arr != nullptr);
            for (size_t j = 0; j < 1024U; ++j) {
                arr = mem::ObjectAllocator::AllocArray(100_KB, ClassRoot::ARRAY_U1, false);
                ASSERT(arr != nullptr);
            }
            gc->WaitForGCInManaged(GCTask(GCTaskCause::MIXED));
        }
    }

private:
    ark::MTManagedThread *thread_;
};

void SetNeedInterruptFlag()
{
    /* @sync 1
     * @description Set needInterrupt flag
     */
}

TEST_F(IntrusiveInterruptReleasePagesTest, TestFullGCInterruptReleasePages)
{
    WarmUpMmapMemPool();
    auto *gc = Runtime::GetCurrent()->GetPandaVM()->GetGC();

    auto *arr = mem::ObjectAllocator::AllocArray(300_MB, ClassRoot::ARRAY_U1, false);
    ASSERT(arr != nullptr);

    SetNeedInterruptFlag();

    // Start release pages to OS at the end of FULL GC
    gc->WaitForGCInManaged(GCTask(GCTaskCause::OOM_CAUSE));

    // Interrupt the page release process and restart after Mixed GC
    arr = mem::ObjectAllocator::AllocArray(1_KB, ClassRoot::ARRAY_U1, false);
    gc->WaitForGCInManaged(GCTask(GCTaskCause::MIXED));

    // Check that allocation of big array is successful
    arr = mem::ObjectAllocator::AllocArray(200_MB, ClassRoot::ARRAY_U1, false);
    ASSERT(arr != nullptr);
}

}  // namespace ark::test
