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
#include "opt.impl.hpp"
#include <iostream>
#include "opt.ReturnResult.proj.1.hpp"
#include "taihe/array.hpp"
#include "taihe/optional.hpp"
#include "taihe/string.hpp"

using namespace taihe;

namespace {

class TestImpl {
public:
    string str = "this is str";
    optional<string> a_;

    TestImpl() {}

    void SetMemberStr(::taihe::optional_view<::taihe::string> a)
    {
        this->a_ = a;
    }

    ::taihe::optional<::taihe::string> GetMemberStr()
    {
        return a_;
    }

    void SetIntData(::taihe::string_view a)
    {
        this->str = a;
    }

    ::taihe::optional<::taihe::string> ShowOptionalString(::taihe::optional_view<::taihe::string> a)
    {
        if (a) {
            return a;
        } else {
            return optional<string>(nullptr);
        }
    }

    ::taihe::optional<int32_t> ShowOptionalInt32(::taihe::optional_view<int32_t> a)
    {
        if (a) {
            return a;
        } else {
            return optional<int32_t>(nullptr);
        }
    }

    ::taihe::optional<bool> ShowOptionalBool(::taihe::optional_view<bool> a)
    {
        if (a) {
            return a;
        } else {
            return optional<bool>(nullptr);
        }
    }

    ::taihe::optional<::taihe::map<::taihe::string, bool>> ShowOptionalRecord(
        ::taihe::optional_view<::taihe::map<::taihe::string, bool>> a)
    {
        if (a) {
            return a;
        } else {
            return optional<map<string, bool>>(nullptr);
        }
    }

    ::taihe::optional<::opt::MyStruct> ShowOptionalStruct(::taihe::optional_view<::opt::MyStruct> a)
    {
        if (a) {
            return a;
        } else {
            return optional<::opt::MyStruct>(nullptr);
        }
    }
};

void ShowOptionalInt(optional_view<int32_t> x)
{
    if (x) {
        std::cout << *x << std::endl;
    } else {
        std::cout << "Null" << std::endl;
    }
}

optional<int32_t> MakeOptionalInt(bool b)
{
    if (b) {
        int const optionalMakeValue = 10;
        return optional<int32_t>::make(optionalMakeValue);
    } else {
        return optional<int32_t>(nullptr);
    }
}

optional<array<int32_t>> MakeOptionalArray(bool b, int32_t val, int32_t num)
{
    if (b) {
        return optional<array<int32_t>>::make(array<int32_t>::make(num, val));
    } else {
        return optional<array<int32_t>>(nullptr);
    }
}

optional<string> SendReturnResult(::opt::ReturnResult const &result)
{
    if (result.results) {
        string ret = "";
        for (auto str : *result.results) {
            ret = ret + str;
        }
        return optional<string>::make(ret);
    } else {
        return optional<string>(nullptr);
    }
}

::opt::Test GetTest()
{
    return taihe::make_holder<TestImpl, ::opt::Test>();
}

void CallCallback(bool second,
                  ::taihe::callback_view<void(::taihe::string_view a, ::taihe::optional_view<::taihe::string> b)> cb)
{
    if (second) {
        cb("Hello", optional<string> {std::in_place, "World"});
    } else {
        cb("Hello", optional<string> {std::nullopt});
    }
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_ShowOptionalInt(ShowOptionalInt);
TH_EXPORT_CPP_API_MakeOptionalInt(MakeOptionalInt);
TH_EXPORT_CPP_API_MakeOptionalArray(MakeOptionalArray);
TH_EXPORT_CPP_API_SendReturnResult(SendReturnResult);
TH_EXPORT_CPP_API_GetTest(GetTest);
TH_EXPORT_CPP_API_CallCallback(CallCallback);
// NOLINTEND