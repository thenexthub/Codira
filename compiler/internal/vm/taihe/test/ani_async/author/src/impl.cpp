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
#include <async_test.impl.hpp>
#include <cstdint>
#include <iostream>

#include "taihe/runtime.hpp"

namespace {
int32_t add_impl(int32_t a, int32_t b)
{
    if (a == 0) {
        taihe::set_business_error(1, "some error happen in add impl");
        return b;
    } else {
        std::cout << "add impl " << a + b << std::endl;
        return a + b;
    }
}

::async_test::IBase getIBase_impl()
{
    struct AuthorIBase {
        taihe::string name;

        AuthorIBase() : name("My IBase") {}

        ~AuthorIBase() {}

        taihe::string get()
        {
            return name;
        }

        void set(taihe::string_view a)
        {
            this->name = a;
            return;
        }

        void makeSync()
        {
            TH_THROW(std::runtime_error, "makeSync not implemented");
        }
    };

    return taihe::make_holder<AuthorIBase, ::async_test::IBase>();
}

void fromStructSync_impl(::async_test::Data const &data)
{
    std::cout << data.a.c_str() << " " << data.b.c_str() << " " << data.c << std::endl;
    if (data.c == 0) {
        taihe::set_business_error(1, "some error happen in fromStructSync_impl");
    }
    return;
}

::async_test::Data toStructSync_impl(taihe::string_view a, taihe::string_view b, int32_t c)
{
    if (c == 0) {
        taihe::set_business_error(1, "some error happen in toStructSync_impl");
        return {a, b, c};
    }
    return {a, b, c};
}

void PrintSync()
{
    std::cout << "print Sync" << std::endl;
}

void makeGlobalSync()
{
    TH_THROW(std::runtime_error, "makeGlobalSync not implemented");
}
}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_addSync(add_impl);
TH_EXPORT_CPP_API_getIBase(getIBase_impl);
TH_EXPORT_CPP_API_fromStructSync(fromStructSync_impl);
TH_EXPORT_CPP_API_toStructSync(toStructSync_impl);
TH_EXPORT_CPP_API_PrintSync(PrintSync);
TH_EXPORT_CPP_API_makeGlobalSync(makeGlobalSync);
// NOLINTEND