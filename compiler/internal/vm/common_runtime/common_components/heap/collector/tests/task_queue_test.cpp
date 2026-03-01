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

#include "common_components/heap/collector/task_queue.h"
#include "common_components/heap/collector/collector_proxy.h"
#include "common_components/heap/collector/gc_request.h"
#include "common_components/tests/test_helper.h"

namespace common {
class StubAllocator : public Allocator {
public:
    HeapAddress Allocate(size_t size, AllocType allocType) override { return 0; }
    HeapAddress AllocateNoGC(size_t size, AllocType allocType) override { return 0; }
    bool ForEachObject(const std::function<void(BaseObject*)>&, bool safe) const override { return true; }
    size_t ReclaimGarbageMemory(bool releaseAll) override { return 0; }
    size_t LargeObjectSize() const override { return 0; }
    size_t GetAllocatedBytes() const override { return 0; }
    void Init(const RuntimeParam& param) override {}
    size_t GetMaxCapacity() const override { return 0; }
    size_t GetCurrentCapacity() const override { return 0; }
    size_t GetUsedPageSize() const override { return 0; }
    HeapAddress GetSpaceStartAddress() const override { return 0; }
    HeapAddress GetSpaceEndAddress() const override { return 0; }
#ifndef NDEBUG
    bool IsHeapObject(HeapAddress) const override { return false; }
#endif
    void FeedHungryBuffers() override {}
    size_t GetSurvivedSize() const override { return 0; }
};

class StubCollectorProxy : public CollectorProxy {
public:
    explicit StubCollectorProxy(Allocator& allocator, CollectorResources& resources)
        : CollectorProxy(allocator, resources) {}

    void RunGarbageCollection(uint64_t gcIndex, GCReason reason, GCType gcType) override {}
};
}

namespace common {
class DummyCollectorProxy : public CollectorProxy {
public:
    explicit DummyCollectorProxy(Allocator& alloc, CollectorResources& res)
        : CollectorProxy(alloc, res) {}
    void RunGarbageCollection(uint64_t gcIndex, GCReason reason, GCType gcType) override {}
};

class DummyCollectorResources : public CollectorResources {
private:
    DummyCollectorProxy proxy_;

public:
    explicit DummyCollectorResources(Allocator& alloc)
        : CollectorResources(proxy_),
          proxy_(alloc, *this) {}
};
}

namespace common::test {
class GCRunnerTest : public common::test::BaseTestWithScope {
protected:
    void SetUp() override
    {
        allocator_ = std::make_unique<common::StubAllocator>();
        dummyResources_ = std::make_unique<common::DummyCollectorResources>(*allocator_);
        proxy_ = std::make_unique<common::StubCollectorProxy>(*allocator_, *dummyResources_);
        proxyStorage_ = std::make_unique<common::StubCollectorProxy>(*allocator_, *dummyResources_);
    }

    void TearDown() override
    {
        proxyStorage_.reset();
        dummyResources_.reset();
        proxy_.reset();
        allocator_.reset();
    }

    std::unique_ptr<common::StubAllocator> allocator_;
    std::unique_ptr<common::StubCollectorProxy> proxy_;
    std::unique_ptr<common::StubCollectorProxy> proxyStorage_;
    std::unique_ptr<common::DummyCollectorResources> dummyResources_;
};

HWTEST_F_L0(GCRunnerTest, Execute_TerminateGC) {
    common::GCRunner runner(common::GCTask::GCTaskType::GC_TASK_TERMINATE_GC);
    bool result = runner.Execute(proxyStorage_.get());
    EXPECT_FALSE(result);
}

HWTEST_F_L0(GCRunnerTest, Execute_InvokeGC) {
    common::GCRunner runner(common::GCTask::GCTaskType::GC_TASK_INVOKE_GC, GC_REASON_BACKUP);
    bool result = runner.Execute(proxyStorage_.get());
    EXPECT_TRUE(result);
}

HWTEST_F_L0(GCRunnerTest, Execute_InvalidTaskType) {
    common::GCRunner runner(static_cast<common::GCTask::GCTaskType>(
        static_cast<uint32_t>(common::GCTask::GCTaskType::GC_TASK_DUMP_HEAP_IDE) + 1));
    bool result = runner.Execute(proxyStorage_.get());
    EXPECT_TRUE(result);
}
}  // namespace common::test
