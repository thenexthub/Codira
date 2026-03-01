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
#ifndef PANDA_RUNTIME_CORETYPES_ARRAY_H_
#define PANDA_RUNTIME_CORETYPES_ARRAY_H_

#include <securec.h>
#include <cstddef>
#include <cstdint>

#include "libarkbase/macros.h"
#include "libarkbase/mem/mem.h"
#include "libarkbase/mem/space.h"
#include "libarkbase/utils/span.h"
#include "libarkfile/bytecode_instruction-inl.h"
#include "runtime/include/class-inl.h"
#include "runtime/include/language_context.h"
#include "runtime/include/object_header.h"
#include "runtime/mem/heap_manager.h"
#include "runtime/include/coretypes/tagged_value.h"

namespace ark {
class ManagedThread;
class PandaVM;
}  // namespace ark

namespace ark::interpreter {
template <BytecodeInstruction::Format FORMAT, bool IS_DYNAMIC = false>
class DimIterator;
}  // namespace ark::interpreter

namespace ark::coretypes {
class DynClass;
using ArraySizeT = ark::ArraySizeT;
using ArraySsizeT = ark::ArraySsizeT;

class Array : public ObjectHeader {
public:
    static constexpr ArraySizeT MAX_ARRAY_INDEX = std::numeric_limits<ArraySizeT>::max();

    static Array *Cast(ObjectHeader *object)
    {
        // NOTE(linxiang) to do assert
        return reinterpret_cast<Array *>(object);
    }

    PANDA_PUBLIC_API static Array *Create(ark::Class *arrayClass, const uint8_t *data, ArraySizeT length,
                                          ark::SpaceType spaceType = ark::SpaceType::SPACE_TYPE_OBJECT,
                                          bool pinned = false);

    PANDA_PUBLIC_API static Array *Create(ark::Class *arrayClass, ArraySizeT length,
                                          ark::SpaceType spaceType = ark::SpaceType::SPACE_TYPE_OBJECT,
                                          bool pinned = false);

    PANDA_PUBLIC_API static Array *Create(DynClass *dynarrayclass, ArraySizeT length,
                                          ark::SpaceType spaceType = ark::SpaceType::SPACE_TYPE_OBJECT,
                                          bool pinned = false);

    static Array *CreateTagged(const PandaVM *vm, ark::BaseClass *arrayClass, ArraySizeT length,
                               ark::SpaceType spaceType = ark::SpaceType::SPACE_TYPE_OBJECT,
                               TaggedValue initValue = TaggedValue::Undefined());

    static size_t ComputeSize(size_t elemSize, ArraySizeT length)
    {
        ASSERT(elemSize != 0);
        size_t size = sizeof(Array) + elemSize * length;
#ifdef PANDA_TARGET_32
        // NOLINTNEXTLINE(clang-analyzer-core.DivideZero)
        size_t sizeLimit = (std::numeric_limits<size_t>::max() - sizeof(Array)) / elemSize;
        if (UNLIKELY(sizeLimit < static_cast<size_t>(length))) {
            return 0;
        }
#endif
        return size;
    }

    ArraySizeT GetLength() const
    {
        // Atomic with relaxed order reason: data race with length_ with no synchronization or ordering constraints
        // imposed on other reads or writes
        return length_.load(std::memory_order_relaxed);
    }

    uint32_t *GetData()
    {
        return data_;
    }

    const uint32_t *GetData() const
    {
        return data_;
    }

    template <class T, bool IS_VOLATILE = false>
    T GetPrimitive(size_t offset) const;

    template <class T, bool IS_VOLATILE = false>
    void SetPrimitive(size_t offset, T value);

    template <bool IS_VOLATILE = false, bool NEED_READ_BARRIER = true, bool IS_DYN = false>
    ObjectHeader *GetObject(int offset) const;

    template <bool IS_VOLATILE = false, bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    void SetObject(size_t offset, ObjectHeader *value);

    template <class T>
    T GetPrimitive(size_t offset, std::memory_order memoryOrder) const;

    template <class T>
    void SetPrimitive(size_t offset, T value, std::memory_order memoryOrder);

    template <bool NEED_READ_BARRIER = true, bool IS_DYN = false>
    ObjectHeader *GetObject(size_t offset, std::memory_order memoryOrder) const;

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    void SetObject(size_t offset, ObjectHeader *value, std::memory_order memoryOrder);

    template <typename T>
    bool CompareAndSetPrimitive(size_t offset, T oldValue, T newValue, std::memory_order memoryOrder, bool strong);

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    bool CompareAndSetObject(size_t offset, ObjectHeader *oldValue, ObjectHeader *newValue,
                             std::memory_order memoryOrder, bool strong);

    template <typename T>
    T CompareAndExchangePrimitive(size_t offset, T oldValue, T newValue, std::memory_order memoryOrder, bool strong);

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    ObjectHeader *CompareAndExchangeObject(size_t offset, ObjectHeader *oldValue, ObjectHeader *newValue,
                                           std::memory_order memoryOrder, bool strong);

    template <typename T>
    T GetAndSetPrimitive(size_t offset, T value, std::memory_order memoryOrder);

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    ObjectHeader *GetAndSetObject(size_t offset, ObjectHeader *value, std::memory_order memoryOrder);

    template <typename T>
    T GetAndAddPrimitive(size_t offset, T value, std::memory_order memoryOrder);

    template <typename T>
    T GetAndBitwiseOrPrimitive(size_t offset, T value, std::memory_order memoryOrder);

    template <typename T>
    T GetAndBitwiseAndPrimitive(size_t offset, T value, std::memory_order memoryOrder);

    template <typename T>
    T GetAndBitwiseXorPrimitive(size_t offset, T value, std::memory_order memoryOrder);

    template <class T, bool NEED_WRITE_BARRIER = true, bool IS_DYN = false, bool IS_VOLATILE = false>
    std::enable_if_t<std::is_arithmetic_v<T> || is_object_v<T>, void> Set(ArraySizeT idx, T elem,
                                                                          uint32_t byteOffset = 0);

    template <class T, bool NEED_READ_BARRIER = true, bool IS_DYN = false, bool IS_VOLATILE = false>
    std::enable_if_t<std::is_arithmetic_v<T> || is_object_v<T>, T> Get(ArraySizeT idx, uint32_t byteOffset = 0) const;

    template <class T, bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    std::enable_if_t<std::is_arithmetic_v<T> || is_object_v<T>, std::pair<bool, T>> CompareAndExchange(
        ArraySizeT idx, T oldValue, T newValue, std::memory_order memoryOrder, bool strong, uint32_t byteOffset = 0);

    template <class T, bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    std::enable_if_t<std::is_arithmetic_v<T> || is_object_v<T>, T> Exchange(ArraySizeT idx, T value,
                                                                            std::memory_order memoryOrder,
                                                                            uint32_t byteOffset = 0);

    template <class T>
    std::enable_if_t<std::is_arithmetic_v<T>, T> GetAndAdd(ArraySizeT idx, T value, std::memory_order memoryOrder,
                                                           uint32_t byteOffset = 0);

    template <class T>
    std::enable_if_t<std::is_arithmetic_v<T>, T> GetAndSub(ArraySizeT idx, T value, std::memory_order memoryOrder,
                                                           uint32_t byteOffset = 0);

    template <class T>
    std::enable_if_t<std::is_arithmetic_v<T>, T> GetAndBitwiseOr(ArraySizeT idx, T value, std::memory_order memoryOrder,
                                                                 uint32_t byteOffset = 0);

    template <class T>
    std::enable_if_t<std::is_arithmetic_v<T>, T> GetAndBitwiseAnd(ArraySizeT idx, T value,
                                                                  std::memory_order memoryOrder,
                                                                  uint32_t byteOffset = 0);

    template <class T>
    std::enable_if_t<std::is_arithmetic_v<T>, T> GetAndBitwiseXor(ArraySizeT idx, T value,
                                                                  std::memory_order memoryOrder,
                                                                  uint32_t byteOffset = 0);

    template <class T, bool IS_DYN>
    static constexpr size_t GetElementSize();

    template <class T>
    T GetBase() const;

    // Pass thread parameter to speed up interpreter
    template <class T, bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    void Set([[maybe_unused]] const ManagedThread *thread, ArraySizeT idx, T elem);

    template <class T, bool NEED_READ_BARRIER = true, bool IS_DYN = false>
    T Get([[maybe_unused]] const ManagedThread *thread, ArraySizeT idx) const;

    template <class T, bool NEED_BARRIER = true, bool IS_DYN = false>
    void Fill(T elem, ArraySizeT start, ArraySizeT end);

    size_t ObjectSize(uint32_t componentSize) const
    {
        return ComputeSize(componentSize, length_);
    }

    static constexpr uint32_t GetLengthOffset()
    {
        return MEMBER_OFFSET(Array, length_);
    }

    static constexpr uint32_t GetDataOffset()
    {
        return MEMBER_OFFSET(Array, data_);
    }

    template <bool IS_DYN>
    ArraySizeT GetElementOffset(ArraySizeT idx) const
    {
        size_t elemSize;
        // NOLINTNEXTLINE(readability-braces-around-statements)
        if constexpr (IS_DYN) {
            elemSize = TaggedValue::TaggedTypeSize();
        } else {  // NOLINT(readability-misleading-indentation)
            elemSize = ClassAddr<ark::Class>()->GetComponentSize();
        }
        return GetDataOffset() + idx * elemSize;
    }

    template <class DimIterator>
    static Array *CreateMultiDimensionalArray(ManagedThread *thread, ark::Class *klass, uint32_t nargs,
                                              const DimIterator &iter, size_t dimIdx = 0);

protected:
    void SetLength(ArraySizeT length)
    {
        // Atomic with relaxed order reason: data race with length_ with no synchronization or ordering constraints
        // imposed on other reads or writes
        length_.store(length, std::memory_order_relaxed);
    }

private:
    template <class T>
    void FillPrimitiveElem(T elem, ArraySizeT start, ArraySizeT end, size_t elemSize);

    std::atomic<ArraySizeT> length_;
    // Align by 64bits, because dynamic language data is always 64bits
    __extension__ alignas(sizeof(uint64_t)) uint32_t data_[0];  // NOLINT(modernize-avoid-c-arrays)
};

static_assert(Array::GetLengthOffset() == sizeof(ObjectHeader));
static_assert(Array::GetDataOffset() == AlignUp(Array::GetLengthOffset() + sizeof(ArraySizeT), sizeof(uint64_t)));
static_assert(Array::GetDataOffset() % sizeof(uint64_t) == 0);

constexpr uint32_t ARRAY_LENGTH_OFFSET = sizeof(ObjectHeader);
static_assert(ARRAY_LENGTH_OFFSET == ark::coretypes::Array::GetLengthOffset());
constexpr uint32_t ARRAY_DATA_OFFSET = ARRAY_LENGTH_OFFSET + sizeof(uint64_t);
static_assert(ARRAY_DATA_OFFSET == ark::coretypes::Array::GetDataOffset());

}  // namespace ark::coretypes

#endif  // PANDA_RUNTIME_CORETYPES_ARRAY_H_
