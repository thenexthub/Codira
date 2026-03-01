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
#include "inner.impl.hpp"

#include "inner.Color.proj.1.hpp"
#include "inner.ErrorResponse.proj.1.hpp"
#include "inner.Mystruct.proj.1.hpp"
#include "inner.Test1.proj.2.hpp"
#include "inner.Test20.proj.2.hpp"
#include "inner.TestA.proj.2.hpp"
#include "inner.TestInterface.proj.2.hpp"
#include "inner.union_primitive.proj.1.hpp"
#include "stdexcept"
#include "taihe/array.hpp"
#include "taihe/map.hpp"
#include "taihe/string.hpp"

// Please delete <stdexcept> include when you implement
using namespace taihe;

namespace {
class TestInterface {
public:
    void Noparam_noreturn() {}

    void Primitives_noreturn(int8_t a) {}

    void Primitives_noreturn1(int16_t a) {}

    void Primitives_noreturn2(int32_t a) {}

    void Primitives_noreturn3(float a) {}

    void Primitives_noreturn4(double a) {}

    void Primitives_noreturn5(bool a) {}

    void Primitives_noreturn6(string_view a) {}

    void Primitives_noreturn7(int64_t a) {}

    void Primitives_noreturn8(int8_t a) {}

    void Primitives_noreturn9(int32_t a) {}

    int32_t Primitives_return(int32_t a)
    {
        return 1;
    }

    void Containers_noreturn1(array_view<int8_t> a) {}

    void Containers_noreturn3(array_view<uint8_t> a) {}

    void Containers_noreturn2(::inner::union_primitive const &a) {}

    void Containers_noreturn4(::inner::Color a) {}

    void Containers_noreturn5(map_view<string, int32_t> a) {}

    string Containers_return(::inner::union_primitive const &a)
    {
        return "containers_return";
    }

    ::inner::ErrorResponse Func_ErrorResponse()
    {
        return {true, 10000, "test58"};
    }

    void OverloadFunc_i8(int8_t a, int8_t b) {}

    string OverloadFunc_i16(array_view<int8_t> a, array_view<uint8_t> b)
    {
        return "overload array";
    }

    void OverloadFunc_i32() {}

    ::inner::Mystruct OverloadFunc_f32(::inner::Mystruct const &a)
    {
        return a;
    }

    int8_t i8 = -128;           // 范围: -128 到 127
    int16_t i16 = -32768;       // 16 位有符号整数，范围: -32,768 到 32,767
    int32_t i32 = -2147483648;  // 32 位有符号整数，范围: -2,147,483,648 到 2,147,483,647
    int64_t i64 = 1000;         // 64 位有符号整数，范围: -9,223,372,036,854,775,808 到
                                // 9,223,372,036,854,775,807

    // 浮点类型
    float f32 = 3.14159265f;         // 32 位单精度浮点，约 7 位有效数字
    double f64 = 3.141592653589793;  // 64 位双精度浮点，约 15 位有效数字

    // 其他类型
    string name_ {"String"};
    bool flag = true;  // 布尔类型，值: true 或 false

    string getName()
    {
        std::cout << __func__ << " " << name_ << std::endl;
        return name_;
    }

    int8_t geti8()
    {
        std::cout << __func__ << " " << (int)i8 << std::endl;
        return i8;
    }

    int16_t geti16()
    {
        std::cout << __func__ << " " << i16 << std::endl;
        return i16;
    }

    int32_t geti32()
    {
        std::cout << __func__ << " " << i32 << std::endl;
        return i32;
    }

    int64_t geti64()
    {
        std::cout << __func__ << " " << i64 << std::endl;
        return i64;
    }

    float getf32()
    {
        std::cout << __func__ << " " << f32 << std::endl;
        return f32;
    }

    double getf64()
    {
        std::cout << __func__ << " " << f64 << std::endl;
        return f64;
    }

    bool getbool()
    {
        std::cout << __func__ << " " << flag << std::endl;
        return flag;
    }

    array<uint8_t> getArraybuffer()
    {
        int const len = 5;
        int const member = 3;
        array<uint8_t> result = array<uint8_t>::make(len);
        std::fill(result.begin(), result.end(), member);
        return result;
    }

    array<int8_t> getArray()
    {
        int const len = 5;
        int const member = 3;
        array<int8_t> result = array<int8_t>::make(len);
        std::fill(result.begin(), result.end(), member);
        return result;
    }

    ::inner::union_primitive getunion()
    {
        return ::inner::union_primitive::make_sValue("union string");
    }

    map<string, int8_t> getrecord()
    {
        map<string, int8_t> m;
        int const key1num = 1;
        int const key2num = 2;
        int const key3num = 3;
        m.emplace("key1", static_cast<int8_t>(key1num));
        m.emplace("key2", static_cast<int8_t>(key2num));
        m.emplace("key3", static_cast<int8_t>(key3num));
        return m;
    }

    ::inner::Color getColorEnum()
    {
        return (::inner::Color::key_t)((int)1);
    }
};

class Test1 {
public:
    void Fun1() {}

    void Fun2() {}
};

class Test2 {
public:
    void Fun1() {}

    void Fun2() {}
};

class Test3 {
public:
    void Fun1() {}

    void Fun2() {}
};

class Test4 {
public:
    void Fun1() {}

    void Fun2() {}
};

class Test5 {
public:
    void Fun1() {}

    void Fun2() {}
};

class Test6 {
public:
    void Fun1() {}

    void Fun2() {}
};

class Test7 {
public:
    void Fun1() {}

    void Fun2() {}
};

class Test8 {
public:
    void Fun1() {}

    void Fun2() {}
};

class Test9 {
public:
    void Fun1() {}

    void Fun2() {}
};

class Test10 {
public:
    void Fun1() {}

    void Fun2() {}
};

class Test11 {
public:
    void Fun1() {}

    void Fun2() {}
};

class Test12 {
public:
    void Fun1() {}

    void Fun2() {}
};

class Test13 {
public:
    void Fun1() {}

    void Fun2() {}
};

class Test14 {
public:
    void Fun1() {}

    void Fun2() {}
};

class Test15 {
public:
    void Fun1() {}

    void Fun2() {}
};

class Test16 {
public:
    void Fun1() {}

    void Fun2() {}
};

class Test17 {
public:
    void Fun1() {}

    void Fun2() {}
};

class Test18 {
public:
    void Fun1() {}

    void Fun2() {}
};

class Test19 {
public:
    void Fun1() {}

    void Fun2() {}
};

class Test20 {
public:
    void Fun1() {}

    void Fun2() {}
};

class TestA {
public:
    string Fun1()
    {
        std::cout << "fun1" << std::endl;
        return "fun1";
    }
};

class TestB {
public:
    string Fun2()
    {
        std::cout << "IfaceB func_b" << std::endl;
        return "fun2";
    }

    string Fun1()
    {
        std::cout << "IfaceB func_a" << std::endl;
        return "fun1";
    }
};

class TestC {
public:
    string Fun3()
    {
        std::cout << "IfaceC func_c" << std::endl;
        return "fun3";
    }

    string Fun1()
    {
        std::cout << "IfaceC func_a" << std::endl;
        return "fun1";
    }
};

void Primitives_noreturn(int32_t a, double b, bool c, string_view d, int8_t e) {}

string Primitives_return(int32_t a, double b, bool c, string_view d, int8_t e)
{
    return "primitives_return";
}

void Containers_noreturn(array_view<int8_t> a, array_view<int16_t> b, array_view<float> c, array_view<double> d,
                         ::inner::union_primitive const &e)
{
}

string Containers_return(array_view<int8_t> a, array_view<int16_t> b, array_view<float> c, array_view<double> d,
                         ::inner::union_primitive const &e)
{
    return "containers_return";
}

void Enum_noreturn(::inner::Color a, ::inner::Color b, ::inner::Color c, ::inner::Color d, ::inner::Color e) {}

string Enum_return(::inner::Color a, ::inner::Color b, ::inner::Color c, ::inner::Color d, ::inner::Color e)
{
    return "enum_return";
}

::inner::TestInterface get_interface()
{
    return make_holder<TestInterface, ::inner::TestInterface>();
}

string PrintTestInterfaceName(::inner::weak::TestInterface testiface)
{
    auto name = testiface->getName();
    std::cout << __func__ << ": " << name << std::endl;
    return name;
}

int8_t PrintTestInterfaceNumberi8(::inner::weak::TestInterface testiface)
{
    auto name = testiface->geti8();
    std::cout << __func__ << ": " << (int)name << std::endl;
    return name;
}

int16_t PrintTestInterfaceNumberi16(::inner::weak::TestInterface testiface)
{
    auto name = testiface->geti16();
    std::cout << __func__ << ": " << name << std::endl;
    return name;
}

int32_t PrintTestInterfaceNumberi32(::inner::weak::TestInterface testiface)
{
    auto name = testiface->geti32();
    std::cout << __func__ << ": " << name << std::endl;
    return name;
}

int64_t PrintTestInterfaceNumberi64(::inner::weak::TestInterface testiface)
{
    auto name = testiface->geti64();
    std::cout << __func__ << ": " << name << std::endl;
    return name;
}

float PrintTestInterfaceNumberf32(::inner::weak::TestInterface testiface)
{
    auto name = testiface->getf32();
    std::cout << __func__ << ": " << name << std::endl;
    return name;
}

double PrintTestInterfaceNumberf64(::inner::weak::TestInterface testiface)
{
    auto name = testiface->getf64();
    std::cout << __func__ << ": " << name << std::endl;
    return name;
}

bool PrintTestInterfacebool(::inner::weak::TestInterface testiface)
{
    auto name = testiface->getbool();
    std::cout << __func__ << ": " << name << std::endl;
    return name;
}

array<uint8_t> PrintTestInterfaceArraybuffer(::inner::weak::TestInterface testiface)
{
    array<uint8_t> arr = testiface->getArraybuffer();
    for (size_t i = 0; i < arr.size(); ++i) {
        std::cout << static_cast<int>(arr.data()[i]);
        if (i < arr.size() - 1) {
            std::cout << ", ";
        }
    }
    return arr;
}

array<int8_t> PrintTestInterfaceArray(::inner::weak::TestInterface testiface)
{
    array<int8_t> arr = testiface->getArray();
    for (size_t i = 0; i < arr.size(); ++i) {
        std::cout << static_cast<int>(arr.data()[i]);
        if (i < arr.size() - 1) {
            std::cout << ", ";
        }
    }
    return arr;
}

::inner::union_primitive PrintTestInterfaceUnion(::inner::weak::TestInterface testiface)
{
    ::inner::union_primitive up = testiface->getunion();
    std::cout << "s: " << up.get_sValue_ref() << std::endl;
    return up;
}

map<string, int8_t> PrintTestInterfaceRecord(::inner::weak::TestInterface testiface)
{
    map<string, int8_t> m = testiface->getrecord();
    for (auto const &[key, value] : m) {
        std::cout << "Key: " << key << ", Value: " << static_cast<int>(value) << std::endl;
        // 注意：int8_t 需要转为 int 打印，否则会输出 ASCII 字符
    }
    return m;
}

::inner::Color PrintTestInterfaceEnum(::inner::weak::TestInterface testiface)
{
    ::inner::Color c = testiface->getColorEnum();
    std::cout << "enum get_key " << (int)c.get_key() << std::endl;
    return c;
}

::inner::Test1 get_interface_1()
{
    return make_holder<Test1, ::inner::Test1>();
}

::inner::Test20 get_interface_20()
{
    return make_holder<Test20, ::inner::Test20>();
}

::inner::TestA get_interface_A()
{
    return make_holder<TestA, ::inner::TestA>();
}

::inner::TestB get_interface_B()
{
    return make_holder<TestB, ::inner::TestB>();
}

::inner::TestC get_interface_C()
{
    return make_holder<TestC, ::inner::TestC>();
}

}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_Primitives_noreturn(Primitives_noreturn);
TH_EXPORT_CPP_API_Primitives_return(Primitives_return);
TH_EXPORT_CPP_API_Containers_noreturn(Containers_noreturn);
TH_EXPORT_CPP_API_Containers_return(Containers_return);
TH_EXPORT_CPP_API_Enum_noreturn(Enum_noreturn);
TH_EXPORT_CPP_API_Enum_return(Enum_return);
TH_EXPORT_CPP_API_get_interface(get_interface);
TH_EXPORT_CPP_API_PrintTestInterfaceName(PrintTestInterfaceName);
TH_EXPORT_CPP_API_PrintTestInterfaceNumberi8(PrintTestInterfaceNumberi8);
TH_EXPORT_CPP_API_PrintTestInterfaceNumberi16(PrintTestInterfaceNumberi16);
TH_EXPORT_CPP_API_PrintTestInterfaceNumberi32(PrintTestInterfaceNumberi32);
TH_EXPORT_CPP_API_PrintTestInterfaceNumberi64(PrintTestInterfaceNumberi64);
TH_EXPORT_CPP_API_PrintTestInterfaceNumberf32(PrintTestInterfaceNumberf32);
TH_EXPORT_CPP_API_PrintTestInterfaceNumberf64(PrintTestInterfaceNumberf64);
TH_EXPORT_CPP_API_PrintTestInterfacebool(PrintTestInterfacebool);
TH_EXPORT_CPP_API_PrintTestInterfaceArraybuffer(PrintTestInterfaceArraybuffer);
TH_EXPORT_CPP_API_PrintTestInterfaceArray(PrintTestInterfaceArray);
TH_EXPORT_CPP_API_PrintTestInterfaceUnion(PrintTestInterfaceUnion);
TH_EXPORT_CPP_API_PrintTestInterfaceRecord(PrintTestInterfaceRecord);
TH_EXPORT_CPP_API_PrintTestInterfaceEnum(PrintTestInterfaceEnum);
TH_EXPORT_CPP_API_get_interface_1(get_interface_1);
TH_EXPORT_CPP_API_get_interface_20(get_interface_20);
TH_EXPORT_CPP_API_get_interface_A(get_interface_A);
TH_EXPORT_CPP_API_get_interface_B(get_interface_B);
TH_EXPORT_CPP_API_get_interface_C(get_interface_C);
// NOLINTEND