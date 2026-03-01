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
#include "test_union.impl.hpp"

#include <numeric>

#include "stdexcept"
#include "taihe/runtime.hpp"
#include "taihe/string.hpp"
#include "test_union.MyInterface.proj.2.hpp"
#include "test_union.UnionArrayMap1.proj.1.hpp"
#include "test_union.UnionArrayMap2.proj.1.hpp"
#include "test_union.UnionArrayMap3.proj.1.hpp"
#include "test_union.UnionArrayMap4.proj.1.hpp"
#include "test_union.UnionArrayMap5.proj.1.hpp"
#include "test_union.UnionArrayMap6.proj.1.hpp"
#include "test_union.UnionArrayMap7.proj.1.hpp"
#include "test_union.UnionArrayMap8.proj.1.hpp"
#include "test_union.UnionArrayMap9.proj.1.hpp"
#include "test_union.color_map_value1.proj.1.hpp"
#include "test_union.color_map_value2.proj.1.hpp"
#include "test_union.color_map_value3.proj.1.hpp"
#include "test_union.color_map_value4.proj.1.hpp"
#include "test_union.color_map_value5.proj.1.hpp"
#include "test_union.color_map_value6.proj.1.hpp"
#include "test_union.color_map_value7.proj.1.hpp"
#include "test_union.color_map_value8.proj.1.hpp"
#include "test_union.color_map_value9.proj.1.hpp"
#include "test_union.union_mix.proj.1.hpp"
#include "test_union.union_mix_5.proj.1.hpp"
#include "test_union.union_primitive.proj.1.hpp"
#include "test_union.union_primitive_2.proj.1.hpp"
#include "test_union.union_primitive_2_1.proj.1.hpp"
#include "test_union.union_primitive_2_2.proj.1.hpp"
#include "test_union.union_primitive_2_3.proj.1.hpp"
#include "test_union.union_primitive_2_4.proj.1.hpp"
#include "test_union.union_primitive_2_5.proj.1.hpp"
#include "test_union.union_primitive_2_6.proj.1.hpp"
#include "test_union.union_primitive_2_7.proj.1.hpp"
#include "test_union.union_primitive_2_8.proj.1.hpp"
#include "test_union.union_primitive_2_9.proj.1.hpp"
// Please delete <stdexcept> include when you implement
using namespace taihe;

namespace {
class MyInterface {
public:
    string FuncUnionPrimitive(::test_union::union_primitive const &data, ::test_union::union_primitive const &data2)
    {
        switch (data.get_tag()) {
            case ::test_union::union_primitive::tag_t::sValue:
                std::cout << "s: " << data.get_sValue_ref() << std::endl;
                return "s";
            case ::test_union::union_primitive::tag_t::i8Value:
                std::cout << "i8: " << (int)data.get_i8Value_ref() << std::endl;
                return "i8";
            case ::test_union::union_primitive::tag_t::i16Value:
                std::cout << "i16: " << data.get_i16Value_ref() << std::endl;
                return "i16";
            case ::test_union::union_primitive::tag_t::i32Value:
                std::cout << "i32: " << data.get_i32Value_ref() << std::endl;
                return "i32";
            case ::test_union::union_primitive::tag_t::f32Value:
                std::cout << "i32: " << data.get_f32Value_ref() << std::endl;
                return "f32";
            case ::test_union::union_primitive::tag_t::f64Value:
                std::cout << "i32: " << data.get_f64Value_ref() << std::endl;
                return "f64";
            case ::test_union::union_primitive::tag_t::bValue:
                std::cout << "bool: " << data.get_bValue_ref() << std::endl;
                return "bool";
        }
    }

    string FuncUnionPrimitive5Param(::test_union::union_primitive const &data1,
                                    ::test_union::union_primitive const &data2,
                                    ::test_union::union_primitive const &data3,
                                    ::test_union::union_primitive const &data4,
                                    ::test_union::union_primitive const &data5)
    {
        return "func_union_primitive_5param";
    }

    string FuncColorMapValue1(::test_union::color_map_value1 const &data1)
    {
        return "func_color_map_value1";
    }

    string FuncColorMapValue2(::test_union::color_map_value2 const &data1)
    {
        return "func_color_map_value2";
    }

    string FuncColorMapValue3(::test_union::color_map_value3 const &data1)
    {
        return "func_color_map_value3";
    }

    string FuncColorMapValue4(::test_union::color_map_value4 const &data1)
    {
        return "func_color_map_value4";
    }

    string FuncColorMapValue5(::test_union::color_map_value5 const &data1)
    {
        return "func_color_map_value5";
    }

    string FuncColorMapValue6(::test_union::color_map_value6 const &data1)
    {
        return "func_color_map_value6";
    }

    string FuncColorMapValue7(::test_union::color_map_value7 const &data1)
    {
        return "func_color_map_value7";
    }

    string FuncColorMapValue8(::test_union::color_map_value8 const &data1)
    {
        return "func_color_map_value8";
    }

    string FuncColorMapValue9(::test_union::color_map_value9 const &data1)
    {
        return "func_color_map_value9";
    }

    string FuncColorMapValue10(::test_union::color_map_value9 const &data1, ::test_union::color_map_value9 const &data2)
    {
        return "func_color_map_value10";
    }

    string FuncUnionArrayMap1(::test_union::UnionArrayMap1 const &data1, ::test_union::UnionArrayMap1 const &data2)
    {
        return "func_union_array_map1";
    }

    string FuncUnionArrayMap2(::test_union::UnionArrayMap2 const &data1, ::test_union::UnionArrayMap2 const &data2)
    {
        return "func_union_array_map2";
    }

    string FuncUnionArrayMap3(::test_union::UnionArrayMap3 const &data1, ::test_union::UnionArrayMap3 const &data2)
    {
        return "func_union_array_map3";
    }

    string FuncUnionArrayMap4(::test_union::UnionArrayMap4 const &data1, ::test_union::UnionArrayMap4 const &data2)
    {
        return "func_union_array_map4";
    }

    string FuncUnionArrayMap5(::test_union::UnionArrayMap5 const &data1, ::test_union::UnionArrayMap5 const &data2)
    {
        return "func_union_array_map5";
    }

    string FuncUnionArrayMap6(::test_union::UnionArrayMap6 const &data1, ::test_union::UnionArrayMap6 const &data2)
    {
        return "func_union_array_map6";
    }

    string FuncUnionArrayMap7(::test_union::UnionArrayMap7 const &data1, ::test_union::UnionArrayMap7 const &data2)
    {
        return "func_union_array_map7";
    }

    string FuncUnionArrayMap8(::test_union::UnionArrayMap8 const &data1, ::test_union::UnionArrayMap8 const &data2)
    {
        return "func_union_array_map8";
    }

    string FuncUnionArrayMap9(::test_union::UnionArrayMap9 const &data1, ::test_union::UnionArrayMap9 const &data2)
    {
        return "func_union_array_map9";
    }

    string FuncUnionArrayMap10(::test_union::UnionArrayMap1 const &data1, ::test_union::UnionArrayMap2 const &data2)
    {
        return "func_union_array_map10";
    }

    string FuncUnionMix(::test_union::union_mix const &data1, ::test_union::union_mix const &data2,
                        ::test_union::union_mix const &data3, ::test_union::union_mix const &data4,
                        ::test_union::union_mix const &data5)
    {
        return "func_union_mix";
    }

    // This is corn case for function parameters, so skip lint error
    // NOLINTNEXTLINE(readability-function-size)
    string FuncUnionMix10Param(::test_union::union_mix const &data1, ::test_union::union_mix const &data2,
                               ::test_union::union_mix const &data3, ::test_union::union_mix const &data4,
                               ::test_union::union_mix const &data5, ::test_union::union_mix const &data6,
                               ::test_union::union_mix const &data7, ::test_union::union_mix const &data8,
                               ::test_union::union_mix const &data9, ::test_union::union_mix const &data10)
    {
        return "func_union_mix_10param";
    }

    ::test_union::union_primitive_2 FuncUnionPrimitiveReturn(string_view kind)
    {
        static std::string const sValue = "string";
        static int8_t const i8Value = 1;
        if (kind == "s") {
            return ::test_union::union_primitive_2::make_sValue(sValue);
        } else {
            return ::test_union::union_primitive_2::make_i8Value(i8Value);
        }
    }

    ::test_union::union_primitive_2_1 FuncUnionPrimitiveReturn1(string_view kind)
    {
        static int8_t const i8Value = 1;
        static int16_t const i16Value = 12;
        if (kind == "i8") {
            return ::test_union::union_primitive_2_1::make_i8Value(i8Value);
        } else {
            return ::test_union::union_primitive_2_1::make_i16Value(i16Value);
        }
    }

    ::test_union::union_primitive_2_2 FuncUnionPrimitiveReturn2(string_view kind)
    {
        static int8_t const i8Value = 1;
        static int const i8Value2 = 2;
        if (kind == "i8_1") {
            return ::test_union::union_primitive_2_2::make_i8Value(i8Value);
        } else {
            return ::test_union::union_primitive_2_2::make_i8Value2(i8Value2);
        }
    }

    ::test_union::union_primitive_2_3 FuncUnionPrimitiveReturn3(string_view kind)
    {
        static int16_t const i16Value = 12;
        static int const i32Value = 123;
        if (kind == "i16") {
            return ::test_union::union_primitive_2_3::make_i16Value(i16Value);
        } else {
            return ::test_union::union_primitive_2_3::make_i32Value(i32Value);
        }
    }

    ::test_union::union_primitive_2_4 FuncUnionPrimitiveReturn4(string_view kind)
    {
        static int const i32Value = 123;
        static float const f32Value = 1.1f;
        if (kind == "i32") {
            return ::test_union::union_primitive_2_4::make_i32Value(i32Value);
        } else {
            return ::test_union::union_primitive_2_4::make_f32Value(f32Value);
        }
    }

    ::test_union::union_primitive_2_5 FuncUnionPrimitiveReturn5(string_view kind)
    {
        static float const f32Value = 1.1f;
        static double const f64Value = 1.234;
        if (kind == "f32") {
            return ::test_union::union_primitive_2_5::make_f32Value(f32Value);
        } else {
            return ::test_union::union_primitive_2_5::make_f64Value(f64Value);
        }
    }

    ::test_union::union_primitive_2_6 FuncUnionPrimitiveReturn6(string_view kind)
    {
        if (kind == "s") {
            return ::test_union::union_primitive_2_6::make_sValue("hello");
        } else {
            return ::test_union::union_primitive_2_6::make_bValue(true);
        }
    }

    ::test_union::union_primitive_2_7 FuncUnionPrimitiveReturn7(string_view kind)
    {
        if (kind == "b") {
            return ::test_union::union_primitive_2_7::make_bValue(true);
        } else {
            int8_t const i8Value = 1;
            return ::test_union::union_primitive_2_7::make_i8Value(i8Value);
        }
    }

    ::test_union::union_primitive_2_8 FuncUnionPrimitiveReturn8(string_view kind)
    {
        if (kind == "b") {
            return ::test_union::union_primitive_2_8::make_bValue(true);
        } else {
            return ::test_union::union_primitive_2_8::make_b1Value(false);
        }
    }

    ::test_union::union_primitive_2_9 FuncUnionPrimitiveReturn9(string_view kind)
    {
        if (kind == "s") {
            return ::test_union::union_primitive_2_9::make_sValue("hello");
        } else {
            return ::test_union::union_primitive_2_9::make_s1Value("world");
        }
    }

    ::test_union::union_mix_5 FuncUnionMix5Return(string_view kind)
    {
        if (kind == "s") {
            return ::test_union::union_mix_5::make_sValue("hello");
        }
        if (kind == "i8") {
            static int8_t const i8Value = 1;
            return ::test_union::union_mix_5::make_i8Value(i8Value);
        }
        if (kind == "b") {
            return ::test_union::union_mix_5::make_bValue(true);
        }
        if (kind == "c") {
            return ::test_union::union_mix_5::make_enumValue((::test_union::Color::key_t)((int)(1)));
        } else {
            static int const arrSize = 5;
            static int const arrNum = 3;
            array<int32_t> result = array<int32_t>::make(arrSize);
            std::fill(result.begin(), result.end(), arrNum);
            return ::test_union::union_mix_5::make_arr32(result);
        }
    }

    ::test_union::union_mix FuncUnionMix10Return(string_view kind)
    {
        return ::test_union::union_mix::make_bValue(true);
    }
};

string PrintUnion(::test_union::union_primitive const &data)
{
    switch (data.get_tag()) {
        case ::test_union::union_primitive::tag_t::sValue:
            std::cout << "s: " << data.get_sValue_ref() << std::endl;
            return "s";
        case ::test_union::union_primitive::tag_t::i8Value:
            std::cout << "i8: " << (int)data.get_i8Value_ref() << std::endl;
            return "i8";
        case ::test_union::union_primitive::tag_t::i16Value:
            std::cout << "i16: " << data.get_i16Value_ref() << std::endl;
            return "i16";
        case ::test_union::union_primitive::tag_t::i32Value:
            std::cout << "i32: " << data.get_i32Value_ref() << std::endl;
            return "i32";
        case ::test_union::union_primitive::tag_t::f32Value:
            std::cout << "i32: " << data.get_f32Value_ref() << std::endl;
            return "f32";
        case ::test_union::union_primitive::tag_t::f64Value:
            std::cout << "i32: " << data.get_f64Value_ref() << std::endl;
            return "f64";
        case ::test_union::union_primitive::tag_t::bValue:
            std::cout << "bool: " << data.get_bValue_ref() << std::endl;
            return "bool";
    }
}

::test_union::union_primitive MakeUnion(std::string_view kind)
{
    constexpr std::string_view sValue = "string";
    constexpr int8_t i8Value = 1;
    constexpr int16_t i16Value = 123;
    constexpr int32_t i32Value = 1234;
    constexpr float f32Value = 1.12f;
    constexpr double f64Value = 1.12345;
    constexpr bool boolValue = true;

    if (kind == "s") {
        return ::test_union::union_primitive::make_sValue(sValue);
    }
    if (kind == "i8") {
        return ::test_union::union_primitive::make_i8Value(i8Value);
    }
    if (kind == "i16") {
        return ::test_union::union_primitive::make_i16Value(i16Value);
    }
    if (kind == "i32") {
        return ::test_union::union_primitive::make_i32Value(i32Value);
    }
    if (kind == "f32") {
        return ::test_union::union_primitive::make_f32Value(f32Value);
    }
    if (kind == "f64") {
        return ::test_union::union_primitive::make_f64Value(f64Value);
    }
    if (kind == "bool") {
        return ::test_union::union_primitive::make_bValue(boolValue);
    }

    // 处理未知的 kind 值
    // ...
}

::test_union::MyInterface GetInterface()
{
    return make_holder<MyInterface, ::test_union::MyInterface>();
}
}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_PrintUnion(PrintUnion);
TH_EXPORT_CPP_API_MakeUnion(MakeUnion);
TH_EXPORT_CPP_API_GetInterface(GetInterface);
// NOLINTEND