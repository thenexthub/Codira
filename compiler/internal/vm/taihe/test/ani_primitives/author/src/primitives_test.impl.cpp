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
#include "primitives_test.impl.hpp"

#include <iomanip>
#include <iostream>

#include "primitives_test.PrimitivesVoid.proj.2.hpp"
#include "stdexcept"
#include "taihe/optional.hpp"
#include "taihe/runtime.hpp"
#include "taihe/string.hpp"
// Please delete this include when you implement
using namespace taihe;

namespace {
int testInt_add10_ {10};
int testInt_add15_ {15};
int testInt_add32_ {32};
float testFloat_1_ {1.2};
double testDouble_2_ {3.14159};
double testDouble_1_ {3.1};

class PrimitivesVoid {
public:
    void TestBaseFunc1()
    {
        std::cout << "TestBaseFunc1 is true " << std::endl;
    }

    void TestBaseFunc2(int32_t option1, bool option2)
    {
        if (option2) {
            std::cout << "TestBaseFunc2 is option1  " << option1 << std::endl;
            std::cout << "TestBaseFunc2 is option2  " << option2 << std::endl;
        } else {
            std::cout << "TestBaseFunc2 is option1  " << option1 << std::endl;
            std::cout << "TestBaseFunc2 is option2  " << option2 << std::endl;
        }
    }

    void TestBaseFunc3(int32_t option1, int64_t option2)
    {
        std::cout << "TestBaseFunc3 is option1  " << option1 << std::endl;
        std::cout << "TestBaseFunc3 is option2  " << option2 << std::endl;
    }

    void TestBaseFunc4(int32_t option1, string_view option2)
    {
        std::cout << "TestBaseFunc4 is option1  " << option1 << std::endl;
        std::cout << "TestBaseFunc4 is option2  " << option2 << std::endl;
    }

    void TestBaseFunc5(int64_t option1, bool option2)
    {
        if (option2) {
            std::cout << "TestBaseFunc5 is option1  " << option1 << std::endl;
            std::cout << "TestBaseFunc5 is option2  " << option2 << std::endl;
        } else {
            std::cout << "TestBaseFunc5 is option1  " << option1 << std::endl;
            std::cout << "TestBaseFunc5 is option2  " << option2 << std::endl;
        }
    }

    void TestBaseFunc6(int64_t option1, float option2)
    {
        std::cout << "TestBaseFunc6 is option1  " << option1 << std::endl;
        std::cout << "TestBaseFunc6 is option2  " << option2 << std::endl;
    }

    void TestBaseFunc7(int64_t option1, double option2)
    {
        std::cout << "TestBaseFunc7 is option1  " << option1 << std::endl;
        std::cout << "TestBaseFunc7 is option2  " << option2 << std::endl;
    }

    void TestBaseFunc8(float option1, bool option2)
    {
        if (option2) {
            std::cout << "TestBaseFunc8 is option1  " << option1 << std::endl;
            std::cout << "TestBaseFunc8 is option2  " << option2 << std::endl;
        } else {
            std::cout << "TestBaseFunc8 is option1  " << option1 << std::endl;
            std::cout << "TestBaseFunc8 is option2  " << option2 << std::endl;
        }
    }

    void TestBaseFunc9(float option1, string_view option2)
    {
        std::cout << "TestBaseFunc9 is option1  " << option1 << std::endl;
        std::cout << "TestBaseFunc9 is option2  " << option2 << std::endl;
    }

    void TestBaseFunc10(double option1, string_view option2)
    {
        std::cout << "TestBaseFunc10 is option1  " << option1 << std::endl;
        std::cout << "TestBaseFunc10 is option2  " << option2 << std::endl;
    }

    void TestBaseFunc11(double option1, bool option2)
    {
        if (option1) {
            std::cout << "TestBaseFunc11 is option1  " << option1 << std::endl;
            std::cout << "TestBaseFunc11 is option2  " << option2 << std::endl;
        } else {
            std::cout << "TestBaseFunc11 is option1  " << option1 << std::endl;
            std::cout << "TestBaseFunc11 is option2  " << option2 << std::endl;
        }
    }

    void TestBaseFunc12(optional_view<int32_t> option1, optional_view<int64_t> option2)
    {
        if (option1) {
            std::cout << *option1 << std::endl;
        } else if (option2) {
            std::cout << *option2 << std::endl;
        } else {
            std::cout << "Null" << std::endl;
        }
    }

    void TestBaseFunc13(optional_view<float> option1, optional_view<double> option2)
    {
        if (option1) {
            std::cout << *option1 << std::endl;
        } else if (option2) {
            std::cout << *option2 << std::endl;
        } else {
            std::cout << "Null" << std::endl;
        }
    }

    void TestBaseFunc14(optional_view<string> option1, optional_view<bool> option2)
    {
        if (option1) {
            std::cout << *option1 << std::endl;
        } else if (option2) {
            std::cout << *option2 << std::endl;
        } else {
            std::cout << "Null" << std::endl;
        }
    }

    void TestBaseFunc15(optional_view<int16_t> option1, optional_view<int64_t> option2)
    {
        if (option1) {
            std::cout << *option1 << std::endl;
        } else if (option2) {
            std::cout << *option2 << std::endl;
        } else {
            std::cout << "Null" << std::endl;
        }
    }

    void TestBaseFunc16(int8_t option1, int16_t option2)
    {
        std::cout << "TestBaseFunc16 is option1  " << static_cast<int>(option1) << std::endl;
        std::cout << "TestBaseFunc16 is option2  " << static_cast<int>(option2) << std::endl;
    }

    void TestBaseFunc17(array_view<int32_t> option1, array_view<int8_t> option2)
    {
        // 输出 option1 的内容
        std::cout << "TestBaseFunc17 option1: ";
        for (int32_t value : option1) {
            std::cout << value << " ";
        }
        std::cout << std::endl;

        // 输出 option2 的内容
        std::cout << "TestBaseFunc17 option2: ";
        for (int8_t value : option2) {
            std::cout << (int)value << " ";
        }
        std::cout << std::endl;
    }

    void TestBaseFunc18(array_view<int16_t> option1, array_view<int64_t> option2)
    {
        // 输出 option1 的内容
        std::cout << "TestBaseFunc18 option1: ";
        for (int16_t value : option1) {
            std::cout << value << " ";
        }
        std::cout << std::endl;

        // 输出 option2 的内容
        std::cout << "TestBaseFunc18 option2: ";
        for (int64_t value : option2) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }

    void TestBaseFunc19(array_view<float> option1, array_view<double> option2)
    {
        // 输出 option1 的内容
        std::cout << "TestBaseFunc19 option1: ";
        for (float value : option1) {
            std::cout << value << " ";
        }
        std::cout << std::endl;

        // 输出 option2 的内容
        std::cout << "TestBaseFunc19 option2: ";
        // 位小数
        for (double value : option2) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }

    void TestBaseFunc20(array_view<bool> option1, array_view<string> option2)
    {
        // 输出 option1 的内容
        std::cout << "TestBaseFunc20 option1: ";
        for (bool value : option1) {
            std::cout << value << " ";
        }
        std::cout << std::endl;

        // 输出 option2 的内容
        std::cout << "TestBaseFunc20 option2: ";
        for (string value : option2) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }
};

class PrimitivesBooleanClass {
public:
};

class PrimitivesBoolean {
    int testInt_65535_ {65535};

public:
    void TestBaseBoolFunc1(bool option1)
    {
        if (option1) {
            std::cout << "TestBaseBoolFunc1 is true " << option1 << std::endl;
        } else {
            std::cout << "TestBaseBoolFunc1 is false  " << option1 << std::endl;
        }
    }

    int32_t TestBaseBoolFunc2(bool option1)
    {
        if (option1) {
            return testInt_65535_;
        } else {
            return 0;
        }
    }

    bool TestBaseBoolFunc3(bool option1)
    {
        if (option1) {
            return false;
        } else {
            return true;
        }
    }

    bool TestBaseBoolFunc4(optional_view<bool> option1)
    {
        if (option1) {
            return false;
        } else {
            return true;
        }
    }

    bool TestBaseBoolFunc5(bool option1)
    {
        if (option1) {
            return true;
        } else {
            return false;
        }
    }

    bool TestBaseBoolFunc7(map_view<string, bool> option1)
    {
        for (auto const &pair : option1) {
            if (pair.second) {
                return false;
            }
        }
        return true;
    }

    bool getBoolTest()
    {
        return true;
    }
};

class PrimitivesInteger {
public:
    int8_t TestBaseIntegerFunc1(int8_t option1)
    {
        if (option1 == -1) {
            taihe::set_error("out of range The i8 maximum value is 127 and minnum values is -128");
            return -1;
        }
        return option1 + 1;
    }

    int8_t TestBaseIntegerFunc2(int8_t option1, int16_t option2)
    {
        if (option1 == -1) {
            taihe::set_error("out of range The i8 maximum value is 127 and minnum values is -128");
            return -1;
        }
        if (option2 == -1) {
            taihe::set_error(
                "out of range The i16 maximum value is 32767 and minnum values is "
                "-32768");
            return -1;
        }
        return option1 + option2;
    }

    void TestBaseIntegerFunc3(int8_t option1, int16_t option2)
    {
        std::cout << "TestBaseIntegerFunc3 is option1  " << static_cast<int>(option1) << std::endl;
        std::cout << "TestBaseIntegerFunc3 is option2  " << option2 << std::endl;
    }

    int16_t TestBaseIntegerFunc4(int8_t option1, int16_t option2)
    {
        return option1 + option2;
    }

    int8_t TestBaseIntegerFunc5(int8_t option1, int32_t option2)
    {
        return option1 + option2;
    }

    int32_t TestBaseIntegerFunc6(int8_t option1, int32_t option2)
    {
        return option1 + option2;
    }

    void TestBaseIntegerFunc7(int8_t option1, int32_t option2)
    {
        if (option2 == -1) {
            taihe::set_error(
                "out of range The i32 maximum value is 2147483647 and minnum values "
                "is -2147483648");
        }
        std::cout << "TestBaseIntegerFunc7 is option1  " << static_cast<int>(option1) << std::endl;
        std::cout << "TestBaseIntegerFunc7 is option2  " << option2 << std::endl;
    }

    int64_t TestBaseIntegerFunc8(int8_t option1, int64_t option2)
    {
        if (option2 == -1) {
            taihe::set_error(
                "out of range The i64 maximum value is 9223372036854775807 and "
                "minnum values is -9223372036854775808");
            return -1;
        }
        return option2 - option1;
    }

    int8_t TestBaseIntegerFunc9(int8_t option1, int64_t option2)
    {
        if (option1 > INT8_MAX || option1 < INT8_MIN) {
            taihe::set_error("out of range The i8 maximum value is 127 and minnum values is -128");
            return -1;
        }
        if (option2 > INT64_MAX || option2 < INT64_MIN) {
            taihe::set_error(
                "out of range The i64 maximum value is 9223372036854775807 and "
                "minnum values is -9223372036854775808");
            return -1;
        }
        return option2 - option1;
    }

    float TestBaseIntegerFunc10(int8_t option1, float option2)
    {
        return option1 + option2;
    }

    int8_t TestBaseIntegerFunc11(int8_t option1, float option2)
    {
        return option1 + option2;
    }

    double TestBaseIntegerFunc12(int8_t option1, double option2)
    {
        return option1 + option2;
    }

    int8_t TestBaseIntegerFunc13(int8_t option1, int64_t option2)
    {
        return option1 + option2;
    }

    string TestBaseIntegerFunc14(int8_t option1, string_view option2)
    {
        if (option2 == "TestBaseIntegerFunc14") {
            return std::string(option2) + std::to_string(option1);
        } else {
            return std::string(option2);
        }
    }

    int8_t TestBaseIntegerFunc15(int8_t option1, string_view option2)
    {
        if (option2 == "TestBaseIntegerFunc15") {
            return option1 + testInt_add10_;
        } else {
            return option1;
        }
    }

    bool TestBaseIntegerFunc16(int8_t option1, bool option2)
    {
        if (option2) {
            return true;
        } else {
            return false;
        }
    }

    int8_t TestBaseIntegerFunc17(int8_t option1, bool option2)
    {
        if (option2) {
            return option1 + 1;
        } else {
            return option1;
        }
    }

    int16_t TestBaseIntegerFunc18(int16_t option1)
    {
        // 检查结果是否超出 int16_t 的范围
        int32_t result = static_cast<int32_t>(option1) * testInt_add10_;  // 使用 int32_t 避免溢出
        if (result > INT16_MAX || result < INT16_MIN) {
            taihe::set_error("TestBaseIntegerFunc18: result exceeds int16_t range");
        }
        // 返回结果
        return static_cast<int16_t>(result);
    }

    void TestBaseIntegerFunc19(int16_t option1)
    {
        std::cout << "TestBaseIntegerFunc19 is option1  " << option1 << std::endl;
    }

    int16_t TestBaseIntegerFunc20(int16_t option1, int32_t option2)
    {
        // 检查结果是否超出 int16_t 的范围
        int32_t result = static_cast<int32_t>(option1) + option2;  // 使用 int32_t 避免溢出
        if (result > INT16_MAX || result < INT16_MIN) {
            taihe::set_error("TestBaseIntegerFunc20: result exceeds int16_t range");
        }
        // 返回结果
        return static_cast<int16_t>(result);
    }

    int16_t TestBaseIntegerFunc21(int16_t option1, int64_t option2)
    {
        // 检查结果是否超出 int16_t 的范围
        int64_t result = static_cast<int64_t>(option1) + option2;
        if (result > INT16_MAX || result < INT16_MIN) {
            taihe::set_error("TestBaseIntegerFunc21: result exceeds int16_t range");
        }
        // 返回结果
        return static_cast<int16_t>(result);
    }

    int32_t TestBaseIntegerFunc22(int32_t option1)
    {
        // 检查结果是否超出 int32_t 的范围
        int64_t result = static_cast<int32_t>(option1) * 100;
        if (result > INT32_MAX || result < INT32_MIN) {
            taihe::set_error("TestBaseIntegerFunc22: result exceeds int32_t range");
        }
        // 返回结果
        return static_cast<int32_t>(result);
    }

    void TestBaseIntegerFunc23(int32_t option1)
    {
        if (option1 > INT32_MAX || option1 < INT32_MIN) {
            taihe::set_error("TestBaseIntegerFunc23: result exceeds int32_t range");
        }
        std::cout << "TestBaseIntegerFunc23 is option1  " << option1 << std::endl;
    }

    int32_t TestBaseIntegerFunc24(int32_t option1, int64_t option2)
    {
        // 检查结果是否超出 int32_t 的范围
        int64_t result = static_cast<int64_t>(option1) + option2;
        if (result > INT32_MAX || result < INT32_MIN) {
            taihe::set_error("TestBaseIntegerFunc24: result exceeds int32_t range");
        }
        // 返回结果
        return static_cast<int32_t>(result);
    }

    int32_t TestBaseIntegerFunc25(int32_t option1, int8_t option2)
    {
        // 检查结果是否超出 int32_t 的范围
        int32_t result = static_cast<int32_t>(option2) + option1;
        if (result > INT32_MAX || result < INT32_MIN) {
            taihe::set_error("TestBaseIntegerFunc25: result exceeds int32_t range");
        }
        // 返回结果
        return static_cast<int32_t>(result);
    }

    int64_t TestBaseIntegerFunc26(int64_t option1)
    {
        // 检查结果是否超出 int32_t 的范围
        int64_t result = option1 * 100;
        if (result > INT64_MAX || result < INT64_MIN) {
            taihe::set_error("TestBaseIntegerFunc25: result exceeds int64_t range");
        }
        // 返回结果
        return static_cast<int64_t>(result);
    }

    void TestBaseIntegerFunc27(int64_t option1)
    {
        std::cout << "TestBaseIntegerFunc27 is option1  " << option1 << std::endl;
    }

    string TestBaseIntegerFunc28(int64_t option1, string_view option2)
    {
        if (option2 == "TestBaseIntegerFunc28") {
            return std::string(option2) + std::to_string(option1);
        } else {
            return std::string(option2);
        }
    }

    int64_t TestBaseIntegerFunc29(int64_t option1, string_view option2)
    {
        if (option2 == "TestBaseIntegerFunc29") {
            int64_t result = option1 * testInt_add10_;
            if (result > INT64_MAX || result < INT64_MIN) {
                taihe::set_error("TestBaseIntegerFunc29: result exceeds int32_t range");
            }
            // 返回结果
            return static_cast<int64_t>(result);
        } else {
            return option1;
        }
    }

    float TestBaseIntegerFunc30(float option1)
    {
        return option1 + 1.0;
    }

    void TestBaseIntegerFunc31(float option1)
    {
        std::cout << std::fixed << std::setprecision(6);  // float 保留 6 位小数
        std::cout << "TestBaseIntegerFunc31 is option1  " << option1 << std::endl;
    }

    float TestBaseIntegerFunc32(float option1, double option2)
    {
        return option1 + option2;
    }

    double TestBaseIntegerFunc33(float option1, double option2)
    {
        double result = static_cast<double>(option1) + option2;
        // 打印调试信息，保留 6 位小数
        std::cout << std::fixed << std::setprecision(6);
        std::cout << "Debug: option1 = " << option1 << ", option2 = " << option2 << ", result = " << result
                  << std::endl;
        return result;
    }

    double TestBaseIntegerFunc34(double option1)
    {
        return option1 + 1;
    }

    void TestBaseIntegerFunc35(double option1)
    {
        std::cout << std::fixed << std::setprecision(testInt_add15_);  // float 保留 6 位小数
        std::cout << "TestBaseIntegerFunc35 is option1  " << option1 << std::endl;
    }

    int8_t getI8testattribute()
    {
        return INT8_MAX;
    }

    int16_t getI16testattribute()
    {
        return INT16_MIN;
    }

    int32_t getI32testattribute()
    {
        return INT32_MAX;
    }

    int64_t getI64testattribute()
    {
        return INT64_MAX;
    }

    float getf32testattribute()
    {
        float const getf32TestAttributeValue = 3.14;
        return getf32TestAttributeValue;
    }

    double getf64testattribute()
    {
        double const getf64TestAttributeValue = 123.45678;
        return getf64TestAttributeValue;
    }
};

static void parseOption(::primitives_test::Foo const &opt)
{
    std::cout << "num: " << opt.testNum << std::endl;
    std::cout << "str: " << opt.testStr << std::endl;
}

int32_t Multiply(int32_t a, int32_t b)
{
    return a * b;
}

bool BaseCFunc(int32_t testBoolean)
{
    if (testBoolean == testInt_add10_) {
        return true;
    } else {
        return false;
    }
}

void BaseAFunc(bool testBoolean)
{
    if (testBoolean) {
        std::cout << "testBoolean is true " << testBoolean << std::endl;
    } else {
        std::cout << "testBoolean is false  " << testBoolean << std::endl;
    }
}

bool BaseBFunc(bool testBoolean)
{
    if (testBoolean) {
        return false;
    } else {
        return true;
    }
}

bool BaseDFunc(string_view testBoolean)
{
    if (testBoolean == "test123") {
        return true;
    } else {
        return false;
    }
}

string BaseEFunc(::primitives_test::Foo const &b)
{
    parseOption(b);
    return "success";
}

string BaseHFunc(int32_t a, int64_t b)
{
    int64_t sum = a + b;
    return std::to_string(sum);
}

string BaseIFunc(double a, float b)
{
    double result = static_cast<double>(b) + a;
    std::cout << "BaseIFunc is true " << result << std::endl;
    return std::to_string(result);
}

float BaseFunc1(float b)
{
    return b + 1.0;
}

void BaseFunc2(float b)
{
    if (b == testFloat_1_) {
        std::cout << "BaseFunc2 is true " << b << std::endl;
    } else {
        std::cout << "BaseFunc2 is false  " << b << std::endl;
    }
}

double BaseFunc3(float a, double b)
{
    return static_cast<double>(a) + b;
}

double BaseFunc4(double b)
{
    return b + 1.0;
}

void BaseFunc5(double b)
{
    if (b == testDouble_2_) {
        std::cout << "BaseFunc5 is true " << b << std::endl;
    } else {
        std::cout << "BaseFunc5 is false  " << b << std::endl;
    }
}

void BaseFunc6(string_view a)
{
    if (a == "TestBaseFunc6") {
        std::cout << "BaseFunc6 is true " << a << std::endl;
    } else {
        std::cout << "BaseFunc6 is false  " << a << std::endl;
    }
}

string BaseFunc7(string_view a)
{
    if (a == "TestbaseFunc7") {
        return std::string(a);  // 返回 a
    } else {
        return std::string(a) + "false";  // 返回 a + "false";
    }
}

string BaseFunc8(string_view a, int32_t b)
{
    if (a == "TestBaseFunc8") {
        return std::string(a) + std::to_string(b);  // 返回 a + b（b 转换为字符串）
    } else {
        return std::string(a);  // 返回 a
    }
}

void BaseFunc9(string_view a, int32_t b, int64_t c, bool d, float e)
{
    if (a == "TestBaseFunc9") {
        std::cout << "str: " << a << std::endl;
    } else if (b == testInt_add32_) {
        std::cout << "int32: " << b << std::endl;
    } else if (c == INT64_MAX) {
        std::cout << "int64: " << c << std::endl;
    } else if (d) {
        std::cout << "boolean: " << d << std::endl;
    } else if (e == testDouble_1_) {
        std::cout << "testFloat: " << e << std::endl;
    } else {
        std::cout << "testError: " << std::endl;
    }
}

void BaseFunc10()
{
    std::cout << "BaseFunc10 is true " << std::endl;
}

void BaseFunc11(int32_t a, bool b)
{
    if (b) {
        std::cout << "BaseFunc11 is a  " << a << std::endl;
        std::cout << "BaseFunc11 is b  " << b << std::endl;
    } else {
        std::cout << "BaseFunc11 is a  " << a << std::endl;
        std::cout << "BaseFunc11 is b  " << b << std::endl;
    }
}

void BaseFunc12(int32_t a, int64_t b)
{
    std::cout << "BaseFunc12 is a  " << a << std::endl;
    std::cout << "BaseFunc12 is b  " << b << std::endl;
}

void BaseFunc13(int32_t a, string_view b)
{
    std::cout << "BaseFunc13 is a  " << a << std::endl;
    std::cout << "BaseFunc13 is b  " << b << std::endl;
}

void BaseFunc14(int64_t a, bool b)
{
    if (b) {
        std::cout << "BaseFunc14 is a  " << a << std::endl;
        std::cout << "BaseFunc14 is b  " << b << std::endl;
    } else {
        std::cout << "BaseFunc14 is a  " << a << std::endl;
        std::cout << "BaseFunc14 is b  " << b << std::endl;
    }
}

void BaseFunc15(int64_t a, float b)
{
    std::cout << "BaseFunc15 is a  " << a << std::endl;
    std::cout << "BaseFunc15 is b  " << b << std::endl;
}

void BaseFunc16(int64_t a, double b)
{
    std::cout << "BaseFunc16 is a  " << a << std::endl;
    std::cout << "BaseFunc16 is b  " << b << std::endl;
}

void BaseFunc17(float a, bool b)
{
    if (b) {
        std::cout << "BaseFunc17 is a  " << a << std::endl;
        std::cout << "BaseFunc17 is b  " << b << std::endl;
    } else {
        std::cout << "BaseFunc17 is a  " << a << std::endl;
        std::cout << "BaseFunc17 is b  " << b << std::endl;
    }
}

void BaseFunc18(float a, string_view b)
{
    std::cout << "BaseFunc18 is a  " << a << std::endl;
    std::cout << "BaseFunc18 is b  " << b << std::endl;
}

void BaseFunc19(double a, string_view b)
{
    std::cout << "BaseFunc19 is a  " << a << std::endl;
    std::cout << "BaseFunc19 is b  " << b << std::endl;
}

void BaseFunc20(double a, bool b)
{
    if (b) {
        std::cout << "BaseFunc20 is a  " << a << std::endl;
        std::cout << "BaseFunc20 is b  " << b << std::endl;
    } else {
        std::cout << "BaseFunc20 is a  " << a << std::endl;
        std::cout << "BaseFunc20 is b  " << b << std::endl;
    }
}

void BaseFunc21(optional_view<int32_t> option1, optional_view<int64_t> option2)
{
    if (option1) {
        std::cout << *option1 << std::endl;
    } else if (option2) {
        std::cout << *option2 << std::endl;
    } else {
        std::cout << "Null" << std::endl;
    }
}

void BaseFunc22(optional_view<float> option1, optional_view<double> option2)
{
    if (option1) {
        std::cout << *option1 << std::endl;
    } else if (option2) {
        std::cout << *option2 << std::endl;
    } else {
        std::cout << "Null" << std::endl;
    }
}

void BaseFunc23(optional_view<string> option1, optional_view<bool> option2)
{
    if (option1) {
        std::cout << *option1 << std::endl;
    } else if (option2) {
        std::cout << *option2 << std::endl;
    } else {
        std::cout << "Null" << std::endl;
    }
}

void BaseFunc24(optional_view<int16_t> option1, optional_view<int64_t> option2)
{
    if (option1) {
        std::cout << *option1 << std::endl;
    } else if (option2) {
        std::cout << *option2 << std::endl;
    } else {
        std::cout << "Null" << std::endl;
    }
}

::primitives_test::PrimitivesVoid get_interface()
{
    return make_holder<PrimitivesVoid, ::primitives_test::PrimitivesVoid>();
}

::primitives_test::PrimitivesBoolean get_interface_bool()
{
    return make_holder<PrimitivesBoolean, ::primitives_test::PrimitivesBoolean>();
}

bool TestBaseBoolFunc6()
{
    return false;
}

::primitives_test::PrimitivesInteger get_interface_interger()
{
    return make_holder<PrimitivesInteger, ::primitives_test::PrimitivesInteger>();
}

}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_Multiply(Multiply);
TH_EXPORT_CPP_API_BaseCFunc(BaseCFunc);
TH_EXPORT_CPP_API_BaseAFunc(BaseAFunc);
TH_EXPORT_CPP_API_BaseBFunc(BaseBFunc);
TH_EXPORT_CPP_API_BaseDFunc(BaseDFunc);
TH_EXPORT_CPP_API_BaseEFunc(BaseEFunc);
TH_EXPORT_CPP_API_BaseHFunc(BaseHFunc);
TH_EXPORT_CPP_API_BaseIFunc(BaseIFunc);
TH_EXPORT_CPP_API_BaseFunc1(BaseFunc1);
TH_EXPORT_CPP_API_BaseFunc2(BaseFunc2);
TH_EXPORT_CPP_API_BaseFunc3(BaseFunc3);
TH_EXPORT_CPP_API_BaseFunc4(BaseFunc4);
TH_EXPORT_CPP_API_BaseFunc5(BaseFunc5);
TH_EXPORT_CPP_API_BaseFunc6(BaseFunc6);
TH_EXPORT_CPP_API_BaseFunc7(BaseFunc7);
TH_EXPORT_CPP_API_BaseFunc8(BaseFunc8);
TH_EXPORT_CPP_API_BaseFunc9(BaseFunc9);
TH_EXPORT_CPP_API_BaseFunc10(BaseFunc10);
TH_EXPORT_CPP_API_BaseFunc11(BaseFunc11);
TH_EXPORT_CPP_API_BaseFunc12(BaseFunc12);
TH_EXPORT_CPP_API_BaseFunc13(BaseFunc13);
TH_EXPORT_CPP_API_BaseFunc14(BaseFunc14);
TH_EXPORT_CPP_API_BaseFunc15(BaseFunc15);
TH_EXPORT_CPP_API_BaseFunc16(BaseFunc16);
TH_EXPORT_CPP_API_BaseFunc17(BaseFunc17);
TH_EXPORT_CPP_API_BaseFunc18(BaseFunc18);
TH_EXPORT_CPP_API_BaseFunc19(BaseFunc19);
TH_EXPORT_CPP_API_BaseFunc20(BaseFunc20);
TH_EXPORT_CPP_API_BaseFunc21(BaseFunc21);
TH_EXPORT_CPP_API_BaseFunc22(BaseFunc22);
TH_EXPORT_CPP_API_BaseFunc23(BaseFunc23);
TH_EXPORT_CPP_API_BaseFunc24(BaseFunc24);
TH_EXPORT_CPP_API_get_interface(get_interface);
TH_EXPORT_CPP_API_get_interface_bool(get_interface_bool);
TH_EXPORT_CPP_API_TestBaseBoolFunc6(TestBaseBoolFunc6);
TH_EXPORT_CPP_API_get_interface_interger(get_interface_interger);
// NOLINTEND