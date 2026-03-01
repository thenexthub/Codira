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
#include "union_ani.impl.hpp"

#include "stdexcept"
#include "taihe/runtime.hpp"
#include "taihe/string.hpp"
#include "union_ani.MyUnion.proj.1.hpp"
#include "union_ani.TypedArrayUnion.proj.1.hpp"
using namespace taihe;

namespace {
string printInnerUnion(::union_ani::InnerUnion const &data)
{
    switch (data.get_tag()) {
        case ::union_ani::InnerUnion::tag_t::stringValue:
            std::cout << "s: " << data.get_stringValue_ref() << std::endl;
            return "s";
        case ::union_ani::InnerUnion::tag_t::pairValue:
            std::cout << "p: " << data.get_pairValue_ref().a << ", " << data.get_pairValue_ref().b << std::endl;
            return "p";
        case ::union_ani::InnerUnion::tag_t::undefinedValue:
            std::cout << "u" << std::endl;
            return "u";
    }
}

string PrintMyUnion(::union_ani::MyUnion const &data)
{
    switch (data.get_tag()) {
        case ::union_ani::MyUnion::tag_t::innerValue:
            return printInnerUnion(data.get_innerValue_ref());
        case ::union_ani::MyUnion::tag_t::floatValue:
            std::cout << "f: " << data.get_floatValue_ref() << std::endl;
            return "f";
    }
}

::union_ani::MyUnion MakeMyUnion(string_view kind)
{
    float const testFloat = 123.0f;
    if (kind == "s") {
        return ::union_ani::MyUnion::make_innerValue(::union_ani::InnerUnion::make_stringValue("string"));
    }
    if (kind == "p") {
        ::union_ani::Pair pair = {"a", "b"};
        return ::union_ani::MyUnion::make_innerValue(::union_ani::InnerUnion::make_pairValue(pair));
    }
    if (kind == "f") {
        return ::union_ani::MyUnion::make_floatValue(testFloat);
    }
    return ::union_ani::MyUnion::make_innerValue(::union_ani::InnerUnion::make_undefinedValue());
}

::union_ani::BasicUnion MakeAndShowBasicUnion(::union_ani::BasicUnion const &data)
{
    switch (data.get_tag()) {
        case ::union_ani::BasicUnion::tag_t::int8Value: {
            int8_t value = data.get_int8Value_ref();
            return ::union_ani::BasicUnion::make_int8Value(value);
        }

        case ::union_ani::BasicUnion::tag_t::int16Value: {
            int16_t value = data.get_int16Value_ref();
            return ::union_ani::BasicUnion::make_int16Value(value);
        }

        case ::union_ani::BasicUnion::tag_t::int32Value: {
            int32_t value = data.get_int32Value_ref();
            return ::union_ani::BasicUnion::make_int32Value(value);
        }

        case ::union_ani::BasicUnion::tag_t::int64Value: {
            int64_t value = data.get_int64Value_ref();
            return ::union_ani::BasicUnion::make_int64Value(value);
        }

        case ::union_ani::BasicUnion::tag_t::float32Value: {
            float value = data.get_float32Value_ref();
            return ::union_ani::BasicUnion::make_float32Value(value);
        }

        case ::union_ani::BasicUnion::tag_t::float64Value: {
            double value = data.get_float64Value_ref();
            return ::union_ani::BasicUnion::make_float64Value(value);
        }

        case ::union_ani::BasicUnion::tag_t::stringValue: {
            string value = data.get_stringValue_ref();
            return ::union_ani::BasicUnion::make_stringValue(value);
        }

        case ::union_ani::BasicUnion::tag_t::boolValue: {
            bool value = data.get_boolValue_ref();
            return ::union_ani::BasicUnion::make_boolValue(value);
        }

        case ::union_ani::BasicUnion::tag_t::empty: {
            return ::union_ani::BasicUnion::make_empty();
        }
    }
}

::union_ani::TypedArrayUnion MakeAndShowTypedArrayUnion(::union_ani::TypedArrayUnion const &data)
{
    switch (data.get_tag()) {
        case ::union_ani::TypedArrayUnion::tag_t::u8Value: {
            ::taihe::array<uint8_t> const value = data.get_u8Value_ref();
            return ::union_ani::TypedArrayUnion::make_u8Value(value);
        }
        case ::union_ani::TypedArrayUnion::tag_t::i8Value: {
            array<int8_t> value = data.get_i8Value_ref();
            return ::union_ani::TypedArrayUnion::make_i8Value(value);
        }

        case ::union_ani::TypedArrayUnion::tag_t::u16Value: {
            array<uint16_t> value = data.get_u16Value_ref();
            return ::union_ani::TypedArrayUnion::make_u16Value(value);
        }

        case ::union_ani::TypedArrayUnion::tag_t::i16Value: {
            array<int16_t> value = data.get_i16Value_ref();
            return ::union_ani::TypedArrayUnion::make_i16Value(value);
        }

        case ::union_ani::TypedArrayUnion::tag_t::u32Value: {
            array<uint32_t> value = data.get_u32Value_ref();
            return ::union_ani::TypedArrayUnion::make_u32Value(value);
        }

        case ::union_ani::TypedArrayUnion::tag_t::i32Value: {
            array<int32_t> value = data.get_i32Value_ref();
            return ::union_ani::TypedArrayUnion::make_i32Value(value);
        }

        case ::union_ani::TypedArrayUnion::tag_t::u64Value: {
            array<uint64_t> value = data.get_u64Value_ref();
            return ::union_ani::TypedArrayUnion::make_u64Value(value);
        }

        case ::union_ani::TypedArrayUnion::tag_t::i64Value: {
            array<int64_t> value = data.get_i64Value_ref();
            return ::union_ani::TypedArrayUnion::make_i64Value(value);
        }

        case ::union_ani::TypedArrayUnion::tag_t::f32Value: {
            array<float> value = data.get_f32Value_ref();
            return ::union_ani::TypedArrayUnion::make_f32Value(value);
        }

        case ::union_ani::TypedArrayUnion::tag_t::f64Value: {
            array<double> value = data.get_f64Value_ref();
            return ::union_ani::TypedArrayUnion::make_f64Value(value);
        }

        case ::union_ani::TypedArrayUnion::tag_t::empty: {
            return ::union_ani::TypedArrayUnion::make_empty();
        }
    }
}

::union_ani::ArrayUnion MakeAndShowArrayUnion(::union_ani::ArrayUnion const &data)
{
    switch (data.get_tag()) {
        case ::union_ani::ArrayUnion::tag_t::floatValue: {
            auto &value = data.get_floatValue_ref();
            return ::union_ani::ArrayUnion::make_floatValue(value);
        }

        case ::union_ani::ArrayUnion::tag_t::empty: {
            return ::union_ani::ArrayUnion::make_empty();
        }
    }
}

::union_ani::StructUnion MakeAndShowStructUnion(::union_ani::StructUnion const &data)
{
    switch (data.get_tag()) {
        case ::union_ani::StructUnion::tag_t::strValue: {
            auto &strValue = data.get_strValue_ref();
            return ::union_ani::StructUnion::make_strValue(strValue);
        }

        case ::union_ani::StructUnion::tag_t::intValue: {
            auto &intValue = data.get_intValue_ref();
            return ::union_ani::StructUnion::make_intValue(intValue);
        }

        case ::union_ani::StructUnion::tag_t::empty: {
            return ::union_ani::StructUnion::make_empty();
        }
    }
}
}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_PrintMyUnion(PrintMyUnion);
TH_EXPORT_CPP_API_MakeMyUnion(MakeMyUnion);
TH_EXPORT_CPP_API_MakeAndShowBasicUnion(MakeAndShowBasicUnion);
TH_EXPORT_CPP_API_MakeAndShowTypedArrayUnion(MakeAndShowTypedArrayUnion);
TH_EXPORT_CPP_API_MakeAndShowArrayUnion(MakeAndShowArrayUnion);
TH_EXPORT_CPP_API_MakeAndShowStructUnion(MakeAndShowStructUnion);
// NOLINTEND