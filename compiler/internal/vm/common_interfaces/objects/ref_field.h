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

// NOLINTBEGIN(readability-identifier-naming, cppcoreguidelines-macro-usage,
//             cppcoreguidelines-special-member-functions, modernize-deprecated-headers,
//             readability-else-after-return, readability-duplicate-include,
//             misc-non-private-member-variables-in-classes, cppcoreguidelines-pro-type-member-init,
//             google-explicit-constructor, cppcoreguidelines-pro-type-union-access,
//             modernize-use-auto, llvm-namespace-comment,
//             cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays,
//             readability-implicit-bool-conversion)

#ifndef COMMON_INTERFACES_OBJECTS_REF_FIELD_H
#define COMMON_INTERFACES_OBJECTS_REF_FIELD_H

#include <atomic>
#include <functional>
#include "objects/base_state_word.h"
namespace common {
class BaseObject;

template <bool isAtomic = false>
class RefField {
public:
    static constexpr uint64_t TAG_WEAK = 0x01ULL;
    static constexpr MAddress REF_UNDEFINED = 0x02ULL;

    struct BitFieldLayout {
        MAddress address : 48;
        MAddress isTagged : 1;
        MAddress tagID : 1;
        MAddress padding : 14;
    };

    // size in bytes
    static constexpr size_t GetSize()
    {
        return sizeof(RefFieldValue);
    }

    BaseObject *GetTargetObject(std::memory_order order = std::memory_order_relaxed) const
    {
        if (isAtomic) {
            MAddress value = __atomic_load_n(&fieldVal, order);
            return reinterpret_cast<BaseObject *>(RefField<>(value).GetAddress() & (~TAG_WEAK));
        } else {
            return reinterpret_cast<BaseObject *>(this->GetAddress() & (~TAG_WEAK));
        }
    }

    MAddress GetFieldValue(std::memory_order order = std::memory_order_relaxed) const
    {
        if (isAtomic) {
            MAddress value = __atomic_load_n(&fieldVal, order);
            return value;
        } else {
            return fieldVal;
        }
    }

    void SetTargetObject(const BaseObject *obj, std::memory_order order = std::memory_order_relaxed)
    {
        RefField<> newField(obj);
        MAddress newVal = newField.GetFieldValue();
        RefFieldValue oldVal = fieldVal;
        (void)oldVal;

        if (isAtomic) {
            __atomic_store_n(&fieldVal, static_cast<RefFieldValue>(newVal), order);
        } else {
            fieldVal = static_cast<RefFieldValue>(newVal);
        }
    }

    void SetFieldValue(MAddress newVal, std::memory_order order = std::memory_order_relaxed)
    {
        RefFieldValue oldVal = fieldVal;
        (void)oldVal;

        if (isAtomic) {
            __atomic_store_n(&fieldVal, static_cast<RefFieldValue>(newVal), order);
        } else {
            fieldVal = static_cast<RefFieldValue>(newVal);
        }
    }

    bool ClearRef(MAddress expectedValue)
    {
        return CompareExchange(expectedValue, REF_UNDEFINED);
    }

    bool CompareExchange(MAddress expectedValue, MAddress newValue,
                         std::memory_order succOrder = std::memory_order_relaxed,
                         std::memory_order failOrder = std::memory_order_relaxed)
    {
        return __atomic_compare_exchange(&fieldVal, &expectedValue, &newValue, false, succOrder, failOrder);
    }

    bool CompareExchange(const BaseObject *expectedObj, const BaseObject *newObj,
                         std::memory_order succOrder = std::memory_order_relaxed,
                         std::memory_order failOrder = std::memory_order_relaxed)
    {
        return CompareExchange(reinterpret_cast<MAddress>(expectedObj), reinterpret_cast<MAddress>(newObj), succOrder,
                               failOrder);
    }

    MAddress Exchange(MAddress newRef, std::memory_order order = std::memory_order_relaxed)
    {
        MAddress ret = 0;
        __atomic_exchange(&fieldVal, &newRef, &ret, order);
        return static_cast<MAddress>(ret);
    }

    MAddress Exchange(const BaseObject *obj, std::memory_order order = std::memory_order_relaxed)
    {
        return Exchange(reinterpret_cast<MAddress>(obj), order);
    }

    MAddress GetAddress() const
    {
        return layout.address;
    }

    bool IsWeak() const
    {
        return (layout.address & TAG_WEAK);
    }

    bool IsTagged() const
    {
        return false;
    }

    uint16_t GetTagID() const
    {
        return layout.tagID;
    }

    ~RefField() = default;
    explicit RefField(MAddress val) : fieldVal(val) {}
    RefField(const RefField &ref) : fieldVal(ref.fieldVal) {}
    explicit RefField(const BaseObject *obj) : fieldVal(0)
    {
        layout.address = reinterpret_cast<MAddress>(obj);
    }
    RefField(const BaseObject *obj, bool forWeak) : fieldVal(0)
    {
        MAddress tag = forWeak ? TAG_WEAK : 0;
        layout.address = reinterpret_cast<MAddress>(obj) | tag;
    }
    RefField(const BaseObject *obj, uint16_t tagged, uint16_t tagid) : fieldVal(0)
    {
        layout.address = reinterpret_cast<MAddress>(obj);
        layout.isTagged = tagged;
        layout.tagID = tagid;
        layout.padding = 0;
    }

    RefField(RefField &&ref) : fieldVal(ref.fieldVal) {}
    RefField() = delete;
    RefField &operator=(const RefField &) = delete;
    RefField &operator=(const RefField &&) = delete;

private:
    using RefFieldValue = MAddress;

    union {
        BitFieldLayout layout;
        RefFieldValue fieldVal;
    };
};
}  // namespace common
#endif  // COMMON_INTERFACES_OBJECTS_REF_FIELD_H
// NOLINTEND(readability-identifier-naming, cppcoreguidelines-macro-usage,
//           cppcoreguidelines-special-member-functions, modernize-deprecated-headers,
//           readability-else-after-return, readability-duplicate-include,
//           misc-non-private-member-variables-in-classes, cppcoreguidelines-pro-type-member-init,
//           google-explicit-constructor, cppcoreguidelines-pro-type-union-access,
//           modernize-use-auto, llvm-namespace-comment,
//           cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays,
//           readability-implicit-bool-conversion)
