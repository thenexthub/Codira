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
#include "hello.impl.hpp"
#include "hello.proj.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"

namespace {
class TestInterfaceImpl {
    taihe::array<bool> m_fixedArrayBoolean = {};
    taihe::array<double> m_fixedArrayNumber = {};
    taihe::array<::taihe::string> m_fixedArrayString = {};
    taihe::array<::hello::Data> m_fixedArrayData = {};
    taihe::optional<taihe::array<::hello::Data>> m_optionalFixedArrayData;

public:
    TestInterfaceImpl() {}

    ::taihe::array<bool> getFixedArrayBoolean()
    {
        return m_fixedArrayBoolean;
    }

    ::taihe::array<double> getFixedArrayNumber()
    {
        return m_fixedArrayNumber;
    }

    ::taihe::array<::taihe::string> getFixedArrayString()
    {
        return m_fixedArrayString;
    }

    ::taihe::array<::hello::Data> getFixedArrayData()
    {
        return m_fixedArrayData;
    }

    ::taihe::optional<::taihe::array<::hello::Data>> getOptionalFixedArrayData()
    {
        return m_optionalFixedArrayData;
    }

    void setFixedArrayBoolean(::taihe::array_view<bool> value)
    {
        std::cout << "setFixedArrayBoolean called with values: ";
        for (auto const &v : value) {
            std::cout << v << " ";
        }
        std::cout << std::endl;
        m_fixedArrayBoolean = value;
    }

    void setFixedArrayNumber(::taihe::array_view<double> value)
    {
        std::cout << "setFixedArrayNumber called with values: ";
        for (auto const &v : value) {
            std::cout << v << " ";
        }
        std::cout << std::endl;
        m_fixedArrayNumber = value;
    }

    void setFixedArrayString(::taihe::array_view<::taihe::string> value)
    {
        std::cout << "setFixedArrayString called with values: ";
        for (auto const &v : value) {
            std::cout << v << " ";
        }
        std::cout << std::endl;
        m_fixedArrayString = value;
    }

    void setFixedArrayData(::taihe::array_view<::hello::Data> value)
    {
        std::cout << "setFixedArrayData called with " << value.size() << " elements." << std::endl;
        m_fixedArrayData = value;
    }

    void setOptionalFixedArrayData(::taihe::optional<::taihe::array<::hello::Data>> value)
    {
        std::cout << "setOptionalFixedArrayData called with "
                  << (value.has_value() ? std::to_string(value->size()) : "no") << " elements." << std::endl;
        m_optionalFixedArrayData = value;
    }
};

::hello::TestInterface newTestInterface()
{
    // The parameters in the make_holder function should be of the same type
    // as the parameters in the constructor of the actual implementation class.
    return taihe::make_holder<TestInterfaceImpl, ::hello::TestInterface>();
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_newTestInterface(newTestInterface);
// NOLINTEND