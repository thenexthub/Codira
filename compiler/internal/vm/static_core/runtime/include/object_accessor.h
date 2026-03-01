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
#ifndef PANDA_RUNTIME_OBJECT_ACCESSOR_H_
#define PANDA_RUNTIME_OBJECT_ACCESSOR_H_

#include <cstddef>

#include "libarkbase/utils/atomic.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/mem/gc/gc_barrier_set.h"

namespace ark {

class ObjectHeader;
class Field;
class ManagedThread;

class ObjectAccessor {
public:
    template <class T, bool IS_VOLATILE = false>
    static T GetPrimitive(const void *obj, size_t offset)
    {
        return Get<T, IS_VOLATILE>(obj, offset);
    }

    template <class T, bool IS_VOLATILE = false>
    static void SetPrimitive(void *obj, size_t offset, T value)
    {
        Set<T, IS_VOLATILE>(obj, offset, value);
    }

    template <bool IS_VOLATILE = false, bool NEED_READ_BARRIER = true, bool IS_DYN = false>
    static ObjectHeader *GetObject(const void *obj, size_t offset);

    template <bool IS_VOLATILE = false, bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    static void SetObject(void *obj, size_t offset, ObjectHeader *value);

    template <class T>
    static T GetFieldPrimitive(const void *obj, const Field &field);

    template <class T>
    static void SetFieldPrimitive(void *obj, const Field &field, T value);

    template <bool NEED_READ_BARRIER = true, bool IS_DYN = false>
    static ObjectHeader *GetFieldObject(const void *obj, const Field &field);

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    static void SetFieldObject(void *obj, const Field &field, ObjectHeader *value);

    // Pass thread parameter to speed up interpreter
    template <bool IS_VOLATILE = false, bool NEED_READ_BARRIER = true, bool IS_DYN = false>
    static ObjectHeader *GetObject(const ManagedThread *thread, const void *obj, size_t offset);

    template <bool IS_VOLATILE = false, bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    static void SetObject(const ManagedThread *thread, void *obj, size_t offset, ObjectHeader *value);

    template <bool NEED_READ_BARRIER = true, bool IS_DYN = false>
    static ObjectHeader *GetFieldObject(const ManagedThread *thread, const void *obj, const Field &field);

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    static void SetFieldObject(const ManagedThread *thread, void *obj, const Field &field, ObjectHeader *value);

    template <class T>
    static T GetFieldPrimitive(const void *obj, size_t offset, std::memory_order memoryOrder);

    template <class T>
    static void SetFieldPrimitive(void *obj, size_t offset, T value, std::memory_order memoryOrder);

    template <bool NEED_READ_BARRIER = true, bool IS_DYN = false>
    static ObjectHeader *GetFieldObject(const void *obj, int offset, std::memory_order memoryOrder);

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    static void SetFieldObject(void *obj, size_t offset, ObjectHeader *value, std::memory_order memoryOrder);

    template <typename T>
    static std::pair<bool, T> CompareAndSetFieldPrimitive(void *obj, size_t offset, T oldValue, T newValue,
                                                          std::memory_order memoryOrder, bool strong);

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    static std::pair<bool, ObjectHeader *> CompareAndSetFieldObject(void *obj, size_t offset, ObjectHeader *oldValue,
                                                                    ObjectHeader *newValue,
                                                                    std::memory_order memoryOrder, bool strong);

    template <typename T>
    static T GetAndSetFieldPrimitive(void *obj, size_t offset, T value, std::memory_order memoryOrder);

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    static ObjectHeader *GetAndSetFieldObject(void *obj, size_t offset, ObjectHeader *value,
                                              std::memory_order memoryOrder);

    template <typename T, bool USE_UBYTE_ARITHMETIC = false>
    static T GetAndAddFieldPrimitive(void *obj, size_t offset, T value, std::memory_order memoryOrder);

    template <typename T, bool USE_UBYTE_ARITHMETIC = false>
    static std::enable_if_t<!std::is_same_v<T, uint8_t> || USE_UBYTE_ARITHMETIC, T> GetAndSubFieldPrimitive(
        void *obj, size_t offset, T value, std::memory_order memoryOrder);

    template <typename T>
    static T GetAndBitwiseOrFieldPrimitive(void *obj, size_t offset, T value, std::memory_order memoryOrder);

    template <typename T>
    static T GetAndBitwiseAndFieldPrimitive(void *obj, size_t offset, T value, std::memory_order memoryOrder);

    template <typename T>
    static T GetAndBitwiseXorFieldPrimitive(void *obj, size_t offset, T value, std::memory_order memoryOrder);

    static inline void SetDynValueWithoutBarrier(void *obj, size_t offset, coretypes::TaggedType value);

    static inline void SetDynValue(const ManagedThread *thread, void *obj, size_t offset, coretypes::TaggedType value);

    template <typename T>
    static inline void SetDynPrimitive(const ManagedThread *thread, void *obj, size_t offset, T value);

    template <bool IS_VOLATILE = false, bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    static void FillObjects(void *objArr, size_t dataOffset, size_t count, size_t elemSize, ObjectHeader *value);

    template <class T>
    static inline T GetDynValue(const void *obj, size_t offset)
    {
        uintptr_t addr = ToUintPtr(obj) + offset;
        ASSERT(IsAddressInObjectsHeap(addr));
        // Atomic with relaxed order reason: concurrent access from GC
        return reinterpret_cast<const std::atomic<T> *>(addr)->load(std::memory_order_relaxed);
    }

    static void SetClass(ObjectHeader *obj, BaseClass *newClass);

    template <LangTypeT LANGUAGE_TYPE>
    static bool IsHeapObject(
        std::conditional_t<LANGUAGE_TYPE == LangTypeT::LANG_TYPE_STATIC, ObjectPointerType, coretypes::TaggedType> v)
    {
        if constexpr (LANGUAGE_TYPE == LangTypeT::LANG_TYPE_STATIC) {
            return reinterpret_cast<ObjectHeader *>(v) != nullptr;
        } else {
            return coretypes::TaggedValue(v).IsHeapObject();
        }
    }

    template <LangTypeT LANGUAGE_TYPE>
    static ObjectHeader *DecodeNotNull(
        std::conditional_t<LANGUAGE_TYPE == LangTypeT::LANG_TYPE_STATIC, ObjectPointerType, coretypes::TaggedType> v)
    {
        if constexpr (LANGUAGE_TYPE == LangTypeT::LANG_TYPE_STATIC) {
            auto *p = reinterpret_cast<ObjectHeader *>(v);
            ASSERT(p != nullptr);
            return p;
        } else {
            return coretypes::TaggedValue(v).GetHeapObject();
        }
    }

    template <typename P>
    static P Load(P *p)
    {
        return *p;
    }

    template <typename P>
    static P LoadAtomic(P *p)
    {
        return AtomicLoad(p, std::memory_order_relaxed);
    }

    template <LangTypeT LANGUAGE_TYPE>
    static void Store(
        std::conditional_t<LANGUAGE_TYPE == LangTypeT::LANG_TYPE_STATIC, ObjectPointerType, coretypes::TaggedType> *ref,
        ObjectHeader *val)
    {
        if constexpr (LANGUAGE_TYPE == LangTypeT::LANG_TYPE_STATIC) {
            *ref = EncodeObjectPointerType(val);
        } else {
            *ref = EncodeTaggedType(val);
        }
    }

private:
    template <class T, bool IS_VOLATILE>
    static T Get(const void *obj, size_t offset)
    {
        auto *addr = reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(obj) + offset);
        ASSERT(IsAddressInObjectsHeap(addr));
        if (IS_VOLATILE) {
            // Atomic with seq_cst order reason: required for volatile
            return reinterpret_cast<const std::atomic<T> *>(addr)->load(std::memory_order_seq_cst);
        }
        // Atomic with relaxed order reason: to be compatible with other vms
        return reinterpret_cast<const std::atomic<T> *>(addr)->load(std::memory_order_relaxed);
    }

    template <bool IS_VOLATILE = false, bool IS_DYN = false>
    static void FillObjsWithPreBarrier(void *objArr, size_t dataOffset, size_t count, size_t elemSize,
                                       ObjectHeader *value);

    template <bool IS_VOLATILE = false, bool IS_DYN = false>
    static void FillObjsNoBarrier(void *objArr, size_t dataOffset, size_t count, size_t elemSize, ObjectHeader *value);

    template <typename T>
    static T GetAndSubFieldPrimitiveFloat(void *obj, size_t offset, T value, std::memory_order memoryOrder);

    template <class T, bool IS_VOLATILE>
    static void Set(void *obj, size_t offset, T value)
    {
        auto *addr = reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(obj) + offset);
        ASSERT(IsAddressInObjectsHeap(addr));
        if (IS_VOLATILE) {
            // Atomic with seq_cst order reason: required for volatile
            return reinterpret_cast<std::atomic<T> *>(addr)->store(value, std::memory_order_seq_cst);
        }
        // Atomic with relaxed order reason: to be compatible with other vms
        return reinterpret_cast<std::atomic<T> *>(addr)->store(value, std::memory_order_relaxed);
    }

    template <class T>
    static T Get(const void *obj, size_t offset, std::memory_order memoryOrder)
    {
        auto *addr = reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(obj) + offset);
        ASSERT(IsAddressInObjectsHeap(addr));
        // Atomic with parameterized order reason: memory order passed as argument
        return reinterpret_cast<const std::atomic<T> *>(addr)->load(memoryOrder);
    }

    template <class T>
    static void Set(void *obj, size_t offset, T value, std::memory_order memoryOrder)
    {
        auto *addr = reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(obj) + offset);
        ASSERT(IsAddressInObjectsHeap(addr));
        // Atomic with parameterized order reason: memory order passed as argument
        return reinterpret_cast<std::atomic<T> *>(addr)->store(value, memoryOrder);
    }

    static ObjectPointerType EncodeObjectPointerType(ObjectHeader *obj)
    {
        return static_cast<ObjectPointerType>(ToUintPtr(obj));
    }

    static coretypes::TaggedType EncodeTaggedType(ObjectHeader *obj)
    {
        return coretypes::TaggedValue::Cast(obj);
    }

    PANDA_PUBLIC_API static mem::GCBarrierSet *GetBarrierSet();

    PANDA_PUBLIC_API static mem::GCBarrierSet *GetBarrierSet(const ManagedThread *thread);

    static mem::BarrierType GetPreBarrierType(const ManagedThread *thread);

    PANDA_PUBLIC_API static mem::BarrierType GetPostBarrierType(const ManagedThread *thread);
};

}  // namespace ark

#endif  // PANDA_RUNTIME_OBJECT_ACCESSOR_H_
