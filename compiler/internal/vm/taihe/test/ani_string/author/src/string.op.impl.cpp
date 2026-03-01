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
#include "taihe/string.hpp"

#include "stdexcept"
#include "string_op.PlayString.proj.2.hpp"
#include "string_op.StringPair.proj.1.hpp"
#include "string_op.impl.hpp"
#include "taihe/array.hpp"
// Please delete <stdexcept> include when you implement
using namespace taihe;

namespace {
class PlayString {
    string name_ {"PlayString"};

public:
    string pickString(array_view<string> nums, int32_t n1, int32_t n2)
    {
        int32_t size = static_cast<int32_t>(nums.size());
        if (n1 > n2 || n1 < 0 || n2 >= size) {
            throw std::runtime_error("index error");
        }
        string res {};
        for (int32_t i = n1; i <= n2; i++) {
            res = res + nums[i];
        }
        return res;
    }

    string getName()
    {
        return name_;
    }

    void setName(string_view name)
    {
        name_ = name;
    }
};

string concatString(string_view a, string_view b)
{
    return a + b;
}

string makeString(string_view a, int32_t b)
{
    string result = "";
    while (b-- > 0) {
        result = result + a;
    }
    return result;
}

::string_op::StringPair split(string_view a, int32_t n)
{
    int32_t l = a.size();
    if (n > l) {
        n = l;
    } else if (n + l < 0) {
        n = 0;
    } else if (n < 0) {
        n = n + l;
    }
    return {
        a.substr(0, n),
        a.substr(n, l - n),
    };
}

array<string> split2(string_view a, int32_t n)
{
    int32_t l = a.size();
    if (n > l) {
        n = l;
    } else if (n + l < 0) {
        n = 0;
    } else if (n < 0) {
        n = n + l;
    }
    return {a.substr(0, n), a.substr(n, l - n)};
}

int32_t to_i32(string_view a)
{
    return std::atoi(a.c_str());
}

string from_i32(int32_t a)
{
    return to_string(a);
}

::string_op::PlayString makePlayStringIface()
{
    return make_holder<PlayString, ::string_op::PlayString>();
}

float to_f32(string_view a)
{
    return std::atof(a.c_str());
}

string from_f32(float a)
{
    return to_string(a);
}

string concatString2(string_view s, int32_t n, array_view<string> sArr, bool b, array_view<uint8_t> buffer)
{
    string result = "";
    for (auto i = 0; i < n; i++) {
        result = result + s;
    }
    if (b) {
        for (auto c : sArr) {
            result = result + c;
        }
        for (auto j : buffer) {
            result = result + to_string(j);
        }
    }
    return result;
}
}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_concatString(concatString);
TH_EXPORT_CPP_API_makeString(makeString);
TH_EXPORT_CPP_API_split(split);
TH_EXPORT_CPP_API_split2(split2);
TH_EXPORT_CPP_API_to_i32(to_i32);
TH_EXPORT_CPP_API_from_i32(from_i32);
TH_EXPORT_CPP_API_makePlayStringIface(makePlayStringIface);
TH_EXPORT_CPP_API_to_f32(to_f32);
TH_EXPORT_CPP_API_from_f32(from_f32);
TH_EXPORT_CPP_API_concatString2(concatString2);
// NOLINTEND