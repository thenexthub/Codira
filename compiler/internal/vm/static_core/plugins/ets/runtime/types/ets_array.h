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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_ARRAY_H_
#define PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_ARRAY_H_

#include "libarkbase/macros.h"
#include "libarkbase/mem/space.h"
#include "runtime/include/coretypes/array.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/ets_class_root.h"
#include "plugins/ets/runtime/ets_vm.h"

namespace ark::ets {

// Private inheritance, because need to disallow implicit conversion to core type
class EtsArray : private coretypes::Array {
public:
    EtsArray() = delete;
    ~EtsArray() = delete;

    PANDA_PUBLIC_API size_t GetLength() const
    {
        return GetCoreType()->GetLength();
    }

    size_t GetElementSize() const
    {
        return GetCoreType()->ClassAddr<Class>()->GetComponentSize();
    }

    size_t ObjectSize() const
    {
        return GetCoreType()->ObjectSize(GetElementSize());
    }

    template <class T>
    T *GetData()
    {
        return reinterpret_cast<T *>(GetCoreType()->GetData());
    }

    template <class T>
    const T *GetData() const
    {
        return reinterpret_cast<const T *>(GetCoreType()->GetData());
    }

    EtsClass *GetClass() const
    {
        return EtsClass::FromRuntimeClass(GetCoreType()->ClassAddr<Class>());
    }

    bool IsPrimitive() const
    {
        auto componentType = GetCoreType()->ClassAddr<Class>()->GetComponentType();
        ASSERT(componentType != nullptr);
        return componentType->IsPrimitive();
    }

    static constexpr uint32_t GetDataOffset()
    {
        return coretypes::Array::GetDataOffset();
    }

    static EtsArray *FromEtsObject(EtsObject *object)
    {
        ASSERT(object->IsArrayClass());
        return reinterpret_cast<EtsArray *>(object);
    }

    static coretypes::Array *GetCoreType(EtsArray *array)
    {
        return reinterpret_cast<coretypes::Array *>(array);
    }

    EtsObject *AsObject()
    {
        return reinterpret_cast<EtsObject *>(this);
    }

    const EtsObject *AsObject() const
    {
        return reinterpret_cast<const EtsObject *>(this);
    }

    coretypes::Array *GetCoreType()
    {
        return reinterpret_cast<coretypes::Array *>(this);
    }

    const coretypes::Array *GetCoreType() const
    {
        return reinterpret_cast<const coretypes::Array *>(this);
    }

    NO_COPY_SEMANTIC(EtsArray);
    NO_MOVE_SEMANTIC(EtsArray);

    void UpdateLength(ArraySizeT length)
    {
        SetLength(length);
    }

protected:
    // Use type alias to allow using into derived classes
    using ObjectHeader = ::ark::ObjectHeader;

    template <class T>
    static T *Create(EtsClass *arrayClass, uint32_t length, SpaceType spaceType = SpaceType::SPACE_TYPE_OBJECT,
                     bool pinned = false)
    {
        return reinterpret_cast<T *>(
            coretypes::Array::Create(arrayClass->GetRuntimeClass(), length, spaceType, pinned));
    }

    template <class T, bool IS_VOLATILE = false>
    void SetImpl(uint32_t idx, T elem, uint32_t byteOffset = 0)
    {
        GetCoreType()->Set<T, true, false, IS_VOLATILE>(idx, elem, byteOffset);
    }

    template <class T, bool IS_VOLATILE = false>
    T GetImpl(uint32_t idx, uint32_t byteOffset = 0) const
    {
        return GetCoreType()->Get<T, true, false, IS_VOLATILE>(idx, byteOffset);
    }

    template <class T>
    void FillImpl(T elem, uint32_t start, uint32_t end)
    {
        GetCoreType()->Fill(elem, start, end);
    }

    template <class T>
    std::pair<bool, T> CompareAndExchangeImpl(uint32_t idx, T oldElemValue, T newValue, bool strong,
                                              uint32_t byteOffset = 0)
    {
        return GetCoreType()->CompareAndExchange(idx, oldElemValue, newValue, std::memory_order_seq_cst, strong,
                                                 byteOffset);
    }

    template <class T>
    T ExchangeImpl(uint32_t idx, T val, uint32_t byteOffset = 0)
    {
        return GetCoreType()->Exchange(idx, val, std::memory_order_seq_cst, byteOffset);
    }

    template <class T>
    T GetAndAddImpl(uint32_t idx, T value, uint32_t byteOffset = 0)
    {
        return GetCoreType()->GetAndAdd(idx, value, std::memory_order_seq_cst, byteOffset);
    }

    template <class T>
    T GetAndSubImpl(uint32_t idx, T value, uint32_t byteOffset = 0)
    {
        return GetCoreType()->GetAndSub(idx, value, std::memory_order_seq_cst, byteOffset);
    }

    template <class T>
    T GetAndBitwiseOrImpl(uint32_t idx, T value, uint32_t byteOffset = 0)
    {
        return GetCoreType()->GetAndBitwiseOr(idx, value, std::memory_order_seq_cst, byteOffset);
    }

    template <class T>
    T GetAndBitwiseAndImpl(uint32_t idx, T value, uint32_t byteOffset = 0)
    {
        return GetCoreType()->GetAndBitwiseAnd(idx, value, std::memory_order_seq_cst, byteOffset);
    }

    template <class T>
    T GetAndBitwiseXorImpl(uint32_t idx, T value, uint32_t byteOffset = 0)
    {
        return GetCoreType()->GetAndBitwiseXor(idx, value, std::memory_order_seq_cst, byteOffset);
    }
};

template <typename Component>
class EtsTypedObjectArray : public EtsArray {
public:
    using ValueType = Component *;

    static EtsTypedObjectArray *Create(EtsClass *objectClass, uint32_t length,
                                       ark::SpaceType spaceType = ark::SpaceType::SPACE_TYPE_OBJECT)
    {
        ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();
        // Generate Array class name  "[L<object_class>;"
        EtsClassLinker *classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
        ASSERT(objectClass != nullptr);
        PandaString arrayClassName = PandaString("[") + objectClass->GetDescriptor();
        EtsClass *arrayClass = classLinker->GetClass(arrayClassName.c_str(), true, objectClass->GetLoadContext());
        if (arrayClass == nullptr) {
            return nullptr;
        }
        return EtsArray::Create<EtsTypedObjectArray>(arrayClass, length, spaceType);
    }

    void Set(uint32_t index, Component *element)
    {
        if (element == nullptr) {
            SetImpl<ObjectHeader *>(index, nullptr);
        } else {
            SetImpl(index, element->GetCoreType());
        }
    }

    PANDA_PUBLIC_API Component *Get(uint32_t index) const
    {
        return reinterpret_cast<Component *>(
            GetImpl<std::invoke_result_t<decltype(&Component::GetCoreType), Component>>(index));
    }

    void Fill(Component *element, uint32_t start, uint32_t end)
    {
        if (element == nullptr) {
            FillImpl<ObjectHeader *>(nullptr, start, end);
        } else {
            FillImpl(element->GetCoreType(), start, end);
        }
    }

    void Set(uint32_t index, Component *element, std::memory_order memoryOrder)
    {
        auto offset = index * sizeof(ObjectPointerType);
        GetCoreType()->SetObject(offset, element == nullptr ? nullptr : element->GetCoreType(), memoryOrder);
    }

    Component *Get(uint32_t index, std::memory_order memoryOrder) const
    {
        auto offset = index * sizeof(ObjectPointerType);
        return Component::FromCoreType(GetCoreType()->GetObject(offset, memoryOrder));
    }

    static EtsTypedObjectArray *FromEtsObject(EtsObject *object)
    {
        ASSERT(object->IsArrayClass());
        return reinterpret_cast<EtsTypedObjectArray *>(object);
    }

    static EtsTypedObjectArray *FromCoreType(ObjectHeader *objectHeader)
    {
        return reinterpret_cast<EtsTypedObjectArray *>(objectHeader);
    }

    void CopyDataTo(EtsTypedObjectArray *dst) const
    {
        ASSERT(dst != nullptr);
        ASSERT(GetLength() <= dst->GetLength());

        if (std::size_t count = GetLength() * OBJECT_POINTER_SIZE) {
            Span<const uint8_t> srcSpan(GetData<uint8_t>(), count);
            Span<uint8_t> dstSpan(dst->GetData<uint8_t>(), count);
            CopyData(srcSpan, dstSpan);
            // Call barriers.
            // PreBarrier isn't needed as links inside the source object arn't changed.
            auto *barrierSet = ManagedThread::GetCurrent()->GetBarrierSet();
            if (!mem::IsEmptyBarrier(barrierSet->GetPostType())) {
                barrierSet->PostBarrier(dst, GetDataOffset(), count);
            }
        }
    }

    EtsTypedObjectArray() = delete;
    ~EtsTypedObjectArray() = delete;

private:
    NO_COPY_SEMANTIC(EtsTypedObjectArray);
    NO_MOVE_SEMANTIC(EtsTypedObjectArray);

    using WordType = uintptr_t;
    using AtomicWord = std::atomic<WordType>;
    using AtomicRef = std::atomic<ark::ObjectPointerType>;

    static constexpr const std::size_t WORD_SIZE = sizeof(WordType);

    void CopyData(Span<const uint8_t> &src, Span<uint8_t> &dst) const
    {
        ASSERT((src.Size() % OBJECT_POINTER_SIZE) == 0);
        ASSERT(src.Size() <= dst.Size());

        // WORDs and REFERENCEs must be loaded/stored atomically
        constexpr const std::memory_order ORDER = std::memory_order_relaxed;
        // 1. copy by words if any
        std::size_t i = 0;
        std::size_t stop = (src.Size() / WORD_SIZE) * WORD_SIZE;
        for (; i < stop; i += WORD_SIZE) {
            // Atomic with parameterized order reason: memory order defined as constexpr
            reinterpret_cast<AtomicWord *>(&dst[i])->store(reinterpret_cast<const AtomicWord *>(&src[i])->load(ORDER),
                                                           ORDER);
        }
        // 2. copy by references if any
        stop = src.Size();
        for (; i < stop; i += OBJECT_POINTER_SIZE) {
            // Atomic with parameterized order reason: memory order defined as constexpr
            reinterpret_cast<AtomicRef *>(&dst[i])->store(reinterpret_cast<const AtomicRef *>(&src[i])->load(ORDER),
                                                          ORDER);
        }
    }
};

using EtsObjectArray = EtsTypedObjectArray<EtsObject>;

template <class ClassType, EtsClassRoot ETS_CLASS_ROOT>
class EtsPrimitiveArray : public EtsArray {
public:
    using ValueType = ClassType;

    static constexpr const EtsPrimitiveArray<ClassType, ETS_CLASS_ROOT> *FromCoreType(const ObjectHeader *object)
    {
        return reinterpret_cast<const EtsPrimitiveArray<ClassType, ETS_CLASS_ROOT> *>(object);
    }

    static constexpr EtsPrimitiveArray<ClassType, ETS_CLASS_ROOT> *FromEtsObject(EtsObject *object)
    {
        return reinterpret_cast<EtsPrimitiveArray<ClassType, ETS_CLASS_ROOT> *>(object);
    }

    static EtsPrimitiveArray *Create(uint32_t length, SpaceType spaceType = SpaceType::SPACE_TYPE_OBJECT,
                                     bool pinned = false)
    {
        ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();
        return EtsArray::Create<EtsPrimitiveArray>(GetComponentClass(), length, spaceType, pinned);
    }
    void Set(uint32_t index, ClassType element)
    {
        SetImpl(index, element);
    }
    ClassType Get(uint32_t index) const
    {
        return GetImpl<ClassType>(index);
    }
    void SetVolatile(uint32_t index, uint32_t byteOffset, ClassType element)
    {
        SetImpl<ClassType, true>(index, element, byteOffset);
    }
    ClassType GetVolatile(uint32_t index, uint32_t byteOffset) const
    {
        return GetImpl<ClassType, true>(index, byteOffset);
    }
    std::pair<bool, ClassType> CompareAndExchange(uint32_t index, uint32_t byteOffset, ClassType oldVal,
                                                  ClassType newVal, bool strong)
    {
        return CompareAndExchangeImpl(index, oldVal, newVal, strong, byteOffset);
    }
    ClassType Exchange(uint32_t index, uint32_t byteOffset, ClassType val)
    {
        return ExchangeImpl(index, val, byteOffset);
    }
    ClassType GetAndAdd(uint32_t index, uint32_t byteOffset, ClassType val)
    {
        return GetAndAddImpl(index, val, byteOffset);
    }
    ClassType GetAndSub(uint32_t index, uint32_t byteOffset, ClassType val)
    {
        return GetAndSubImpl(index, val, byteOffset);
    }
    ClassType GetAndBitwiseAnd(uint32_t index, uint32_t byteOffset, ClassType val)
    {
        return GetAndBitwiseAndImpl(index, val, byteOffset);
    }
    ClassType GetAndBitwiseOr(uint32_t index, uint32_t byteOffset, ClassType val)
    {
        return GetAndBitwiseOrImpl(index, val, byteOffset);
    }
    ClassType GetAndBitwiseXor(uint32_t index, uint32_t byteOffset, ClassType val)
    {
        return GetAndBitwiseXorImpl(index, val, byteOffset);
    }

    static EtsClass *GetComponentClass()
    {
        return PandaEtsVM::GetCurrent()->GetClassLinker()->GetClassRoot(ETS_CLASS_ROOT);
    }

    EtsPrimitiveArray() = delete;
    ~EtsPrimitiveArray() = delete;

private:
    NO_COPY_SEMANTIC(EtsPrimitiveArray);
    NO_MOVE_SEMANTIC(EtsPrimitiveArray);
};

using EtsBooleanArray = EtsPrimitiveArray<EtsBoolean, EtsClassRoot::BOOLEAN_ARRAY>;
using EtsByteArray = EtsPrimitiveArray<EtsByte, EtsClassRoot::BYTE_ARRAY>;
using EtsCharArray = EtsPrimitiveArray<EtsChar, EtsClassRoot::CHAR_ARRAY>;
using EtsShortArray = EtsPrimitiveArray<EtsShort, EtsClassRoot::SHORT_ARRAY>;
using EtsUintArray = EtsPrimitiveArray<EtsUint, EtsClassRoot::UINT_ARRAY>;
using EtsIntArray = EtsPrimitiveArray<EtsInt, EtsClassRoot::INT_ARRAY>;
using EtsUlongArray = EtsPrimitiveArray<EtsUlong, EtsClassRoot::ULONG_ARRAY>;
using EtsLongArray = EtsPrimitiveArray<EtsLong, EtsClassRoot::LONG_ARRAY>;
using EtsFloatArray = EtsPrimitiveArray<EtsFloat, EtsClassRoot::FLOAT_ARRAY>;
using EtsDoubleArray = EtsPrimitiveArray<EtsDouble, EtsClassRoot::DOUBLE_ARRAY>;

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_ARRAY_H_
