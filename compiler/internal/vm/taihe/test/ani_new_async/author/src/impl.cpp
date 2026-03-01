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
int32_t AddImpl(int32_t a, int32_t b)
{
    if (a == 0) {
        taihe::set_business_error(1, "some error happen in add impl");
        return b;
    } else {
        std::cout << "add impl " << a + b << std::endl;
        return a + b;
    }
}

::async_test::IBase GetIBaseImpl()
{
    struct AuthorIBase {
        taihe::string name;

        AuthorIBase() : name("My IBase") {}

        ~AuthorIBase() {}

        taihe::string Get()
        {
            return name;
        }

        taihe::string GetWithCallback()
        {
            return name;
        }

        taihe::string GetReturnsPromise()
        {
            return name;
        }

        void Set(taihe::string_view a)
        {
            this->name = a;
            return;
        }

        void SetWithCallback(taihe::string_view a)
        {
            this->name = a;
            return;
        }

        void SetReturnsPromise(taihe::string_view a)
        {
            this->name = a;
            return;
        }

        void MakeSync()
        {
            TH_THROW(std::runtime_error, "makeSync not implemented");
        }

        void MakeWithCallback()
        {
            TH_THROW(std::runtime_error, "makeSync not implemented");
        }

        void MakeReturnsPromise()
        {
            TH_THROW(std::runtime_error, "makeSync not implemented");
        }
    };

    return taihe::make_holder<AuthorIBase, ::async_test::IBase>();
}

void FromStructSyncImpl(::async_test::Data const &data)
{
    std::cout << data.a.c_str() << " " << data.b.c_str() << " " << data.c << std::endl;
    if (data.c == 0) {
        taihe::set_business_error(1, "some error happen in fromStructSyncImpl");
    }
    return;
}

::async_test::Data ToStructSyncImpl(taihe::string_view a, taihe::string_view b, int32_t c)
{
    if (c == 0) {
        taihe::set_business_error(1, "some error happen in toStructSyncImpl");
        return {a, b, c};
    }
    return {a, b, c};
}

void PrintSync()
{
    std::cout << "print Sync" << std::endl;
}

void MakeGlobalSync()
{
    std::cout << "makeGlobal" << std::endl;
}
}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_addSync(AddImpl);
TH_EXPORT_CPP_API_addWithAsync(AddImpl);
TH_EXPORT_CPP_API_addReturnsPromise(AddImpl);
TH_EXPORT_CPP_API_getIBase(GetIBaseImpl);
TH_EXPORT_CPP_API_getIBaseWithCallback(GetIBaseImpl);
TH_EXPORT_CPP_API_getIBaseReturnsPromise(GetIBaseImpl);
TH_EXPORT_CPP_API_fromStructSync(FromStructSyncImpl);
TH_EXPORT_CPP_API_fromStructWithCallback(FromStructSyncImpl);
TH_EXPORT_CPP_API_fromStructReturnsPromise(FromStructSyncImpl);
TH_EXPORT_CPP_API_toStructSync(ToStructSyncImpl);
TH_EXPORT_CPP_API_toStructWithCallback(ToStructSyncImpl);
TH_EXPORT_CPP_API_toStructReturnsPromise(ToStructSyncImpl);
TH_EXPORT_CPP_API_printSync(PrintSync);
TH_EXPORT_CPP_API_printWithCallback(PrintSync);
TH_EXPORT_CPP_API_printReturnsPromise(PrintSync);
TH_EXPORT_CPP_API_makeGlobalSync(MakeGlobalSync);
TH_EXPORT_CPP_API_makeGlobalWithCallback(MakeGlobalSync);
TH_EXPORT_CPP_API_makeGlobalReturnsPromise(MakeGlobalSync);
// NOLINTEND