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

#include "heap/heap_allocator.h"
#include "common_components/heap/allocator/region_desc.h"
#include "common_components/heap/allocator/regional_heap.h"
#include "common_components/tests/test_helper.h"
#include "common_interfaces/base_runtime.h"

using namespace common;

namespace common::test {
class HeapAllocatorTest : public BaseTestWithScope {
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
        holder_ = ThreadHolder::CreateAndRegisterNewThreadHolder(nullptr);
        scope_ = new ThreadHolder::TryBindMutatorScope(holder_);
    }

    void TearDown() override
    {
        if (scope_ != nullptr) {
            delete scope_;
            scope_ = nullptr;
        }
    }

    ThreadHolder *holder_ {nullptr};
    ThreadHolder::TryBindMutatorScope *scope_ {nullptr};
};

HWTEST_F_L0(HeapAllocatorTest, AllocLargeObject)
{
    uintptr_t addr = common::HeapAllocator::AllocateInHuge(Heap::NORMAL_UNIT_SIZE, common::LanguageType::DYNAMIC);
    ASSERT(addr > 0);
    RegionDesc* region = RegionDesc::GetAliveRegionDescAt(addr);
    ASSERT(region->IsLargeRegion());
}

HWTEST_F_L0(HeapAllocatorTest, AllocLargeRegion)
{
    uintptr_t addr = common::HeapAllocator::AllocateLargeRegion(Heap::NORMAL_UNIT_SIZE);
    ASSERT(addr > 0);
    RegionDesc *region = RegionDesc::GetAliveRegionDescAt(addr);
    ASSERT(region->IsLargeRegion());
}
}