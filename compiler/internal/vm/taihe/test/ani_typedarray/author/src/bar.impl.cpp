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
#include "bar.impl.hpp"
#include "bar.TypedArray1.proj.0.hpp"
#include "bar.TypedArray1.proj.1.hpp"
#include "bar.proj.hpp"
#include "stdexcept"
#include "taihe/map.hpp"
#include "taihe/runtime.hpp"

#include <cmath>
#include <iomanip>
#include <limits>
#include <numeric>

using namespace taihe;
using namespace bar;

namespace {
// To be implemented.

class TypedArrInfoImpl {
public:
    array<uint8_t> uint8Arr = {1, 2, 3, 4, 5};
    array<int8_t> int8Arr = {1, -2, 3, -4, 5};
    array<uint16_t> uint16Arr = {10, 20, 30, 40, 50};
    array<int16_t> int16Arr = {10, -20, 30, -40, 50};
    array<uint32_t> uint32Arr = {100, 200, 300, 400, 500};
    array<int32_t> int32Arr = {100, -200, 300, -400, 500};
    array<uint64_t> uint64Arr = {1000, 2000, 3000, 4000, 5000};
    array<int64_t> int64Arr = {1000, -2000, 3000, -4000, 5000};
    array<float> float32Arr = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    array<double> float64Arr = {1.0, 2.0, 3.0, 4.0, 5.0};

    TypedArrInfoImpl() {}

    void CreateUint8Array(::taihe::array_view<uint8_t> a)
    {
        this->uint8Arr = a;
        std::cout << "createUint8Array uint8Arr length: " << this->uint8Arr.size() << std::endl;
        std::cout << "createUint8Array uint8Arr value: ";
        for (auto val : this->uint8Arr) {
            std::cout << static_cast<int>(val) << " ";
        }
        std::cout << std::endl;
    }

    ::taihe::array<uint8_t> PrintUint8Array(::taihe::array_view<uint8_t> a)
    {
        this->uint8Arr = a;
        return this->uint8Arr;
    }

    void SetUint8Array(::taihe::array_view<uint8_t> a)
    {
        this->uint8Arr = a;
        std::cout << "setUint8Array uint8Arr length: " << this->uint8Arr.size() << std::endl;
        std::cout << "setUint8Array uint8Arr value: ";
        for (auto val : this->uint8Arr) {
            std::cout << static_cast<int>(val) << " ";
        }
        std::cout << std::endl;
    }

    ::taihe::array<uint8_t> GetUint8Array()
    {
        return this->uint8Arr;
    }

    void CreateInt8Array(::taihe::array_view<int8_t> a)
    {
        this->int8Arr = a;
        std::cout << "createInt8Array int8Arr length: " << this->int8Arr.size() << std::endl;
        std::cout << "createInt8Array int8Arr value: ";
        for (auto val : this->int8Arr) {
            std::cout << static_cast<int>(val) << " ";
        }
        std::cout << std::endl;
    }

    ::taihe::array<int8_t> PrintInt8Array(::taihe::array_view<int8_t> a)
    {
        this->int8Arr = a;
        return this->int8Arr;
    }

    void SetInt8Array(::taihe::array_view<int8_t> a)
    {
        this->int8Arr = a;
        std::cout << "setInt8Array int8Arr length: " << this->int8Arr.size() << std::endl;
        std::cout << "setInt8Array int8Arr value: ";
        for (auto val : this->int8Arr) {
            std::cout << static_cast<int>(val) << " ";
        }
        std::cout << std::endl;
    }

    ::taihe::array<int8_t> GetInt8Array()
    {
        return this->int8Arr;
    }

    void CreateUint16Array(::taihe::array_view<uint16_t> a)
    {
        this->uint16Arr = a;
        std::cout << "createUint16Array uint16Arr length: " << this->uint16Arr.size() << std::endl;
        std::cout << "createUint16Array uint16Arr value: ";
        for (auto val : this->uint16Arr) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    ::taihe::array<uint16_t> PrintUint16Array(::taihe::array_view<uint16_t> a)
    {
        this->uint16Arr = a;
        return this->uint16Arr;
    }

    void SetUint16Array(::taihe::array_view<uint16_t> a)
    {
        this->uint16Arr = a;
        std::cout << "setUint16Array uint16Arr length: " << this->uint16Arr.size() << std::endl;
        std::cout << "setUint16Array uint16Arr value: ";
        for (auto val : this->uint16Arr) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    ::taihe::array<uint16_t> GetUint16Array()
    {
        return this->uint16Arr;
    }

    void CreateInt16Array(::taihe::array_view<int16_t> a)
    {
        this->int16Arr = a;
        std::cout << "createInt16Array int16Arr length: " << this->int16Arr.size() << std::endl;
        std::cout << "createInt16Array int16Arr value: ";
        for (auto val : this->int16Arr) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    ::taihe::array<int16_t> PrintInt16Array(::taihe::array_view<int16_t> a)
    {
        this->int16Arr = a;
        return this->int16Arr;
    }

    void SetInt16Array(::taihe::array_view<int16_t> a)
    {
        this->int16Arr = a;
        std::cout << "setInt16Array int16Arr length: " << this->int16Arr.size() << std::endl;
        std::cout << "setInt16Array int16Arr value: ";
        for (auto val : this->int16Arr) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    ::taihe::array<int16_t> GetInt16Array()
    {
        return this->int16Arr;
    }

    void CreateUint32Array(::taihe::array_view<uint32_t> a)
    {
        this->uint32Arr = a;
        std::cout << "createUint32Array uint32Arr length: " << this->uint32Arr.size() << std::endl;
        std::cout << "createUint32Array uint32Arr value: ";
        for (auto val : this->uint32Arr) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    ::taihe::array<uint32_t> PrintUint32Array(::taihe::array_view<uint32_t> a)
    {
        this->uint32Arr = a;
        return this->uint32Arr;
    }

    void SetUint32Array(::taihe::array_view<uint32_t> a)
    {
        this->uint32Arr = a;
        std::cout << "setUint32Array uint32Arr length: " << this->uint32Arr.size() << std::endl;
        std::cout << "setUint32Array uint32Arr value: ";
        for (auto val : this->uint32Arr) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    ::taihe::array<uint32_t> GetUint32Array()
    {
        return this->uint32Arr;
    }

    void CreateInt32Array(::taihe::array_view<int32_t> a)
    {
        this->int32Arr = a;
        std::cout << "createInt32Array int32Arr length: " << this->int32Arr.size() << std::endl;
        std::cout << "createInt32Array int32Arr value: ";
        for (auto val : this->int32Arr) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    ::taihe::array<int32_t> PrintInt32Array(::taihe::array_view<int32_t> a)
    {
        this->int32Arr = a;
        return this->int32Arr;
    }

    void SetInt32Array(::taihe::array_view<int32_t> a)
    {
        this->int32Arr = a;
        std::cout << "setInt32Array int32Arr length: " << this->int32Arr.size() << std::endl;
        std::cout << "setInt32Array int32Arr value: ";
        for (auto val : this->int32Arr) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    ::taihe::array<int32_t> GetInt32Array()
    {
        return this->int32Arr;
    }

    void CreateUint64Array(::taihe::array_view<uint64_t> a)
    {
        this->uint64Arr = a;
        std::cout << "createUint64Array uint64Arr length: " << this->uint64Arr.size() << std::endl;
        std::cout << "createUint64Array uint64Arr value: ";
        for (auto val : this->uint64Arr) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    ::taihe::array<uint64_t> PrintUint64Array(::taihe::array_view<uint64_t> a)
    {
        this->uint64Arr = a;
        return this->uint64Arr;
    }

    void SetUint64Array(::taihe::array_view<uint64_t> a)
    {
        this->uint64Arr = a;
        std::cout << "setUint64Array uint64Arr length: " << this->uint64Arr.size() << std::endl;
        std::cout << "setUint64Array uint64Arr value: ";
        for (auto val : this->uint64Arr) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    ::taihe::array<uint64_t> GetUint64Array()
    {
        return this->uint64Arr;
    }

    void CreateInt64Array(::taihe::array_view<int64_t> a)
    {
        this->int64Arr = a;
        std::cout << "createInt64Array int64Arr length: " << this->int64Arr.size() << std::endl;
        std::cout << "createInt64Array int64Arr value: ";
        for (auto val : this->int64Arr) {
            std::cout << static_cast<long long>(val) << " ";
        }
        std::cout << std::endl;
    }

    ::taihe::array<int64_t> PrintInt64Array(::taihe::array_view<int64_t> a)
    {
        this->int64Arr = a;
        return this->int64Arr;
    }

    void SetInt64Array(::taihe::array_view<int64_t> a)
    {
        this->int64Arr = a;
        std::cout << "setInt64Array int64Arr length: " << this->int64Arr.size() << std::endl;
        std::cout << "setInt64Array int64Arr value: ";
        for (auto val : this->int64Arr) {
            std::cout << static_cast<long long>(val) << " ";
        }
        std::cout << std::endl;
    }

    ::taihe::array<int64_t> GetInt64Array()
    {
        return this->int64Arr;
    }

    void CreateFloat32Array(::taihe::array_view<float> a)
    {
        this->float32Arr = a;
        std::cout << "createFloat32Array float32Arr length: " << this->float32Arr.size() << std::endl;
        std::cout << "createFloat32Array float32Arr value: ";
        for (auto val : this->float32Arr) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    ::taihe::array<float> PrintFloat32Array(::taihe::array_view<float> a)
    {
        this->float32Arr = a;
        return this->float32Arr;
    }

    void SetFloat32Array(::taihe::array_view<float> a)
    {
        this->float32Arr = a;
        std::cout << "setFloat32Array float32Arr length: " << this->float32Arr.size() << std::endl;
        std::cout << "setFloat32Array float32Arr value: ";
        for (auto val : this->float32Arr) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    ::taihe::array<float> GetFloat32Array()
    {
        return this->float32Arr;
    }

    void CreateFloat64Array(::taihe::array_view<double> a)
    {
        this->float64Arr = a;
        std::cout << "createFloat64Array float64Arr length: " << this->float64Arr.size() << std::endl;
        std::cout << "createFloat64Array float64Arr value: ";
        for (auto val : this->float64Arr) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    ::taihe::array<double> PrintFloat64Array(::taihe::array_view<double> a)
    {
        this->float64Arr = a;
        return this->float64Arr;
    }

    void SetFloat64Array(::taihe::array_view<double> a)
    {
        this->float64Arr = a;
        std::cout << "setFloat64Array float64Arr length: " << this->float64Arr.size() << std::endl;
        std::cout << "setFloat64Array float64Arr value: ";
        for (auto val : this->float64Arr) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    ::taihe::array<double> GetFloat64Array()
    {
        return this->float64Arr;
    }

    ::taihe::array<int64_t> ConvertToInt64Array(::taihe::array_view<uint8_t> a, ::taihe::array_view<uint16_t> b,
                                                ::taihe::array_view<uint32_t> c, ::taihe::array_view<uint64_t> d,
                                                ::taihe::array_view<int8_t> e)
    {
        if (a.size() != b.size() || a.size() != c.size() || a.size() != d.size() || a.size() != e.size()) {
            taihe::set_error("Input arrays must have the same length!");
        }
        this->uint8Arr = a;
        this->uint16Arr = b;
        this->uint32Arr = c;
        this->uint64Arr = d;
        this->int8Arr = e;
        array<int64_t> result(this->uint8Arr.size());
        for (size_t i = 0; i < this->uint8Arr.size(); ++i) {
            result[i] = static_cast<int64_t>(this->uint8Arr[i]) + static_cast<int64_t>(this->uint16Arr[i]) +
                        static_cast<int64_t>(this->uint32Arr[i]) + static_cast<int64_t>(this->uint64Arr[i]) +
                        static_cast<int64_t>(this->int8Arr[i]);
        }
        return result;
    }

    ::taihe::array<double> ConvertToFloat64Array(::taihe::array_view<int16_t> a, ::taihe::array_view<int32_t> b,
                                                 ::taihe::array_view<int64_t> c, ::taihe::array_view<float> d,
                                                 ::taihe::array_view<double> e)
    {
        if (a.size() != b.size() || a.size() != c.size() || a.size() != d.size() || a.size() != e.size()) {
            taihe::set_error("Input arrays must have the same length!");
        }
        this->int16Arr = a;
        this->int32Arr = b;
        this->int64Arr = c;
        this->float32Arr = d;
        this->float64Arr = e;
        array<double> result(this->int16Arr.size());
        for (size_t i = 0; i < this->int16Arr.size(); ++i) {
            result[i] = static_cast<double>(this->int16Arr[i]) + static_cast<double>(this->int32Arr[i]) +
                        static_cast<double>(this->int64Arr[i]) + static_cast<double>(this->float32Arr[i]) +
                        this->float64Arr[i];
        }
        return result;
    }

    ::taihe::array<int32_t> GetUint8ArrayOptional(::taihe::optional_view<::taihe::array<uint8_t>> a)
    {
        if (!a.has_value()) {
            return ::taihe::array<int32_t>(0);
        }
        auto const &src = a.value();
        ::taihe::array<int32_t> result(src.size());
        for (size_t i = 0; i < src.size(); ++i) {
            result[i] = static_cast<int32_t>(src[i]);
        }
        return result;
    }

    ::taihe::array<int32_t> GetInt8ArrayOptional(::taihe::optional_view<::taihe::array<int8_t>> a)
    {
        if (!a.has_value()) {
            return ::taihe::array<int32_t>(0);
        }
        auto const &src = a.value();
        ::taihe::array<int32_t> result(src.size());
        for (size_t i = 0; i < src.size(); ++i) {
            result[i] = static_cast<int32_t>(src[i]);
        }
        return result;
    }

    ::taihe::array<int32_t> GetUint16ArrayOptional(::taihe::optional_view<::taihe::array<uint16_t>> a)
    {
        if (!a.has_value()) {
            return ::taihe::array<int32_t>(0);
        }
        auto const &src = a.value();
        ::taihe::array<int32_t> result(src.size());
        for (size_t i = 0; i < src.size(); ++i) {
            result[i] = static_cast<int32_t>(src[i]);
        }
        return result;
    }

    ::taihe::array<int32_t> GetInt16ArrayOptional(::taihe::optional_view<::taihe::array<int16_t>> a)
    {
        if (!a.has_value()) {
            return ::taihe::array<int32_t>(0);
        }
        auto const &src = a.value();
        ::taihe::array<int32_t> result(src.size());
        for (size_t i = 0; i < src.size(); ++i) {
            result[i] = static_cast<int32_t>(src[i]);
        }
        return result;
    }

    ::taihe::array<int32_t> GetUint32ArrayOptional(::taihe::optional_view<::taihe::array<uint32_t>> a)
    {
        if (!a.has_value()) {
            return ::taihe::array<int32_t>(0);
        }
        auto const &src = a.value();
        ::taihe::array<int32_t> result(src.size());
        for (size_t i = 0; i < src.size(); ++i) {
            if (src[i] > static_cast<uint32_t>(std::numeric_limits<int32_t>::max())) {
                result[i] = std::numeric_limits<int32_t>::max();
            } else {
                result[i] = static_cast<int32_t>(src[i]);
            }
        }
        return result;
    }

    ::taihe::array<int32_t> GetInt32ArrayOptional(::taihe::optional_view<::taihe::array<int32_t>> a)
    {
        if (!a.has_value()) {
            return ::taihe::array<int32_t>(0);
        }
        return a.value();
    }

    ::taihe::array<int32_t> GetUint64ArrayOptional(::taihe::optional_view<::taihe::array<uint64_t>> a)
    {
        if (!a.has_value()) {
            return ::taihe::array<int32_t>(0);
        }
        auto const &src = a.value();
        ::taihe::array<int32_t> result(src.size());
        for (size_t i = 0; i < src.size(); ++i) {
            uint64_t const val = src[i];
            if (val > static_cast<uint64_t>(std::numeric_limits<int32_t>::max())) {
                result[i] = std::numeric_limits<int32_t>::max();
            } else {
                result[i] = static_cast<int32_t>(val);
            }
        }
        return result;
    }

    ::taihe::array<int32_t> GetInt64ArrayOptional(::taihe::optional_view<::taihe::array<int64_t>> a)
    {
        if (!a.has_value()) {
            return ::taihe::array<int32_t>(0);
        }
        auto const &src = a.value();
        ::taihe::array<int32_t> result(src.size());
        for (size_t i = 0; i < src.size(); ++i) {
            int64_t const val = src[i];
            if (val < std::numeric_limits<int32_t>::min()) {
                result[i] = std::numeric_limits<int32_t>::min();
            } else if (val > std::numeric_limits<int32_t>::max()) {
                result[i] = std::numeric_limits<int32_t>::max();
            } else {
                result[i] = static_cast<int32_t>(val);
            }
        }
        return result;
    }

    ::taihe::array<int32_t> GetFloat32ArrayOptional(::taihe::optional_view<::taihe::array<float>> a)
    {
        if (!a.has_value()) {
            return ::taihe::array<int32_t>(0);
        }
        auto const &src = a.value();
        ::taihe::array<int32_t> result(src.size());
        for (size_t i = 0; i < src.size(); ++i) {
            float const val = src[i];
            if (std::isnan(val)) {
                // 处理非数字
                std::cout << "NaN" << std::endl;
                taihe::set_error("NaN value is not allowed!");
            }
            // 处理数据溢出
            if (val < static_cast<float>(std::numeric_limits<int32_t>::min())) {
                result[i] = std::numeric_limits<int32_t>::min();
            } else if (val > static_cast<float>(std::numeric_limits<int32_t>::max())) {
                result[i] = std::numeric_limits<int32_t>::max();
            } else {
                result[i] = static_cast<int32_t>(val);
            }
        }
        return result;
    }

    ::taihe::array<int32_t> GetFloat64ArrayOptional(::taihe::optional_view<::taihe::array<double>> a)
    {
        if (!a.has_value()) {
            return ::taihe::array<int32_t>(0);
        }
        auto const &src = a.value();
        ::taihe::array<int32_t> result(src.size());
        for (size_t i = 0; i < src.size(); ++i) {
            double const val = src[i];
            if (std::isnan(val)) {
                std::cout << "NaN" << std::endl;
                taihe::set_error("NaN value is not allowed!");
            }
            if (val < static_cast<double>(std::numeric_limits<int32_t>::min())) {
                result[i] = std::numeric_limits<int32_t>::min();
            } else if (val > static_cast<double>(std::numeric_limits<int32_t>::max())) {
                result[i] = std::numeric_limits<int32_t>::max();
            } else {
                result[i] = static_cast<int32_t>(val);
            }
        }
        return result;
    }

    ::taihe::map<::taihe::string, ::taihe::array<uint8_t>> MapUint8Arrays(
        ::taihe::map_view<::taihe::string, ::taihe::array<uint8_t>> m)
    {
        ::taihe::map<::taihe::string, ::taihe::array<uint8_t>> result;
        for (auto const &val : m) {
            result.emplace(val.first, val.second);
        }
        return result;
    }

    ::taihe::map<::taihe::string, ::taihe::array<int8_t>> MapInt8Arrays(
        ::taihe::map_view<::taihe::string, ::taihe::array<int8_t>> m)
    {
        ::taihe::map<::taihe::string, ::taihe::array<int8_t>> result;
        for (auto const &val : m) {
            result.emplace(val.first, val.second);
        }
        return result;
    }

    ::taihe::map<::taihe::string, ::taihe::array<uint16_t>> MapUint16Arrays(
        ::taihe::map_view<::taihe::string, ::taihe::array<uint16_t>> m)
    {
        ::taihe::map<::taihe::string, ::taihe::array<uint16_t>> result;
        for (auto const &val : m) {
            result.emplace(val.first, val.second);
        }
        return result;
    }

    ::taihe::map<::taihe::string, ::taihe::array<int16_t>> MapInt16Arrays(
        ::taihe::map_view<::taihe::string, ::taihe::array<int16_t>> m)
    {
        ::taihe::map<::taihe::string, ::taihe::array<int16_t>> result;
        for (auto const &val : m) {
            result.emplace(val.first, val.second);
        }
        return result;
    }

    ::taihe::map<::taihe::string, ::taihe::array<uint32_t>> MapUint32Arrays(
        ::taihe::map_view<::taihe::string, ::taihe::array<uint32_t>> m)
    {
        ::taihe::map<::taihe::string, ::taihe::array<uint32_t>> result;
        for (auto const &val : m) {
            result.emplace(val.first, val.second);
        }
        return result;
    }

    ::taihe::map<::taihe::string, ::taihe::array<int32_t>> MapInt32Arrays(
        ::taihe::map_view<::taihe::string, ::taihe::array<int32_t>> m)
    {
        ::taihe::map<::taihe::string, ::taihe::array<int32_t>> result;
        for (auto const &val : m) {
            result.emplace(val.first, val.second);
        }
        return result;
    }

    ::taihe::map<::taihe::string, ::taihe::array<uint64_t>> MapUint64Arrays(
        ::taihe::map_view<::taihe::string, ::taihe::array<uint64_t>> m)
    {
        ::taihe::map<::taihe::string, ::taihe::array<uint64_t>> result;
        for (auto const &val : m) {
            result.emplace(val.first, val.second);
        }
        return result;
    }

    ::taihe::map<::taihe::string, ::taihe::array<int64_t>> MapInt64Arrays(
        ::taihe::map_view<::taihe::string, ::taihe::array<int64_t>> m)
    {
        ::taihe::map<::taihe::string, ::taihe::array<int64_t>> result;
        for (auto const &val : m) {
            result.emplace(val.first, val.second);
        }
        return result;
    }

    ::taihe::map<::taihe::string, ::taihe::array<float>> MapFloat32Arrays(
        ::taihe::map_view<::taihe::string, ::taihe::array<float>> m)
    {
        ::taihe::map<::taihe::string, ::taihe::array<float>> result;
        for (auto const &val : m) {
            result.emplace(val.first, val.second);
        }
        return result;
    }

    ::taihe::map<::taihe::string, ::taihe::array<double>> MapFloat64Arrays(
        ::taihe::map_view<::taihe::string, ::taihe::array<double>> m)
    {
        ::taihe::map<::taihe::string, ::taihe::array<double>> result;
        for (auto const &val : m) {
            result.emplace(val.first, val.second);
        }
        return result;
    }
};

int8_t SumUint8Array(array_view<uint8_t> v)
{
    return std::accumulate(v.begin(), v.end(), 0);
}

array<uint8_t> NewUint8Array(int64_t n, int8_t v)
{
    array<uint8_t> result(n);
    std::fill(result.begin(), result.end(), v);
    return result;
}

int16_t SumUint16Array(array_view<uint16_t> v)
{
    return std::accumulate(v.begin(), v.end(), 0);
}

array<uint16_t> NewUint16Array(int64_t n, int16_t v)
{
    array<uint16_t> result(n);
    std::fill(result.begin(), result.end(), v);
    return result;
}

int32_t SumUint32Array(array_view<uint32_t> v)
{
    return std::accumulate(v.begin(), v.end(), 0);
}

array<uint32_t> NewUint32Array(int64_t n, int32_t v)
{
    array<uint32_t> result(n);
    std::fill(result.begin(), result.end(), v);
    return result;
}

int64_t SumBigUint64Array(array_view<uint64_t> v)
{
    return std::accumulate(v.begin(), v.end(), 0);
}

array<uint64_t> NewBigUint64Array(int64_t n, int64_t v)
{
    array<uint64_t> result(n);
    std::fill(result.begin(), result.end(), v);
    return result;
}

int8_t SumInt8Array(array_view<int8_t> v)
{
    return std::accumulate(v.begin(), v.end(), 0);
}

array<int8_t> NewInt8Array(int64_t n, int8_t v)
{
    array<int8_t> result(n);
    std::fill(result.begin(), result.end(), v);
    return result;
}

int16_t SumInt16Array(array_view<int16_t> v)
{
    return std::accumulate(v.begin(), v.end(), 0);
}

array<int16_t> NewInt16Array(int64_t n, int16_t v)
{
    array<int16_t> result(n);
    std::fill(result.begin(), result.end(), v);
    return result;
}

int32_t SumInt32Array(array_view<int32_t> v)
{
    return std::accumulate(v.begin(), v.end(), 0);
}

array<int32_t> NewInt32Array(int64_t n, int32_t v)
{
    array<int32_t> result(n);
    std::fill(result.begin(), result.end(), v);
    return result;
}

int64_t SumBigInt64Array(array_view<int64_t> v)
{
    return std::accumulate(v.begin(), v.end(), 0);
}

array<int64_t> NewBigInt64Array(int64_t n, int64_t v)
{
    array<int64_t> result(n);
    std::fill(result.begin(), result.end(), v);
    return result;
}

float SumFloat32Array(array_view<float> v)
{
    return std::accumulate(v.begin(), v.end(), 0.0f);
}

array<float> NewFloat32Array(int64_t n, float v)
{
    array<float> result(n);
    std::fill(result.begin(), result.end(), v);
    return result;
}

double SumFloat64Array(array_view<double> v)
{
    return std::accumulate(v.begin(), v.end(), 0.0);
}

array<double> NewFloat64Array(int64_t n, double v)
{
    array<double> result(n);
    std::fill(result.begin(), result.end(), v);
    return result;
}

TypedArrInfo GetTypedArrInfo()
{
    return make_holder<TypedArrInfoImpl, TypedArrInfo>();
}

template <typename Iterator>
void PrintArray(std::string const &name, Iterator begin, Iterator end)
{
    std::cout << name << ": ";
    for (auto it = begin; it != end; ++it) {
        using ValueType = typename std::iterator_traits<Iterator>::value_type;
        // 处理整数类型（包括 int8_t, uint8_t, int16_t, uint16_t, ...）
        if constexpr (std::is_integral_v<ValueType>) {
            // 特殊处理 uint8_t 和 int8_t（避免被当作字符打印）
            if constexpr (std::is_same_v<ValueType, uint8_t> || std::is_same_v<ValueType, int8_t>) {
                std::cout << static_cast<int>(*it) << " ";
            } else {
                // 其他整型直接打印
                std::cout << *it << " ";
            }
        } else if constexpr (std::is_floating_point_v<ValueType>) {
            // 处理浮点类型（float, double）
            int const precision = 6;
            std::cout << std::fixed << std::setprecision(precision) << *it << " ";
        } else {
            // 其他未知类型（如 bool、char 等）
            std::cout << *it << " ";
        }
    }
    std::cout << std::endl;
}

void SetupStructAndPrint(::bar::TypedArray1 const &v)
{
    PrintArray("a (Uint8Array)", v.a.begin(), v.a.end());
    PrintArray("b (Int8Array)", v.b.begin(), v.b.end());
    PrintArray("c (Uint16Array)", v.c.begin(), v.c.end());
    PrintArray("d (Int16Array)", v.d.begin(), v.d.end());
    PrintArray("e (Uint32Array)", v.e.begin(), v.e.end());
    PrintArray("f (Int32Array)", v.f.begin(), v.f.end());
    PrintArray("g (Uint64Array)", v.g.begin(), v.g.end());
    PrintArray("h (Int64Array)", v.h.begin(), v.h.end());
    PrintArray("i (Float32Array)", v.i.begin(), v.i.end());
    PrintArray("j (Float64Array)", v.j.begin(), v.j.end());
}

::bar::MyUnion MakeMyUnion(int32_t n)
{
    int32_t const case1Key = 1;
    int32_t const case2Key = 2;
    int32_t const case3Key = 3;
    int32_t const case4Key = 4;
    int32_t const case5Key = 5;
    int32_t const case6Key = 6;
    int32_t const case7Key = 7;
    int32_t const case8Key = 8;
    int32_t const case9Key = 9;
    int32_t const case10Key = 10;

    array<uint8_t> u8Arr = {1, 2, 3, 4, 5};
    array<int8_t> i8Arr = {1, -2, 3, -4, 5};
    array<uint16_t> u16Arr = {10, 20, 30, 40, 50};
    array<int16_t> i16Arr = {10, -20, 30, -40, 50};
    array<uint32_t> u32Arr = {100, 200, 300, 400, 500};
    array<int32_t> i32Arr = {100, -200, 300, -400, 500};
    array<uint64_t> u64Arr = {1000, 2000, 3000, 4000, 5000};
    array<int64_t> i64Arr = {1000, -2000, 3000, -4000, 5000};
    array<float> f32Arr = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    array<double> f64Arr = {1.0, 2.0, 3.0, 4.0, 5.0};

    switch (n) {
        case case1Key:
            return MyUnion::make_u8Value(u8Arr);
        case case2Key:
            return MyUnion::make_i8Value(i8Arr);
        case case3Key:
            return MyUnion::make_u16Value(u16Arr);
        case case4Key:
            return MyUnion::make_i16Value(i16Arr);
        case case5Key:
            return MyUnion::make_u32Value(u32Arr);
        case case6Key:
            return MyUnion::make_i32Value(i32Arr);
        case case7Key:
            return MyUnion::make_u64Value(u64Arr);
        case case8Key:
            return MyUnion::make_i64Value(i64Arr);
        case case9Key:
            return MyUnion::make_f32Value(f32Arr);
        case case10Key:
            return MyUnion::make_f64Value(f64Arr);
        default:
            return MyUnion::make_empty();
    }
}

void ShowMyUnion(::bar::MyUnion const &u)
{
    auto printArray = [](auto const &ptr, char const *typeName) {
        std::cout << typeName << ": [";
        bool first = true;
        for (auto const &val : *ptr) {
            if (!first)
                std::cout << ", ";
            if constexpr (std::is_same_v<std::decay_t<decltype(val)>, uint8_t> ||
                          std::is_same_v<std::decay_t<decltype(val)>, int8_t>) {
                std::cout << static_cast<int>(val);
            } else {
                std::cout << val;
            }
            first = false;
        }
        std::cout << "]\n";
    };

    // 检查联合体中的各种可能类型
    if (auto ptr = u.get_u8Value_ptr()) {
        printArray(ptr, "u8");
    } else if (auto ptr = u.get_i8Value_ptr()) {
        printArray(ptr, "i8");
    } else if (auto ptr = u.get_u16Value_ptr()) {
        printArray(ptr, "u16");
    } else if (auto ptr = u.get_i16Value_ptr()) {
        printArray(ptr, "i16");
    } else if (auto ptr = u.get_u32Value_ptr()) {
        printArray(ptr, "u32");
    } else if (auto ptr = u.get_i32Value_ptr()) {
        printArray(ptr, "i32");
    } else if (auto ptr = u.get_u64Value_ptr()) {
        printArray(ptr, "u64");
    } else if (auto ptr = u.get_i64Value_ptr()) {
        printArray(ptr, "i64");
    } else if (auto ptr = u.get_f32Value_ptr()) {
        printArray(ptr, "f32");
    } else if (auto ptr = u.get_f64Value_ptr()) {
        printArray(ptr, "f64");
    } else {
        std::cout << "empty\n";
    }
}

}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_SumUint8Array(SumUint8Array);
TH_EXPORT_CPP_API_NewUint8Array(NewUint8Array);
TH_EXPORT_CPP_API_SumUint16Array(SumUint16Array);
TH_EXPORT_CPP_API_NewUint16Array(NewUint16Array);
TH_EXPORT_CPP_API_SumUint32Array(SumUint32Array);
TH_EXPORT_CPP_API_NewUint32Array(NewUint32Array);
TH_EXPORT_CPP_API_SumBigUint64Array(SumBigUint64Array);
TH_EXPORT_CPP_API_NewBigUint64Array(NewBigUint64Array);
TH_EXPORT_CPP_API_SumInt8Array(SumInt8Array);
TH_EXPORT_CPP_API_NewInt8Array(NewInt8Array);
TH_EXPORT_CPP_API_SumInt16Array(SumInt16Array);
TH_EXPORT_CPP_API_NewInt16Array(NewInt16Array);
TH_EXPORT_CPP_API_SumInt32Array(SumInt32Array);
TH_EXPORT_CPP_API_NewInt32Array(NewInt32Array);
TH_EXPORT_CPP_API_SumBigInt64Array(SumBigInt64Array);
TH_EXPORT_CPP_API_NewBigInt64Array(NewBigInt64Array);
TH_EXPORT_CPP_API_SumFloat32Array(SumFloat32Array);
TH_EXPORT_CPP_API_NewFloat32Array(NewFloat32Array);
TH_EXPORT_CPP_API_SumFloat64Array(SumFloat64Array);
TH_EXPORT_CPP_API_NewFloat64Array(NewFloat64Array);
TH_EXPORT_CPP_API_GetTypedArrInfo(GetTypedArrInfo);
TH_EXPORT_CPP_API_SetupStructAndPrint(SetupStructAndPrint);
TH_EXPORT_CPP_API_MakeMyUnion(MakeMyUnion);
TH_EXPORT_CPP_API_ShowMyUnion(ShowMyUnion);
// NOLINTEND