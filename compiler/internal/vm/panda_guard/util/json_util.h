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

#ifndef PANDA_GUARD_UTIL_JSON_UTIL_H
#define PANDA_GUARD_UTIL_JSON_UTIL_H

#include <map>
#include <string>

#include "utils/json_parser.h"

namespace panda::guard {

class JsonUtil {
public:
    /**
     * Obtain the content of a JSON object with a field type of object
     * @param object json object
     * @param field field name in json object
     */
    static panda::JsonObject *GetJsonObject(const panda::JsonObject *object, const std::string_view &field);

    /**
     * Obtain the content of a JSON object with a field type of string
     * @param object json object
     * @param field field name in json object
     * @param optionalField Optional field, default to true (optional). When the field is false,
     * if there is no information about the field in the JSON object, an error will be reported
     */
    static std::string GetStringValue(const panda::JsonObject *object, const std::string_view &field,
                                      bool optionalField = true);

    /**
     * Obtain the content of a JSON object with a field type of double
     * @param object json object
     * @param field field name in json object
     * @param optionalField Optional field, default to true (optional). When the field is false,
     * if there is no information about the field in the JSON object, an error will be reported
     */
    static double GetDoubleValue(const panda::JsonObject *object, const std::string_view &field,
                                 bool optionalField = true);

    /**
     * Obtain the content of a JSON object with a field type of bool
     * @param object json object
     * @param field field name in json object
     * @param optionalField Optional field, default to true (optional). When the field is false,
     * if there is no information about the field in the JSON object, an error will be reported
     */
    static bool GetBoolValue(const panda::JsonObject *object, const std::string_view &field, bool optionalField = true);

    /**
     * Obtain the content of a JSON object with a field type of string array
     * @param object json object
     * @param field field name in json object
     */
    static std::vector<std::string> GetArrayStringValue(const panda::JsonObject *object, const std::string_view &field);

    /**
     * Obtain the content of a JSON object with a field type of string map
     * @param object json object
     * @param field field name in json object
     */
    static std::map<std::string, std::string> GetMapStringValue(const panda::JsonObject *object,
                                                                const std::string_view &field);
};

}  // namespace panda::guard

#endif  // PANDA_GUARD_UTIL_JSON_UTIL_H
