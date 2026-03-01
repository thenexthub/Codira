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

#ifndef PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_BIGINT_H
#define PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_BIGINT_H

#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_array.h"

namespace ark::ets {

namespace test {
class EtsBigIntMembers;
}  // namespace test

class EtsBigInt : public EtsObject {
public:
    static EtsBigInt *FromEtsObject(EtsObject *etsObj)
    {
        ASSERT(etsObj->GetClass()->IsBigInt());
        return reinterpret_cast<EtsBigInt *>(etsObj);
    }

    /* The sign field can have the following values: -1, 0, 1
     * -1 - for negative
     *  0 - for zero value
     *  1 - for positive
     */
    EtsInt GetSign() const
    {
        return GetFieldPrimitive<EtsInt>(GetSignOffset());
    }

    const EtsIntArray *GetBytes() const
    {
        return const_cast<EtsBigInt *>(this)->GetBytes();
    }

    EtsIntArray *GetBytes()
    {
        return reinterpret_cast<EtsIntArray *>(GetFieldObject(GetBytesOffset()));
    }

    static constexpr size_t GetBytesOffset()
    {
        return MEMBER_OFFSET(EtsBigInt, bytes_);
    }

    static constexpr size_t GetSignOffset()
    {
        return MEMBER_OFFSET(EtsBigInt, sign_);
    }

    uint32_t GetHashCode() const
    {
        auto hashCode = static_cast<uint32_t>(GetSign());
        auto *bytes = reinterpret_cast<const EtsIntArray *>(GetBytes());
        if (bytes == nullptr) {
            return hashCode;
        }
        for (size_t i = 0; i < bytes->GetLength(); i++) {
            hashCode = hashCode * HASH_SIGN_SHIFT + static_cast<int32_t>(bytes->Get(i));
        }
        return hashCode;
    }

    EtsBigInt() = delete;
    ~EtsBigInt() = delete;

private:
    NO_COPY_SEMANTIC(EtsBigInt);
    NO_MOVE_SEMANTIC(EtsBigInt);

    static constexpr uint32_t HASH_SIGN_SHIFT = 31;

    ObjectPointer<EtsIntArray> bytes_;
    EtsInt sign_;

    friend class test::EtsBigIntMembers;
};

}  // namespace ark::ets

#endif  // PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_BIGINT_H
