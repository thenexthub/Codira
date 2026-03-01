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


#ifndef MRT_HEAP_MANAGER_INLINE_H
#define MRT_HEAP_MANAGER_INLINE_H

// inline managed-heap functions
// common headers
#include "Base/Globals.h"
#include "Mutator/Mutator.h"
#include "Mutator/MutatorManager.h"
#include "schedule.h"
// module internal headers
#include "Heap/Collector/Collector.h"
#include "Heap/Heap.h"
#include "HeapManager.h"

namespace MapleRuntime {
inline void HeapManager::RequestGC(GCReason reason, bool async)
{
    if (!Heap::GetHeap().IsGCEnabled()) {
        return;
    }
    Collector& collector = Heap::GetHeap().GetCollector();
    collector.RequestGC(reason, async);
}
} // namespace MapleRuntime

#endif // MRT_HEAP_MANAGER_INLINE_H
