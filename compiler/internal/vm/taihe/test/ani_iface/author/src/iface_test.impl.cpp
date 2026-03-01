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
#include "iface_test.impl.hpp"

#include "iface_test.Foo.proj.2.hpp"
#include "stdexcept"
#include "taihe/string.hpp"
// Please delete <stdexcept> include when you implement
using namespace taihe;

namespace {

class Foo {
    string name_ {"foo"};

public:
    void Bar()
    {
        std::cout << "Fooimpl: " << __func__ << std::endl;
    }

    string GetName()
    {
        std::cout << "Fooimpl: " << __func__ << " " << name_ << std::endl;
        return name_;
    }

    void SetName(string_view name)
    {
        std::cout << "Fooimpl: " << __func__ << " " << name << std::endl;
        name_ = name;
    }
};

class BaseFunImpl {
public:
    BaseFunImpl() {}

    ::taihe::string Base()
    {
        return "BaseFun.base";
    }

    ::taihe::string Fun()
    {
        return "BaseFun.fun";
    }
};

class SubBaseFunImpl {
public:
    SubBaseFunImpl() {}

    ::taihe::string Sub()
    {
        return "SubBaseFun.sub";
    }

    ::taihe::string Base()
    {
        return "SubBaseFun.base";
    }

    ::taihe::string Fun()
    {
        return "SubBaseFun.fun";
    }
};

class BaseElemImpl {
public:
    string base = "base";

    BaseElemImpl() {}

    ::taihe::string GetBase()
    {
        return base;
    }

    void SetBase(::taihe::string_view s)
    {
        this->base = s;
    }
};

class SubBaseElemImpl {
public:
    string base = "SubBaseElem";

    SubBaseElemImpl() {}

    ::taihe::string Sub()
    {
        return "SubBaseElem.sub";
    }

    ::taihe::string GetBase()
    {
        return base;
    }

    void SetBase(::taihe::string_view s)
    {
        this->base = s;
    }
};

::iface_test::Foo GetFooIface()
{
    std::cout << __func__ << std::endl;
    return make_holder<Foo, ::iface_test::Foo>();
}

string PrintFooName(::iface_test::weak::Foo foo)
{
    auto name = foo->GetName();
    std::cout << __func__ << ": " << name << std::endl;
    return name;
}

::iface_test::BaseFun GetBaseFun()
{
    return taihe::make_holder<BaseFunImpl, ::iface_test::BaseFun>();
}

::iface_test::SubBaseFun GetSubBaseFun()
{
    return taihe::make_holder<SubBaseFunImpl, ::iface_test::SubBaseFun>();
}

::iface_test::BaseElem GetBaseElem()
{
    return taihe::make_holder<BaseElemImpl, ::iface_test::BaseElem>();
}

::iface_test::SubBaseElem GetSubBaseElem()
{
    return taihe::make_holder<SubBaseElemImpl, ::iface_test::SubBaseElem>();
}
}  // namespace

TH_EXPORT_CPP_API_GetFooIface(GetFooIface);
TH_EXPORT_CPP_API_PrintFooName(PrintFooName);
TH_EXPORT_CPP_API_GetBaseFun(GetBaseFun);
TH_EXPORT_CPP_API_GetSubBaseFun(GetSubBaseFun);
TH_EXPORT_CPP_API_GetBaseElem(GetBaseElem);
TH_EXPORT_CPP_API_GetSubBaseElem(GetSubBaseElem);
// NOLINTEND