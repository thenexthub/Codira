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
#include "keep_name_test.impl.hpp"
#include "keep_name_test.proj.hpp"

#include "stdexcept"
#include "taihe/string.hpp"
// Please delete <stdexcept> include when you implement
using namespace taihe;

namespace {
class IBase {
public:
    IBase(string a, string b) : str(a), new_str(b) {}

    ~IBase() {}

    void OnSet(callback_view<void()> a)
    {
        a();
        std::cout << "IBase::onSet" << std::endl;
    }

    void OffSet(callback_view<void()> a)
    {
        a();
        std::cout << "IBase::offSet" << std::endl;
    }

private:
    string str;
    string new_str;
};

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

::keep_name_test::IBase GetIBase(string_view a, string_view b)
{
    return make_holder<IBase, ::keep_name_test::IBase>(a, b);
}

::keep_name_test::Foo GetFooIface()
{
    std::cout << __func__ << std::endl;
    return make_holder<Foo, ::keep_name_test::Foo>();
}

string PrintFooName(::keep_name_test::weak::Foo foo)
{
    auto name = foo->GetName();
    std::cout << __func__ << ": " << name << std::endl;
    return name;
}

}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_GetFooIface(GetFooIface);
TH_EXPORT_CPP_API_PrintFooName(PrintFooName);
TH_EXPORT_CPP_API_GetIBase(GetIBase);
// NOLINTEND