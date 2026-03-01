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
#include "optional.impl.hpp"
#include "optional.ExampleInterface.proj.2.hpp"
#include "optional.Union.proj.1.hpp"
#include "taihe/map.hpp"
#include "taihe/optional.hpp"
// Please delete <stdexcept> include when you implement
using namespace taihe;

namespace {
class ExampleInterface {
public:
    void FuncPrimitiveI8(optional_view<int8_t> param1)
    {
        std::cout << "opt1 has value: " << *param1 << std::endl;
    }

    void FuncPrimitiveI16(optional_view<int16_t> param1)
    {
        std::cout << "opt1 has value: " << *param1 << std::endl;
    }

    void FuncPrimitiveI32(optional_view<int32_t> param1)
    {
        std::cout << "opt1 has value: " << *param1 << std::endl;
    }

    void FuncPrimitiveI64(optional_view<int64_t> param1)
    {
        std::cout << "opt1 has value: " << *param1 << std::endl;
    }

    void FuncPrimitiveF32(optional_view<float> param1)
    {
        std::cout << "opt1 has value: " << *param1 << std::endl;
    }

    void FuncPrimitiveF64(optional_view<double> param1)
    {
        std::cout << "opt1 has value: " << *param1 << std::endl;
    }

    void FuncPrimitiveBool(optional_view<bool> param1)
    {
        std::cout << "opt1 has value: " << *param1 << std::endl;
    }

    void FuncPrimitiveString(optional_view<string> param1)
    {
        std::cout << "opt1 has value: " << *param1 << std::endl;
    }

    void FuncArray(optional_view<array<int32_t>> param1)
    {
        std::cout << "func_Array: [";
        for (auto const &elem : *param1) {
            std::cout << elem << " ";
        }
        std::cout << "]" << std::endl;
    }

    void FuncBuffer(optional_view<array<uint8_t>> param1)
    {
        std::cout << "func_Array: [";
        for (auto const &elem : *param1) {
            std::cout << (int)elem << " ";
        }
        std::cout << "]" << std::endl;
    }

    void FuncUnion(optional_view<::optional::Union> param1)
    {
        std::cout << (*param1).get_sValue_ref() << std::endl;
        std::cout << (*param1).get_iValue_ref() << std::endl;
    }

    void FuncMap(optional_view<map<string, int32_t>> param1)
    {
        for (auto it = (*param1).begin(); it != (*param1).end(); ++it) {
            auto const &[key, value] = *it;
            std::cout << "Key: " << key << ", Value: " << value << std::endl;
        }
    }

    taihe::optional<string> GetName()
    {
        return taihe::optional<string>::make("hello");
    }

    taihe::optional<int8_t> Geti8()
    {
        int8_t const geti8Value = 1;
        return taihe::optional<int8_t>::make(geti8Value);
    }

    taihe::optional<int16_t> Geti16()
    {
        int16_t const geti16Value = 100;
        return taihe::optional<int16_t>::make(geti16Value);
    }

    taihe::optional<int32_t> Geti32()
    {
        int32_t const geti32Value = 1024;
        return taihe::optional<int32_t>::make(geti32Value);  // 默认返回 0
    }

    taihe::optional<int64_t> Geti64()
    {
        int64_t const geti64Value = 999999;
        return taihe::optional<int64_t>::make(geti64Value);  // 默认返回 0
    }

    taihe::optional<float> Getf32()
    {
        float const getf32Value = 0.0f;
        return taihe::optional<float>::make(getf32Value);
    }

    taihe::optional<double> Getf64()
    {
        double const getf64Value = 0.0;
        return taihe::optional<double>::make(getf64Value);
    }

    taihe::optional<bool> Getbool()
    {
        return taihe::optional<bool>::make(false);
    }

    taihe::optional<array<uint8_t>> GetArraybuffer()
    {
        int const arrSize = 10;
        int const arrNum = 6;
        return taihe::optional<array<uint8_t>>::make(array<uint8_t>::make(arrSize, arrNum));
    }
};

::optional::ExampleInterface GetInterface()
{
    return make_holder<ExampleInterface, ::optional::ExampleInterface>();
}

void PrintTestInterfaceName(::optional::weak::ExampleInterface testiface)
{
    auto res = testiface->GetName();
    std::cout << __func__ << ": " << *res << std::endl;
}

void PrintTestInterfaceNumberi8(::optional::weak::ExampleInterface testiface)
{
    auto res = testiface->Geti8();
    std::cout << __func__ << ": " << *res << std::endl;
}

void PrintTestInterfaceNumberi16(::optional::weak::ExampleInterface testiface)
{
    auto res = testiface->Geti16();
    std::cout << __func__ << ": " << *res << std::endl;
}

void PrintTestInterfaceNumberi32(::optional::weak::ExampleInterface testiface)
{
    auto res = testiface->Geti16();
    std::cout << __func__ << ": " << *res << std::endl;
}

void PrintTestInterfaceNumberi64(::optional::weak::ExampleInterface testiface)
{
    auto res = testiface->Geti64();
    std::cout << __func__ << ": " << *res << std::endl;
}

void PrintTestInterfaceNumberf32(::optional::weak::ExampleInterface testiface)
{
    auto res = testiface->Getf32();
    std::cout << __func__ << ": " << *res << std::endl;
}

void PrintTestInterfaceNumberf64(::optional::weak::ExampleInterface testiface)
{
    auto res = testiface->Getf64();
    std::cout << __func__ << ": " << *res << std::endl;
}

void PrintTestInterfacebool(::optional::weak::ExampleInterface testiface)
{
    auto res = testiface->Getbool();
    std::cout << __func__ << ": " << *res << std::endl;
}

void PrintTestInterfaceArraybuffer(::optional::weak::ExampleInterface testiface)
{
    auto arr = testiface->GetArraybuffer();

    for (size_t i = 0; i < (*arr).size(); ++i) {
        std::cout << static_cast<int>((*arr).data()[i]);
        if (i < (*arr).size() - 1) {
            std::cout << ", ";
        }
    }
}
}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_GetInterface(GetInterface);
TH_EXPORT_CPP_API_PrintTestInterfaceName(PrintTestInterfaceName);
TH_EXPORT_CPP_API_PrintTestInterfaceNumberi8(PrintTestInterfaceNumberi8);
TH_EXPORT_CPP_API_PrintTestInterfaceNumberi16(PrintTestInterfaceNumberi16);
TH_EXPORT_CPP_API_PrintTestInterfaceNumberi32(PrintTestInterfaceNumberi32);
TH_EXPORT_CPP_API_PrintTestInterfaceNumberi64(PrintTestInterfaceNumberi64);
TH_EXPORT_CPP_API_PrintTestInterfaceNumberf32(PrintTestInterfaceNumberf32);
TH_EXPORT_CPP_API_PrintTestInterfaceNumberf64(PrintTestInterfaceNumberf64);
TH_EXPORT_CPP_API_PrintTestInterfacebool(PrintTestInterfacebool);
TH_EXPORT_CPP_API_PrintTestInterfaceArraybuffer(PrintTestInterfaceArraybuffer);
// NOLINTEND