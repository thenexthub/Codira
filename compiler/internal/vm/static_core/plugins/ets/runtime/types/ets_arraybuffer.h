/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ARRAYBUFFER_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ARRAYBUFFER_H

#include "include/object_accessor.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/ets_coroutine.h"

#include <cstdint>

namespace ark::ets {

namespace test {
class EtsArrayBufferTest;
}  // namespace test

using EtsFinalize = void (*)(void *, void *);

class EtsEscompatArrayBuffer : public EtsObject {
public:
    EtsEscompatArrayBuffer() = delete;
    ~EtsEscompatArrayBuffer() = delete;

    NO_COPY_SEMANTIC(EtsEscompatArrayBuffer);
    NO_MOVE_SEMANTIC(EtsEscompatArrayBuffer);

    static EtsEscompatArrayBuffer *FromEtsObject(EtsObject *arrayBuffer)
    {
        return reinterpret_cast<EtsEscompatArrayBuffer *>(arrayBuffer);
    }

    static constexpr size_t GetClassSize()
    {
        return sizeof(EtsEscompatArrayBuffer);
    }

    /**
     * Creates a byte array in non-movable space.
     * @param length of created array.
     * NOTE: Non-movable creation ensures that native code can obtain raw pointer to buffer.
     */
    ALWAYS_INLINE static EtsByteArray *AllocateNonMovableArray(EtsInt length)
    {
        return EtsByteArray::Create(length, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    }

    /**
     * Creates a managed byte array.
     * @param length of created array.
     * NOTE: Uses common allocation for objects.
     */
    ALWAYS_INLINE static EtsByteArray *AllocateArray(EtsInt length)
    {
        return EtsByteArray::Create(length, SpaceType::SPACE_TYPE_OBJECT);
    }

    ALWAYS_INLINE static EtsLong GetAddress(const EtsByteArray *array)
    {
        return reinterpret_cast<EtsLong>(array->GetData<void>());
    }

    /// Creates ArrayBuffer with managed buffer.
    static EtsEscompatArrayBuffer *Create(EtsCoroutine *coro, size_t length);
    static EtsEscompatArrayBuffer *CreateNonMovable(EtsCoroutine *coro, size_t length, void **resultData);

    static bool IsNonMovableArray(EtsCoroutine *coro, EtsEscompatArrayBuffer *self);
    static bool IsNativeArray(EtsEscompatArrayBuffer *self);

    static void ReallocateNonMovableArray(EtsCoroutine *coro, EtsEscompatArrayBuffer *self, EtsInt bytesLen);

    EtsObject *AsObject()
    {
        return this;
    }

    EtsInt GetByteLength() const;

    /// NOTE: behavior of this method must repeat implementation of `detached` property in ArkTS `ArrayBuffer`
    ALWAYS_INLINE bool WasDetached() const
    {
        return GetManagedDataImpl() != nullptr ? false : GetNativeDataImpl() == nullptr;
    }

    /// @brief Returns managed or native data. Data can be movable in memory or empty.
    ALWAYS_INLINE void *GetData() const
    {
        ASSERT(!WasDetached());
        auto managedData = GetManagedDataImpl();
        return managedData != nullptr ? managedData->GetData<void>() : GetNativeDataImpl();
    }

    template <typename T>
    ALWAYS_INLINE T GetData() const
    {
        return reinterpret_cast<T>(GetData());
    }

    bool IsExternal() const
    {
        return ObjectAccessor::GetObject(this, GetManagedDataOffset()) == nullptr;
    }

    bool IsDetachable() const
    {
        return !WasDetached() && IsExternal();
    }

    EtsByte At(EtsInt pos) const;
    void Set(EtsInt pos, EtsByte val);

    void SetValues(EtsEscompatArrayBuffer *other, EtsInt begin);

    static constexpr size_t GetByteLengthOffset()
    {
        return MEMBER_OFFSET(EtsEscompatArrayBuffer, byteLength_);
    }

    static constexpr size_t GetNativeDataOffset()
    {
        return MEMBER_OFFSET(EtsEscompatArrayBuffer, nativeData_);
    }

    static constexpr size_t GetManagedDataOffset()
    {
        return MEMBER_OFFSET(EtsEscompatArrayBuffer, managedData_);
    }

    static constexpr size_t GetIsResizableOffset()
    {
        return MEMBER_OFFSET(EtsEscompatArrayBuffer, isResizable_);
    }

    /// Initializes ArrayBuffer with its own non-movable array
    void Initialize(EtsCoroutine *coro, size_t length, EtsByteArray *array);

    template <typename T>
    T GetElement(uint32_t index, uint32_t offset);
    template <typename T>
    void SetElement(uint32_t index, uint32_t offset, T element);
    template <typename T>
    T GetVolatileElement(uint32_t index, uint32_t offset);
    template <typename T>
    void SetVolatileElement(uint32_t index, uint32_t offset, T element);
    template <typename T>
    std::pair<bool, T> CompareAndExchangeElement(uint32_t index, uint32_t offset, T oldElement, T newElement,
                                                 bool strong);
    template <typename T>
    T ExchangeElement(uint32_t index, uint32_t offset, T element);
    template <typename T>
    T GetAndAdd(uint32_t index, uint32_t offset, T element);
    template <typename T>
    T GetAndSub(uint32_t index, uint32_t offset, T element);
    template <typename T>
    T GetAndBitwiseOr(uint32_t index, uint32_t offset, T element);
    template <typename T>
    T GetAndBitwiseAnd(uint32_t index, uint32_t offset, T element);
    template <typename T>
    T GetAndBitwiseXor(uint32_t index, uint32_t offset, T element);

private:
    ALWAYS_INLINE ObjectPointer<EtsByteArray> GetManagedDataImpl() const
    {
        return ObjectAccessor::GetPrimitive<ObjectPointer<EtsByteArray>>(this, GetManagedDataOffset());
    }

    ALWAYS_INLINE void *GetNativeDataImpl() const
    {
        return ObjectAccessor::GetPrimitive<void *>(this, GetNativeDataOffset());
    }
    /**
     * @brief Checks position is inside array, throws ets exception if not.
     * NOTE: behavior of this method must repeat initialization from managed `doBoundaryCheck`.
     */
    bool DoBoundaryCheck(EtsInt pos) const;

private:
    // Managed array used in this `ArrayBuffer`, null if buffer is external
    ObjectPointer<EtsByteArray> managedData_;
    ObjectPointer<EtsFinalizableWeakRef> weakRef_;
    // Contains pointer to either managed non-movable data or external data.
    // Null if `ArrayBuffer` was detached, non-null otherwise
    EtsLong nativeData_;
    EtsInt byteLength_;
    EtsBoolean isResizable_;

    friend class test::EtsArrayBufferTest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ARRAYBUFFER_H
