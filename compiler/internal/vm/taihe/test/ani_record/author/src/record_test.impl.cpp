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
#include "record_test.impl.hpp"

#include "record_test.Color.proj.1.hpp"
#include "record_test.Data.proj.1.hpp"
#include "record_test.ICpu.proj.2.hpp"
#include "record_test.ICpuInfo.proj.2.hpp"
#include "record_test.ICpuZero.proj.2.hpp"
#include "record_test.Pair.proj.1.hpp"
#include "record_test.TypeUnion.proj.1.hpp"
#include "stdexcept"
#include "taihe/array.hpp"
#include "taihe/map.hpp"
#include "taihe/optional.hpp"
#include "taihe/string.hpp"
// Please delete <stdexcept> include when you implement
using namespace taihe;

namespace {
class ICpu {
public:
    int32_t Add(int32_t a, int32_t b)
    {
        return a + b;
    }

    int32_t Sub(int32_t a, int32_t b)
    {
        return a - b;
    }
};

class ICpuZero {
public:
    int32_t Add(int32_t a, int32_t b)
    {
        return a + b;
    }

    int32_t Sub(int32_t a, int32_t b)
    {
        return a - b;
    }
};

class ICpuInfo {
public:
    int32_t Add(int32_t a, int32_t b)
    {
        return a + b;
    }

    int32_t Sub(int32_t a, int32_t b)
    {
        return a - b;
    }
};

::record_test::ICpu MakeCpu()
{
    return make_holder<ICpu, ::record_test::ICpu>();
}

int32_t GetCpuSize(map_view<string, ::record_test::ICpu> r)
{
    return r.size();
}

int32_t GetASize(map_view<int32_t, uintptr_t> r)
{
    return r.size();
}

int32_t GetStringIntSize(map_view<string, int32_t> r)
{
    return r.size();
}

map<string, string> CreateStringString(int32_t a)
{
    map<string, string> m;
    while (a--) {
        m.emplace(to_string(a), "abc");
    }
    return m;
}

map<string, int32_t> GetMapfromArray(array_view<::record_test::Data> d)
{
    map<string, int32_t> m;
    for (std::size_t i = 0; i < d.size(); ++i) {
        m.emplace(d[i].a, d[i].b);
    }
    return m;
}

::record_test::Data GetDatafromMap(map_view<string, ::record_test::Data> m, string_view k)
{
    auto iter = m.find_item(k);
    if (iter == nullptr) {
        return {"su", 7};
    }
    return {iter->second.a, iter->second.b};
}

void ForeachMap(map_view<string, string> my_map)
{
    std::cout << "Using begin() and end() for traversal:" << std::endl;
    for (auto it = my_map.begin(); it != my_map.end(); ++it) {
        auto const &[key, value] = *it;
        std::cout << "Key: " << key << ", Value: " << value << std::endl;
    }

    std::cout << "Using range-based for loop for traversal:" << std::endl;
    for (auto const &[key, value] : my_map) {
        std::cout << "Key: " << key << ", Value: " << value << std::endl;
    }

    std::cout << "Using const iterator for traversal:" << std::endl;
    auto const &const_map = my_map;
    for (auto it = const_map.begin(); it != const_map.end(); ++it) {
        auto const &[key, value] = *it;
        std::cout << "Key: " << key << ", Value: " << value << std::endl;
    }

    std::cout << "Using cbegin() and cend() for traversal:" << std::endl;
    for (auto it = my_map.cbegin(); it != my_map.cend(); ++it) {
        auto const &[key, value] = *it;
        std::cout << "Key: " << key << ", Value: " << value << std::endl;
    };
}

bool Mapfunc01(map_view<string, bool> m)
{
    return true;
}

bool Mapfunc02(map_view<string, int8_t> m)
{
    int const threshold = 0;
    for (auto const &pair : m) {
        if (pair.second <= threshold) {
            return false;
        }
    }
    return true;
}

bool Mapfunc03(map_view<string, int16_t> m)
{
    int const threshold = 0;
    for (auto const &pair : m) {
        if (pair.second <= threshold) {
            return false;
        }
    }
    return true;
}

bool Mapfunc04(map_view<string, int32_t> m)
{
    int const threshold = 0;
    for (auto const &pair : m) {
        if (pair.second <= threshold) {
            return false;
        }
    }
    return true;
}

bool Mapfunc05(map_view<string, int64_t> m)
{
    int const threshold = 0;
    for (auto const &pair : m) {
        if (pair.second <= threshold) {
            return false;
        }
    }
    return true;
}

bool Mapfunc06(map_view<string, float> m)
{
    float const threshold = 0.0f;
    for (auto const &pair : m) {
        if (pair.second <= threshold) {
            return false;
        }
    }
    return true;
}

bool Mapfunc07(map_view<string, double> m)
{
    double const threshold = 0.0;
    for (auto const &pair : m) {
        if (pair.second <= threshold) {
            return false;
        }
    }
    return true;
}

bool Mapfunc08(map_view<string, string> m)
{
    for (auto const &pair : m) {
        if (pair.second.empty()) {
            return false;
        }
    }
    return true;
}

bool Mapfunc09(map_view<string, array<int8_t>> m)
{
    for (auto const &pair : m) {
        if (pair.second.empty()) {
            return false;
        }
    }
    return true;
}

bool Mapfunc10(map_view<string, array<int16_t>> m)
{
    for (auto const &pair : m) {
        if (pair.second.empty()) {
            return false;
        }
    }
    return true;
}

bool Mapfunc11(map_view<string, array<int32_t>> m)
{
    for (auto const &pair : m) {
        if (pair.second.empty()) {
            return false;
        }
    }
    return true;
}

bool Mapfunc12(map_view<string, array<int64_t>> m)
{
    for (auto const &pair : m) {
        if (pair.second.empty()) {
            return false;
        }
    }
    return true;
}

bool Mapfunc13(map_view<string, array<array<uint8_t>>> m)
{
    for (auto const &pair : m) {
        if (pair.second.empty()) {
            return false;
        }
    }
    return true;
}

bool Mapfunc14(map_view<string, array<bool>> m)
{
    for (auto const &pair : m) {
        if (pair.second.empty()) {
            return false;
        }
    }
    return true;
}

bool Mapfunc15(map_view<string, array<string>> m)
{
    for (auto const &pair : m) {
        if (pair.second.empty()) {
            return false;
        }
    }
    return true;
}

bool Mapfunc16(map_view<string, record_test::TypeUnion> m)
{
    return true;
}

bool Mapfunc17(map_view<string, record_test::Color> m)
{
    return true;
}

bool Mapfunc18(map_view<string, record_test::Pair> m)
{
    return true;
}

::record_test::ICpuZero MakeICpuZero()
{
    return make_holder<ICpuZero, ::record_test::ICpuZero>();
}

bool Mapfunc19(map_view<string, record_test::ICpuZero> m)
{
    return true;
}

::record_test::ICpuInfo MakeICpuInfo()
{
    return make_holder<ICpuInfo, ::record_test::ICpuInfo>();
}

bool Mapfunc20(map_view<string, record_test::ICpuInfo> m)
{
    return true;
}

bool Mapfunc21(map_view<string, uintptr_t> m)
{
    uintptr_t const zero = 0;
    for (auto const &pair : m) {
        if (pair.second == zero) {
            return false;
        }
    }
    return true;
}

bool Mapfunc22(map_view<string, map<string, bool>> m)
{
    size_t const emptySize = 0;
    for (auto const &pair : m) {
        if (pair.second.size() == emptySize) {
            return false;
        }
    }
    return true;
}

bool Mapfunc23(map_view<string, map<string, int32_t>> m)
{
    size_t const emptySize = 0;
    for (auto const &pair : m) {
        if (pair.second.size() == emptySize) {
            return false;
        }
    }
    return true;
}

bool Mapfunc24(map_view<string, map<string, array<int32_t>>> m)
{
    size_t const emptySize = 0;
    for (auto const &pair : m) {
        if (pair.second.size() == emptySize) {
            return false;
        }
    }
    return true;
}

bool Mapfunc25(map_view<string, map<string, string>> m)
{
    size_t const emptySize = 0;
    for (auto const &pair : m) {
        if (pair.second.size() == emptySize) {
            return false;
        }
    }
    return true;
}

map<string, bool> Mapfunc26()
{
    map<string, bool> result;
    bool const value1 = true;
    bool const value2 = false;
    result.emplace("key1", value1);
    result.emplace("key2", value2);
    return result;
}

map<string, int8_t> Mapfunc27()
{
    map<string, int8_t> result;
    int8_t const value1 = 123;
    int8_t const value2 = 45;
    result.emplace("key1", value1);
    result.emplace("key2", value2);
    return result;
}

map<string, int16_t> Mapfunc28()
{
    map<string, int16_t> result;
    int16_t const value1 = 1234;
    int16_t const value2 = 5678;
    result.emplace("key1", value1);
    result.emplace("key2", value2);
    return result;
}

map<string, int32_t> Mapfunc29()
{
    map<string, int32_t> result;
    int32_t const value1 = 12345;
    int32_t const value2 = 67890;
    result.emplace("key1", value1);
    result.emplace("key2", value2);
    return result;
}

map<string, int64_t> Mapfunc30()
{
    map<string, int64_t> result;
    int64_t const value1 = 123456;
    int64_t const value2 = 789012;
    result.emplace("key1", value1);
    result.emplace("key2", value2);
    return result;
}

map<string, float> Mapfunc31()
{
    map<string, float> result;
    float const value1 = 123.45f;
    float const value2 = 67.89f;
    result.emplace("key1", value1);
    result.emplace("key2", value2);
    return result;
}

map<string, double> Mapfunc32()
{
    map<string, double> result;
    double const value1 = 123.456;
    double const value2 = 789.012;
    result.emplace("key1", value1);
    result.emplace("key2", value2);
    return result;
}

map<string, string> Mapfunc33()
{
    map<string, string> result;
    string const value1 = "value1";
    string const value2 = "value2";
    result.emplace("key1", value1);
    result.emplace("key2", value2);
    return result;
}

map<string, array<int8_t>> Mapfunc34()
{
    map<string, array<int8_t>> result;
    array<int8_t> const a = {1, 2, 3};
    array<int8_t> const b = {4, 5, 6};
    result.emplace("key1", a);
    result.emplace("key2", b);
    return result;
}

map<string, array<int16_t>> Mapfunc35()
{
    map<string, array<int16_t>> result;
    array<int16_t> const a = {123, 456};
    array<int16_t> const b = {789, 1011};
    result.emplace("key1", a);
    result.emplace("key2", b);
    return result;
}

map<string, array<int32_t>> Mapfunc36()
{
    map<string, array<int32_t>> result;
    array<int32_t> const a = {1234, 5678};
    array<int32_t> const b = {9012, 3456};
    result.emplace("key1", a);
    result.emplace("key2", b);
    return result;
}

map<string, array<int64_t>> Mapfunc37()
{
    map<string, array<int64_t>> result;
    array<int64_t> const a = {12345, 67890};
    array<int64_t> const b = {11111, 22222};
    result.emplace("key1", a);
    result.emplace("key2", b);
    return result;
}

map<string, array<uint8_t>> Mapfunc38()
{
    map<string, array<uint8_t>> result;
    array<uint8_t> const a = {1, 2, 3};
    array<uint8_t> const b = {4, 5, 6};
    result.emplace("key1", a);
    result.emplace("key2", b);
    return result;
}

map<string, array<bool>> Mapfunc39()
{
    map<string, array<bool>> result;
    array<bool> const a = {true, false};
    array<bool> const b = {false, true};
    result.emplace("key1", a);
    result.emplace("key2", b);
    return result;
}

map<string, array<string>> Mapfunc40()
{
    map<string, array<string>> result;
    array<string> const a = {"value1", "value2"};
    array<string> const b = {"value3", "value4"};
    result.emplace("key1", a);
    result.emplace("key2", b);
    return result;
}

map<string, record_test::TypeUnion> Mapfunc41()
{
    int32_t const value = 123;
    map<string, record_test::TypeUnion> result;
    result.emplace("key1", record_test::TypeUnion::make_a(value));
    result.emplace("key2", record_test::TypeUnion::make_b(true));
    result.emplace("key3", record_test::TypeUnion::make_c("value"));
    return result;
}

map<string, record_test::Color> Mapfunc42()
{
    map<string, record_test::Color> result;
    result.emplace("key1", record_test::Color::key_t::RED);
    result.emplace("key2", record_test::Color::key_t::GREEN);
    return result;
}

map<string, record_test::Pair> Mapfunc43()
{
    map<string, record_test::Pair> result;
    record_test::Pair p1 {
        .a = "one",
        .b = true,
    };
    record_test::Pair p2 {
        .a = "two",
        .b = false,
    };
    result.emplace("key1", p1);
    result.emplace("key2", p2);
    return result;
}

map<string, record_test::ICpuZero> Mapfunc44()
{
    map<string, record_test::ICpuZero> result;
    result.emplace("key1", make_holder<ICpuZero, ::record_test::ICpuZero>());
    result.emplace("key2", make_holder<ICpuZero, ::record_test::ICpuZero>());
    return result;
}

map<string, record_test::ICpuInfo> Mapfunc45()
{
    map<string, record_test::ICpuInfo> result;
    result.emplace("key1", make_holder<ICpuInfo, ::record_test::ICpuInfo>());
    result.emplace("key2", make_holder<ICpuInfo, ::record_test::ICpuInfo>());
    return result;
}

map<string, uintptr_t> Mapfunc46()
{
    map<string, uintptr_t> result;
    result.emplace("key1", reinterpret_cast<uintptr_t>(nullptr));
    result.emplace("key2", reinterpret_cast<uintptr_t>(nullptr));
    return result;
}

map<string, map<string, bool>> Mapfunc47()
{
    map<string, map<string, bool>> result;
    map<string, bool> m1;
    bool const value1 = true;
    bool const value2 = false;
    m1.emplace("subkey1", value1);
    m1.emplace("subkey2", value2);
    result.emplace("key1", m1);
    map<string, bool> m2;
    bool const value3 = true;
    bool const value4 = false;
    m2.emplace("subkey3", value3);
    m2.emplace("subkey4", value4);
    result.emplace("key2", m2);
    return result;
}

map<string, map<string, int32_t>> Mapfunc48()
{
    map<string, map<string, int32_t>> result;
    map<string, int32_t> m1;
    int32_t const value1 = 100;
    int32_t const value2 = 200;
    m1.emplace("subkey1", value1);
    m1.emplace("subkey2", value2);
    result.emplace("key1", m1);
    map<string, int32_t> m2;
    int32_t const value3 = 300;
    int32_t const value4 = 400;
    m2.emplace("subkey3", value3);
    m2.emplace("subkey4", value4);
    result.emplace("key2", m2);
    return result;
}

map<string, map<string, array<int32_t>>> Mapfunc49()
{
    map<string, map<string, array<int32_t>>> result;
    map<string, array<int32_t>> m1;
    array<int32_t> const a1 = {1, 2, 3};
    array<int32_t> const b1 = {4, 5, 6};
    m1.emplace("subkey1", a1);
    m1.emplace("subkey2", b1);
    result.emplace("key1", m1);
    map<string, array<int32_t>> m2;
    array<int32_t> const a2 = {7, 8, 9};
    array<int32_t> const b2 = {10, 11, 12};
    m2.emplace("subkey3", a2);
    m2.emplace("subkey4", b2);
    result.emplace("key2", m2);
    return result;
}

map<string, map<string, string>> Mapfunc50()
{
    map<string, map<string, string>> result;
    map<string, string> m1;
    string const value1 = "value1";
    string const value2 = "value2";
    m1.emplace("subkey1", value1);
    m1.emplace("subkey2", value2);
    result.emplace("key1", m1);
    map<string, string> m2;
    string const value3 = "value3";
    string const value4 = "value4";
    m2.emplace("subkey3", value3);
    m2.emplace("subkey4", value4);
    result.emplace("key2", m2);
    return result;
}

map<string, map<string, string>> Mapfunc51(optional_view<map<string, string>> op)
{
    map<string, map<string, string>> result;
    map<string, string> m1;
    string const value1 = "value1";
    string const value2 = "value2";
    m1.emplace("subkey1", value1);
    m1.emplace("subkey2", value2);
    result.emplace("key1", m1);
    map<string, string> m2;
    string const value3 = "value3";
    string const value4 = "value4";
    m2.emplace("subkey3", value3);
    m2.emplace("subkey4", value4);
    result.emplace("key2", m2);
    return result;
}

}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_MakeCpu(MakeCpu);
TH_EXPORT_CPP_API_GetCpuSize(GetCpuSize);
TH_EXPORT_CPP_API_GetASize(GetASize);
TH_EXPORT_CPP_API_GetStringIntSize(GetStringIntSize);
TH_EXPORT_CPP_API_CreateStringString(CreateStringString);
TH_EXPORT_CPP_API_GetMapfromArray(GetMapfromArray);
TH_EXPORT_CPP_API_GetDatafromMap(GetDatafromMap);
TH_EXPORT_CPP_API_ForeachMap(ForeachMap);
TH_EXPORT_CPP_API_Mapfunc01(Mapfunc01);
TH_EXPORT_CPP_API_Mapfunc02(Mapfunc02);
TH_EXPORT_CPP_API_Mapfunc03(Mapfunc03);
TH_EXPORT_CPP_API_Mapfunc04(Mapfunc04);
TH_EXPORT_CPP_API_Mapfunc05(Mapfunc05);
TH_EXPORT_CPP_API_Mapfunc06(Mapfunc06);
TH_EXPORT_CPP_API_Mapfunc07(Mapfunc07);
TH_EXPORT_CPP_API_Mapfunc08(Mapfunc08);
TH_EXPORT_CPP_API_Mapfunc09(Mapfunc09);
TH_EXPORT_CPP_API_Mapfunc10(Mapfunc10);
TH_EXPORT_CPP_API_Mapfunc11(Mapfunc11);
TH_EXPORT_CPP_API_Mapfunc12(Mapfunc12);
TH_EXPORT_CPP_API_Mapfunc13(Mapfunc13);
TH_EXPORT_CPP_API_Mapfunc14(Mapfunc14);
TH_EXPORT_CPP_API_Mapfunc15(Mapfunc15);
TH_EXPORT_CPP_API_Mapfunc16(Mapfunc16);
TH_EXPORT_CPP_API_Mapfunc17(Mapfunc17);
TH_EXPORT_CPP_API_Mapfunc18(Mapfunc18);
TH_EXPORT_CPP_API_MakeICpuZero(MakeICpuZero);
TH_EXPORT_CPP_API_Mapfunc19(Mapfunc19);
TH_EXPORT_CPP_API_MakeICpuInfo(MakeICpuInfo);
TH_EXPORT_CPP_API_Mapfunc20(Mapfunc20);
TH_EXPORT_CPP_API_Mapfunc21(Mapfunc21);
TH_EXPORT_CPP_API_Mapfunc22(Mapfunc22);
TH_EXPORT_CPP_API_Mapfunc23(Mapfunc23);
TH_EXPORT_CPP_API_Mapfunc24(Mapfunc24);
TH_EXPORT_CPP_API_Mapfunc25(Mapfunc25);
TH_EXPORT_CPP_API_Mapfunc26(Mapfunc26);
TH_EXPORT_CPP_API_Mapfunc27(Mapfunc27);
TH_EXPORT_CPP_API_Mapfunc28(Mapfunc28);
TH_EXPORT_CPP_API_Mapfunc29(Mapfunc29);
TH_EXPORT_CPP_API_Mapfunc30(Mapfunc30);
TH_EXPORT_CPP_API_Mapfunc31(Mapfunc31);
TH_EXPORT_CPP_API_Mapfunc32(Mapfunc32);
TH_EXPORT_CPP_API_Mapfunc33(Mapfunc33);
TH_EXPORT_CPP_API_Mapfunc34(Mapfunc34);
TH_EXPORT_CPP_API_Mapfunc35(Mapfunc35);
TH_EXPORT_CPP_API_Mapfunc36(Mapfunc36);
TH_EXPORT_CPP_API_Mapfunc37(Mapfunc37);
TH_EXPORT_CPP_API_Mapfunc38(Mapfunc38);
TH_EXPORT_CPP_API_Mapfunc39(Mapfunc39);
TH_EXPORT_CPP_API_Mapfunc40(Mapfunc40);
TH_EXPORT_CPP_API_Mapfunc41(Mapfunc41);
TH_EXPORT_CPP_API_Mapfunc42(Mapfunc42);
TH_EXPORT_CPP_API_Mapfunc43(Mapfunc43);
TH_EXPORT_CPP_API_Mapfunc44(Mapfunc44);
TH_EXPORT_CPP_API_Mapfunc45(Mapfunc45);
TH_EXPORT_CPP_API_Mapfunc46(Mapfunc46);
TH_EXPORT_CPP_API_Mapfunc47(Mapfunc47);
TH_EXPORT_CPP_API_Mapfunc48(Mapfunc48);
TH_EXPORT_CPP_API_Mapfunc49(Mapfunc49);
TH_EXPORT_CPP_API_Mapfunc50(Mapfunc50);
TH_EXPORT_CPP_API_Mapfunc51(Mapfunc51);
// NOLINTEND