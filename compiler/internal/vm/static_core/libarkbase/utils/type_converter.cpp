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

#include "type_converter.h"

#include <array>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <string_view>

namespace ark::helpers {

ValueUnit::ValueUnit(uint64_t value, std::string_view literal) : value_(value), literal_(literal) {}

ValueUnit::ValueUnit(double value, std::string_view literal) : value_(value), literal_(literal) {}

void ValueUnit::SetPrecision(size_t precision)
{
    precision_ = precision;
}

std::variant<double, uint64_t> ValueUnit::GetValue() const
{
    return value_;
}

double ValueUnit::GetDoubleValue() const
{
    return std::get<double>(value_);
}

uint64_t ValueUnit::GetUint64Value() const
{
    return std::get<uint64_t>(value_);
}

std::string_view ValueUnit::GetLiteral() const
{
    return literal_;
}

size_t ValueUnit::GetPrecision() const
{
    return precision_;
}

bool operator==(const ValueUnit &lhs, const ValueUnit &rhs)
{
    constexpr size_t NUMERAL_SYSTEM = 10;
    if (lhs.GetLiteral() != rhs.GetLiteral()) {
        return false;
    }
    if (lhs.GetValue().index() != rhs.GetValue().index()) {
        return false;
    }
    if (lhs.GetValue().index() == 0U) {
        return std::fabs(lhs.GetDoubleValue() - rhs.GetDoubleValue()) <
               std::pow(NUMERAL_SYSTEM, -std::max(lhs.GetPrecision(), rhs.GetPrecision()));
    }
    if (lhs.GetValue().index() == 1U) {
        return lhs.GetUint64Value() == rhs.GetUint64Value();
    }
    return false;
}

bool operator!=(const ValueUnit &lhs, const ValueUnit &rhs)
{
    return !(lhs == rhs);
}

std::ostream &operator<<(std::ostream &os, const ValueUnit &element)
{
    if (element.GetValue().index() == 0U) {
        os << std::fixed << std::setprecision(static_cast<int>(element.GetPrecision())) << element.GetDoubleValue()
           << std::setprecision(-1);
    } else {
        os << element.GetUint64Value();
    }
    return os << element.GetLiteral();
}

template <size_t SIZE>
ValueUnit TypeConverter(const std::array<double, SIZE> &coeffs, const std::array<std::string_view, SIZE + 1> &literals,
                        uint64_t valueBaseDimension)
{
    constexpr double LIMIT = 1.0;
    double divisionRatio = 1;
    auto valueBaseDimensionDouble = static_cast<double>(valueBaseDimension);
    for (size_t indexCoeff = 0; indexCoeff < SIZE; ++indexCoeff) {
        if (valueBaseDimensionDouble / (divisionRatio * coeffs[indexCoeff]) < LIMIT) {
            return ValueUnit(valueBaseDimensionDouble / divisionRatio, literals[indexCoeff]);
        }
        divisionRatio *= coeffs[indexCoeff];
    }

    return ValueUnit(valueBaseDimensionDouble / divisionRatio, literals[SIZE]);
}

ValueUnit TimeConverter(uint64_t timesInNanos)
{
    return TypeConverter(COEFFS_TIME, LITERALS_TIME, timesInNanos);
}

ValueUnit MemoryConverter(uint64_t bytes)
{
    ValueUnit bytesFormat = TypeConverter(COEFFS_MEMORY, LITERALS_MEMORY, bytes);
    bytesFormat.SetPrecision(0);
    return bytesFormat;
}

ValueUnit ValueConverter(uint64_t value, ValueType type)
{
    switch (type) {
        case ValueType::VALUE_TYPE_TIME:
            return TimeConverter(value);
        case ValueType::VALUE_TYPE_MEMORY:
            return MemoryConverter(value);
        default:
            return ValueUnit(value, "");
    }
}

}  // namespace ark::helpers
