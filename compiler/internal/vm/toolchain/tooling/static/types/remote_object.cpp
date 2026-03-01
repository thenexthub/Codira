/**
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "types/remote_object.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <variant>

#include "libarkfile/helpers.h"
#include "libarkbase/macros.h"
#include "libarkbase/utils/json_builder.h"

#include "types/object_preview.h"
#include "types/property_descriptor.h"

namespace ark::tooling::inspector {

std::string RemoteObject::GetDescription(const RemoteObjectType::BigIntT &bigint)
{
    return (bigint.sign >= 0 ? "" : "-") + std::to_string(bigint.value);
}

std::string RemoteObject::GetDescription(const RemoteObjectType::ObjectT &object)
{
    return object.description.value_or(object.className);
}

std::string RemoteObject::GetDescription(const RemoteObjectType::ArrayT &array)
{
    return array.className + "(" + std::to_string(array.length) + ")";
}

std::string RemoteObject::GetDescription(const RemoteObjectType::FunctionT &function)
{
    std::stringstream desc;

    desc << "function " << function.name << "(";

    for (auto argIdx = 0U; argIdx < function.length; ++argIdx) {
        if (argIdx != 0) {
            desc << ", ";
        }
        desc << static_cast<char>('a' + argIdx);
    }

    desc << ") { [not available] }";

    return desc.str();
}

std::optional<RemoteObjectId> RemoteObject::GetObjectId() const
{
    if (auto object = std::get_if<RemoteObjectType::ObjectT>(&value_)) {
        return object->objectId;
    }
    if (auto array = std::get_if<RemoteObjectType::ArrayT>(&value_)) {
        return array->objectId;
    }
    if (auto function = std::get_if<RemoteObjectType::FunctionT>(&value_)) {
        return function->objectId;
    }
    return {};
}

RemoteObjectType RemoteObject::GetType() const
{
    if (std::holds_alternative<std::monostate>(value_)) {
        return RemoteObjectType("undefined");
    }
    if (std::holds_alternative<std::nullptr_t>(value_)) {
        return RemoteObjectType("object", "null");
    }
    if (std::holds_alternative<bool>(value_)) {
        return RemoteObjectType("boolean");
    }
    if (std::holds_alternative<RemoteObjectType::NumberT>(value_)) {
        return RemoteObjectType("number");
    }
    if (std::holds_alternative<RemoteObjectType::BigIntT>(value_)) {
        return RemoteObjectType("bigint");
    }
    if (std::holds_alternative<std::string>(value_)) {
        return RemoteObjectType("string");
    }
    if (std::holds_alternative<RemoteObjectType::SymbolT>(value_)) {
        return RemoteObjectType("symbol");
    }
    if (std::holds_alternative<RemoteObjectType::ObjectT>(value_)) {
        return RemoteObjectType("object");
    }
    if (std::holds_alternative<RemoteObjectType::ArrayT>(value_)) {
        return RemoteObjectType("object", "array");
    }
    if (std::holds_alternative<RemoteObjectType::FunctionT>(value_)) {
        return RemoteObjectType("function");
    }

    UNREACHABLE();
}

void RemoteObject::Serialize(JsonObjectBuilder &builder) const
{
    GetType().Serialize(builder);

    if (std::holds_alternative<std::nullptr_t>(value_)) {
        builder.AddProperty("value", nullptr);
    } else if (auto boolean = std::get_if<bool>(&value_)) {
        builder.AddProperty("value", *boolean);
    } else if (auto number = std::get_if<RemoteObjectType::NumberT>(&value_)) {
        if (auto integer = std::get_if<int32_t>(number)) {
            builder.AddProperty("value", *integer);
        } else if (auto floatingPoint = std::get_if<double>(number)) {
            builder.AddProperty("value", *floatingPoint);
        } else {
            UNREACHABLE();
        }
    } else if (auto bigint = std::get_if<RemoteObjectType::BigIntT>(&value_)) {
        builder.AddProperty("unserializableValue", GetDescription(*bigint));
    } else if (auto string = std::get_if<std::string>(&value_)) {
        builder.AddProperty("value", *string);
    } else if (auto symbol = std::get_if<RemoteObjectType::SymbolT>(&value_)) {
        builder.AddProperty("description", symbol->description);
    } else if (auto object = std::get_if<RemoteObjectType::ObjectT>(&value_)) {
        builder.AddProperty("className", object->className);
        builder.AddProperty("description", GetDescription(*object));
    } else if (auto array = std::get_if<RemoteObjectType::ArrayT>(&value_)) {
        builder.AddProperty("className", array->className);
        builder.AddProperty("description", GetDescription(*array));
    } else if (auto function = std::get_if<RemoteObjectType::FunctionT>(&value_)) {
        builder.AddProperty("className", function->className);
        builder.AddProperty("description", GetDescription(*function));
    }

    if (auto objectId = GetObjectId()) {
        builder.AddProperty("objectId", std::to_string(*objectId));
    }

    if (preview_.has_value()) {
        builder.AddProperty("preview", *preview_);
    }
}

}  // namespace ark::tooling::inspector
