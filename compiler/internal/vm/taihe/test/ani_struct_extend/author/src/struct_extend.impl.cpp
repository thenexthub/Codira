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
#include "struct_extend.impl.hpp"

#include <iostream>

#include "stdexcept"
#include "struct_extend.A.proj.1.hpp"
#include "struct_extend.B.proj.1.hpp"
#include "struct_extend.Bar.proj.2.hpp"
#include "struct_extend.C.proj.1.hpp"
#include "struct_extend.D.proj.1.hpp"
#include "struct_extend.E.proj.1.hpp"
#include "struct_extend.F.proj.1.hpp"
#include "struct_extend.G.proj.1.hpp"
#include "taihe/runtime.hpp"
using namespace taihe;

namespace {
class Bar {
public:
    explicit Bar(::struct_extend::E const &e)
    {
        this->e_.d.param4 = e.d.param4;
        this->e_.param5 = e.param5;
    }

    ::struct_extend::E getE()
    {
        return e_;
    }

    void setE(::struct_extend::E const &e)
    {
        this->e_.d.param4 = e.d.param4;
        this->e_.param5 = e.param5;
    }

private:
    ::struct_extend::E e_;
};

void check_A(::struct_extend::A const &i)
{
    std::cout << i.param1 << std::endl;
}

::struct_extend::A create_A()
{
    return {1};
}

void check_B(::struct_extend::B const &i)
{
    std::cout << i.a.param1 << std::endl;
    std::cout << i.param2 << std::endl;
}

::struct_extend::B create_B()
{
    return {{1}, 2};
}

void check_C(::struct_extend::C const &i)
{
    std::cout << i.b.a.param1 << std::endl;
    std::cout << i.b.param2 << std::endl;
    std::cout << i.param3 << std::endl;
}

::struct_extend::C create_C()
{
    return {{{1}, 2}, 3};
}

void check_D(::struct_extend::D const &i)
{
    std::cout << i.param4 << std::endl;
}

::struct_extend::D create_D()
{
    return {4};
}

void check_E(::struct_extend::E const &i)
{
    std::cout << i.d.param4 << std::endl;
    std::cout << i.param5 << std::endl;
}

::struct_extend::E create_E()
{
    return {{4}, 5};
}

::struct_extend::Bar getBar(::struct_extend::E const &e)
{
    return make_holder<Bar, ::struct_extend::Bar>(e);
}

bool check_Bar(::struct_extend::weak::Bar bar)
{
    return true;
}

bool check_F(::struct_extend::F const &f)
{
    return true;
}

bool check_G(::struct_extend::G const &g)
{
    return true;
}

bool check_H(::struct_extend::H const &h)
{
    return true;
}

bool check_I(::struct_extend::I const &i)
{
    return true;
}

bool check_J(::struct_extend::J const &j)
{
    return true;
}

::struct_extend::Bar create_Bar(::struct_extend::E const &e)
{
    return make_holder<Bar, ::struct_extend::Bar>(e);
}

::struct_extend::F create_F(::struct_extend::E const &e)
{
    ::struct_extend::F f {
        .barF = make_holder<Bar, ::struct_extend::Bar>(e),
    };
    return f;
}

::struct_extend::G create_G(::struct_extend::E const &e)
{
    ::struct_extend::G g {
        .f = create_F(e),
        .barG = make_holder<Bar, ::struct_extend::Bar>(e),
    };
    return g;
}

::struct_extend::H create_H(::struct_extend::E const &e)
{
    ::struct_extend::H h{
      .g = create_G(e),
      .barH = make_holder<Bar, ::struct_extend::Bar>(e),
    };
    return h;
}

::struct_extend::I create_I(::struct_extend::E const &e)
{
    ::struct_extend::I i{
      .h = create_H(e),
      .barI = make_holder<Bar, ::struct_extend::Bar>(e),
    };
    return i;
}

::struct_extend::J create_J(::struct_extend::E const &e)
{
    ::struct_extend::J j{
      .i = create_I(e),
      .barJ = make_holder<Bar, ::struct_extend::Bar>(e),
    };
    return j;
}
}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_check_A(check_A);
TH_EXPORT_CPP_API_create_A(create_A);
TH_EXPORT_CPP_API_check_B(check_B);
TH_EXPORT_CPP_API_create_B(create_B);
TH_EXPORT_CPP_API_check_C(check_C);
TH_EXPORT_CPP_API_create_C(create_C);
TH_EXPORT_CPP_API_check_D(check_D);
TH_EXPORT_CPP_API_create_D(create_D);
TH_EXPORT_CPP_API_check_E(check_E);
TH_EXPORT_CPP_API_create_E(create_E);
TH_EXPORT_CPP_API_getBar(getBar);
TH_EXPORT_CPP_API_check_Bar(check_Bar);
TH_EXPORT_CPP_API_check_F(check_F);
TH_EXPORT_CPP_API_check_G(check_G);
TH_EXPORT_CPP_API_check_H(check_H);
TH_EXPORT_CPP_API_check_I(check_I);
TH_EXPORT_CPP_API_check_J(check_J);
TH_EXPORT_CPP_API_create_Bar(create_Bar);
TH_EXPORT_CPP_API_create_F(create_F);
TH_EXPORT_CPP_API_create_G(create_G);
TH_EXPORT_CPP_API_create_H(create_H);
TH_EXPORT_CPP_API_create_I(create_I);
TH_EXPORT_CPP_API_create_J(create_J);
// NOLINTEND