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
#include "bigint_new.impl.hpp"
#include <cstdint>
#include "bigint_new.MyUnion.proj.0.hpp"
#include "bigint_new.MyUnion.proj.1.hpp"
#include "bigint_new.proj.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"

#include <iostream>

using namespace taihe;

namespace {
// To be implemented.

class WantImpl {
public:
    array<uint64_t> a_ = {1, 2, 3};

    WantImpl() {}

    void SetBigInt(::taihe::array_view<uint64_t> a)
    {
        a_ = a;
        std::cout << "Received array (size = " << a.size() << "): ";
        for (size_t i = 0; i < a.size(); ++i) {
            std::cout << a[i] << " ";
        }
        std::cout << std::endl;
    }

    ::taihe::array<uint64_t> GetBigInt()
    {
        return a_;
    }
};

array<uint64_t> ProcessBigInt(array_view<uint64_t> a)
{
    array<uint64_t> res(a.size() + 1);
    res[0] = 0;
    for (int i = 0; i < a.size(); i++) {
        res[i + 1] = a[i];
        std::cerr << "arr[" << i << "] = " << a[i] << std::endl;
    }
    return res;
}

void PrintBigInt(::taihe::array_view<uint64_t> a)
{
    for (int i = 0; i < a.size(); i++) {
        std::cerr << "arr[" << i << "] = " << a[i] << std::endl;
    }
}

::taihe::array<uint64_t> CreateBigInt(::taihe::array_view<uint64_t> a)
{
    array<uint64_t> res(a.size());
    for (int i = 0; i < a.size(); i++) {
        res[i] = a[i];
        std::cerr << "arr[" << i << "] = " << a[i] << std::endl;
    }
    return res;
}

::bigint_new::Want GetWant()
{
    return taihe::make_holder<WantImpl, ::bigint_new::Want>();
}

void SetupStructAndPrint(::bigint_new::BigInt1 const &v)
{
    for (auto const &value : v.a) {
        std::cout << "a bigint array<u64>: " << value << " ";
    }
    std::cout << std::endl;
}

::taihe::array<uint64_t> GetBigIntOptional(::taihe::optional_view<::taihe::array<uint64_t>> a)
{
    if (!a.has_value()) {
        return {0, 0};
    }
    auto const &value = a.value();
    array<uint64_t> result(value.size());
    for (int i = 0; i < value.size(); i++) {
        result[i] = value[i];
    }
    return result;
}

::taihe::map<::taihe::string, ::taihe::array<uint64_t>> MapBigInt(
    ::taihe::map_view<::taihe::string, ::taihe::array<uint64_t>> a)
{
    map<string, array<uint64_t>> result;
    for (auto const &val : a) {
        result.emplace(val.first, val.second);
    }
    return result;
}

::bigint_new::MyUnion MakeMyUnion(int32_t n)
{
    int32_t const case1Key = 1;
    int32_t const case2Key = 2;

    array<uint64_t> bigIntData = {1, 2, 3};
    string str = "this is string data";

    switch (n) {
        case case1Key:
            return ::bigint_new::MyUnion::make_bigIntValue(bigIntData);  // 自己做的转换
        case case2Key:
            return ::bigint_new::MyUnion::make_stringValue(str);
        default:
            return ::bigint_new::MyUnion::make_empty();
    }
}

void ShowMyUnion(::bigint_new::MyUnion const &u)
{
    if (auto ptr = u.get_bigIntValue_ptr()) {
        std::cout << "bigIntValue: [";
        for (auto const &val : *ptr) {
            std::cout << " " << val << " ";
        }
        std::cout << "]\n";
    } else if (auto ptr = u.get_stringValue_ptr()) {
        std::cout << "stringValue: " << *ptr << "\n";
    } else {
        std::cout << "empty\n";
    }
}

}  // namespace

TH_EXPORT_CPP_API_ProcessBigInt(ProcessBigInt);
TH_EXPORT_CPP_API_PrintBigInt(PrintBigInt);
TH_EXPORT_CPP_API_CreateBigInt(CreateBigInt);
TH_EXPORT_CPP_API_GetWant(GetWant);
TH_EXPORT_CPP_API_SetupStructAndPrint(SetupStructAndPrint);
TH_EXPORT_CPP_API_GetBigIntOptional(GetBigIntOptional);
TH_EXPORT_CPP_API_MapBigInt(MapBigInt);
TH_EXPORT_CPP_API_MakeMyUnion(MakeMyUnion);
TH_EXPORT_CPP_API_ShowMyUnion(ShowMyUnion);
// NOLINTEND