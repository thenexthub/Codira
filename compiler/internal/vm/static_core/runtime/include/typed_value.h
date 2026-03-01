/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_TYPED_VALUE_H
#define PANDA_RUNTIME_TYPED_VALUE_H

#include <cstdint>
#include <string>
#include <variant>

#include "libarkbase/macros.h"
#include "libarkfile/file_items.h"
#include "runtime/include/coretypes/tagged_value.h"

namespace ark {
class ObjectHeader;

class TypedValue {
    using Value = std::variant<std::monostate, std::monostate, bool, int8_t, uint8_t, int16_t, uint16_t, int32_t,
                               uint32_t, float, double, int64_t, uint64_t, coretypes::TaggedValue, ObjectHeader *>;

public:
    ~TypedValue() = default;

    static TypedValue Invalid()
    {
        return TypedValue {Value(std::in_place_index<0>)};
    }

    static TypedValue Void()
    {
        return TypedValue {Value(std::in_place_index<1>)};
    }

    static TypedValue U1(bool value)
    {
        return TypedValue {value};
    }

    static TypedValue I8(int8_t value)
    {
        return TypedValue {value};
    }

    static TypedValue U8(uint8_t value)
    {
        return TypedValue {value};
    }

    static TypedValue I16(int16_t value)
    {
        return TypedValue {value};
    }

    static TypedValue U16(uint16_t value)
    {
        return TypedValue {value};
    }

    static TypedValue I32(int32_t value)
    {
        return TypedValue {value};
    }

    static TypedValue U32(uint32_t value)
    {
        return TypedValue {value};
    }

    static TypedValue F32(float value)
    {
        return TypedValue {value};
    }

    static TypedValue F64(double value)
    {
        return TypedValue {value};
    }

    static TypedValue I64(int64_t value)
    {
        return TypedValue {value};
    }

    static TypedValue U64(uint64_t value)
    {
        return TypedValue {value};
    }

    static TypedValue Reference(ObjectHeader *value)
    {
        return TypedValue {value};
    }

    static TypedValue Tagged(coretypes::TaggedValue value)
    {
        return TypedValue {value};
    }

    DEFAULT_COPY_SEMANTIC(TypedValue);
    DEFAULT_MOVE_SEMANTIC(TypedValue);

    panda_file::Type::TypeId GetType() const
    {
        return static_cast<panda_file::Type::TypeId>(value_.index());
    }

    bool IsInvalid() const
    {
        return GetType() == panda_file::Type::TypeId::INVALID;
    }

    bool IsVoid() const
    {
        return GetType() == panda_file::Type::TypeId::VOID;
    }

    bool IsU1() const
    {
        return GetType() == panda_file::Type::TypeId::U1;
    }

    bool IsI8() const
    {
        return GetType() == panda_file::Type::TypeId::I8;
    }

    bool IsU8() const
    {
        return GetType() == panda_file::Type::TypeId::U8;
    }

    bool IsI16() const
    {
        return GetType() == panda_file::Type::TypeId::I16;
    }

    bool IsU16() const
    {
        return GetType() == panda_file::Type::TypeId::U16;
    }

    bool IsI32() const
    {
        return GetType() == panda_file::Type::TypeId::I32;
    }

    bool IsU32() const
    {
        return GetType() == panda_file::Type::TypeId::U32;
    }

    bool IsF32() const
    {
        return GetType() == panda_file::Type::TypeId::F32;
    }

    bool IsF64() const
    {
        return GetType() == panda_file::Type::TypeId::F64;
    }

    bool IsI64() const
    {
        return GetType() == panda_file::Type::TypeId::I64;
    }

    bool IsU64() const
    {
        return GetType() == panda_file::Type::TypeId::U64;
    }

    bool IsReference() const
    {
        return GetType() == panda_file::Type::TypeId::REFERENCE;
    }

    bool IsTagged() const
    {
        return GetType() == panda_file::Type::TypeId::TAGGED;
    }

    bool GetAsU1() const
    {
        ASSERT(IsU1());
        return std::get<bool>(value_);
    }

    int8_t GetAsI8() const
    {
        ASSERT(IsI8());
        return std::get<int8_t>(value_);
    }

    uint8_t GetAsU8() const
    {
        ASSERT(IsU8());
        return std::get<uint8_t>(value_);
    }

    int16_t GetAsI16() const
    {
        ASSERT(IsI16());
        return std::get<int16_t>(value_);
    }

    uint16_t GetAsU16() const
    {
        ASSERT(IsU16());
        return std::get<uint16_t>(value_);
    }

    int32_t GetAsI32() const
    {
        ASSERT(IsI32());
        return std::get<int32_t>(value_);
    }

    uint32_t GetAsU32() const
    {
        ASSERT(IsU32());
        return std::get<uint32_t>(value_);
    }

    float GetAsF32() const
    {
        ASSERT(IsF32());
        return std::get<float>(value_);
    }

    double GetAsF64() const
    {
        ASSERT(IsF64());
        return std::get<double>(value_);
    }

    int64_t GetAsI64() const
    {
        ASSERT(IsI64());
        return std::get<int64_t>(value_);
    }

    uint64_t GetAsU64() const
    {
        ASSERT(IsU64());
        return std::get<uint64_t>(value_);
    }

    ObjectHeader *GetAsReference() const
    {
        ASSERT(IsReference());
        return std::get<ObjectHeader *>(value_);
    }

    coretypes::TaggedValue GetAsTagged() const
    {
        ASSERT(IsTagged());
        return std::get<coretypes::TaggedValue>(value_);
    }

private:
    explicit TypedValue(Value value) : value_(value) {}

    Value value_;
};
}  // namespace ark

#endif  // PANDA_RUNTIME_TYPED_VALUE_H
