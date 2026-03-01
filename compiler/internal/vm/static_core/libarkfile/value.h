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

#ifndef LIBPANDAFILE_VALUE_H_
#define LIBPANDAFILE_VALUE_H_

#include <type_traits>

#include "libarkfile/file.h"
#include "libarkfile/helpers.h"

namespace ark::panda_file {

class ScalarValue {
public:
    ScalarValue(const File &pandaFile, uint32_t value) : pandaFile_(pandaFile), value_(value) {}

    ~ScalarValue() = default;

    NO_COPY_SEMANTIC(ScalarValue);
    NO_MOVE_SEMANTIC(ScalarValue);

    template <class T>
    T Get() const
    {
        static_assert(std::is_arithmetic_v<T> || std::is_same_v<T, File::EntityId>);

        if constexpr (std::is_same_v<T, float>) {  // NOLINT
            return bit_cast<float>(value_);
        }

        constexpr size_t T_SIZE = sizeof(T);

        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (T_SIZE <= sizeof(uint32_t)) {
            return static_cast<T>(value_);
        }

        File::EntityId id(value_);
        auto sp = pandaFile_.GetSpanFromId(id);
        auto res = helpers::Read<T_SIZE>(&sp);

        if constexpr (std::is_floating_point_v<T>) {  // NOLINT
            return bit_cast<T>(res);
        }

        return static_cast<T>(res);
    }

    uint32_t GetValue() const
    {
        return value_;
    }

private:
    const File &pandaFile_;
    uint32_t value_;
};

class ArrayValue {
public:
    ArrayValue(const File &pandaFile, File::EntityId id) : pandaFile_(pandaFile), id_(id)
    {
        auto sp = pandaFile_.GetSpanFromId(id_);
        count_ = helpers::ReadULeb128(&sp);
        data_ = sp;
    }

    ~ArrayValue() = default;

    NO_COPY_SEMANTIC(ArrayValue);
    NO_MOVE_SEMANTIC(ArrayValue);

    template <class T>
    T Get(size_t idx) const
    {
        static_assert(std::is_arithmetic_v<T> || std::is_same_v<T, File::EntityId>);

        constexpr size_t T_SIZE = sizeof(T);

        auto sp = data_.SubSpan(T_SIZE * idx);
        auto res = helpers::Read<T_SIZE>(&sp);

        if constexpr (std::is_floating_point_v<T>) {  // NOLINT
            return bit_cast<T>(res);
        }

        return static_cast<T>(res);
    }

    uint32_t GetCount() const
    {
        return count_;
    }

    File::EntityId GetId() const
    {
        return id_;
    }

private:
    static constexpr size_t COUNT_SIZE = sizeof(uint32_t);

    const File &pandaFile_;
    File::EntityId id_;
    uint32_t count_;
    Span<const uint8_t> data_ {nullptr, nullptr};
};

}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_VALUE_H_
