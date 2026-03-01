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

#ifndef PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_OBJECT_H_
#define PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_OBJECT_H_

#include <cstdint>
#include "plugins/ets/runtime/ets_mark_word.h"
#include "mark_word.h"
#include "libarkbase/mem/mem.h"
#include "runtime/include/object_header-inl.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_field.h"

namespace ark::ets {

class EtsCoroutine;

// Private inheritance, because need to disallow implicit conversion to core type
class EtsObject : private ObjectHeader {
public:
    PANDA_PUBLIC_API static EtsObject *Create(EtsCoroutine *etsCoroutine, EtsClass *klass);
    PANDA_PUBLIC_API static EtsObject *CreateNonMovable(EtsClass *klass);

    PANDA_PUBLIC_API static EtsObject *Create(EtsClass *klass);

    PANDA_PUBLIC_API EtsClass *GetClass() const
    {
        return EtsClass::FromRuntimeClass(GetCoreType()->ClassAddr<Class>());
    }

    void SetClass(EtsClass *cls)
    {
        GetCoreType()->SetClass(UNLIKELY(cls == nullptr) ? nullptr : cls->GetRuntimeClass());
    }

    bool IsInstanceOf(EtsClass *klass) const
    {
        ASSERT(klass != nullptr);
        return klass->IsAssignableFrom(GetClass());
    }

    bool HasField(EtsField *field) const
    {
        ASSERT(field != nullptr);
        return field->GetDeclaringClass()->IsAssignableFrom(field->GetDeclaringClass());
    }

    EtsObject *GetAndSetFieldObject(size_t offset, EtsObject *value, std::memory_order memoryOrder)
    {
        ASSERT(value != nullptr);
        return FromCoreType(GetCoreType()->GetAndSetFieldObject(offset, value->GetCoreType(), memoryOrder));
    }

    template <class T>
    T GetFieldPrimitive(EtsField *field) const
    {
        ASSERT(field != nullptr);
        ASSERT(field->GetEtsType() == GetEtsTypeByPrimitive<T>());
        ASSERT(HasField(field));
        return GetCoreType()->GetFieldPrimitive<T>(*field->GetRuntimeField());
    }

    template <class T, bool IS_VOLATILE = false>
    T GetFieldPrimitive(size_t offset) const
    {
        return GetCoreType()->GetFieldPrimitive<T, IS_VOLATILE>(offset);
    }

    template <class T>
    T GetFieldPrimitive(int32_t fieldOffset, bool isVolatile) const
    {
        if (isVolatile) {
            return GetCoreType()->GetFieldPrimitive<T, true>(fieldOffset);
        }
        return GetCoreType()->GetFieldPrimitive<T, false>(fieldOffset);
    }

    template <class T>
    void SetFieldPrimitive(EtsField *field, T value)
    {
        ASSERT(field != nullptr);
        ASSERT(field->GetEtsType() == GetEtsTypeByPrimitive<T>());
        ASSERT(HasField(field));
        GetCoreType()->SetFieldPrimitive<T>(*field->GetRuntimeField(), value);
    }

    template <class T>
    void SetFieldPrimitive(int32_t fieldOffset, bool isVolatile, T value)
    {
        if (isVolatile) {
            GetCoreType()->SetFieldPrimitive<T, true>(fieldOffset, value);
        }
        GetCoreType()->SetFieldPrimitive<T, false>(fieldOffset, value);
    }

    template <class T, bool IS_VOLATILE = false>
    void SetFieldPrimitive(size_t offset, T value)
    {
        GetCoreType()->SetFieldPrimitive<T, IS_VOLATILE>(offset, value);
    }

    template <bool NEED_READ_BARRIER = true>
    PANDA_PUBLIC_API EtsObject *GetFieldObject(EtsField *field) const
    {
        ASSERT(field != nullptr);
        ASSERT(field->GetEtsType() == EtsType::OBJECT);
        ASSERT(HasField(field));
        return reinterpret_cast<EtsObject *>(
            GetCoreType()->GetFieldObject<NEED_READ_BARRIER>(*field->GetRuntimeField()));
    }

    EtsObject *GetFieldObject(int32_t fieldOffset, bool isVolatile) const
    {
        if (isVolatile) {
            return reinterpret_cast<EtsObject *>(GetCoreType()->GetFieldObject<true>(fieldOffset));
        }
        return reinterpret_cast<EtsObject *>(GetCoreType()->GetFieldObject<false>(fieldOffset));
    }

    template <bool IS_VOLATILE = false>
    EtsObject *GetFieldObject(size_t offset)
    {
        return reinterpret_cast<EtsObject *>(GetCoreType()->GetFieldObject<IS_VOLATILE>(offset));
    }

    template <bool NEED_WRITE_BARRIER = true>
    void SetFieldObject(EtsField *field, EtsObject *value)
    {
        ASSERT(field != nullptr);
        ASSERT(field->GetEtsType() == EtsType::OBJECT);
        ASSERT(HasField(field));
        GetCoreType()->SetFieldObject<NEED_WRITE_BARRIER>(*field->GetRuntimeField(),
                                                          reinterpret_cast<ObjectHeader *>(value));
    }

    template <bool NEED_WRITE_BARRIER = true>
    void SetFieldObject(int32_t fieldOffset, bool isVolatile, EtsObject *value)
    {
        if (isVolatile) {
            GetCoreType()->SetFieldObject<true, NEED_WRITE_BARRIER>(fieldOffset,
                                                                    reinterpret_cast<ObjectHeader *>(value));
        } else {
            GetCoreType()->SetFieldObject<false, NEED_WRITE_BARRIER>(fieldOffset,
                                                                     reinterpret_cast<ObjectHeader *>(value));
        }
    }

    template <bool IS_VOLATILE = false>
    void SetFieldObject(size_t offset, EtsObject *value)
    {
        GetCoreType()->SetFieldObject<IS_VOLATILE>(offset, reinterpret_cast<ObjectHeader *>(value));
    }

    void SetFieldObject(size_t offset, EtsObject *value, std::memory_order memoryOrder)
    {
        GetCoreType()->SetFieldObject(offset, value->GetCoreType(), memoryOrder);
    }

    template <typename T>
    bool CompareAndSetFieldPrimitive(size_t offset, T oldValue, T newValue, std::memory_order memoryOrder, bool strong)
    {
        return GetCoreType()->CompareAndSetFieldPrimitive(offset, oldValue, newValue, memoryOrder, strong);
    }

    bool CompareAndSetFieldObject(size_t offset, EtsObject *oldValue, EtsObject *newValue,
                                  std::memory_order memoryOrder, bool strong)
    {
        return GetCoreType()->CompareAndSetFieldObject(offset, reinterpret_cast<ObjectHeader *>(oldValue),
                                                       reinterpret_cast<ObjectHeader *>(newValue), memoryOrder, strong);
    }

    EtsObject *Clone() const
    {
        return FromCoreType(ObjectHeader::Clone(GetCoreType()));
    }

    static constexpr ObjectHeader *ToCoreType(EtsObject *obj)
    {
        return static_cast<ObjectHeader *>(obj);
    }

    static constexpr const ObjectHeader *ToCoreType(const EtsObject *obj)
    {
        return static_cast<const ObjectHeader *>(obj);
    }

    ObjectHeader *GetCoreType() const
    {
        return ToCoreType(const_cast<EtsObject *>(this));
    }

    static constexpr EtsObject *FromCoreType(ObjectHeader *objectHeader)
    {
        return static_cast<EtsObject *>(objectHeader);
    }

    static constexpr const EtsObject *FromCoreType(const ObjectHeader *objectHeader)
    {
        return static_cast<const EtsObject *>(objectHeader);
    }

    PANDA_PUBLIC_API bool IsStringClass()
    {
        return GetClass()->IsStringClass();
    }

    PANDA_PUBLIC_API bool IsArrayClass()
    {
        return GetClass()->IsArrayClass();
    }

    // NOTE(ipetrov, #20886): Support separated Interop and Hash states
    /**
     * Get hash code if object has been already hashed,
     * or generate and set hash code in the object header and return it
     * @return hash code for the object
     */
    uint32_t GetHashCode();

    /**
     * @brief Get interop index of object. It should be setted. You can check if it's setted by
     * HasInteropIndexed method.
     * This method is thread safe to GetHashCode, HasInteropIndexed methods. Also it's thread safe to itself.
     * @see HasInteropIndexed, GetHashCode.
     * @return interop index of object.
     */
    uint32_t GetInteropIndex() const;

    /**
     * @brief This method sets interop index of the object. Object must not have it. You can check if it's setted by
     * HasInteropIndexed method.
     * This method is thread safe to GetHashCode, HasInteropIndexed methods. IT IN NOT THREAD SAFE TO ITSELF!
     * @see HasInteropIndexed, GetHashCode.
     */
    void SetInteropIndex(uint32_t index);

    /**
     * @brief This method drops interop index of the object. Object must have it. You can check if it's setted by
     * HasInteropIndexed method. If object is in USE_INFO state, it will no be changed until Deflate method is called.
     * This method is thread safe to GetHashCode, HasInteropIndexed methods. IT IN NOT THREAD SAFE TO ITSELF!
     * @see HasInteropIndexed, GetHashCode, Deflate
     */
    void DropInteropIndex();

    bool HasInteropIndex() const
    {
        bool hasInteropIndex = false;
        while (!TryCheckIfHasInteropIndex(&hasInteropIndex)) {
        }
        return hasInteropIndex;
    }

    bool IsHashed() const
    {
        auto mark = AtomicGetMark(std::memory_order_relaxed);
        auto markState = mark.GetState();
        return markState == EtsMarkWord::STATE_USE_INFO || markState == EtsMarkWord::STATE_HASHED;
    }

    bool IsUsedInfo() const
    {
        auto mark = AtomicGetMark(std::memory_order_relaxed);
        auto markState = mark.GetState();
        return markState == EtsMarkWord::STATE_USE_INFO;
    }

    ALWAYS_INLINE void SetMark(EtsMarkWord word)
    {
        ObjectHeader::SetMark(word.ToMark());
    }

    ALWAYS_INLINE EtsMarkWord AtomicGetMark(std::memory_order memoryOrder = std::memory_order_seq_cst) const
    {
        return EtsMarkWord::FromMarkWord(ObjectHeader::AtomicGetMark(memoryOrder));
    }

    ALWAYS_INLINE EtsMarkWord GetMark() const
    {
        return EtsMarkWord::FromMarkWord(ObjectHeader::GetMark());
    }

    template <bool STRONG = true>
    ALWAYS_INLINE bool AtomicSetMark(EtsMarkWord &oldMarkWord, EtsMarkWord newMarkWord,
                                     std::memory_order memoryOrder = std::memory_order_seq_cst)
    {
        return ObjectHeader::AtomicSetMark<STRONG>(oldMarkWord, newMarkWord.ToMark(), memoryOrder);
    }

    EtsObject() = delete;
    ~EtsObject() = delete;

protected:
    // Use type alias to allow using into derived classes
    using ObjectHeader = ::ark::ObjectHeader;

private:
    static inline uint32_t GenerateHashCode();

    [[nodiscard]] bool TryGetHashCode(uint32_t *hash);
    [[nodiscard]] bool TryGetInteropIndex(uint32_t *index) const;
    [[nodiscard]] bool TrySetInteropIndex(uint32_t index);
    [[nodiscard]] bool TryDropInteropIndex();
    [[nodiscard]] bool TryDeflate();
    [[nodiscard]] PANDA_PUBLIC_API bool TryCheckIfHasInteropIndex(bool *hasInteropIndexed) const;

    NO_COPY_SEMANTIC(EtsObject);
    NO_MOVE_SEMANTIC(EtsObject);
};

// Size of EtsObject must be equal size of ObjectHeader
static_assert(sizeof(EtsObject) == sizeof(ObjectHeader));

}  // namespace ark::ets

#endif  // PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_OBJECT_H_
