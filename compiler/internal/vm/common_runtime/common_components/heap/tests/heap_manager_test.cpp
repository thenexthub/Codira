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

#include "common_components/heap/heap.h"
#include "common_components/heap/heap_manager.h"
#include "common_components/tests/test_helper.h"

using namespace common;
namespace common::test {
class HeapManagerTest : public common::test::BaseTestWithScope {
protected:
    static void SetUpTestCase()
    {
        BaseRuntime::GetInstance()->InitFromDynamic();
    }
};

HWTEST_F_L0(HeapManagerTest, RequestGC)
{
    HeapManager manager;
    Heap::GetHeap().EnableGC(false);
    manager.RequestGC(GCReason::GC_REASON_USER, true, common::GC_TYPE_FULL);
    ASSERT_FALSE(Heap::GetHeap().IsGCEnabled());

    Heap::GetHeap().EnableGC(true);
    manager.RequestGC(GCReason::GC_REASON_USER, true, common::GC_TYPE_FULL);
    ASSERT_TRUE(Heap::GetHeap().IsGCEnabled());
}
} // namespace common::test
