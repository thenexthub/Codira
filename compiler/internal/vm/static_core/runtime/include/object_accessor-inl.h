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
#ifndef PANDA_RUNTIME_OBJECT_ACCESSOR_INL_H_
#define PANDA_RUNTIME_OBJECT_ACCESSOR_INL_H_

#include <securec.h>

#include "libarkbase/mem/mem.h"
#include "runtime/include/class.h"
#include "runtime/include/field.h"
#include "runtime/include/object_accessor.h"
#include "runtime/mem/gc/gc_barrier_set.h"

namespace ark {

/* static */
template <bool IS_VOLATILE /* = false */, bool NEED_READ_BARRIER /* = true */, bool IS_DYN /* = false */>
inline ObjectHeader *ObjectAccessor::GetObject(const void *obj, size_t offset)
{
#ifdef ARK_HYBRID
    if (NEED_READ_BARRIER) {
        auto *barrierSet = GetBarrierSet();
        if (barrierSet->IsPreReadBarrierEnabled()) {
            return reinterpret_cast<ObjectHeader *>(barrierSet->PreReadBarrier(obj, offset));
        }
    }
#endif
    // We don't have GC with read barriers now
    if (!IS_DYN) {
        return reinterpret_cast<ObjectHeader *>(Get<ObjectPointerType, IS_VOLATILE>(obj, offset));
    }
    return Get<ObjectHeader *, IS_VOLATILE>(obj, offset);
}

/* static */
template <bool IS_VOLATILE /* = false */, bool NEED_WRITE_BARRIER /* = true */, bool IS_DYN /* = false */>
// CC-OFFNXT(G.FUD.06) perf critical
inline void ObjectAccessor::SetObject(void *obj, size_t offset, ObjectHeader *value)
{
    if constexpr (NEED_WRITE_BARRIER) {
        auto *barrierSet = GetBarrierSet();

        if (barrierSet->IsPreBarrierEnabled()) {
            ObjectHeader *preVal = GetObject<IS_VOLATILE, false, IS_DYN>(obj, offset);
            barrierSet->PreBarrier(preVal);
        }

        if (!IS_DYN) {
            Set<ObjectPointerType, IS_VOLATILE>(obj, offset, ToObjPtrType(value));
        } else {
            Set<ObjectHeader *, IS_VOLATILE>(obj, offset, value);
        }
        auto gcPostBarrierType = barrierSet->GetPostType();
        if (!mem::IsEmptyBarrier(gcPostBarrierType)) {
            barrierSet->PostBarrier(ToVoidPtr(ToUintPtr(obj)), offset, value);
        }
    } else {
        if constexpr (!IS_DYN) {
            Set<ObjectPointerType, IS_VOLATILE>(obj, offset, ToObjPtrType(value));
        } else {
            Set<ObjectHeader *, IS_VOLATILE>(obj, offset, value);
        }
    }
}

/* static */
template <bool IS_VOLATILE /* = false */, bool NEED_READ_BARRIER /* = true */, bool IS_DYN /* = false */>
inline ObjectHeader *ObjectAccessor::GetObject([[maybe_unused]] const ManagedThread *thread, const void *obj,
                                               size_t offset)
{
#ifdef ARK_HYBRID
    if (NEED_READ_BARRIER) {
        auto *barrierSet = GetBarrierSet();
        if (barrierSet->IsPreReadBarrierEnabled()) {
            return reinterpret_cast<ObjectHeader *>(barrierSet->PreReadBarrier(obj, offset));
        }
    }
#endif
    // We don't have GC with read barriers now
    if (!IS_DYN) {
        return reinterpret_cast<ObjectHeader *>(Get<ObjectPointerType, IS_VOLATILE>(obj, offset));
    }
    return Get<ObjectHeader *, IS_VOLATILE>(obj, offset);
}

/* static */
template <bool IS_VOLATILE /* = false */, bool NEED_WRITE_BARRIER /* = true */, bool IS_DYN /* = false */>
inline void ObjectAccessor::FillObjects(void *objArr, size_t dataOffset, size_t count, size_t elemSize,
                                        ObjectHeader *value)
{
    auto *barrierSet = GetBarrierSet();
    if (NEED_WRITE_BARRIER && barrierSet->IsPreBarrierEnabled()) {
        FillObjsWithPreBarrier<IS_VOLATILE, IS_DYN>(objArr, dataOffset, count, elemSize, value);
    } else {
        FillObjsNoBarrier<IS_VOLATILE, IS_DYN>(objArr, dataOffset, count, elemSize, value);
    }
    if (NEED_WRITE_BARRIER && (!mem::IsEmptyBarrier(barrierSet->GetPostType()))) {
        barrierSet->PostBarrier(objArr, dataOffset, count * elemSize);
    }
}

/* static */
template <bool IS_VOLATILE /* = false */, bool IS_DYN /* = false */>
inline void ObjectAccessor::FillObjsWithPreBarrier(void *objArr, size_t dataOffset, size_t count, size_t elemSize,
                                                   ObjectHeader *value)
{
    auto *barrierSet = GetBarrierSet();
    for (size_t i = 0; i < count; i++) {
        auto offset = dataOffset + elemSize * i;
        barrierSet->PreBarrier(GetObject<IS_VOLATILE, false, IS_DYN>(objArr, offset));
        if (!IS_DYN) {
            Set<ObjectPointerType, IS_VOLATILE>(objArr, offset, ToObjPtrType(value));
        } else {
            Set<ObjectHeader *, IS_VOLATILE>(objArr, offset, value);
        }
    }
}

/* static */
template <bool IS_VOLATILE /* = false */, bool IS_DYN /* = false */>
inline void ObjectAccessor::FillObjsNoBarrier(void *objArr, size_t dataOffset, size_t count, size_t elemSize,
                                              ObjectHeader *value)
{
    for (size_t i = 0; i < count; i++) {
        auto offset = dataOffset + elemSize * i;
        if (!IS_DYN) {
            Set<ObjectPointerType, IS_VOLATILE>(objArr, offset, ToObjPtrType(value));
        } else {
            Set<ObjectHeader *, IS_VOLATILE>(objArr, offset, value);
        }
    }
}

/* static */
template <bool IS_VOLATILE /* = false */, bool NEED_WRITE_BARRIER /* = true */, bool IS_DYN /* = false */>
// CC-OFFNXT(G.FUD.06) perf critical
inline void ObjectAccessor::SetObject(const ManagedThread *thread, void *obj, size_t offset, ObjectHeader *value)
{
    if constexpr (NEED_WRITE_BARRIER) {
        auto *barrierSet = GetBarrierSet(thread);
        if (barrierSet->IsPreBarrierEnabled()) {
            ObjectHeader *preVal = GetObject<IS_VOLATILE, IS_DYN>(obj, offset);
            barrierSet->PreBarrier(preVal);
        }

        if (!IS_DYN) {
            Set<ObjectPointerType, IS_VOLATILE>(obj, offset, ToObjPtrType(value));
        } else {
            Set<ObjectHeader *, IS_VOLATILE>(obj, offset, value);
        }
        if (!mem::IsEmptyBarrier(barrierSet->GetPostType())) {
            barrierSet->PostBarrier(ToVoidPtr(ToUintPtr(obj)), offset, value);
        }
    } else {
        if constexpr (!IS_DYN) {
            Set<ObjectPointerType, IS_VOLATILE>(obj, offset, ToObjPtrType(value));
        } else {
            Set<ObjectHeader *, IS_VOLATILE>(obj, offset, value);
        }
    }
}

/* static */
template <class T>
inline T ObjectAccessor::GetFieldPrimitive(const void *obj, const Field &field)
{
    if (UNLIKELY(field.IsVolatile())) {
        return GetPrimitive<T, true>(obj, field.GetOffset());
    }
    return GetPrimitive<T, false>(obj, field.GetOffset());
}

/* static */
template <class T>
inline void ObjectAccessor::SetFieldPrimitive(void *obj, const Field &field, T value)
{
    if (UNLIKELY(field.IsVolatile())) {
        SetPrimitive<T, true>(obj, field.GetOffset(), value);
    } else {
        SetPrimitive<T, false>(obj, field.GetOffset(), value);
    }
}

/* static */
// NEED_READ_BARRIER = true , IS_DYN = false
template <bool NEED_READ_BARRIER, bool IS_DYN>
inline ObjectHeader *ObjectAccessor::GetFieldObject(const void *obj, const Field &field)
{
    if (UNLIKELY(field.IsVolatile())) {
        return GetObject<true, NEED_READ_BARRIER, IS_DYN>(obj, field.GetOffset());
    }
    return GetObject<false, NEED_READ_BARRIER, IS_DYN>(obj, field.GetOffset());
}

/* static */
// NEED_READ_BARRIER = true , IS_DYN = false
template <bool NEED_WRITE_BARRIER, bool IS_DYN>
inline void ObjectAccessor::SetFieldObject(void *obj, const Field &field, ObjectHeader *value)
{
    ASSERT(IsAddressInObjectsHeapOrNull(value));
    if (UNLIKELY(field.IsVolatile())) {
        SetObject<true, NEED_WRITE_BARRIER, IS_DYN>(obj, field.GetOffset(), value);
    } else {
        SetObject<false, NEED_WRITE_BARRIER, IS_DYN>(obj, field.GetOffset(), value);
    }
}

/* static */
// NEED_READ_BARRIER = true , IS_DYN = false
template <bool NEED_READ_BARRIER, bool IS_DYN>
inline ObjectHeader *ObjectAccessor::GetFieldObject(const ManagedThread *thread, const void *obj, const Field &field)
{
    if (UNLIKELY(field.IsVolatile())) {
        return GetObject<true, NEED_READ_BARRIER, IS_DYN>(thread, obj, field.GetOffset());
    }
    return GetObject<false, NEED_READ_BARRIER, IS_DYN>(thread, obj, field.GetOffset());
}

/* static */
// NEED_READ_BARRIER = true , IS_DYN = false
template <bool NEED_WRITE_BARRIER, bool IS_DYN>
inline void ObjectAccessor::SetFieldObject(const ManagedThread *thread, void *obj, const Field &field,
                                           ObjectHeader *value)
{
    if (UNLIKELY(field.IsVolatile())) {
        SetObject<true, NEED_WRITE_BARRIER, IS_DYN>(thread, obj, field.GetOffset(), value);
    } else {
        SetObject<false, NEED_WRITE_BARRIER, IS_DYN>(thread, obj, field.GetOffset(), value);
    }
}

/* static */
template <class T>
inline T ObjectAccessor::GetFieldPrimitive(const void *obj, size_t offset, std::memory_order memoryOrder)
{
    return Get<T>(obj, offset, memoryOrder);
}

/* static */
template <class T>
inline void ObjectAccessor::SetFieldPrimitive(void *obj, size_t offset, T value, std::memory_order memoryOrder)
{
    Set<T>(obj, offset, value, memoryOrder);
}

/* static */
// NEED_READ_BARRIER = true , IS_DYN = false
template <bool NEED_READ_BARRIER, bool IS_DYN>
inline ObjectHeader *ObjectAccessor::GetFieldObject(const void *obj, int offset, std::memory_order memoryOrder)
{
#ifdef ARK_HYBRID
    if (NEED_READ_BARRIER) {
        auto *barrierSet = GetBarrierSet();
        if (barrierSet->IsPreReadBarrierEnabled()) {
            return reinterpret_cast<ObjectHeader *>(barrierSet->PreReadBarrier(obj, offset));
        }
    }
#endif
    if (!IS_DYN) {
        return reinterpret_cast<ObjectHeader *>(Get<ObjectPointerType>(obj, offset, memoryOrder));
    }
    return Get<ObjectHeader *>(obj, offset, memoryOrder);
}

static inline std::memory_order GetComplementMemoryOrder(std::memory_order memoryOrder)
{
    if (memoryOrder == std::memory_order_acquire) {
        memoryOrder = std::memory_order_release;
    } else if (memoryOrder == std::memory_order_release) {
        memoryOrder = std::memory_order_acquire;
    }
    return memoryOrder;
}

/* static */
// NEED_WRITE_BARRIER = true , IS_DYN = false
template <bool NEED_WRITE_BARRIER, bool IS_DYN>
// CC-OFFNXT(G.FUD.06) perf critical
inline void ObjectAccessor::SetFieldObject(void *obj, size_t offset, ObjectHeader *value, std::memory_order memoryOrder)
{
    if (NEED_WRITE_BARRIER) {
        auto *barrierSet = GetBarrierSet();

        if (barrierSet->IsPreBarrierEnabled()) {
            // If SetFieldObject is called with std::memory_order_release
            // we need to use the complement memory order std::memory_order_acquire
            // because we read the value.
            ObjectHeader *preVal = GetFieldObject<IS_DYN>(obj, offset, GetComplementMemoryOrder(memoryOrder));
            barrierSet->PreBarrier(preVal);
        }

        if (!IS_DYN) {
            Set<ObjectPointerType>(obj, offset, ToObjPtrType(value), memoryOrder);
        } else {
            Set<ObjectHeader *>(obj, offset, value, memoryOrder);
        }
        auto gcPostBarrierType = barrierSet->GetPostType();
        if (!mem::IsEmptyBarrier(gcPostBarrierType)) {
            barrierSet->PostBarrier(ToVoidPtr(ToUintPtr(obj)), offset, value);
        }
    } else {
        if (!IS_DYN) {
            Set<ObjectPointerType>(obj, offset, ToObjPtrType(value), memoryOrder);
        } else {
            Set<ObjectHeader *>(obj, offset, value, memoryOrder);
        }
    }
}

/* static */
template <typename T>
inline std::pair<bool, T> ObjectAccessor::CompareAndSetFieldPrimitive(void *obj, size_t offset, T oldValue, T newValue,
                                                                      std::memory_order memoryOrder, bool strong)
{
    uintptr_t rawAddr = reinterpret_cast<uintptr_t>(obj) + offset;
    ASSERT(IsAddressInObjectsHeap(rawAddr));
    auto *atomicAddr = reinterpret_cast<std::atomic<T> *>(rawAddr);
    if (strong) {
        return {atomicAddr->compare_exchange_strong(oldValue, newValue, memoryOrder), oldValue};
    }
    return {atomicAddr->compare_exchange_weak(oldValue, newValue, memoryOrder), oldValue};
}

/* static */
// NEED_READ_BARRIER = true , IS_DYN = false
template <bool NEED_WRITE_BARRIER, bool IS_DYN>
// CC-OFFNXT(G.FUD.06) perf critical
inline std::pair<bool, ObjectHeader *> ObjectAccessor::CompareAndSetFieldObject(void *obj, size_t offset,
                                                                                ObjectHeader *oldValue,
                                                                                ObjectHeader *newValue,
                                                                                std::memory_order memoryOrder,
                                                                                bool strong)
{
    bool success = false;
    ObjectHeader *result = nullptr;
    auto getResult = [&]() {
        if (IS_DYN) {
            auto value =
                CompareAndSetFieldPrimitive<ObjectHeader *>(obj, offset, oldValue, newValue, memoryOrder, strong);
            success = value.first;
            result = value.second;
        } else {
            auto value = CompareAndSetFieldPrimitive<ObjectPointerType>(obj, offset, ToObjPtrType(oldValue),
                                                                        ToObjPtrType(newValue), memoryOrder, strong);
            success = value.first;
            result = reinterpret_cast<ObjectHeader *>(value.second);
        }
    };

    if (NEED_WRITE_BARRIER) {
        // update field with pre barrier
        auto *barrierSet = GetBarrierSet();
        if (barrierSet->IsPreBarrierEnabled()) {
            barrierSet->PreBarrier(GetObject<false, IS_DYN>(obj, offset));
        }

        getResult();
        if (success && !mem::IsEmptyBarrier(barrierSet->GetPostType())) {
            barrierSet->PostBarrier(ToVoidPtr(ToUintPtr(obj)), offset, newValue);
        }
        return {success, result};
    }

    getResult();
    return {success, result};
}

/* static */
template <typename T>
inline T ObjectAccessor::GetAndSetFieldPrimitive(void *obj, size_t offset, T value, std::memory_order memoryOrder)
{
    uintptr_t rawAddr = reinterpret_cast<uintptr_t>(obj) + offset;
    ASSERT(IsAddressInObjectsHeap(rawAddr));
    auto *atomicAddr = reinterpret_cast<std::atomic<T> *>(rawAddr);
    return atomicAddr->exchange(value, memoryOrder);
}

/* static */
// NEED_WRITE_BARRIER = true , IS_DYN = false
template <bool NEED_WRITE_BARRIER, bool IS_DYN>
// CC-OFFNXT(G.FUD.06) perf critical
inline ObjectHeader *ObjectAccessor::GetAndSetFieldObject(void *obj, size_t offset, ObjectHeader *value,
                                                          std::memory_order memoryOrder)
{
    if (NEED_WRITE_BARRIER) {
        // update field with pre barrier
        auto *barrierSet = GetBarrierSet();
        if (barrierSet->IsPreBarrierEnabled()) {
            barrierSet->PreBarrier(GetObject<false, IS_DYN>(obj, offset));
        }
        ObjectHeader *result = IS_DYN ? GetAndSetFieldPrimitive<ObjectHeader *>(obj, offset, value, memoryOrder)
                                      : reinterpret_cast<ObjectHeader *>(GetAndSetFieldPrimitive<ObjectPointerType>(
                                            obj, offset, ToObjPtrType(value), memoryOrder));
        if (result != nullptr && !mem::IsEmptyBarrier(barrierSet->GetPostType())) {
            barrierSet->PostBarrier(ToVoidPtr(ToUintPtr(obj)), offset, value);
        }

        return result;
    }

    return IS_DYN ? GetAndSetFieldPrimitive<ObjectHeader *>(obj, offset, value, memoryOrder)
                  : reinterpret_cast<ObjectHeader *>(
                        GetAndSetFieldPrimitive<ObjectPointerType>(obj, offset, ToObjPtrType(value), memoryOrder));
}

/* static */
template <typename T, bool USE_UBYTE_ARITHMETIC>
// CC-OFFNXT(G.FUD.06) perf critical
inline T ObjectAccessor::GetAndAddFieldPrimitive([[maybe_unused]] void *obj, [[maybe_unused]] size_t offset,
                                                 [[maybe_unused]] T value,
                                                 [[maybe_unused]] std::memory_order memoryOrder)
{
    if constexpr (std::is_same_v<T, uint8_t> &&
                  !USE_UBYTE_ARITHMETIC) {  // NOLINT(readability-braces-around-statements)
        LOG(FATAL, RUNTIME) << "Could not do add for boolean";
        UNREACHABLE();
    } else {                                          // NOLINT(readability-misleading-indentation)
        if constexpr (std::is_floating_point_v<T>) {  // NOLINT(readability-braces-around-statements)
            // Atmoic fetch_add only defined in the atomic specializations for integral and pointer
            uintptr_t rawAddr = reinterpret_cast<uintptr_t>(obj) + offset;
            ASSERT(IsAddressInObjectsHeap(rawAddr));
            auto *atomicAddr = reinterpret_cast<std::atomic<T> *>(rawAddr);
            // Atomic with parameterized order reason: memory order passed as argument
            T oldValue = atomicAddr->load(memoryOrder);
            T newValue;
            do {
                newValue = oldValue + value;
            } while (!atomicAddr->compare_exchange_weak(oldValue, newValue, memoryOrder));
            return oldValue;
        } else {  // NOLINT(readability-misleading-indentation, readability-else-after-return)
            uintptr_t rawAddr = reinterpret_cast<uintptr_t>(obj) + offset;
            ASSERT(IsAddressInObjectsHeap(rawAddr));
            auto *atomicAddr = reinterpret_cast<std::atomic<T> *>(rawAddr);
            // Atomic with parameterized order reason: memory order passed as argument
            return atomicAddr->fetch_add(value, memoryOrder);
        }
    }
}

/* static */
template <typename T>
// CC-OFFNXT(G.FMT.06, G.FUD.06) project code style
T ObjectAccessor::GetAndSubFieldPrimitiveFloat(void *obj, size_t offset, T value, std::memory_order memoryOrder)
{
    // Atmoic fetch_add only defined in the atomic specializations for integral and pointer
    uintptr_t rawAddr = reinterpret_cast<uintptr_t>(obj) + offset;
    ASSERT(IsAddressInObjectsHeap(rawAddr));
    auto *atomicAddr = reinterpret_cast<std::atomic<T> *>(rawAddr);
    // Atomic with parameterized order reason: memory order passed as argument
    T oldValue = atomicAddr->load(memoryOrder);
    T newValue;
    do {
        newValue = oldValue - value;
    } while (!atomicAddr->compare_exchange_weak(oldValue, newValue, memoryOrder));
    return oldValue;
}

/* static */
template <typename T, bool USE_UBYTE_ARITHMETIC>
// CC-OFFNXT(G.FMT.06, G.FUD.06) project code style
inline std::enable_if_t<!std::is_same_v<T, uint8_t> || USE_UBYTE_ARITHMETIC, T> ObjectAccessor::GetAndSubFieldPrimitive(
    // CC-OFFNXT(G.FMT.06, G.FUD.06) project code style
    [[maybe_unused]] void *obj, [[maybe_unused]] size_t offset, [[maybe_unused]] T value,
    // CC-OFFNXT(G.FMT.06, G.FUD.06) project code style
    [[maybe_unused]] std::memory_order memoryOrder)
{
    if constexpr (std::is_floating_point_v<T>) {  // NOLINT(readability-braces-around-statements)
        return GetAndSubFieldPrimitiveFloat<T>(obj, offset, value, memoryOrder);
    } else {  // NOLINT(readability-misleading-indentation, readability-else-after-return)
        uintptr_t rawAddr = reinterpret_cast<uintptr_t>(obj) + offset;
        ASSERT(IsAddressInObjectsHeap(rawAddr));
        auto *atomicAddr = reinterpret_cast<std::atomic<T> *>(rawAddr);
        // Atomic with parameterized order reason: memory order passed as argument
        return atomicAddr->fetch_sub(value, memoryOrder);
    }
}

/* static */
template <typename T>
inline T ObjectAccessor::GetAndBitwiseOrFieldPrimitive([[maybe_unused]] void *obj, [[maybe_unused]] size_t offset,
                                                       [[maybe_unused]] T value,
                                                       [[maybe_unused]] std::memory_order memoryOrder)
{
    if constexpr (std::is_floating_point_v<T>) {  // NOLINT(readability-braces-around-statements)
        LOG(FATAL, RUNTIME) << "Could not do bitwise or for float/double";
        UNREACHABLE();
    } else {  // NOLINT(readability-misleading-indentation)
        uintptr_t rawAddr = reinterpret_cast<uintptr_t>(obj) + offset;
        ASSERT(IsAddressInObjectsHeap(rawAddr));
        auto *atomicAddr = reinterpret_cast<std::atomic<T> *>(rawAddr);
        // Atomic with parameterized order reason: memory order passed as argument
        return atomicAddr->fetch_or(value, memoryOrder);
    }
}

/* static */
template <typename T>
inline T ObjectAccessor::GetAndBitwiseAndFieldPrimitive([[maybe_unused]] void *obj, [[maybe_unused]] size_t offset,
                                                        [[maybe_unused]] T value,
                                                        [[maybe_unused]] std::memory_order memoryOrder)
{
    if constexpr (std::is_floating_point_v<T>) {  // NOLINT(readability-braces-around-statements)
        LOG(FATAL, RUNTIME) << "Could not do bitwise and for float/double";
        UNREACHABLE();
    } else {  // NOLINT(readability-misleading-indentation)
        uintptr_t rawAddr = reinterpret_cast<uintptr_t>(obj) + offset;
        ASSERT(IsAddressInObjectsHeap(rawAddr));
        auto *atomicAddr = reinterpret_cast<std::atomic<T> *>(rawAddr);
        // Atomic with parameterized order reason: memory order passed as argument
        return atomicAddr->fetch_and(value, memoryOrder);
    }
}

/* static */
template <typename T>
inline T ObjectAccessor::GetAndBitwiseXorFieldPrimitive([[maybe_unused]] void *obj, [[maybe_unused]] size_t offset,
                                                        [[maybe_unused]] T value,
                                                        [[maybe_unused]] std::memory_order memoryOrder)
{
    if constexpr (std::is_floating_point_v<T>) {  // NOLINT(readability-braces-around-statements)
        LOG(FATAL, RUNTIME) << "Could not do bitwise xor for float/double";
        UNREACHABLE();
    } else {  // NOLINT(readability-misleading-indentation)
        uintptr_t rawAddr = reinterpret_cast<uintptr_t>(obj) + offset;
        ASSERT(IsAddressInObjectsHeap(rawAddr));
        auto *atomicAddr = reinterpret_cast<std::atomic<T> *>(rawAddr);
        // Atomic with parameterized order reason: memory order passed as argument
        return atomicAddr->fetch_xor(value, memoryOrder);
    }
}

/* static */
inline void ObjectAccessor::SetDynValueWithoutBarrier(void *obj, size_t offset, coretypes::TaggedType value)
{
    uintptr_t addr = ToUintPtr(obj) + offset;
    ASSERT(IsAddressInObjectsHeap(addr));
    // Atomic with relaxed order reason: concurrent access from GC
    reinterpret_cast<std::atomic<coretypes::TaggedType> *>(addr)->store(value, std::memory_order_relaxed);
}

/* static */
// CC-OFFNXT(G.FUD.06) solid logic
inline void ObjectAccessor::SetDynValue(const ManagedThread *thread, void *obj, size_t offset,
                                        coretypes::TaggedType value)
{
    if (UNLIKELY(GetBarrierSet(thread)->IsPreBarrierEnabled())) {
        coretypes::TaggedValue preVal(GetDynValue<coretypes::TaggedType>(obj, offset));
        if (preVal.IsHeapObject()) {
            GetBarrierSet(thread)->PreBarrier(preVal.GetRawHeapObject());
        }
    }
    SetDynValueWithoutBarrier(obj, offset, value);
    coretypes::TaggedValue tv(value);
    if (tv.IsHeapObject() && tv.GetRawHeapObject() != nullptr) {
        auto gcPostBarrierType = GetPostBarrierType(thread);
        if (!mem::IsEmptyBarrier(gcPostBarrierType)) {
            GetBarrierSet(thread)->PostBarrier(obj, offset, tv.GetRawHeapObject());
        }
    }
}

/* static */
template <typename T>
inline void ObjectAccessor::SetDynPrimitive(const ManagedThread *thread, void *obj, size_t offset, T value)
{
    // Need pre-barrier becuase the previous value may be a reference.
    if (UNLIKELY(GetBarrierSet(thread)->IsPreBarrierEnabled())) {
        coretypes::TaggedValue preVal(GetDynValue<coretypes::TaggedType>(obj, offset));
        if (preVal.IsHeapObject()) {
            GetBarrierSet(thread)->PreBarrier(preVal.GetRawHeapObject());
        }
    }
    SetDynValueWithoutBarrier(obj, offset, value);
    // Don't need post barrier because the value is a primitive.
}

// CC-OFFNXT(G.FUD.06) solid logic
inline void ObjectAccessor::SetClass(ObjectHeader *obj, BaseClass *newClass)
{
    auto *barrierSet = GetBarrierSet();

    if (barrierSet->IsPreBarrierEnabled()) {
        ASSERT(obj->ClassAddr<BaseClass>() != nullptr);
        ObjectHeader *preVal = obj->ClassAddr<BaseClass>()->GetManagedObject();
        barrierSet->PreBarrier(preVal);
    }

    obj->SetClass(newClass);

    auto gcPostBarrierType = barrierSet->GetPostType();
    if (!mem::IsEmptyBarrier(gcPostBarrierType)) {
        ASSERT(newClass->GetManagedObject() != nullptr);
        barrierSet->PostBarrier(ToVoidPtr(ToUintPtr(obj)), 0, newClass->GetManagedObject());
    }
}
}  // namespace ark

#endif  // PANDA_RUNTIME_OBJECT_ACCESSOR_INL_H_
