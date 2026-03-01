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
#include "taihe/array.hpp"

#include <algorithm>
#include <numeric>

#include "array_test.Color.proj.0.hpp"
#include "array_test.Data.proj.1.hpp"
#include "array_test.impl.hpp"
#include "stdexcept"
#include "taihe/map.hpp"
#include "taihe/runtime.hpp"
#include "taihe/string.hpp"

using namespace taihe;

namespace {

class ArrayI8Impl {
public:
    ArrayI8Impl() {}

    ::taihe::array<int8_t> ArrayI8Value(int8_t value)
    {
        taihe::array<int8_t> res = {value, value, value};
        return res;
    }
};

class ArrayI16Impl {
public:
    ArrayI16Impl() {}

    ::taihe::array<int16_t> ArrayI16Value(int16_t value)
    {
        taihe::array<int16_t> res = {value, value, value};
        return res;
    }
};

class ArrayI32Impl {
public:
    ArrayI32Impl() {}

    ::taihe::array<int32_t> ArrayI32Value(int32_t value)
    {
        taihe::array<int32_t> res = {value, value, value};
        return res;
    }
};

class ArrayI64Impl {
public:
    ArrayI64Impl() {}

    ::taihe::array<int64_t> ArrayI64Value(int64_t value)
    {
        taihe::array<int64_t> res = {value, value, value};
        return res;
    }
};

class ArrayF32Impl {
public:
    ArrayF32Impl() {}

    ::taihe::array<float> ArrayF32Value(float value)
    {
        taihe::array<float> res = {value, value, value};
        return res;
    }
};

class ArrayF64Impl {
public:
    ArrayF64Impl() {}

    ::taihe::array<double> ArrayF64Value(double value)
    {
        taihe::array<double> res = {value, value, value};
        return res;
    }
};

class ArrayStringImpl {
public:
    ArrayStringImpl() {}

    ::taihe::array<::taihe::string> ArrayStringValue(::taihe::string_view value)
    {
        taihe::array<::taihe::string> res = {value, value, value};
        return res;
    }
};

class ArrayBoolImpl {
public:
    ArrayBoolImpl() {}

    ::taihe::array<bool> ArrayBoolValue(bool value)
    {
        taihe::array<bool> res = {value, value, value};
        return res;
    }
};

class ArrayEnumImpl {
public:
    ArrayEnumImpl() {}

    ::taihe::array<::array_test::Color> ArrayEnumValue(::taihe::string_view value)
    {
        ::taihe::array<::array_test::Color> color = {::array_test::Color::key_t::RED, ::array_test::Color::key_t::GREEN,
                                                     ::array_test::Color::key_t::BLUE};
        return color;
    }
};

class ArrayRecordImpl {
public:
    ArrayRecordImpl() {}

    ::taihe::array<::taihe::map<::taihe::string, int64_t>> ArrayRecordValue(::taihe::string_view valStr,
                                                                            int64_t valLong)
    {
        map<string, int64_t> record;
        record.emplace(valStr, valLong);
        ::taihe::array<::taihe::map<::taihe::string, int64_t>> res = {record, record, record};
        return res;
    }
};

class ArrayRecordStrI8Impl {
public:
    ArrayRecordStrI8Impl() {}

    ::taihe::array<::taihe::map<::taihe::string, int8_t>> ArrayRecordValue(::taihe::string_view valStr, int8_t valNum)
    {
        map<string, int8_t> record;
        record.emplace(valStr, valNum);
        ::taihe::array<::taihe::map<::taihe::string, int8_t>> res = {record, record, record};
        return res;
    }
};

class ArrayRecordStrF32Impl {
public:
    ArrayRecordStrF32Impl() {}

    ::taihe::array<::taihe::map<::taihe::string, float>> ArrayRecordValue(::taihe::string_view valStr, float valNum)
    {
        map<string, float> record;
        record.emplace(valStr, valNum);
        ::taihe::array<::taihe::map<::taihe::string, float>> res = {record, record, record};
        return res;
    }
};

class ArrayRecordStrF64Impl {
public:
    ArrayRecordStrF64Impl() {}

    ::taihe::array<::taihe::map<::taihe::string, double>> ArrayRecordValue(::taihe::string_view valStr, double valNum)
    {
        map<string, double> record;
        record.emplace(valStr, valNum);
        ::taihe::array<::taihe::map<::taihe::string, double>> res = {record, record, record};
        return res;
    }
};

class ArrayRecordStrBoolImpl {
public:
    ArrayRecordStrBoolImpl() {}

    ::taihe::array<::taihe::map<::taihe::string, bool>> ArrayRecordValue(::taihe::string_view valStr, bool valNum)
    {
        map<string, bool> record;
        record.emplace(valStr, valNum);
        ::taihe::array<::taihe::map<::taihe::string, bool>> res = {record, record, record};
        return res;
    }
};

class ArrayUnionImpl {
public:
    ::array_test::UnionA unionA;

    ArrayUnionImpl() : unionA({::array_test::UnionA::make_name("UnionA")}) {}

    ::taihe::array<::array_test::UnionA> ArrayValue(::taihe::string_view value)
    {
        ::array_test::UnionA record = ::array_test::UnionA::make_name(value);
        ::taihe::array<::array_test::UnionA> res = {record, record, record};
        return res;
    }
};

class ArrayPromiseI8Impl {
public:
    ArrayPromiseI8Impl() {}

    ::taihe::array<int8_t> fetchDataI8()
    {
        ::taihe::array<int8_t> res = {-128, 0, 127};
        return res;
    }
};

class ArrayPromiseI16Impl {
public:
    ArrayPromiseI16Impl() {}

    ::taihe::array<int16_t> fetchDataI16()
    {
        ::taihe::array<int16_t> res = {-32768, 0, 32767};
        return res;
    }
};

class ArrayPromiseI32Impl {
public:
    ArrayPromiseI32Impl() {}

    ::taihe::array<int32_t> fetchDataI32()
    {
        ::taihe::array<int32_t> res = {-2147483648, 0, 2147483647};
        return res;
    }
};

class ArrayPromiseI64Impl {
public:
    ArrayPromiseI64Impl() {}

    ::taihe::array<int64_t> fetchDataI64()
    {
        ::taihe::array<int64_t> res = {-9223372036854775807, 0, 9223372036854775807};
        return res;
    }
};

class ArrayPromiseF32Impl {
public:
    ArrayPromiseF32Impl() {}

    ::taihe::array<float> fetchDataF32()
    {
        ::taihe::array<float> res = {-202505.22, 0, 15.39};
        return res;
    }
};

class ArrayPromiseF64Impl {
public:
    ArrayPromiseF64Impl() {}

    ::taihe::array<double> fetchDataF64()
    {
        ::taihe::array<double> res = {-202505.221539, 0, 2215.39};
        return res;
    }
};

class ArrayPromiseStringImpl {
public:
    ArrayPromiseStringImpl() {}

    ::taihe::array<::taihe::string> fetchDataString()
    {
        ::taihe::array<string> res = {"String", "Data"};
        return res;
    }
};

class ArrayPromiseBoolImpl {
public:
    ArrayPromiseBoolImpl() {}

    ::taihe::array<bool> fetchDataBool()
    {
        ::taihe::array<bool> res = {true, false};
        return res;
    }
};

class ArrayPromiseU8Impl {
public:
    ArrayPromiseU8Impl() {}

    ::taihe::array<uint8_t> fetchDataPro()
    {
        ::taihe::array<uint8_t> res = {0, 255};
        return res;
    }
};

class ArrayPromiseU16Impl {
public:
    ArrayPromiseU16Impl() {}

    ::taihe::array<uint16_t> fetchDataPro()
    {
        ::taihe::array<uint16_t> res = {0, 65535};
        return res;
    }
};

class ArrayNestImpl {
public:
    ArrayNestImpl() {}

    ::taihe::array<::taihe::array<int8_t>> ArrayNestI8(int8_t param)
    {
        ::taihe::array<::taihe::array<int8_t>> res = {{param}, {-128}, {0}, {127}};
        return res;
    }

    ::taihe::array<::taihe::array<int16_t>> ArrayNestI16(int16_t param)
    {
        ::taihe::array<::taihe::array<int16_t>> res = {{param}, {-32768}, {0}, {32767}};
        return res;
    }

    ::taihe::array<::taihe::array<float>> ArrayNestF32(float param)
    {
        ::taihe::array<::taihe::array<float>> res = {{param}, {-2025.0523}, {0}, {15.58}};
        return res;
    }

    ::taihe::array<::taihe::array<::taihe::string>> ArrayNestStrng(::taihe::string_view param)
    {
        ::taihe::array<::taihe::array<string>> res = {{param}, {"nest"}, {"is"}, {"this"}};
        return res;
    }

    ::taihe::array<::taihe::array<bool>> ArrayNestBool(bool param)
    {
        ::taihe::array<::taihe::array<bool>> res = {{param}, {true}, {false}};
        return res;
    }
};

class ArrayOptionalImpl {
public:
    ArrayOptionalImpl() {}

    ::taihe::optional<::taihe::array<::taihe::string>> ArrayOpt(::taihe::string_view name)
    {
        optional<array<::taihe::string>> res =
            optional<array<::taihe::string>>::make(array<::taihe::string> {name, name, name});
        return res;
    }
};

int32_t SumArray(array_view<int32_t> nums, int32_t base)
{
    return std::accumulate(nums.begin(), nums.end(), base);
}

int64_t GetArrayValue(array_view<int64_t> nums, int32_t idx)
{
    if (idx >= 0 && idx < nums.size()) {
        return nums[idx];
    }
    throw std::runtime_error("Index out of range");
}

array<string> ToStingArray(array_view<int32_t> nums)
{
    auto result = array<string>::make(nums.size(), "");
    std::transform(nums.begin(), nums.end(), result.begin(), [](int32_t n) { return to_string(n); });
    return result;
}

array<int32_t> MakeIntArray(int32_t value, int32_t num)
{
    return array<int32_t>::make(num, value);
}

array<::array_test::Color> MakeEnumArray(::array_test::Color value, int32_t num)
{
    return array<::array_test::Color>::make(num, value);
}

array<map<string, int64_t>> MakeRecordArray(string_view key, int64_t val, int32_t num)
{
    map<string, int64_t> record;
    record.emplace(key, val);
    return array<map<string, int64_t>>::make(num, record);
}

array<::array_test::Data> MakeStructArray(string_view a, string_view b, int32_t c, int32_t num)
{
    return array<::array_test::Data>::make(num, ::array_test::Data {a, b, c});
}

array<array<int32_t>> MakeIntArray2(array_view<int32_t> value, int32_t num)
{
    return array<array<int32_t>>::make(num, value);
}

array<::array_test::Color> ChangeEnumArray(array_view<::array_test::Color> value, ::array_test::Color color)
{
    auto result = array<::array_test::Color>::make(value.size(), value[0]);
    std::transform(value.begin(), value.end(), result.begin(), [color](::array_test::Color c) { return color; });
    return result;
}

array<map<string, int64_t>> ChangeRecordArray(array_view<map<string, int64_t>> value, string_view k, int64_t v)
{
    auto result = array<map<string, int64_t>>::make(value.size(), value[0]);
    map<string, int64_t> record;
    record.emplace(k, v);
    std::transform(value.begin(), value.end(), result.begin(), [record](map<string, int64_t> m) { return record; });
    return result;
}

array<::array_test::Data> ChangeStructArray(array_view<::array_test::Data> value, string_view a, string_view b,
                                            int32_t c)
{
    auto result = array<::array_test::Data>::make(value.size(), value[0]);
    std::transform(value.begin(), value.end(), result.begin(),
                   [a, b, c](::array_test::Data d) { return ::array_test::Data {a, b, c}; });
    return result;
}

array<float> FetchBinaryDataSync(int32_t num)
{
    if (num <= 0) {
        taihe::set_business_error(1, "some error happen in fetchBinaryDataSync");
        return array<float>::make(0);
    } else {
        return array<float>::make(num);
    }
}

array<array<::array_test::Data>> MakeStructArrayArray(string_view a, string_view b, int32_t c, int32_t num1,
                                                      int32_t num2)
{
    auto arr = array<::array_test::Data>::make(num1, ::array_test::Data {a, b, c});
    return array<array<::array_test::Data>>::make(num2, arr);
}

::array_test::ArrayI8 GetArrayI8()
{
    return taihe::make_holder<ArrayI8Impl, ::array_test::ArrayI8>();
}

::array_test::ArrayI16 GetArrayI16()
{
    return taihe::make_holder<ArrayI16Impl, ::array_test::ArrayI16>();
}

::array_test::ArrayI32 GetArrayI32()
{
    return taihe::make_holder<ArrayI32Impl, ::array_test::ArrayI32>();
}

::array_test::ArrayI64 GetArrayI64()
{
    return taihe::make_holder<ArrayI64Impl, ::array_test::ArrayI64>();
}

::array_test::ArrayF32 GetArrayF32()
{
    return taihe::make_holder<ArrayF32Impl, ::array_test::ArrayF32>();
}

::array_test::ArrayF64 GetArrayF64()
{
    return taihe::make_holder<ArrayF64Impl, ::array_test::ArrayF64>();
}

::array_test::ArrayString GetArrayString()
{
    return taihe::make_holder<ArrayStringImpl, ::array_test::ArrayString>();
}

::array_test::ArrayBool GetArrayBool()
{
    return taihe::make_holder<ArrayBoolImpl, ::array_test::ArrayBool>();
}

::array_test::ArrayEnum GetArrayEnum()
{
    return taihe::make_holder<ArrayEnumImpl, ::array_test::ArrayEnum>();
}

::array_test::ArrayRecord GetArrayRecord()
{
    return taihe::make_holder<ArrayRecordImpl, ::array_test::ArrayRecord>();
}

::array_test::ArrayRecordStrI8 GetArrayRecordStrI8()
{
    return taihe::make_holder<ArrayRecordStrI8Impl, ::array_test::ArrayRecordStrI8>();
}

::array_test::ArrayRecordStrF32 GetArrayRecordStrF32()
{
    return taihe::make_holder<ArrayRecordStrF32Impl, ::array_test::ArrayRecordStrF32>();
}

::array_test::ArrayRecordStrF64 GetArrayRecordStrF64()
{
    return taihe::make_holder<ArrayRecordStrF64Impl, ::array_test::ArrayRecordStrF64>();
}

::array_test::ArrayRecordStrBool GetArrayRecordStrBool()
{
    return taihe::make_holder<ArrayRecordStrBoolImpl, ::array_test::ArrayRecordStrBool>();
}

::array_test::ArrayUnion GetArrayUnion()
{
    return taihe::make_holder<ArrayUnionImpl, ::array_test::ArrayUnion>();
}

::array_test::ArrayPromiseI8 GetArrayPromiseI8()
{
    return taihe::make_holder<ArrayPromiseI8Impl, ::array_test::ArrayPromiseI8>();
}

::array_test::ArrayPromiseI16 GetArrayPromiseI16()
{
    return taihe::make_holder<ArrayPromiseI16Impl, ::array_test::ArrayPromiseI16>();
}

::array_test::ArrayPromiseI32 GetArrayPromiseI32()
{
    return taihe::make_holder<ArrayPromiseI32Impl, ::array_test::ArrayPromiseI32>();
}

::array_test::ArrayPromiseI64 GetArrayPromiseI64()
{
    return taihe::make_holder<ArrayPromiseI64Impl, ::array_test::ArrayPromiseI64>();
}

::array_test::ArrayPromiseF32 GetArrayPromiseF32()
{
    return taihe::make_holder<ArrayPromiseF32Impl, ::array_test::ArrayPromiseF32>();
}

::array_test::ArrayPromiseF64 GetArrayPromiseF64()
{
    return taihe::make_holder<ArrayPromiseF64Impl, ::array_test::ArrayPromiseF64>();
}

::array_test::ArrayPromiseString GetArrayPromiseString()
{
    return taihe::make_holder<ArrayPromiseStringImpl, ::array_test::ArrayPromiseString>();
}

::array_test::ArrayPromiseBool GetArrayPromiseBool()
{
    return taihe::make_holder<ArrayPromiseBoolImpl, ::array_test::ArrayPromiseBool>();
}

::array_test::ArrayPromiseU8 GetArrayPromiseU8()
{
    return taihe::make_holder<ArrayPromiseU8Impl, ::array_test::ArrayPromiseU8>();
}

::array_test::ArrayPromiseU16 GetArrayPromiseU16()
{
    return taihe::make_holder<ArrayPromiseU16Impl, ::array_test::ArrayPromiseU16>();
}

::array_test::ArrayNest GetArrayNest()
{
    return taihe::make_holder<ArrayNestImpl, ::array_test::ArrayNest>();
}

::array_test::ArrayOptional GetArrayOptional()
{
    return taihe::make_holder<ArrayOptionalImpl, ::array_test::ArrayOptional>();
}
}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_SumArray(SumArray);
TH_EXPORT_CPP_API_GetArrayValue(GetArrayValue);
TH_EXPORT_CPP_API_ToStingArray(ToStingArray);
TH_EXPORT_CPP_API_MakeIntArray(MakeIntArray);
TH_EXPORT_CPP_API_MakeEnumArray(MakeEnumArray);
TH_EXPORT_CPP_API_MakeRecordArray(MakeRecordArray);
TH_EXPORT_CPP_API_MakeStructArray(MakeStructArray);
TH_EXPORT_CPP_API_MakeIntArray2(MakeIntArray2);
TH_EXPORT_CPP_API_ChangeEnumArray(ChangeEnumArray);
TH_EXPORT_CPP_API_ChangeRecordArray(ChangeRecordArray);
TH_EXPORT_CPP_API_ChangeStructArray(ChangeStructArray);
TH_EXPORT_CPP_API_FetchBinaryDataSync(FetchBinaryDataSync);
TH_EXPORT_CPP_API_MakeStructArrayArray(MakeStructArrayArray);
TH_EXPORT_CPP_API_GetArrayI8(GetArrayI8);
TH_EXPORT_CPP_API_GetArrayI16(GetArrayI16);
TH_EXPORT_CPP_API_GetArrayI32(GetArrayI32);
TH_EXPORT_CPP_API_GetArrayI64(GetArrayI64);
TH_EXPORT_CPP_API_GetArrayF32(GetArrayF32);
TH_EXPORT_CPP_API_GetArrayF64(GetArrayF64);
TH_EXPORT_CPP_API_GetArrayString(GetArrayString);
TH_EXPORT_CPP_API_GetArrayBool(GetArrayBool);
TH_EXPORT_CPP_API_GetArrayEnum(GetArrayEnum);
TH_EXPORT_CPP_API_GetArrayRecord(GetArrayRecord);
TH_EXPORT_CPP_API_GetArrayRecordStrI8(GetArrayRecordStrI8);
TH_EXPORT_CPP_API_GetArrayRecordStrF32(GetArrayRecordStrF32);
TH_EXPORT_CPP_API_GetArrayRecordStrF64(GetArrayRecordStrF64);
TH_EXPORT_CPP_API_GetArrayRecordStrBool(GetArrayRecordStrBool);
TH_EXPORT_CPP_API_GetArrayUnion(GetArrayUnion);
TH_EXPORT_CPP_API_GetArrayPromiseI8(GetArrayPromiseI8);
TH_EXPORT_CPP_API_GetArrayPromiseI16(GetArrayPromiseI16);
TH_EXPORT_CPP_API_GetArrayPromiseI32(GetArrayPromiseI32);
TH_EXPORT_CPP_API_GetArrayPromiseI64(GetArrayPromiseI64);
TH_EXPORT_CPP_API_GetArrayPromiseF32(GetArrayPromiseF32);
TH_EXPORT_CPP_API_GetArrayPromiseF64(GetArrayPromiseF64);
TH_EXPORT_CPP_API_GetArrayPromiseString(GetArrayPromiseString);
TH_EXPORT_CPP_API_GetArrayPromiseBool(GetArrayPromiseBool);
TH_EXPORT_CPP_API_GetArrayPromiseU8(GetArrayPromiseU8);
TH_EXPORT_CPP_API_GetArrayPromiseU16(GetArrayPromiseU16);
TH_EXPORT_CPP_API_GetArrayNest(GetArrayNest);
TH_EXPORT_CPP_API_GetArrayOptional(GetArrayOptional);
// NOLINTEND