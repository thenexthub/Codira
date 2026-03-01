/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_VALUE_H_
#define PANDA_RUNTIME_VALUE_H_

#include <cstdarg>
#include <cstdint>
#include <type_traits>
#include <variant>

#include "libarkfile/file_items.h"
#include "runtime/bridge/bridge.h"
#include "runtime/include/coretypes/tagged_value.h"

namespace ark {

class ObjectHeader;

class Value {
public:
    explicit Value() : value_(0) {}
    DEFAULT_COPY_SEMANTIC(Value);
    DEFAULT_MOVE_SEMANTIC(Value);
    ~Value() = default;

    template <class T>
    explicit Value(T value)
    {
        // Disable checks due to clang-tidy bug https://bugs.llvm.org/show_bug.cgi?id=32203
        // NOLINTNEXTLINE(readability-braces-around-statements, hicpp-braces-around-statements, bugprone-branch-clone)
        if constexpr (std::is_integral_v<T>) {
            value_ = static_cast<int64_t>(value);
            // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
        } else if constexpr (std::is_same_v<T, double>) {
            value_ = bit_cast<int64_t>(value);
            // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
        } else if constexpr (std::is_same_v<T, float>) {
            value_ = bit_cast<int32_t>(value);
        } else if constexpr (std::is_pointer_v<T> && std::is_base_of_v<ObjectHeader, std::remove_pointer_t<T>>) {
            value_ = static_cast<ObjectHeader *>(value);
        } else {  // NOLINTNEXTLINE(readability-misleading-indentation)
            value_ = value;
        }
    }

    template <class T>
    T GetAs() const
    {
        static_assert(std::is_integral_v<T>, "T must be integral type");
        ASSERT(IsPrimitive());
        if constexpr (std::is_same_v<T, bool>) {
            // Discard the top 32 bit which may contain garbage
            return static_cast<bool>(static_cast<int32_t>(std::get<0>(value_)));
        }
        return static_cast<T>(std::get<0>(value_));
    }

    int64_t GetAsLong() const;

    bool IsReference() const
    {
        return std::holds_alternative<ObjectHeader *>(value_);
    }

    bool IsPrimitive() const
    {
        return std::holds_alternative<int64_t>(value_);
    }

    ObjectHeader **GetGCRoot()
    {
        ASSERT(IsReference());
        return &std::get<ObjectHeader *>(value_);
    }

    template <class VRegisterRef>
    static ALWAYS_INLINE Value FromVReg(VRegisterRef vreg)
    {
        if (vreg.HasObject()) {
            return Value(vreg.GetReference());
        }
        return Value(vreg.GetValue());
    }

private:
    std::variant<int64_t, ObjectHeader *> value_;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_VALUE_H_
