/*
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

#ifndef PANDA_LIBPANDABASE_UTILS_JSON_BUILDER_H
#define PANDA_LIBPANDABASE_UTILS_JSON_BUILDER_H

#include <cmath>
#include <cstddef>
#include <functional>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include "libarkbase/macros.h"

WEAK_FOR_LTO_START

namespace ark {
class JsonArrayBuilder;
class JsonObjectBuilder;
PANDA_PUBLIC_API void JsonEscape(std::ostream & /* os */, std::string_view /* string */);

template <char START_DELIMITER, char END_DELIMITER>
class JsonBuilderBase {
public:
    JsonBuilderBase()
    {
        ss_.setf(std::stringstream::boolalpha);
        ss_ << START_DELIMITER;
    }

    std::string Build() &&
    {
        ss_ << END_DELIMITER;
        return ss_.str();
    }

protected:
    void Entry()
    {
        if (firstEntry_) {
            firstEntry_ = false;
        } else {
            ss_ << ',';
        }
    }

    template <typename T>
    void Append(T &&value)
    {
        ss_ << (std::forward<T>(value));
    }

    void Stringify(std::nullptr_t)
    {
        ss_ << "null";
    }

    template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
    void Stringify(T integral)
    {
        ss_ << integral;
    }

    template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    void Stringify(T number)
    {
        if (std::isfinite(number)) {
            ss_ << number;
        } else {
            ss_ << "null";
        }
    }

    void Stringify(std::string_view string)
    {
        JsonEscape(ss_, string);
    }

    void Stringify(const char *string)
    {
        JsonEscape(ss_, string);
    }

    template <typename T, std::enable_if_t<std::is_invocable_v<T, JsonArrayBuilder &>, int> = 0>
    void Stringify(T &&array);

    template <typename T, std::enable_if_t<std::is_invocable_v<T, JsonObjectBuilder &>, int> = 0>
    void Stringify(T &&object);

private:
    std::stringstream ss_;
    bool firstEntry_ {true};
};

class JsonArrayBuilder : public JsonBuilderBase<'[', ']'> {
public:
    template <typename T>
    JsonArrayBuilder &Add(T &&value) &
    {
        Entry();
        Stringify(std::forward<T>(value));
        return *this;
    }

    template <typename T>
    JsonArrayBuilder &&Add(T &&value) &&
    {
        Add(std::forward<T>(value));
        return std::move(*this);
    }
};

// Trick CodeChecker (G.FMT.03).
using JsonObjectBuilderBase = JsonBuilderBase<'{', '}'>;

class JsonObjectBuilder : public JsonObjectBuilderBase {
public:
    template <typename T>
    JsonObjectBuilder &AddProperty(std::string_view key, T &&value) &
    {
        Entry();
        Stringify(key);
        Append(":");
        Stringify(std::forward<T>(value));
        return *this;
    }

    template <typename T>
    JsonObjectBuilder &&AddProperty(std::string_view key, T &&value) &&
    {
        AddProperty(key, std::forward<T>(value));
        return std::move(*this);
    }
};

template <char START_DELIMITER, char END_DELIMITER>
template <typename T, std::enable_if_t<std::is_invocable_v<T, JsonArrayBuilder &>, int>>
void JsonBuilderBase<START_DELIMITER, END_DELIMITER>::Stringify(T &&array)
{
    JsonArrayBuilder builder;
    std::invoke(std::forward<T>(array), builder);
    ss_ << std::move(builder).Build();
}

template <char START_DELIMITER, char END_DELIMITER>
template <typename T, std::enable_if_t<std::is_invocable_v<T, JsonObjectBuilder &>, int>>
void JsonBuilderBase<START_DELIMITER, END_DELIMITER>::Stringify(T &&object)
{
    JsonObjectBuilder builder;
    std::invoke(std::forward<T>(object), builder);
    ss_ << std::move(builder).Build();
}
}  // namespace ark

WEAK_FOR_LTO_END

#endif  // PANDA_LIBPANDABASE_UTILS_JSON_BUILDER_H
