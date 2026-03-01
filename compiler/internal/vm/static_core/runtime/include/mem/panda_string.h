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
#ifndef PANDA_RUNTIME_MEM_PANDA_STRING_H
#define PANDA_RUNTIME_MEM_PANDA_STRING_H

#include <sstream>
#include <string>
#include <string_view>

#include "runtime/mem/allocator_adapter.h"

namespace ark {
namespace coretypes {
class String;
}  // namespace coretypes

using PandaString = std::basic_string<char, std::char_traits<char>, mem::AllocatorAdapter<char>>;
using PandaStringStream = std::basic_stringstream<char, std::char_traits<char>, mem::AllocatorAdapter<char>>;
using PandaIStringStream = std::basic_istringstream<char, std::char_traits<char>, mem::AllocatorAdapter<char>>;
using PandaOStringStream = std::basic_ostringstream<char, std::char_traits<char>, mem::AllocatorAdapter<char>>;

int64_t PandaStringToLL(const PandaString &str);
uint64_t PandaStringToULL(const PandaString &str);
float PandaStringToF(const PandaString &str);
double PandaStringToD(const PandaString &str);
PANDA_PUBLIC_API PandaString ConvertToString(const std::string &str);
PANDA_PUBLIC_API PandaString ConvertToString(coretypes::String *s);

template <class T>
std::enable_if_t<std::is_arithmetic_v<T>, PandaString> ToPandaString(T value)
{
    PandaStringStream strStream;
    strStream << value;
    return strStream.str();
}

struct PandaStringHash {
    using ArgumentType = ark::PandaString;
    using ResultType = std::size_t;

    size_t operator()(const PandaString &str) const noexcept
    {
        return std::hash<std::string_view>()(std::string_view(str.data(), str.size()));
    }
};

inline std::string PandaStringToStd(const PandaString &pandastr)
{
    // NOLINTNEXTLINE(readability-redundant-string-cstr)
    std::string str = pandastr.c_str();
    return str;
}

}  // namespace ark

namespace std {

template <>
struct hash<ark::PandaString> {
    using ArgumentType = ark::PandaStringHash::ArgumentType;
    using ResultType = ark::PandaStringHash::ResultType;

    size_t operator()(const ark::PandaString &str) const noexcept
    {
        return ark::PandaStringHash()(str);
    }
};

}  // namespace std

#endif  // PANDA_RUNTIME_MEM_PANDA_STRING_H
