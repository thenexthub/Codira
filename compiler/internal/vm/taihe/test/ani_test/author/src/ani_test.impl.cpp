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
#include "ani_test.impl.hpp"

using namespace taihe;
using namespace ani_test;

namespace {
Data makeData()
{
    return Data {
        string("C++ Object"),
        (float)1.0,
        array<string>::make(2, "file.txt"),
    };
}

void showData(Data const &s)
{
    std::cout << "src: " << s.src << std::endl;
    std::cout << "dest: " << s.dest << std::endl;
    for (auto const &s : s.files) {
        std::cout << "file: " << s.c_str() << std::endl;
    }
}

Union makeUnion(int32_t v)
{
    int32_t const case1Key = 1;
    int32_t const case2Key = 2;
    int32_t const case3Key = 3;

    int32_t const case1Value = 100;
    float const case2Value = 0.5f;

    switch (v) {
        case case1Key:
            return Union::make_iValue(case1Value);
        case case2Key:
            return Union::make_fValue(case2Value);
        case case3Key:
            return Union::make_sValue("Hello from C++!");
        default:
            return Union::make_empty();
    }
}

void showUnion(Union const &u)
{
    if (auto iPtr = u.get_iValue_ptr()) {
        std::cout << "I " << *iPtr << std::endl;
    } else if (auto fPtr = u.get_fValue_ptr()) {
        std::cout << "F " << *fPtr << std::endl;
    } else if (auto sPtr = u.get_sValue_ptr()) {
        std::cout << "S " << *sPtr << std::endl;
    } else {
        std::cout << "E" << std::endl;
    }
}

void showOptionalInt(optional_view<int32_t> x)
{
    if (x) {
        std::cout << *x << std::endl;
    } else {
        std::cout << "Null" << std::endl;
    }
}

optional<int32_t> makeOptionalInt(bool b)
{
    if (b) {
        int32_t const optionalIntValue = 10;
        return optional<int32_t>::make(optionalIntValue);
    } else {
        return optional<int32_t>(nullptr);
    }
}

void showArrayInt(array_view<int32_t> x)
{
    for (auto i : x) {
        std::cout << i << ", ";
    }
    std::cout << std::endl;
}

array<int32_t> makeArrayInt(int32_t n, int32_t v)
{
    return array<int32_t>::make(n, v);
}

array<Foo> makeFoo(array_view<string> list)
{
    struct AuthorFoo {
        string name;

        AuthorFoo(string_view name) : name(name)
        {
            std::cout << "AuthorFoo(" << this->name << ") is constructing" << std::endl;
        }

        ~AuthorFoo()
        {
            std::cout << "AuthorFoo(" << this->name << ") is destructing" << std::endl;
        }

        void bar()
        {
            std::cout << "AuthorFoo(" << this->name << ") is calling bar()" << std::endl;
        }
    };

    std::vector<Foo> vec;
    for (string_view name : list) {
        vec.push_back(make_holder<AuthorFoo, Foo>(name));
    }
    return array<Foo>(copy_data, vec.data(), vec.size());
}

void callBar(array_view<Foo> arr)
{
    for (weak::Foo foo : arr) {
        foo->bar();
    }
}
}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_makeData(makeData);
TH_EXPORT_CPP_API_showData(showData);
TH_EXPORT_CPP_API_makeUnion(makeUnion);
TH_EXPORT_CPP_API_showUnion(showUnion);
TH_EXPORT_CPP_API_showOptionalInt(showOptionalInt);
TH_EXPORT_CPP_API_makeOptionalInt(makeOptionalInt);
TH_EXPORT_CPP_API_showArrayInt(showArrayInt);
TH_EXPORT_CPP_API_makeArrayInt(makeArrayInt);
TH_EXPORT_CPP_API_makeFoo(makeFoo);
TH_EXPORT_CPP_API_callBar(callBar);
// NOLINTEND