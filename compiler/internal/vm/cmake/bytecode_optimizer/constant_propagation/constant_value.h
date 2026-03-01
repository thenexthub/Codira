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

#ifndef BYTECODE_OPTIMIZER_CONSTANT_PROPAGATION_CONSTANT_VALUE_H
#define BYTECODE_OPTIMIZER_CONSTANT_PROPAGATION_CONSTANT_VALUE_H

namespace panda::bytecodeopt {

class ConstantValue {
public:
    enum ConstantType {
        CONSTANT_BOOL,
        CONSTANT_INT32,
        CONSTANT_INT64,
        CONSTANT_DOUBLE,
        CONSTANT_STRING,
        CONSTANT_INVALID
    };

    explicit ConstantValue()
        : type_(CONSTANT_INVALID), value_() {}

    ConstantValue(const ConstantValue &other) = default;
    ConstantValue& operator=(const ConstantValue &other) = default;
    ~ConstantValue() = default;

    explicit ConstantValue(bool val)
        : type_(CONSTANT_BOOL), value_(val) {}

    explicit ConstantValue(int32_t val)
        : type_(CONSTANT_INT32), value_(val) {}

    explicit ConstantValue(int64_t val)
        : type_(CONSTANT_INT64), value_(val) {}

    explicit ConstantValue(double val)
        : type_(CONSTANT_DOUBLE), value_(val) {}

    explicit ConstantValue(std::string val)
        : type_(CONSTANT_STRING), value_(val) {}

    ConstantType GetType() const
    {
        return type_;
    }

    template<class T>
    T GetValue() const
    {
        ASSERT(std::holds_alternative<T>(value_));
        return std::get<T>(value_);
    }

    auto& GetValue() const
    {
        return value_;
    }

    std::string ToString()
    {
        std::stringstream ss;
        switch (type_) {
            case CONSTANT_BOOL:
                ss << "[Bool: " << (std::get<bool>(value_) ? "True" : "False") << "]";
                break;
            case CONSTANT_INT32:
                ss << "[Int32: " << std::get<int32_t>(value_) << "]";
                break;
            case CONSTANT_INT64:
                ss << "[Int64: " << std::get<int64_t>(value_) << "]";
                break;
            case CONSTANT_DOUBLE:
                ss << "[Double: " << std::get<double>(value_) << "]";
                break;
            case CONSTANT_STRING:
                ss << "[String: " << std::get<std::string>(value_) << "]";
                break;
            default:
                ss << "[INVALID]";
        }
        return ss.str();
    }

    bool operator==(const ConstantValue &other) const noexcept
    {
        return type_ == other.type_ && value_ == other.value_;
    }

private:
    ConstantType type_;
    std::variant<bool, int32_t, int64_t, double, std::string> value_;
};

}  // namespace panda::bytecodeopt

#endif  // BYTECODE_OPTIMIZER_CONSTANT_PROPAGATION_CONSTANT_VALUE_H

