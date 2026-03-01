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

#include "heap_helpers.h"

#include "include/object_header.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_vm.h"

namespace ark::dfx {

uint64_t GetHeapTotalSize(ani_vm *vm)
{
    auto *etsVM = ark::ets::PandaEtsVM::FromAniVM(vm);
    return static_cast<uint64_t>(etsVM->GetHeapManager()->GetTotalMemory());
}

uint64_t GetHeapUsedSize(ani_vm *vm)
{
    auto *etsVM = ark::ets::PandaEtsVM::FromAniVM(vm);
    return etsVM->GetMemStats()->GetFootprintHeap();
}

uint64_t GetArrayBufferSize(ani_vm *vm)
{
    auto *etsVM = ark::ets::PandaEtsVM::FromAniVM(vm);
    auto arrayBufferClass = ark::ets::PlatformTypes()->coreArrayBuffer->GetRuntimeClass();

    size_t totalArrayBufferSize = 0;
    auto arrayBufferVisitor = [&totalArrayBufferSize, &arrayBufferClass](ark::ObjectHeader *obj) {
        if (obj->IsInstanceOf(arrayBufferClass)) {
            totalArrayBufferSize += obj->ObjectSize();
        }
    };

    etsVM->GetHeapManager()->IterateOverObjectsWithSuspendAllThread(arrayBufferVisitor);

    return static_cast<uint64_t>(totalArrayBufferSize);
}

uint64_t GetProcessHeapLimitSize(ani_vm *vm)
{
    auto *etsVM = ark::ets::PandaEtsVM::FromAniVM(vm);
    return static_cast<uint64_t>(etsVM->GetHeapManager()->GetMaxMemory());
}

uint64_t GetHeapLimitSize(ani_vm *vm)
{
    auto *etsVM = ark::ets::PandaEtsVM::FromAniVM(vm);
    return static_cast<uint64_t>(etsVM->GetHeapManager()->GetMaxMemory());
}

uint64_t GetFullGCLongTimeCount(ani_vm *vm)
{
    auto *etsVM = ark::ets::PandaEtsVM::FromAniVM(vm);
    return etsVM->GetFullGCLongTimeCount();
}

uint64_t GetAccumulatedFreeSize(ani_vm *vm)
{
    auto *etsVM = ark::ets::PandaEtsVM::FromAniVM(vm);
    return etsVM->GetGCStats()->GetMemStats()->GetFreedHeap();
}

uint64_t GetAccumulatedAllocateSize(ani_vm *vm)
{
    auto *etsVM = ark::ets::PandaEtsVM::FromAniVM(vm);
    return etsVM->GetGCStats()->GetMemStats()->GetAllocatedHeap();
}

uint64_t GetGCCount(ani_vm *vm)
{
    auto *etsVM = ark::ets::PandaEtsVM::FromAniVM(vm);
    return static_cast<uint64_t>(etsVM->GetGC()->GetCounter());
}

uint64_t GetGCDuration(ani_vm *vm)
{
    auto *etsVM = ark::ets::PandaEtsVM::FromAniVM(vm);
    return etsVM->GetGCStats()->GetTotalDuration();
}

}  // namespace ark::dfx
