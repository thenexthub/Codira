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

#ifndef GUARD_UTIL_JSON_UTIL_H
#define GUARD_UTIL_JSON_UTIL_H

#include <string>

#include "libarkbase/utils/json_parser.h"

namespace ark::guard {
/**
 * @brief JsonUtil
 */
class JsonUtil {
public:
    /**
     * @brief Obtain the content of a JSON object with a field type of object
     * @param object json object
     * @param field field name in json object
     * @return const JsonObject * Specify the value of the field in the JSON object
     */
    static const JsonObject *GetJsonObject(const JsonObject *object, const std::string_view &field);

    /**
     * @brief Obtain the content of a JSON object with a field type of string
     * @param object json object
     * @param field field name in json object
     * @param optionalField Optional field, default to true (optional). When the field is false, if there is no
     * information about the field in the JSON object, an error will be reported
     * @return std::string Specify the value of the field in the JSON object
     */
    static std::string GetStringValue(const JsonObject *object, const std::string_view &field,
                                      bool optionalField = true);

    /**
     * @brief Obtain the content of a JSON object with a field type of bool
     * @param object json object
     * @param field field name in json object
     * @param defaultValue Default value, default to false (not optional)
     * @param optionalField Optional field, default to true (optional). When the field is false,
     * if there is no information about the field in the JSON object, an error will be reported
     * @return bool Specify the value of the field in the JSON object
     */
    static bool GetBoolValue(const JsonObject *object, const std::string_view &field, bool defaultValue = false,
                             bool optionalField = true);

    /**
     * @brief Obtain the content of a JSON object with a field type of string array
     * @param object json object
     * @param field field name in json object
     * @param optionalField Optional field, default to true (optional). When the field is false, if there is no
     * information about the field in the JSON object, an error will be reported
     * @return std::vector<std::string> Specify the value of the field in the JSON object
     */
    static std::vector<std::string> GetArrayStringValue(const JsonObject *object, const std::string_view &field,
                                                        bool optionalField = true);
};
}  // namespace ark::guard

#endif  // GUARD_UTIL_JSON_UTIL_H