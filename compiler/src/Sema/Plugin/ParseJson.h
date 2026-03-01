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

/**
 * @file
 *
 * This file declares json parsing functions.
 */

#ifndef PARSE_JSON_H
#define PARSE_JSON_H

#include <cstdint>
#include <string>
#include <vector>

#include "Codira/Utils/SafePointer.h"

namespace Codira {
namespace PluginCheck {

struct JsonObject;

/**
 * @brief Represents a key-value pair in a JSON object.
 */
struct JsonPair {
    std::string key;
    std::vector<std::string> valueStr;
    std::vector<OwnedPtr<JsonObject>> valueObj;
    std::vector<uint64_t> valueNum;
};

/**
 * @brief Represents a JSON object containing multiple key-value pairs.
 */
struct JsonObject {
    std::vector<OwnedPtr<JsonPair>> pairs;
};

/**
 * @brief Enum to indicate whether we are parsing a key or a value in JSON.
 */
enum class StringMod {
    KEY,
    VALUE,
};

/**
 * @brief Parse a json string from input data.
 * @param pos Current position in input data.
 * @param in Json data.
 * @return Parsed JsonObject.
 */
OwnedPtr<JsonObject> ParseJsonObject(size_t& pos, const std::vector<uint8_t>& in);

/**
 * @brief Get json string values by key from a JsonObject.
 * @param root Root JsonObject.
 * @param key Key to search for.
 * @return Vector of string values associated with the key.
 */
std::vector<std::string> GetJsonString(Ptr<JsonObject> root, const std::string& key);

/**
 * @brief Get json object values by key from a JsonObject.
 * @param root Root JsonObject.
 * @param key Key to search for.
 * @param index Index of the object in the valueObj vector.
 * @return Ptr to the JsonObject associated with the key and index.
 */
Ptr<JsonObject> GetJsonObject(Ptr<JsonObject> root, const std::string& key, const size_t index);
} // namespace PluginCheck
} // namespace Codira
#endif
