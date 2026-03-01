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

#ifndef PANDA_TOOLING_INSPECTOR_TYPES_REMOTE_OBJECT_TYPE_H
#define PANDA_TOOLING_INSPECTOR_TYPES_REMOTE_OBJECT_TYPE_H

#include "libarkbase/utils/json_builder.h"
#include "types/numeric_id.h"
#include "json_serialization/serializable.h"

#include <variant>
#include <optional>

namespace ark::tooling::inspector {

class RemoteObjectType final : public JsonSerializable {
public:
    using NumberT = std::variant<int32_t, double>;

    struct BigIntT {
        int8_t sign;
        uintmax_t value;
    };

    struct SymbolT {
        std::string description;
    };

    struct ObjectT {
        std::string className;
        std::optional<RemoteObjectId> objectId;
        std::optional<std::string> description;
    };

    struct ArrayT {
        std::string className;
        std::optional<RemoteObjectId> objectId;
        size_t length;
    };

    struct FunctionT {
        std::string className;
        std::optional<RemoteObjectId> objectId;
        std::string name;
        size_t length;
    };

    using TypeValue = std::variant<std::monostate, std::nullptr_t, bool, NumberT, BigIntT, std::string, SymbolT,
                                   ObjectT, ArrayT, FunctionT>;

public:
    explicit RemoteObjectType(const char *type, const char *subtype = nullptr) : type_(type), subtype_(subtype) {}

    static RemoteObjectType Accessor()
    {
        return RemoteObjectType("accessor");
    }

    void Serialize(JsonObjectBuilder &builder) const override
    {
        builder.AddProperty("type", type_);

        if (subtype_ != nullptr) {
            builder.AddProperty("subtype", subtype_);
        }
    }

private:
    const char *type_ {nullptr};
    const char *subtype_ {nullptr};
};

}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_TYPES_REMOTE_OBJECT_TYPE_H
