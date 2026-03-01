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

#ifndef PANDA_TOOLING_INSPECTOR_TYPES_PROPERTY_PREVIEW_H
#define PANDA_TOOLING_INSPECTOR_TYPES_PROPERTY_PREVIEW_H

#include "tooling/inspector/types/remote_object_type.h"
#include "tooling/inspector/json_serialization/serializable.h"

#include <optional>
#include <string>

namespace ark::tooling::inspector {

class PropertyPreview final : public JsonSerializable {
public:
    PropertyPreview(std::string name, RemoteObjectType type) : name_(std::move(name)), type_(std::move(type)) {}

    PropertyPreview(std::string name, RemoteObjectType type, const std::string &value)
        : PropertyPreview(std::move(name), std::move(type))
    {
        value_ = value;
    }

    void Serialize(JsonObjectBuilder &builder) const override
    {
        type_.Serialize(builder);
        builder.AddProperty("name", name_);

        if (value_) {
            builder.AddProperty("value", *value_);
        }
    }

private:
    std::string name_;
    RemoteObjectType type_;
    std::optional<std::string> value_;
};

}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_TYPES_PROPERTY_PREVIEW_H
