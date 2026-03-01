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
#include "function_test.impl.hpp"

#include "function_test.Foo.proj.2.hpp"
#include "function_test.MyUnion.proj.1.hpp"
#include "stdexcept"
#include "taihe/optional.hpp"
#include "taihe/string.hpp"
// Please delete <stdexcept> include when you implement
using namespace taihe;

namespace {
class Foo {
public:
    void bar()
    {
        std::cout << "call bar" << std::endl;
    }

    void bar_int(int32_t a)
    {
        std::cout << "call bar_int a is :" << a << std::endl;
    }

    void bar_str(string_view a)
    {
        std::cout << "call bar_str a is :" << std::string(a) << std::endl;
    }

    void bar_union(string_view a, ::function_test::MyUnion const &b)
    {
        std::cout << "call bar_union a is :" << std::string(a) << std::endl;
        switch (b.get_tag()) {
            case ::function_test::MyUnion::tag_t::sValue:
                std::cout << "s: " << b.get_sValue_ref() << std::endl;
            case ::function_test::MyUnion::tag_t::iValue:
                std::cout << "i: " << b.get_iValue_ref() << std::endl;
            case ::function_test::MyUnion::tag_t::fValue:
                std::cout << "i: " << b.get_iValue_ref() << std::endl;
        }
    }

    void bar_union_opt(string_view a, optional_view<string> name)
    {
        std::cout << "call bar_union a is :" << std::string(a) << *name << std::endl;
    }

    void test_cb_v(callback_view<void()> f)
    {
        f();
    }

    void test_cb_i(callback_view<void(int32_t)> f)
    {
        f(1);
    }

    void test_cb_s(callback_view<void(string_view, bool)> f)
    {
        f("hello", true);
    }

    string test_cb_rs(callback_view<int64_t(int64_t)> f)
    {
        int64_t out = f(444);
        return std::to_string(out);
    }

    int32_t addSync(int32_t a, int32_t b)
    {
        std::cout << "call addSync a and b is" << std::to_string(a) << std::to_string(b) << std::endl;
        return a + b;
    }
};

class FooCls {
public:
    string get()
    {
        return "zhangsan";
    }
};

int32_t static_func_add(int32_t a, int32_t b)
{
    return a + b;
}

int32_t static_func_sub(int32_t a, int32_t b)
{
    return a - b;
}

::function_test::FooCls getFooCls1(string_view name)
{
    return make_holder<FooCls, ::function_test::FooCls>();
}

::function_test::FooCls getFooCls2(string_view name, string_view test)
{
    return make_holder<FooCls, ::function_test::FooCls>();
}

::function_test::Foo makeFoo()
{
    return make_holder<Foo, ::function_test::Foo>();
}

static int32_t static_property = 0;

int32_t getStaticProperty()
{
    return static_property;
}

void setStaticProperty(int32_t a)
{
    static_property = a;
}
}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_static_func_add(static_func_add);
TH_EXPORT_CPP_API_static_func_sub(static_func_sub);
TH_EXPORT_CPP_API_getFooCls1(getFooCls1);
TH_EXPORT_CPP_API_getFooCls2(getFooCls2);
TH_EXPORT_CPP_API_makeFoo(makeFoo);

TH_EXPORT_CPP_API_setStaticProperty(setStaticProperty);
TH_EXPORT_CPP_API_getStaticProperty(getStaticProperty);
// NOLINTEND