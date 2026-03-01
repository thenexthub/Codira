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


#include "NativeAllocator.h"
#include "Mutator/ThreadLocal.h"

namespace MapleRuntime {
void* NativeAllocator::NativeAlloc(size_t bytes)
{
    if (bytes > MAX_BYTES) {
        return PagePool::Instance().GetPage(bytes);
    }
    MapleRuntime::ThreadLocalData* tlData = MapleRuntime::ThreadLocal::GetThreadLocalData();
    if (tlData->threadCache == nullptr) {
        tlData->threadCache = new (std::nothrow) ThreadCache();
        CHECK_DETAIL(tlData->threadCache != nullptr, "new alloc threadCache failed");
    }
    return reinterpret_cast<ThreadCache*>(tlData->threadCache)->Allocate(bytes);
}

void NativeAllocator::NativeFree(void* ptr, size_t bytes)
{
    if (bytes > MAX_BYTES) {
        PagePool::Instance().ReturnPage(reinterpret_cast<uint8_t*>(ptr), bytes);
        return;
    }
    MapleRuntime::ThreadLocalData* tlData = MapleRuntime::ThreadLocal::GetThreadLocalData();
    if (tlData->threadCache == nullptr) {
        tlData->threadCache = new (std::nothrow) ThreadCache();
        CHECK_DETAIL(tlData->threadCache != nullptr, "new alloc threadCache failed");
    }
    reinterpret_cast<ThreadCache*>(tlData->threadCache)->Deallocate(ptr, bytes);
}
}
