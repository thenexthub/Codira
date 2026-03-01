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

#include "types/object_preview.h"
#include "types/property_descriptor.h"

#include "libarkbase/utils/string_helpers.h"

namespace ark::tooling::inspector {

// Number of maximum count of properties that can be previewed.
static constexpr size_t PROPERTIES_NMB_LIMIT = 10;

namespace {
std::optional<std::string> GetPropertyPreviewValue(const RemoteObjectType::TypeValue &remobjValue)
{
    std::optional<std::string> propPreviewValue;

    if (std::holds_alternative<std::nullptr_t>(remobjValue)) {
        propPreviewValue.emplace("null");
    } else if (auto boolean = std::get_if<bool>(&remobjValue)) {
        propPreviewValue.emplace(*boolean ? "true" : "false");
    } else if (auto number = std::get_if<RemoteObjectType::NumberT>(&remobjValue)) {
        if (auto integer = std::get_if<int32_t>(number)) {
            propPreviewValue.emplace(std::to_string(*integer));
        } else if (auto floatingPoint = std::get_if<double>(number)) {
            propPreviewValue.emplace(
                ark::helpers::string::Format("%g", *floatingPoint));  // NOLINT(cppcoreguidelines-pro-type-vararg)
        } else {
            UNREACHABLE();
        }
    } else if (auto bigint = std::get_if<RemoteObjectType::BigIntT>(&remobjValue)) {
        propPreviewValue.emplace(RemoteObject::GetDescription(*bigint));
    } else if (auto string = std::get_if<std::string>(&remobjValue)) {
        propPreviewValue.emplace(*string);
    } else if (auto symbol = std::get_if<RemoteObjectType::SymbolT>(&remobjValue)) {
        propPreviewValue.emplace(symbol->description);
    } else if (auto object = std::get_if<RemoteObjectType::ObjectT>(&remobjValue)) {
        propPreviewValue.emplace(RemoteObject::GetDescription(*object));
    } else if (auto array = std::get_if<RemoteObjectType::ArrayT>(&remobjValue)) {
        propPreviewValue.emplace(RemoteObject::GetDescription(*array));
    } else if (auto function = std::get_if<RemoteObjectType::FunctionT>(&remobjValue)) {
        propPreviewValue.emplace(function->name);
    }

    return propPreviewValue;
}
}  // namespace

ObjectPreview::ObjectPreview(RemoteObjectType type, const std::vector<PropertyDescriptor> &properties)
    : type_(std::move(type))
{
    overflow_ = (properties.size() > PROPERTIES_NMB_LIMIT);

    auto start = properties.begin();
    auto end = start + std::min(properties.size(), PROPERTIES_NMB_LIMIT);

    for (auto propertyIt = start; propertyIt != end; ++propertyIt) {
        if (!propertyIt->IsEnumerable()) {
            continue;
        }

        if (propertyIt->IsAccessor()) {
            properties_.emplace_back(propertyIt->GetName(), RemoteObjectType::Accessor());
            continue;
        }

        auto propPreviewValue = GetPropertyPreviewValue(propertyIt->GetValue().GetValue());

        properties_.emplace_back(propertyIt->GetName(), propertyIt->GetValue().GetType(), std::move(*propPreviewValue));
    }
}

void ObjectPreview::Serialize(JsonObjectBuilder &builder) const
{
    type_.Serialize(builder);
    builder.AddProperty("overflow", overflow_);

    builder.AddProperty("properties", [&](JsonArrayBuilder &propertiesBuilder) {
        for (auto &propertyPreview : properties_) {
            propertiesBuilder.Add(propertyPreview);
        }
    });
}

}  // namespace ark::tooling::inspector
