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

#include "runtime/include/coretypes/array.h"

#include "runtime/arch/memory_helpers.h"
#include "runtime/include/class-inl.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/coretypes/dyn_objects.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "libarkbase/utils/utils.h"

namespace ark::coretypes {

// CC-OFFNXT(G.FUN.01) solid logic
static Array *AllocateArray(ark::BaseClass *arrayClass, size_t elemSize, ArraySizeT length, ark::SpaceType spaceType,
                            bool pinned = false, const PandaVM *vm = Thread::GetCurrent()->GetVM())
{
    size_t size = Array::ComputeSize(elemSize, length);
    if (UNLIKELY(size == 0)) {
        LOG(ERROR, RUNTIME) << "Illegal array size: element size: " << elemSize << " array length: " << length;
        ThrowOutOfMemoryError("OOM when allocating array");
        return nullptr;
    }
    if (LIKELY(spaceType == ark::SpaceType::SPACE_TYPE_OBJECT)) {
        return static_cast<coretypes::Array *>(
            vm->GetHeapManager()->AllocateObject(arrayClass, size, DEFAULT_ALIGNMENT, ManagedThread::GetCurrent(),
                                                 mem::ObjectAllocatorBase::ObjMemInitPolicy::REQUIRE_INIT, pinned));
    }
    if (spaceType == ark::SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT) {
        return static_cast<coretypes::Array *>(vm->GetHeapManager()->AllocateNonMovableObject(
            arrayClass, size, DEFAULT_ALIGNMENT, ManagedThread::GetCurrent()));
    }
    UNREACHABLE();
}

/* static */
Array *Array::Create(ark::Class *arrayClass, const uint8_t *data, ArraySizeT length, ark::SpaceType spaceType,
                     bool pinned)
{
    size_t elemSize = arrayClass->GetComponentSize();
    auto *array = AllocateArray(arrayClass, elemSize, length, spaceType, pinned);
    if (UNLIKELY(array == nullptr)) {
        LOG(ERROR, RUNTIME) << "Failed to allocate array.";
        return nullptr;
    }
    // Order is matters here: GC can read data before it copied if we set length first.
    // length == 0 is guaranteed by AllocateArray
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    array->SetLength(length);
    MemcpyUnsafe(array->GetData(), data, length * elemSize);
    TSAN_ANNOTATE_IGNORE_WRITES_END();
    // Witout full memory barrier it is possible that architectures with weak memory order can try fetching array
    // legth before it's set
    arch::FullMemoryBarrier();
    return array;
}

/* static */
Array *Array::Create(ark::Class *arrayClass, ArraySizeT length, ark::SpaceType spaceType, bool pinned)
{
    ASSERT(arrayClass != nullptr);
    size_t elemSize = arrayClass->GetComponentSize();
    auto *array = AllocateArray(arrayClass, elemSize, length, spaceType, pinned);
    if (array == nullptr) {
        return nullptr;
    }
    // No need to memset - it is done in allocator
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    array->SetLength(length);
    TSAN_ANNOTATE_IGNORE_WRITES_END();
    // Without full memory barrier it is possible that architectures with weak memory order can try fetching array
    // length before it's set
    arch::FullMemoryBarrier();
    return array;
}

/* static */
Array *Array::Create(DynClass *dynarrayclass, ArraySizeT length, ark::SpaceType spaceType, bool pinned)
{
    size_t elemSize = coretypes::TaggedValue::TaggedTypeSize();
    HClass *arrayClass = dynarrayclass->GetHClass();
    auto *array = AllocateArray(arrayClass, elemSize, length, spaceType, pinned);
    if (array == nullptr) {
        return nullptr;
    }
    // No need to memset - it is done in allocator
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    array->SetLength(length);
    TSAN_ANNOTATE_IGNORE_WRITES_END();
    // Witout full memory barrier it is possible that architectures with weak memory order can try fetching array
    // legth before it's set
    arch::FullMemoryBarrier();
    return array;
}

/* static */
Array *Array::CreateTagged(const PandaVM *vm, ark::BaseClass *arrayClass, ArraySizeT length, ark::SpaceType spaceType,
                           TaggedValue initValue)
{
    size_t elemSize = coretypes::TaggedValue::TaggedTypeSize();
    auto *array = AllocateArray(arrayClass, elemSize, length, spaceType, false, vm);
    if (array == nullptr) {
        return nullptr;
    }
    // Order is matters here: GC can read data before it copied if we set length first.
    // length == 0 is guaranteed by AllocateArray
    for (ArraySizeT i = 0; i < length; i++) {
        array->Set<TaggedType, false, true>(i, initValue.GetRawData());
    }
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    array->SetLength(length);
    TSAN_ANNOTATE_IGNORE_WRITES_END();
    // Witout full memory barrier it is possible that architectures with weak memory order can try fetching array
    // legth before it's set
    arch::FullMemoryBarrier();
    return array;
}

}  // namespace ark::coretypes
