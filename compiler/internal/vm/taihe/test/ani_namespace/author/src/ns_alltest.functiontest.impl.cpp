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
#include "ns_alltest.functiontest.impl.hpp"

#include <iostream>

#include "ns_alltest.functiontest.Color.proj.1.hpp"
#include "ns_alltest.functiontest.Data.proj.1.hpp"
#include "taihe/array.hpp"
#include "taihe/map.hpp"
#include "taihe/optional.hpp"
#include "taihe/runtime.hpp"
#include "taihe/string.hpp"

using namespace taihe;

namespace {
int g_testEnum {3};
int g_testInter {2};
int g_testIntAdd10 {10};
int g_testIntAdd100 {100};
int g_testIntAdd5 {5};

class TestNameSpace {
    string testStr_ {"testNameSpace"};

public:
    void BaseFunctionTest43()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }

    int8_t BaseFunctionTest44(int8_t param1)
    {
        // 检查结果是否超出 int8_t 的范围
        int32_t result = static_cast<int32_t>(param1) + g_testIntAdd100;
        if (result > INT8_MAX || result < INT8_MIN) {
            taihe::set_error("BaseFunctionTest44: result exceeds int8_t range");
        }
        // 返回结果
        return static_cast<int8_t>(result);
    }

    int16_t BaseFunctionTest45(int16_t param1)
    {
        // 检查结果是否超出 int16_t 的范围
        int32_t result = static_cast<int32_t>(param1) * g_testIntAdd100;
        if (result > INT16_MAX || result < INT16_MIN) {
            taihe::set_error("BaseFunctionTest45: result exceeds int16_t range");
        }
        // 返回结果
        return static_cast<int16_t>(result);
    }

    int32_t BaseFunctionTest46(int32_t param1)
    {
        // 检查结果是否超出 int32_t 的范围
        int64_t result = static_cast<int64_t>(param1) * g_testIntAdd100;
        std::cout << "NameSpaceImpl: " << __func__ << " result " << result << std::endl;
        if (result > INT32_MAX || result < INT32_MIN) {
            taihe::set_error("BaseFunctionTest18: result exceeds int32_t range");
        }
        // 返回结果
        return static_cast<int32_t>(result);
    }

    int64_t BaseFunctionTest47(int64_t param1)
    {
        // 检查结果是否超出 int64_t 的范围
        int64_t result = param1 * g_testIntAdd100;
        std::cout << "NameSpaceImpl: " << __func__ << " result " << result << std::endl;
        if (result > INT64_MAX || result < INT64_MIN) {
            taihe::set_error("BaseFunctionTest19: result exceeds int64_t range");
        }
        // 返回结果
        return static_cast<int64_t>(result);
    }

    float BaseFunctionTest48(float param1)
    {
        int const baseFunctionTest48Value = 10;
        return param1 - baseFunctionTest48Value;
    }

    double BaseFunctionTest49(double param1)
    {
        return param1 - g_testIntAdd100;
    }

    string BaseFunctionTest50(string_view param1)
    {
        return std::string(param1) + "BaseFunctionTest50";
    }

    bool BaseFunctionTest51(bool param1)
    {
        if (param1) {
            return false;
        } else {
            return true;
        }
    }

    array<uint8_t> BaseFunctionTest52(array_view<uint8_t> param1)
    {
        array<uint8_t> result = array<uint8_t>::make(param1.size());
        for (std::size_t i = 0; i < param1.size(); i++) {
            result[i] = param1[i] + g_testIntAdd10;
        }
        return result;
    }

    optional<int8_t> BaseFunctionTest53(optional_view<int8_t> param1)
    {
        if (param1) {
            // 检查结果是否超出 int8_t 的范围
            int32_t result = static_cast<int32_t>(*param1) - g_testIntAdd100;
            std::cout << "NameSpaceImpl: " << __func__ << " result " << result << std::endl;
            if (result > INT8_MAX || result < INT8_MIN) {
                taihe::set_error("BaseFunctionTest53: result exceeds int8_t range");
            }
            // 返回结果
            return optional<int8_t>::make(result);
        } else {
            std::cout << "NameSpaceImpl: " << __func__ << " param1 is null" << std::endl;
            return optional<int8_t>(nullptr);
        }
    }

    optional<int32_t> BaseFunctionTest54(optional_view<int32_t> param1)
    {
        if (param1) {
            // 检查结果是否超出 int32_t 的范围
            int32_t result = static_cast<int32_t>(*param1) - g_testIntAdd100;
            std::cout << "NameSpaceImpl: " << __func__ << " result " << result << std::endl;
            if (result > INT32_MAX || result < INT32_MIN) {
                taihe::set_error("BaseFunctionTest54: result exceeds int32_t range");
            }
            // 返回结果
            return optional<int32_t>::make(result);
        } else {
            std::cout << "NameSpaceImpl: " << __func__ << " param1 is null" << std::endl;
            return optional<int32_t>(nullptr);
        }
    }

    map<int16_t, ::ns_alltest::functiontest::Data> BaseFunctionTest55(
        map_view<int16_t, ::ns_alltest::functiontest::Data> param1)
    {
        map<int16_t, ::ns_alltest::functiontest::Data> m;
        for (auto const &[key, value] : param1) {
            m.emplace(key, value);
        }
        return m;
    }

    int64_t BaseFunctionTest56(int32_t param1)
    {
        int64_t result = static_cast<int64_t>(param1) * g_testIntAdd100;
        return result;
    }

    int32_t BaseFunctionTest57(int8_t param1)
    {
        int32_t result = static_cast<int32_t>(param1) * g_testIntAdd100;
        return result;
    }

    string getTestStr()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " " << testStr_ << std::endl;
        return testStr_;
    }

    ::ns_alltest::functiontest::ErrorResponse BaseFunctionTest58(bool param1)
    {
        if (param1) {
            return {param1, 10000, "test58"};
        } else {
            return {false, static_cast<int16_t>(g_testIntAdd100), std::string("test581")};
        }
    }
};

class TestInterfacePerformance1 {
    int testIntAdd5 {5};
    int testIntAdd10 {10};

public:
    void BasePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }

    int8_t BasePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + testIntAdd5) / testIntAdd10;
    }
};

class TestInterfacePerformance2 {
    int testIntAdd5 {5};
    int testIntAdd10 {10};

public:
    void BasePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }

    int8_t BasePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + testIntAdd5) / testIntAdd10;
    }
};

class TestInterfacePerformance3 {
    int testIntAdd5 {5};
    int testIntAdd10 {10};

public:
    void BasePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }

    int8_t BasePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + testIntAdd5) / testIntAdd10;
    }
};

class TestInterfacePerformance4 {
    int testIntAdd5 {5};
    int testIntAdd10 {10};

public:
    void BasePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }

    int8_t BasePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + testIntAdd5) / testIntAdd10;
    }
};

class TestInterfacePerformance5 {
    int testIntAdd5 {5};
    int testIntAdd10 {10};

public:
    void BasePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }

    int8_t BasePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + testIntAdd5) / testIntAdd10;
    }
};

class TestInterfacePerformance6 {
    int testIntAdd5 {5};
    int testIntAdd10 {10};

public:
    void BasePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }

    int8_t BasePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + testIntAdd5) / testIntAdd10;
    }
};

class TestInterfacePerformance7 {
    int testIntAdd5 {5};
    int testIntAdd10 {10};

public:
    void BasePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }

    int8_t BasePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + testIntAdd5) / testIntAdd10;
    }
};

class TestInterfacePerformance8 {
    int testIntAdd5 {5};
    int testIntAdd10 {10};

public:
    void BasePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }

    int8_t BasePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + testIntAdd5) / testIntAdd10;
    }
};

class TestInterfacePerformance9 {
    int testIntAdd5 {5};
    int testIntAdd10 {10};

public:
    void BasePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }

    int8_t BasePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + testIntAdd5) / testIntAdd10;
    }
};

class TestInterfacePerformance10 {
    int testIntAdd5 {5};
    int testIntAdd10 {10};

public:
    void BasePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }

    int8_t BasePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + testIntAdd5) / testIntAdd10;
    }
};

class TestInterfacePerformance11 {
    int testIntAdd5 {5};
    int testIntAdd10 {10};

public:
    void BasePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }

    int8_t BasePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + testIntAdd5) / testIntAdd10;
    }
};

class TestInterfacePerformance12 {
    int testIntAdd5 {5};
    int testIntAdd10 {10};

public:
    void BasePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }

    int8_t BasePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + testIntAdd5) / testIntAdd10;
    }
};

class TestInterfacePerformance13 {
    int testIntAdd5 {5};
    int testIntAdd10 {10};

public:
    void BasePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }

    int8_t BasePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + testIntAdd5) / testIntAdd10;
    }
};

class TestInterfacePerformance14 {
    int testIntAdd5 {5};
    int testIntAdd10 {10};

public:
    void BasePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }

    int8_t BasePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + testIntAdd5) / testIntAdd10;
    }
};

class TestInterfacePerformance15 {
    int testIntAdd5 {5};
    int testIntAdd10 {10};

public:
    void BasePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }

    int8_t BasePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + testIntAdd5) / testIntAdd10;
    }
};

class TestInterfacePerformance16 {
    int testIntAdd5 {5};
    int testIntAdd10 {10};

public:
    void BasePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }

    int8_t BasePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + testIntAdd5) / testIntAdd10;
    }
};

class TestInterfacePerformance17 {
    int testIntAdd5 {5};
    int testIntAdd10 {10};

public:
    void BasePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }

    int8_t BasePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + testIntAdd5) / testIntAdd10;
    }
};

class TestInterfacePerformance18 {
    int testIntAdd5 {5};
    int testIntAdd10 {10};

public:
    void BasePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }

    int8_t BasePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + testIntAdd5) / testIntAdd10;
    }
};

class TestInterfacePerformance19 {
    int testIntAdd5 {5};
    int testIntAdd10 {10};

public:
    void BasePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }

    int8_t BasePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + testIntAdd5) / testIntAdd10;
    }
};

class TestInterfacePerformance20 {
    int testIntAdd5 {5};
    int testIntAdd10 {10};

public:
    void BasePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }

    int8_t BasePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + testIntAdd5) / testIntAdd10;
    }
};

class TestInterfacePerformance21 {
public:
    TestInterfacePerformance21()
    {
        // Don't forget to implement the constructor.
    }

    int32_t geti32_test()
    {
        return INT32_MAX;
    }

    ::ns_alltest::functiontest::Data getStruct_test()
    {
        return {"testStruct", false, INT32_MIN};
    }

    int8_t BasePerformanceFunctionTest2(int8_t param1)
    {
        // 检查结果是否超出 int8_t 的范围
        int32_t result = static_cast<int32_t>(param1) + g_testIntAdd100;
        if (result > INT8_MAX || result < INT8_MIN) {
            taihe::set_error("BasePerformanceFunctionTest2: result exceeds int8_t range");
        }
        // 返回结果
        return static_cast<int8_t>(result);
    }

    int8_t geti8_test_attribute()
    {
        return INT8_MAX;
    }

    int16_t geti16_test_attribute()
    {
        return INT16_MAX;
    }

    int32_t geti32_test_attribute()
    {
        return INT32_MAX;
    }

    int64_t geti64_test_attribute()
    {
        return INT64_MAX;
    }

    float getf32_test_attribute()
    {
        float const getf32TestAttributeRes = 3.14;
        return getf32TestAttributeRes;
    }

    double getf64_test_attribute()
    {
        double const getf64TestAttributeRes = -1.23;
        return getf64TestAttributeRes;
    }

    string getStr_test_attribute()
    {
        return "TESTSTR";
    }

    bool getbool_test_attribute()
    {
        return false;
    }

    array<uint8_t> get_ArrayBuffer_test_attribute()
    {
        array<uint8_t> result = array<uint8_t>::make(g_testIntAdd10);
        std::fill(result.begin(), result.end(), g_testIntAdd10);
        return result;
    }

    optional<int32_t> getOptional_test_attribute()
    {
        int32_t const baseNum = 1000;
        int32_t result = baseNum + g_testIntAdd10;
        if (result > INT32_MAX || result < INT32_MIN) {
            taihe::set_error("getOptional_test_attribute: result exceeds int32_t range");
        }
        return optional<int32_t>::make(result);
    }

    map<int32_t, ::ns_alltest::functiontest::Data> getRecord_test_attribute()
    {
        map<int32_t, ::ns_alltest::functiontest::Data> result;
        ::ns_alltest::functiontest::Data p1 {
            .data1 = "one",
            .data2 = true,
            .data3 = 100,
        };
        int32_t const getRecordTestAttributeKey1 = 100;
        result.emplace(getRecordTestAttributeKey1, p1);
        ::ns_alltest::functiontest::Data p2 {
            .data1 = "two",
            .data2 = false,
            .data3 = 101,
        };
        int32_t const getRecordTestAttributeKey2 = 101;
        result.emplace(getRecordTestAttributeKey2, p2);
        return result;
    }

    ::ns_alltest::functiontest::TestUnionName getUnion_test_attribute()
    {
        return ::ns_alltest::functiontest::TestUnionName::make_value1(g_testIntAdd100);
    }

    array<bool> getArray_test_attribute()
    {
        return array<bool>::make(1, true);
    }

    ::ns_alltest::functiontest::Color getEnum_test_attribute()
    {
        return ::ns_alltest::functiontest::Color::key_t::GREEN;
    }
};

class IbaseInterface1 {
public:
    IbaseInterface1()
    {
        // Don't forget to implement the constructor.
    }

    int32_t BaseTest()
    {
        return INT32_MIN;
    }
};

class IbaseInterface2 {
public:
    IbaseInterface2()
    {
        // Don't forget to implement the constructor.
    }

    string BaseTest1()
    {
        return "TestInterface";
    }

    int32_t BaseTest()
    {
        return INT32_MIN;
    }
};

class IbaseInterface3 {
public:
    IbaseInterface3()
    {
        // Don't forget to implement the constructor.
    }

    bool BaseTest2()
    {
        return true;
    }

    int32_t BaseTest()
    {
        return INT32_MAX;
    }
};

void BaseFunctionTest1()
{
    std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
}

void BaseFunctionTest2(int8_t param1)
{
    std::cout << "NameSpaceImpl: " << __func__ << " param1 " << static_cast<int>(param1) << std::endl;
}

void BaseFunctionTest3(int16_t param1)
{
    std::cout << "NameSpaceImpl: " << __func__ << " param1 " << param1 << std::endl;
}

void BaseFunctionTest4(int32_t param1)
{
    std::cout << "NameSpaceImpl: " << __func__ << " param1 " << param1 << std::endl;
}

void BaseFunctionTest5(int64_t param1)
{
    std::cout << "NameSpaceImpl: " << __func__ << " param1 " << param1 << std::endl;
}

void BaseFunctionTest6(float param1)
{
    std::cout << "NameSpaceImpl: " << __func__ << " param1 " << param1 << std::endl;
}

void BaseFunctionTest7(double param1)
{
    std::cout << "NameSpaceImpl: " << __func__ << " param1 " << param1 << std::endl;
}

void BaseFunctionTest8(string_view param1)
{
    std::cout << "NameSpaceImpl: " << __func__ << " param1 " << param1 << std::endl;
}

void BaseFunctionTest9(bool param1)
{
    std::cout << "NameSpaceImpl: " << __func__ << " param1 " << param1 << std::endl;
}

void BaseFunctionTest10(array_view<int8_t> param1)
{
    // 输出 param1 的内容
    std::cout << "NameSpaceImpl: " << __func__ << " param1 ";
    for (int8_t value : param1) {
        std::cout << static_cast<int>(value) << " ";
    }
    std::cout << std::endl;
}

void BaseFunctionTest11(array_view<int16_t> param1)
{
    // 输出 param1 的内容
    std::cout << "NameSpaceImpl: " << __func__ << " param1 ";
    for (int16_t value : param1) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}

void BaseFunctionTest12(optional_view<int16_t> param1)
{
    if (param1) {
        std::cout << "NameSpaceImpl: " << __func__ << " param1 " << *param1 << std::endl;
    } else {
        std::cout << "NameSpaceImpl: " << __func__ << " param1 is null" << std::endl;
    }
}

void BaseFunctionTest13(optional_view<int64_t> param1)
{
    if (param1) {
        std::cout << "NameSpaceImpl: " << __func__ << " param1 " << *param1 << std::endl;
    } else {
        std::cout << "NameSpaceImpl: " << __func__ << " param1 is null" << std::endl;
    }
}

void BaseFunctionTest14(array_view<uint8_t> param1)
{
    // 输出 param1 的内容
    std::cout << "NameSpaceImpl: " << __func__ << " param1 ";
    for (int16_t value : param1) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}

void BaseFunctionTest15(map_view<string, int32_t> param1)
{
    std::cout << "NameSpaceImpl: " << __func__ << " param1 contents:" << std::endl;
    // 遍历 map_view 并输出 key 和 value
    for (auto const &pair : param1) {
        std::cout << "  Key: " << pair.first << ", Value: " << pair.second << std::endl;
    }
}

int8_t BaseFunctionTest16(int8_t param1)
{
    // 检查结果是否超出 int8_t 的范围
    int32_t result = param1 + g_testIntAdd10;
    if (result > INT8_MAX || result < INT8_MIN) {
        taihe::set_error("BaseFunctionTest16: result exceeds int8_t range");
    }
    // 返回结果
    return static_cast<int8_t>(result);
}

int16_t BaseFunctionTest17(int16_t param1)
{
    // 检查结果是否超出 int16_t 的范围
    int32_t result = param1 * g_testIntAdd10;
    if (result > INT16_MAX || result < INT16_MIN) {
        taihe::set_error("BaseFunctionTest17: result exceeds int16_t range");
    }
    // 返回结果
    return static_cast<int16_t>(result);
}

int32_t BaseFunctionTest18(int32_t param1)
{
    // 检查结果是否超出 int32_t 的范围
    int64_t result = static_cast<int64_t>(param1) * g_testIntAdd100;
    std::cout << "NameSpaceImpl: " << __func__ << " result " << result << std::endl;
    if (result > INT32_MAX || result < INT32_MIN) {
        taihe::set_error("BaseFunctionTest18: result exceeds int32_t range");
    }
    // 返回结果
    return static_cast<int32_t>(result);
}

int64_t BaseFunctionTest19(int64_t param1)
{
    // 检查结果是否超出 int64_t 的范围
    int64_t result = param1 * g_testIntAdd10;
    std::cout << "NameSpaceImpl: " << __func__ << " result " << result << std::endl;
    if (result > INT64_MAX || result < INT64_MIN) {
        taihe::set_error("BaseFunctionTest19: result exceeds int64_t range");
    }
    // 返回结果
    return static_cast<int64_t>(result);
}

float BaseFunctionTest20(float param1)
{
    return param1 + g_testIntAdd100;
}

double BaseFunctionTest21(double param1)
{
    double const baseFunctionTest21Value = 1.01;
    return param1 + baseFunctionTest21Value;
}

string BaseFunctionTest22(string_view param1)
{
    if (param1 == "BaseFunctionTest22") {
        return std::string(param1) + "hello ani";
    } else {
        return param1;
    }
}

bool BaseFunctionTest23(bool param1)
{
    if (param1) {
        return false;
    } else {
        return true;
    }
}

array<int8_t> BaseFunctionTest24(array_view<int8_t> param1)
{
    array<int8_t> result = array<int8_t>::make(param1.size());
    for (std::size_t i = 0; i < param1.size(); i++) {
        result[i] = param1[i] * g_testInter;
    }
    return result;
}

array<int16_t> BaseFunctionTest25(array_view<int16_t> param1)
{
    array<int16_t> result = array<int16_t>::make(param1.size());
    for (std::size_t i = 0; i < param1.size(); i++) {
        result[i] = param1[i] + g_testInter;
    }
    return result;
}

optional<int16_t> BaseFunctionTest26(optional_view<int16_t> param1)
{
    if (param1) {
        // 检查结果是否超出 int16_t 的范围
        std::cout << "NameSpaceImpl: " << __func__ << " param1 " << *param1 << std::endl;
        int32_t result = static_cast<int32_t>(*param1) + g_testIntAdd10;
        std::cout << "NameSpaceImpl: " << __func__ << " result " << result << std::endl;
        if (result > INT16_MAX || result < INT16_MIN) {
            taihe::set_error("BaseFunctionTest26: result exceeds int16_t range");
        }
        // 返回结果
        return optional<int16_t>::make(result);
    } else {
        std::cout << "NameSpaceImpl: " << __func__ << " param1 is null" << std::endl;
        return optional<int16_t>(nullptr);
    }
}

optional<int64_t> BaseFunctionTest27(optional_view<int64_t> param1)
{
    if (param1) {
        // 检查结果是否超出 int64_t 的范围
        std::cout << "NameSpaceImpl: " << __func__ << " param1 " << *param1 << std::endl;
        int64_t result = *param1 + g_testIntAdd10;
        if (result > INT64_MAX || result < INT64_MIN) {
            taihe::set_error("BaseFunctionTest27: result exceeds int16_t range");
        }
        // 返回结果
        return optional<int64_t>::make(result);
    } else {
        std::cout << "NameSpaceImpl: " << __func__ << " param1 is null" << std::endl;
        return optional<int64_t>(nullptr);
    }
}

array<uint8_t> BaseFunctionTest28(array_view<uint8_t> param1)
{
    array<uint8_t> result = array<uint8_t>::make(param1.size());
    for (std::size_t i = 0; i < param1.size(); i++) {
        result[i] = param1[i] * g_testIntAdd10;
    }
    return result;
}

map<string, int32_t> BaseFunctionTest29(map_view<string, int32_t> param1)
{
    map<string, int32_t> m;
    for (std::size_t i = 0; i < param1.size(); ++i) {
        m.emplace("test" + std::to_string(i), g_testIntAdd10 + static_cast<int32_t>(i));
    }
    return m;
}

::ns_alltest::functiontest::Color BaseFunctionTest30(::ns_alltest::functiontest::Color param1)
{
    return (::ns_alltest::functiontest::Color::key_t)(((int)param1.get_key() + 1) % g_testEnum);
}

void BaseFunctionTest31(::ns_alltest::functiontest::Color param1)
{
    std::cout << "NameSpaceImpl: " << __func__ << " param1 " << param1 << std::endl;
}

int16_t BaseFunctionTest32(int8_t param1, int16_t param2, int32_t param3, int64_t param4, bool param8)
{
    if (param8) {
        return param2;
    } else {
        return static_cast<int16_t>(param1);
    }
}

int32_t BaseFunctionTest33(int8_t param1, int16_t param2, int32_t param3, int64_t param4, bool param8)
{
    if (param8) {
        return param3;
    } else {
        return static_cast<int32_t>(param2);
    }
}

int64_t BaseFunctionTest34(int8_t param1, int16_t param2, int32_t param3, int64_t param4, bool param8)
{
    if (param8) {
        return param4;
    } else {
        return static_cast<int64_t>(param2);
    }
}

int8_t BaseFunctionTest35(int8_t param1, int16_t param2, int32_t param3, int64_t param4, bool param8)
{
    if (param8) {
        return static_cast<int8_t>(param1);
    } else {
        return static_cast<int8_t>(param2);
    }
}

array<int32_t> BaseFunctionTest36(int8_t param1, int16_t param2, bool param8, array_view<int8_t> param9,
                                  array_view<int64_t> param10)
{
    if (param8) {
        array<int32_t> result = array<int32_t>::make(param9.size());
        for (std::size_t i = 0; i < param9.size(); i++) {
            result[i] = param9[i] + param1 + param2;
        }
        return result;
    } else {
        array<int32_t> result = array<int32_t>::make(param10.size());
        for (std::size_t i = 0; i < param10.size(); i++) {
            result[i] = static_cast<int32_t>(param10[i]);
        }
        return result;
    }
}

array<int64_t> BaseFunctionTest37(int8_t param1, int16_t param2, int32_t param3, bool param8,
                                  array_view<int64_t> param10)
{
    if (param8) {
        array<int64_t> result = array<int64_t>::make(param10.size());
        for (std::size_t i = 0; i < param10.size(); i++) {
            result[i] = param10[i] + param1 + param2 + param3;
        }
        return result;
    } else {
        array<int64_t> result = array<int64_t>::make(param10.size());
        for (std::size_t i = 0; i < param10.size(); i++) {
            result[i] = param10[i] * g_testIntAdd10;
        }
        return result;
    }
}

string BaseFunctionTest38(int8_t param1, string_view param7, bool param8, array_view<int8_t> param9,
                          array_view<int16_t> param10)
{
    if (param8) {
        return std::string(param7) + std::to_string(param1);
    } else {
        return std::string(param7) + std::to_string(param9[0]) + std::to_string(param10[0]);
    }
}

bool BaseFunctionTest39(int16_t param2, string_view param7, bool param8, array_view<bool> param9,
                        array_view<int32_t> param10)
{
    if (param2 == g_testIntAdd100) {
        return param8;
    } else {
        return false;
    }
}

array<int8_t> BaseFunctionTest40(double param6, string_view param7, bool param8, array_view<int8_t> param9,
                                 array_view<uint8_t> param10)
{
    if (param8) {
        array<int8_t> result = array<int8_t>::make(param9.size());
        for (std::size_t i = 0; i < param9.size(); i++) {
            result[i] = param9[i] * g_testInter;
        }
        return result;
    } else {
        array<int8_t> result = array<int8_t>::make(param9.size());
        for (std::size_t i = 0; i < param9.size(); i++) {
            result[i] = param9[i] + g_testIntAdd10;
        }
        return result;
    }
}

void BaseFunctionTest41(int8_t param1, int16_t param2, int32_t param3, int64_t param4, float param5)
{
    std::cout << "NameSpaceImpl: " << __func__ << " param1 " << param1 << " param2 " << param2 << " param3 " << param3
              << " param4 " << param4 << std::endl;
}

int32_t BaseFunctionTest42_int(int32_t param1, int16_t param2)
{
    std::cout << "NameSpaceImpl: " << __func__ << " param1 " << param1 << " param2 " << param2 << std::endl;
    int64_t result = static_cast<int64_t>(param1) + static_cast<int64_t>(param2);
    std::cout << "NameSpaceImpl: " << __func__ << " result " << result << std::endl;
    if (result > INT32_MAX || result < INT32_MIN) {
        taihe::set_error("BaseFunctionTest42_int: result exceeds int32_t range");
    }
    // 返回结果
    return static_cast<int32_t>(result);
}

int32_t BaseFunctionTest42_container(optional_view<int32_t> param1, map_view<string, int32_t> param2)
{
    if (param1) {
        return *param1 + g_testIntAdd10;
    } else {
        std::cout << "NameSpaceImpl: " << __func__ << " param1 is null" << std::endl;
        return INT32_MIN;
    }
}

int32_t BaseFunctionTest42_void()
{
    return INT32_MAX;
}

void BaseFunctionTest42_struct(::ns_alltest::functiontest::Data const &param1)
{
    std::cout << "NameSpaceImpl: " << __func__ << "data1: " << param1.data1 << std::endl;
    std::cout << "NameSpaceImpl: " << __func__ << "data2: " << param1.data2 << std::endl;
    std::cout << "NameSpaceImpl: " << __func__ << "data3: " << param1.data3 << std::endl;
}

void BaseFunctionTest59(map_view<string, string> param1)
{
    std::cout << "Using begin() and end() for traversal:" << std::endl;
    for (auto it = param1.begin(); it != param1.end(); ++it) {
        auto const &[key, value] = *it;
        std::cout << "Key: " << key << ", Value: " << value << std::endl;
    }
}

bool BaseFunctionTest60(map_view<int64_t, bool> param1)
{
    for (auto const &pair : param1) {
        if (pair.first <= 0) {
            return false;
        }
    }
    return true;
}

::ns_alltest::functiontest::TestNameSpace get_interface_NameSpace()
{
    return make_holder<TestNameSpace, ::ns_alltest::functiontest::TestNameSpace>();
}

::ns_alltest::functiontest::TestInterfacePerformance1 get_interface_performance1()
{
    return make_holder<TestInterfacePerformance1, ::ns_alltest::functiontest::TestInterfacePerformance1>();
}

::ns_alltest::functiontest::TestInterfacePerformance2 get_interface_performance2()
{
    return make_holder<TestInterfacePerformance2, ::ns_alltest::functiontest::TestInterfacePerformance2>();
}

::ns_alltest::functiontest::TestInterfacePerformance3 get_interface_performance3()
{
    return make_holder<TestInterfacePerformance3, ::ns_alltest::functiontest::TestInterfacePerformance3>();
}

::ns_alltest::functiontest::TestInterfacePerformance4 get_interface_performance4()
{
    return make_holder<TestInterfacePerformance4, ::ns_alltest::functiontest::TestInterfacePerformance4>();
}

::ns_alltest::functiontest::TestInterfacePerformance5 get_interface_performance5()
{
    return make_holder<TestInterfacePerformance5, ::ns_alltest::functiontest::TestInterfacePerformance5>();
}

::ns_alltest::functiontest::TestInterfacePerformance6 get_interface_performance6()
{
    return make_holder<TestInterfacePerformance6, ::ns_alltest::functiontest::TestInterfacePerformance6>();
}

::ns_alltest::functiontest::TestInterfacePerformance7 get_interface_performance7()
{
    return make_holder<TestInterfacePerformance7, ::ns_alltest::functiontest::TestInterfacePerformance7>();
}

::ns_alltest::functiontest::TestInterfacePerformance8 get_interface_performance8()
{
    return make_holder<TestInterfacePerformance8, ::ns_alltest::functiontest::TestInterfacePerformance8>();
}

::ns_alltest::functiontest::TestInterfacePerformance9 get_interface_performance9()
{
    return make_holder<TestInterfacePerformance9, ::ns_alltest::functiontest::TestInterfacePerformance9>();
}

::ns_alltest::functiontest::TestInterfacePerformance10 get_interface_performance10()
{
    return make_holder<TestInterfacePerformance10, ::ns_alltest::functiontest::TestInterfacePerformance10>();
}

::ns_alltest::functiontest::TestInterfacePerformance11 get_interface_performance11()
{
    return make_holder<TestInterfacePerformance11, ::ns_alltest::functiontest::TestInterfacePerformance11>();
}

::ns_alltest::functiontest::TestInterfacePerformance12 get_interface_performance12()
{
    return make_holder<TestInterfacePerformance12, ::ns_alltest::functiontest::TestInterfacePerformance12>();
}

::ns_alltest::functiontest::TestInterfacePerformance13 get_interface_performance13()
{
    return make_holder<TestInterfacePerformance13, ::ns_alltest::functiontest::TestInterfacePerformance13>();
}

::ns_alltest::functiontest::TestInterfacePerformance14 get_interface_performance14()
{
    return make_holder<TestInterfacePerformance14, ::ns_alltest::functiontest::TestInterfacePerformance14>();
}

::ns_alltest::functiontest::TestInterfacePerformance15 get_interface_performance15()
{
    return make_holder<TestInterfacePerformance15, ::ns_alltest::functiontest::TestInterfacePerformance15>();
}

::ns_alltest::functiontest::TestInterfacePerformance16 get_interface_performance16()
{
    return make_holder<TestInterfacePerformance16, ::ns_alltest::functiontest::TestInterfacePerformance16>();
}

::ns_alltest::functiontest::TestInterfacePerformance17 get_interface_performance17()
{
    return make_holder<TestInterfacePerformance17, ::ns_alltest::functiontest::TestInterfacePerformance17>();
}

::ns_alltest::functiontest::TestInterfacePerformance18 get_interface_performance18()
{
    return make_holder<TestInterfacePerformance18, ::ns_alltest::functiontest::TestInterfacePerformance18>();
}

::ns_alltest::functiontest::TestInterfacePerformance19 get_interface_performance19()
{
    return make_holder<TestInterfacePerformance19, ::ns_alltest::functiontest::TestInterfacePerformance19>();
}

::ns_alltest::functiontest::TestInterfacePerformance20 get_interface_performance20()
{
    return make_holder<TestInterfacePerformance20, ::ns_alltest::functiontest::TestInterfacePerformance20>();
}

::ns_alltest::functiontest::TestInterfacePerformance21 get_interface_performance21()
{
    return make_holder<TestInterfacePerformance21, ::ns_alltest::functiontest::TestInterfacePerformance21>();
}

::ns_alltest::functiontest::IbaseInterface2 get_interface_IbaseInterface2()
{
    return make_holder<IbaseInterface2, ::ns_alltest::functiontest::IbaseInterface2>();
}

::ns_alltest::functiontest::IbaseInterface3 get_interface_IbaseInterface3()
{
    return make_holder<IbaseInterface3, ::ns_alltest::functiontest::IbaseInterface3>();
}

}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_BaseFunctionTest1(BaseFunctionTest1);
TH_EXPORT_CPP_API_BaseFunctionTest2(BaseFunctionTest2);
TH_EXPORT_CPP_API_BaseFunctionTest3(BaseFunctionTest3);
TH_EXPORT_CPP_API_BaseFunctionTest4(BaseFunctionTest4);
TH_EXPORT_CPP_API_BaseFunctionTest5(BaseFunctionTest5);
TH_EXPORT_CPP_API_BaseFunctionTest6(BaseFunctionTest6);
TH_EXPORT_CPP_API_BaseFunctionTest7(BaseFunctionTest7);
TH_EXPORT_CPP_API_BaseFunctionTest8(BaseFunctionTest8);
TH_EXPORT_CPP_API_BaseFunctionTest9(BaseFunctionTest9);
TH_EXPORT_CPP_API_BaseFunctionTest10(BaseFunctionTest10);
TH_EXPORT_CPP_API_BaseFunctionTest11(BaseFunctionTest11);
TH_EXPORT_CPP_API_BaseFunctionTest12(BaseFunctionTest12);
TH_EXPORT_CPP_API_BaseFunctionTest13(BaseFunctionTest13);
TH_EXPORT_CPP_API_BaseFunctionTest14(BaseFunctionTest14);
TH_EXPORT_CPP_API_BaseFunctionTest15(BaseFunctionTest15);
TH_EXPORT_CPP_API_BaseFunctionTest16(BaseFunctionTest16);
TH_EXPORT_CPP_API_BaseFunctionTest17(BaseFunctionTest17);
TH_EXPORT_CPP_API_BaseFunctionTest18(BaseFunctionTest18);
TH_EXPORT_CPP_API_BaseFunctionTest19(BaseFunctionTest19);
TH_EXPORT_CPP_API_BaseFunctionTest20(BaseFunctionTest20);
TH_EXPORT_CPP_API_BaseFunctionTest21(BaseFunctionTest21);
TH_EXPORT_CPP_API_BaseFunctionTest22(BaseFunctionTest22);
TH_EXPORT_CPP_API_BaseFunctionTest23(BaseFunctionTest23);
TH_EXPORT_CPP_API_BaseFunctionTest24(BaseFunctionTest24);
TH_EXPORT_CPP_API_BaseFunctionTest25(BaseFunctionTest25);
TH_EXPORT_CPP_API_BaseFunctionTest26(BaseFunctionTest26);
TH_EXPORT_CPP_API_BaseFunctionTest27(BaseFunctionTest27);
TH_EXPORT_CPP_API_BaseFunctionTest28(BaseFunctionTest28);
TH_EXPORT_CPP_API_BaseFunctionTest29(BaseFunctionTest29);
TH_EXPORT_CPP_API_BaseFunctionTest30(BaseFunctionTest30);
TH_EXPORT_CPP_API_BaseFunctionTest31(BaseFunctionTest31);
TH_EXPORT_CPP_API_BaseFunctionTest32(BaseFunctionTest32);
TH_EXPORT_CPP_API_BaseFunctionTest33(BaseFunctionTest33);
TH_EXPORT_CPP_API_BaseFunctionTest34(BaseFunctionTest34);
TH_EXPORT_CPP_API_BaseFunctionTest35(BaseFunctionTest35);
TH_EXPORT_CPP_API_BaseFunctionTest36(BaseFunctionTest36);
TH_EXPORT_CPP_API_BaseFunctionTest37(BaseFunctionTest37);
TH_EXPORT_CPP_API_BaseFunctionTest38(BaseFunctionTest38);
TH_EXPORT_CPP_API_BaseFunctionTest39(BaseFunctionTest39);
TH_EXPORT_CPP_API_BaseFunctionTest40(BaseFunctionTest40);
TH_EXPORT_CPP_API_BaseFunctionTest41(BaseFunctionTest41);
TH_EXPORT_CPP_API_BaseFunctionTest42_int(BaseFunctionTest42_int);
TH_EXPORT_CPP_API_BaseFunctionTest42_container(BaseFunctionTest42_container);
TH_EXPORT_CPP_API_BaseFunctionTest42_void(BaseFunctionTest42_void);
TH_EXPORT_CPP_API_BaseFunctionTest42_struct(BaseFunctionTest42_struct);
TH_EXPORT_CPP_API_BaseFunctionTest59(BaseFunctionTest59);
TH_EXPORT_CPP_API_BaseFunctionTest60(BaseFunctionTest60);
TH_EXPORT_CPP_API_get_interface_NameSpace(get_interface_NameSpace);
TH_EXPORT_CPP_API_get_interface_performance1(get_interface_performance1);
TH_EXPORT_CPP_API_get_interface_performance2(get_interface_performance2);
TH_EXPORT_CPP_API_get_interface_performance3(get_interface_performance3);
TH_EXPORT_CPP_API_get_interface_performance4(get_interface_performance4);
TH_EXPORT_CPP_API_get_interface_performance5(get_interface_performance5);
TH_EXPORT_CPP_API_get_interface_performance6(get_interface_performance6);
TH_EXPORT_CPP_API_get_interface_performance7(get_interface_performance7);
TH_EXPORT_CPP_API_get_interface_performance8(get_interface_performance8);
TH_EXPORT_CPP_API_get_interface_performance9(get_interface_performance9);
TH_EXPORT_CPP_API_get_interface_performance10(get_interface_performance10);
TH_EXPORT_CPP_API_get_interface_performance11(get_interface_performance11);
TH_EXPORT_CPP_API_get_interface_performance12(get_interface_performance12);
TH_EXPORT_CPP_API_get_interface_performance13(get_interface_performance13);
TH_EXPORT_CPP_API_get_interface_performance14(get_interface_performance14);
TH_EXPORT_CPP_API_get_interface_performance15(get_interface_performance15);
TH_EXPORT_CPP_API_get_interface_performance16(get_interface_performance16);
TH_EXPORT_CPP_API_get_interface_performance17(get_interface_performance17);
TH_EXPORT_CPP_API_get_interface_performance18(get_interface_performance18);
TH_EXPORT_CPP_API_get_interface_performance19(get_interface_performance19);
TH_EXPORT_CPP_API_get_interface_performance20(get_interface_performance20);
TH_EXPORT_CPP_API_get_interface_performance21(get_interface_performance21);
TH_EXPORT_CPP_API_get_interface_IbaseInterface2(get_interface_IbaseInterface2);
TH_EXPORT_CPP_API_get_interface_IbaseInterface3(get_interface_IbaseInterface3);
// NOLINTEND