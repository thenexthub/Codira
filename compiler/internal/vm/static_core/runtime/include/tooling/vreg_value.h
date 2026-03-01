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

#ifndef PANDA_RUNTIME_INCLUDE_TOOLING_VREG_VALUE_H
#define PANDA_RUNTIME_INCLUDE_TOOLING_VREG_VALUE_H

#include <cstdint>

#include "libarkbase/macros.h"
#include "include/coretypes/tagged_value.h"
#include "include/typed_value.h"

namespace ark::tooling {
class VRegValue {
public:
    constexpr explicit VRegValue(int64_t value = 0) : value_(value) {}

    constexpr int64_t GetValue() const
    {
        return value_;
    }

    constexpr void SetValue(int64_t value)
    {
        value_ = value;
    }

    TypedValue ToTypedValue(panda_file::Type::TypeId typeId)
    {
        switch (typeId) {
            case panda_file::Type::TypeId::INVALID:
                return TypedValue::Invalid();
            case panda_file::Type::TypeId::VOID:
                return TypedValue::Void();
            case panda_file::Type::TypeId::U1:
                return TypedValue::U1(static_cast<bool>(GetValue()));
            case panda_file::Type::TypeId::I8:
                return TypedValue::I8(static_cast<int8_t>(GetValue()));
            case panda_file::Type::TypeId::U8:
                return TypedValue::U8(static_cast<uint8_t>(GetValue()));
            case panda_file::Type::TypeId::I16:
                return TypedValue::I16(static_cast<int16_t>(GetValue()));
            case panda_file::Type::TypeId::U16:
                return TypedValue::U16(static_cast<uint16_t>(GetValue()));
            case panda_file::Type::TypeId::I32:
                return TypedValue::I32(static_cast<int32_t>(GetValue()));
            case panda_file::Type::TypeId::U32:
                return TypedValue::U32(static_cast<uint32_t>(GetValue()));
            case panda_file::Type::TypeId::F32:
                return TypedValue::F32(bit_cast<float>(static_cast<uint32_t>(GetValue())));
            case panda_file::Type::TypeId::F64:
                return TypedValue::F64(bit_cast<double>(static_cast<uint64_t>(GetValue())));
            case panda_file::Type::TypeId::I64:
                return TypedValue::I64(static_cast<int64_t>(GetValue()));
            case panda_file::Type::TypeId::U64:
                return TypedValue::U64(static_cast<uint64_t>(GetValue()));
            case panda_file::Type::TypeId::REFERENCE:
                return TypedValue::Reference(reinterpret_cast<ObjectHeader *>(GetValue()));
            case panda_file::Type::TypeId::TAGGED:
                return TypedValue::Tagged(coretypes::TaggedValue(GetValue()));
        }
        UNREACHABLE();
    }

    ~VRegValue() = default;

    DEFAULT_COPY_SEMANTIC(VRegValue);
    DEFAULT_MOVE_SEMANTIC(VRegValue);

private:
    int64_t value_;
};
}  // namespace ark::tooling

#endif  // PANDA_RUNTIME_INCLUDE_TOOLING_VREG_VALUE_H
