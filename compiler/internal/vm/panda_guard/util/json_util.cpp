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

#include "json_util.h"

#include <optional>
#include "util/assert_util.h"

namespace {
constexpr std::string_view TAG = "[JsonUtil]";

template <typename T>
std::optional<T> GetJsonValue(const panda::JsonObject *object, const std::string_view &field)
{
    auto value = object->GetValue<T>(field.data());
    if (!value) {
        return std::nullopt;
    }
    return *value;
}
}  // namespace

panda::JsonObject *panda::guard::JsonUtil::GetJsonObject(const panda::JsonObject *object, const std::string_view &field)
{
    auto value = object->GetValue<panda::JsonObject::JsonObjPointer>(field.data());
    if (!value) {
        return nullptr;
    }
    return value->get();
}

std::string panda::guard::JsonUtil::GetStringValue(const panda::JsonObject *object, const std::string_view &field,
                                                   bool optionalField)
{
    auto result = GetJsonValue<panda::JsonObject::StringT>(object, field);
    if (result.has_value()) {
        return result.value();
    }
    PANDA_GUARD_ASSERT_PRINT(!optionalField, TAG, ErrorCode::NOT_CONFIGURED_REQUIRED_FIELD,
                             "fail to obtain required field: " << field << " from json file");
    return "";
}

double panda::guard::JsonUtil::GetDoubleValue(const panda::JsonObject *object, const std::string_view &field,
                                              bool optionalField)
{
    auto result = GetJsonValue<panda::JsonObject::NumT>(object, field);
    if (result.has_value()) {
        return result.value();
    }
    PANDA_GUARD_ASSERT_PRINT(!optionalField, TAG, ErrorCode::NOT_CONFIGURED_REQUIRED_FIELD,
                             "fail to obtain required field: " << field << " from json file");
    return 0;
}

bool panda::guard::JsonUtil::GetBoolValue(const panda::JsonObject *object, const std::string_view &field,
                                          bool optionalField)
{
    auto result = GetJsonValue<panda::JsonObject::BoolT>(object, field);
    if (result.has_value()) {
        return result.value();
    }
    PANDA_GUARD_ASSERT_PRINT(!optionalField, TAG, ErrorCode::NOT_CONFIGURED_REQUIRED_FIELD,
                             "fail to obtain required field: " << field << " from json file");
    return false;
}

std::vector<std::string> panda::guard::JsonUtil::GetArrayStringValue(const panda::JsonObject *object,
                                                                     const std::string_view &field)
{
    std::vector<std::string> res;
    auto arrValues = object->GetValue<panda::JsonObject::ArrayT>(field.data());
    if (!arrValues) {
        return res;
    }
    res.reserve(arrValues->size());
    for (auto &value : *arrValues) {
        res.emplace_back(*(value.Get<panda::JsonObject::StringT>()));
    }
    return res;
}

std::map<std::string, std::string> panda::guard::JsonUtil::GetMapStringValue(const panda::JsonObject *object,
                                                                             const std::string_view &field)
{
    std::map<std::string, std::string> res;
    auto mapObj = GetJsonObject(object, field);
    if (!mapObj) {
        return res;
    }
    for (size_t idx = 0; idx < mapObj->GetSize(); idx++) {
        auto mapKey = mapObj->GetKeyByIndex(idx);
        auto mapValue = GetStringValue(mapObj, mapKey);
        res.emplace(mapKey, mapValue);
    }
    return res;
}
