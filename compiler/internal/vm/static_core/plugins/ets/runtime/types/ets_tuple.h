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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_TUPLE_H_
#define PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_TUPLE_H_

#include <cstddef>
#include <cstdint>
#include <memory>

#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets {

template <std::size_t N>
class EtsTuple : private EtsObject {
public:
    static EtsTuple *FromEtsObject(EtsObject *etsObject)
    {
        return reinterpret_cast<EtsTuple *>(etsObject);
    }

    static constexpr size_t GetClassSize()
    {
        return sizeof(EtsTuple);
    }

    EtsObject *GetElement(size_t index) const
    {
        ASSERT(index < N);

        auto coro = EtsCoroutine::GetCurrent();
        auto res = ObjectAccessor::GetObject(coro, this, this->GetElementOffset(index));

        return EtsObject::FromCoreType(res);
    }

    void SetElement(size_t index, EtsObject *element)
    {
        ASSERT(index < N);

        auto coro = EtsCoroutine::GetCurrent();
        ObjectAccessor::SetObject(coro, this, this->GetElementOffset(index), element->GetCoreType());
    }

    size_t GetSize() const
    {
        return N;
    }

    /**
     * @brief GetRuntimeOffset
     *
     * @param objectPtr Pointer to the object containing the member.
     * @param objectMemberPtr Pointer to the member within the object.
     * @return size_t The offset of the member in bytes.
     */
    size_t GetElementOffset(size_t index) const
    {
        auto elementPtr = &(this->elements_[index]);
        size_t offset = reinterpret_cast<uintptr_t>(elementPtr) - reinterpret_cast<uintptr_t>(this);

        return offset;
    }

private:
    ObjectPointer<EtsObject> elements_[N];  // NOLINT(modernize-avoid-c-arrays)
};

class EtsTupleAccessorBase {
public:
    EtsObject *GetElement(size_t index) const
    {
        return (this->*getElementImpl_)(index);
    }

    void SetElement(size_t index, EtsObject *element)
    {
        return (this->*setElementT_)(index, element);
    }

    size_t GetSize() const
    {
        return (this->*getSizeImpl_)();
    }

    template <typename Derived, typename = std::enable_if_t<std::is_base_of_v<EtsTupleAccessorBase, Derived>>>
    explicit EtsTupleAccessorBase(Derived * /*unused*/)
        : getElementImpl_(static_cast<GetElementT>(&Derived::GetElement)),
          setElementT_(static_cast<SetElementT>(&Derived::SetElement)),
          getSizeImpl_(static_cast<GetSizeT>(&Derived::GetSize))
    {
    }

private:
    using GetElementT = decltype(&EtsTupleAccessorBase::GetElement);
    using SetElementT = decltype(&EtsTupleAccessorBase::SetElement);
    using GetSizeT = decltype(&EtsTupleAccessorBase::GetSize);

    const GetElementT getElementImpl_;
    const SetElementT setElementT_;
    const GetSizeT getSizeImpl_;
};

template <std::size_t N>
class EtsTupleAccessor : public EtsTupleAccessorBase {
public:
    explicit EtsTupleAccessor<N>(EtsObject *etsObject)
        : EtsTupleAccessorBase(this), etsTuple_(EtsTuple<N>::FromEtsObject(etsObject))
    {
    }

    EtsObject *GetElement(size_t index) const
    {
        return this->etsTuple_->GetElement(index);
    }

    void SetElement(size_t index, EtsObject *element)
    {
        return this->etsTuple_->SetElement(index, element);
    }

    size_t GetSize() const
    {
        return this->etsTuple_->GetSize();
    }

private:
    EtsTuple<N> *etsTuple_;
};

template <std::size_t... TUPLE_ELEMENTS_CNT>
static std::unique_ptr<EtsTupleAccessorBase> EtsTupleAccessorFromEtsObjectImpl(
    EtsObject *etsObject, size_t elementCount, std::index_sequence<TUPLE_ELEMENTS_CNT...> /*unused*/)
{
    /*
     * This function is used to create an EtsTupleAccessor based on the element count.
     * It uses a fold expression to handle Tuple0, Tuple1, ..., Tuple16.
     * After unfolding, it will act like this:
     *  switch (elementCount) {
     *      case 0:
     *          UNREACHABLE();
     *      case 1:
     *          return std::make_unique<EtsTupleAccessor<1>>(etsObject);
     *      case 2:
     *          return std::make_unique<EtsTupleAccessor<2>>(etsObject);
     *      // case 3 to case 16: ...
     *  }
     */
    std::unique_ptr<EtsTupleAccessorBase> result = nullptr;
    (
        [&]() {
            if constexpr (TUPLE_ELEMENTS_CNT == 0) {
                if (elementCount == 0) {
                    // Tuple0 is handled formerly
                    // So cannot reach here
                    UNREACHABLE();
                }
            } else {
                if (elementCount == TUPLE_ELEMENTS_CNT) {
                    // Return the corresponding EtsTupleAccessor
                    result = std::make_unique<EtsTupleAccessor<TUPLE_ELEMENTS_CNT>>(etsObject);
                }
            }
        }(),
        ...);
    return result;
}

inline std::unique_ptr<EtsTupleAccessorBase> EtsTupleAccessorFromEtsObject(EtsObject *etsObject, size_t elementCount)
{
    ASSERT(etsObject != nullptr);
    // NOLINTNEXTLINE(readability-magic-numbers)
    ASSERT(elementCount > 0);  // Tuple0 cannot go to here
    // NOLINTNEXTLINE(readability-magic-numbers)
    ASSERT(elementCount <= 16);  // Tuple16 is the maximum supported tuple size

    constexpr size_t MAX_TUPLE_SIZE = 16;
    return EtsTupleAccessorFromEtsObjectImpl(etsObject, elementCount,
                                             std::make_index_sequence<MAX_TUPLE_SIZE + 1>());  // 0 to 16
}

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_TUPLE_H_
