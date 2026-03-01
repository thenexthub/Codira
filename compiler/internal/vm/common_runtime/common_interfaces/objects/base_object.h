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

#ifndef COMMON_INTERFACES_OBJECTS_BASE_OBJECT_H
#define COMMON_INTERFACES_OBJECTS_BASE_OBJECT_H

#include <stdint.h>
#include <stdio.h>

#include "common_interfaces/objects/base_class.h"
#include "common_interfaces/base/common.h"
#include "common_interfaces/objects/base_class.h"
#include "common_interfaces/objects/base_object_operator.h"
#include "common_interfaces/objects/base_state_word.h"
namespace common {
class BaseObject {
public:
    BaseObject() : state_(0) {}
    static BaseObject *Cast(MAddress address)
    {
        return reinterpret_cast<BaseObject *>(address);
    }
    static void RegisterDynamic(BaseObjectOperatorInterfaces *dynamicObjOp);
    static void RegisterStatic(BaseObjectOperatorInterfaces *staticObjOp);

    inline size_t GetSize() const
    {
        if (IsForwarded()) {
            return GetSizeForwarded();
        } else {
            return GetOperator()->GetSize(this);
        }
    }

    inline bool IsValidObject() const
    {
        return GetOperator()->IsValidObject(this);
    }

    void ForEachRefField(const RefFieldVisitor &visitor)
    {
        GetOperator()->ForEachRefField(this, visitor);
    }

    size_t ForEachRefFieldAndGetSize(const RefFieldVisitor &visitor)
    {
        return GetOperator()->ForEachRefFieldAndGetSize(this, visitor);
    }

    inline BaseObject *GetForwardingPointer() const
    {
        if (IsForwarded()) {
            return GetOperator()->GetForwardingPointer(this);
        }
        return nullptr;
    }

    inline void SetForwardingPointerAfterExclusive(BaseObject *fwdPtr)
    {
        CHECK_CC(IsForwarding());
        GetOperator()->SetForwardingPointerAfterExclusive(this, fwdPtr);
        state_.SetForwarded();
    }

    inline void SetSizeForwarded(size_t size)
    {
        // Set size in old obj when forwarded, forwardee obj size may changed.
        size_t objectHeaderSize = sizeof(BaseStateWord);
        DCHECK_CC(size >= objectHeaderSize + sizeof(size_t));
        auto addr = reinterpret_cast<MAddress>(this) + objectHeaderSize;
        *reinterpret_cast<size_t*>(addr) = size;
    }

    inline size_t GetSizeForwarded() const
    {
        auto addr = reinterpret_cast<MAddress>(this) + sizeof(BaseStateWord);
        return *reinterpret_cast<size_t*>(addr);
    }

    inline bool IsForwarding() const
    {
        return state_.IsForwarding();
    }

    inline bool IsForwarded() const
    {
        return state_.IsForwarded();
    }

    ALWAYS_INLINE_CC bool IsDynamic() const
    {
        return state_.IsDynamic();
    }

    ALWAYS_INLINE_CC bool IsStatic() const
    {
        return state_.IsStatic();
    }

    inline bool IsToVersion() const
    {
        return state_.IsToVersion();
    }

    inline void SetLanguageType(LanguageType language)
    {
        state_.SetLanguageType(language);
    }

    BaseStateWord GetBaseStateWord() const
    {
        return state_.AtomicGetBaseStateWord();
    }

    void SetForwardState(BaseStateWord::ForwardState state)
    {
        state_.SetForwardState(state);
    }

    template <typename T>
    Field<T> &GetField(uint32_t offset) const
    {
        auto addr = reinterpret_cast<MAddress>(this) + offset;
        return *reinterpret_cast<Field<T> *>(addr);
    }

    // Locking means that this object forwardstate is forwarding.
    // Any other gc coping thread or mutator thread will be wait if copy the same object.
    bool TryLockExclusive(const BaseStateWord state)
    {
        return state_.TryLockBaseStateWord(state);
    }

    void UnlockExclusive(const BaseStateWord::ForwardState newState)
    {
        state_.UnlockStateWord(newState);
    }

    static inline intptr_t FieldOffset(BaseObject *object, const void *field)
    {
        return reinterpret_cast<intptr_t>(field) - reinterpret_cast<intptr_t>(object);
    }

    // The interfaces following only use for common code compiler. It will be deleted later.
    TypeInfo *GetComponentTypeInfo() const
    {
        return reinterpret_cast<TypeInfo *>(const_cast<BaseObject *>(this));
    }

    inline bool HasRefField() const
    {
        return true;
    }

    inline TypeInfo *GetTypeInfo() const
    {
        return reinterpret_cast<TypeInfo *>(const_cast<BaseObject *>(this));
    }

    bool IsRawArray() const
    {
        return true;
    }

    void ForEachRefInStruct([[maybe_unused]] const RefFieldVisitor &visitor, [[maybe_unused]] MAddress aggStart,
                            [[maybe_unused]] MAddress aggEnd)
    {
    }
    // The interfaces above only use for common code compiler. It will be deleted later.

    void SetFullBaseClassWithoutBarrier(BaseClass* cls)
    {
        state_ = 0;
        state_.SetFullBaseClassAddress(reinterpret_cast<common::StateWordType>(cls));
    }

    BaseClass *GetBaseClass() const
    {
        return reinterpret_cast<BaseClass *>(state_.GetBaseClassAddress());
    }

    // Size of object header
    static constexpr size_t BaseObjectSize()
    {
        return sizeof(BaseObject);
    }
protected:
    inline BaseObjectOperatorInterfaces *GetOperator() const
    {
        if (state_.IsStatic()) {
            return operator_.staticObjOp_;
        }
        return operator_.dynamicObjOp_;
    }

    static PUBLIC_API BaseObjectOperator operator_;
    BaseStateWord state_;
};

static_assert(sizeof(BaseObject) == sizeof(BaseClass::HeaderType));
}  // namespace common
#endif  // COMMON_INTERFACES_OBJECTS_BASE_OBJECT_H
// NOLINTEND(readability-identifier-naming, cppcoreguidelines-macro-usage,
//           cppcoreguidelines-special-member-functions, modernize-deprecated-headers,
//           readability-else-after-return, readability-duplicate-include,
//           misc-non-private-member-variables-in-classes, cppcoreguidelines-pro-type-member-init,
//           google-explicit-constructor, cppcoreguidelines-pro-type-union-access,
//           modernize-use-auto, llvm-namespace-comment,
//           cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays,
//           readability-implicit-bool-conversion)

