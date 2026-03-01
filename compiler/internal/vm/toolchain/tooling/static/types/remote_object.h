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
#ifndef PANDA_TOOLING_INSPECTOR_TYPES_REMOTE_OBJECT_H
#define PANDA_TOOLING_INSPECTOR_TYPES_REMOTE_OBJECT_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "types/numeric_id.h"
#include "types/remote_object_type.h"
#include "types/object_preview.h"

namespace ark {
class JsonObjectBuilder;
}  // namespace ark

namespace ark::tooling::inspector {

class RemoteObject final : public JsonSerializable {
public:
    static RemoteObject Undefined()
    {
        return RemoteObject();
    }

    static RemoteObject Null()
    {
        return RemoteObject(nullptr);
    }

    static RemoteObject Boolean(bool boolean)
    {
        return RemoteObject(boolean);
    }

    static RemoteObject Number(int32_t number)
    {
        return RemoteObject(RemoteObjectType::NumberT {number});
    }

    template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    static RemoteObject Number(T number)
    {
        return RemoteObject(RemoteObjectType::NumberT {number});
    }

    template <typename T,
              std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T> && sizeof(int32_t) < sizeof(T), int> = 0>
    static RemoteObject Number(T number)
    {
        if (INT32_MIN <= number && number <= INT32_MAX) {
            return RemoteObject(RemoteObjectType::NumberT {static_cast<int32_t>(number)});
        }
        if (number < 0) {
            return RemoteObject(RemoteObjectType::BigIntT {-1, -static_cast<uintmax_t>(number)});
        }
        return RemoteObject(RemoteObjectType::BigIntT {1, static_cast<uintmax_t>(number)});
    }

    template <typename T, std::enable_if_t<std::is_unsigned_v<T> && sizeof(int32_t) <= sizeof(T), int> = 0>
    static RemoteObject Number(T number)
    {
        if (number <= INT32_MAX) {
            return RemoteObject(RemoteObjectType::NumberT {static_cast<int32_t>(number)});
        }
        return RemoteObject(RemoteObjectType::BigIntT {1, number});
    }

    static RemoteObject String(std::string string)
    {
        return RemoteObject(std::move(string));
    }

    static RemoteObject Symbol(std::string description)
    {
        return RemoteObject(RemoteObjectType::SymbolT {std::move(description)});
    }

    static RemoteObject Object(std::string className, std::optional<RemoteObjectId> objectId = std::nullopt,
                               std::optional<std::string> description = std::nullopt)
    {
        return RemoteObject(RemoteObjectType::ObjectT {std::move(className), objectId, std::move(description)});
    }

    static RemoteObject Array(std::string className, size_t length,
                              std::optional<RemoteObjectId> objectId = std::nullopt)
    {
        return RemoteObject(RemoteObjectType::ArrayT {std::move(className), objectId, length});
    }

    static RemoteObject Function(std::string className, std::string name, size_t length,
                                 std::optional<RemoteObjectId> objectId = std::nullopt)
    {
        return RemoteObject(RemoteObjectType::FunctionT {std::move(className), objectId, std::move(name), length});
    }

    std::optional<RemoteObjectId> GetObjectId() const;

    void Serialize(JsonObjectBuilder &builder) const override;

    RemoteObjectType GetType() const;

    void SetObjectPreview(ObjectPreview preview)
    {
        preview_ = std::move(preview);
    }

    RemoteObjectType::TypeValue &GetValue()
    {
        return value_;
    }

    const RemoteObjectType::TypeValue &GetValue() const
    {
        return value_;
    }

    static std::string GetDescription(const RemoteObjectType::BigIntT &bigint);
    static std::string GetDescription(const RemoteObjectType::ObjectT &object);
    static std::string GetDescription(const RemoteObjectType::ArrayT &array);
    static std::string GetDescription(const RemoteObjectType::FunctionT &function);

private:
    template <typename... T>
    explicit RemoteObject(T &&...value) : value_(std::forward<T>(value)...)
    {
    }

private:
    RemoteObjectType::TypeValue value_;

    std::optional<ObjectPreview> preview_;
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_TYPES_REMOTE_OBJECT_H
