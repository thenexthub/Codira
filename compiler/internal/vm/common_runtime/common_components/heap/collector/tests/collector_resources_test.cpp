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

#include "common_components/heap/collector/collector_resources.h"
#include "common_components/heap/collector/gc_request.h"
#include "common_components/mutator/mutator_manager.h"
#include "common_components/tests/test_helper.h"
#include "common_interfaces/base_runtime.h"

using namespace common;

namespace common::test {
class CollectorResourcesTest : public BaseTestWithScope {
protected:
    static void SetUpTestCase()
    {
        BaseRuntime::GetInstance()->InitFromDynamic();
    }

    static void TearDownTestCase()
    {
        BaseRuntime::GetInstance()->FiniFromDynamic();
    }

    void SetUp() override
    {
        MutatorManager::Instance().CreateRuntimeMutator(ThreadType::GC_THREAD);
    }

    void TearDown() override
    {
        MutatorManager::Instance().DestroyRuntimeMutator(ThreadType::GC_THREAD);
    }
};

HWTEST_F_L0(CollectorResourcesTest, RequestHeapDumpTest) {
    Heap::GetHeap().GetCollectorResources().RequestHeapDump(
        GCTask::GCTaskType::GC_TASK_INVALID);
    EXPECT_TRUE(Heap::GetHeap().IsGCEnabled());
}

HWTEST_F_L0(CollectorResourcesTest, RequestGC) {
    GCRequest gcRequests = { GC_REASON_BACKUP, "backup", true, false, 0, 0 };
    Heap::GetHeap().EnableGC(false);
    EXPECT_TRUE(!Heap::GetHeap().GetCollectorResources().IsGCActive());
    GCReason reason = gcRequests.reason;
    Heap::GetHeap().GetCollectorResources().RequestGC(reason, true, common::GC_TYPE_FULL);
}

HWTEST_F_L0(CollectorResourcesTest, RequestGCAndWaitTest) {
    GCRequest gcRequests = { GC_REASON_USER, "user", false, false, 0, 0 };
    GCReason reason = gcRequests.reason;
    Heap::GetHeap().EnableGC(true);
    EXPECT_TRUE(Heap::GetHeap().GetCollectorResources().IsGCActive());
    Heap::GetHeap().GetCollectorResources().RequestGC(reason, false, common::GC_TYPE_FULL);
    EXPECT_TRUE(!gcRequests.IsSyncGC());
}

HWTEST_F_L0(CollectorResourcesTest, GetGCThreadCountTest) {
    uint32_t res = Heap::GetHeap().GetCollectorResources().GetGCThreadCount(false);
    EXPECT_EQ(res, 2u);
}
} // namespace common::test