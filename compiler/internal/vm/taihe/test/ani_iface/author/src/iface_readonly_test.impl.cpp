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
#include "iface_readonly_test.impl.hpp"

#include "iface_readonly_test.Noo.proj.2.hpp"
#include "taihe/string.hpp"
using namespace taihe;

namespace {
class Noo {
    string name_ {"noo"};
    ::taihe::optional<int32_t> age_ {::taihe::optional<int32_t>(std::in_place, 1)};

public:
    void Bar()
    {
        std::cout << "Nooimpl: " << __func__ << std::endl;
    }

    string GetName()
    {
        std::cout << "Nooimpl: " << __func__ << " " << name_ << std::endl;
        return name_;
    }

    ::taihe::optional<int32_t> GetAge()
    {
        return age_;
    }

    void SetAge(::taihe::optional_view<int32_t> a)
    {
        this->age_ = a;
        return;
    }
};

::iface_readonly_test::Noo GetNooIface()
{
    return make_holder<Noo, ::iface_readonly_test::Noo>();
}

string PrintNooName(::iface_readonly_test::weak::Noo noo)
{
    auto name = noo->GetName();
    std::cout << __func__ << ": " << name << std::endl;
    return name;
}
}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_GetNooIface(GetNooIface);
TH_EXPORT_CPP_API_PrintNooName(PrintNooName);
// NOLINTEND