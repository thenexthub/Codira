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
 * This file implements json parsing functions.
 */
#include "ParseJson.h"

#include <sstream>

#include "Codira/Utils/StdUtils.h"

namespace Codira {
namespace PluginCheck {

std::string ParseJsonString(size_t& pos, const std::vector<uint8_t>& in)
{
    if (pos >= in.size() || in[pos] != '"') {
        return "";
    }
    ++pos;
    std::stringstream str;
    while (pos < in.size() && in[pos] != '"') {
        str << in[pos];
        ++pos;
    }

    return str.str();
}

uint64_t ParseJsonNumber(size_t& pos, const std::vector<uint8_t>& in)
{
    if (pos >= in.size() || in[pos] < '0' || in[pos] > '9') {
        return 0;
    }
    std::stringstream num;
    while (pos < in.size() && in[pos] >= '0' && in[pos] <= '9') {
        num << in[pos];
        ++pos;
    }
    if (num.str().size()) {
        --pos;
    }
    return Stoull(num.str()).value_or(0);
}

void ParseJsonArray(size_t& pos, const std::vector<uint8_t>& in, Ptr<JsonPair> value)
{
    if (pos >= in.size() || in[pos] != '[' || value == nullptr) {
        return;
    }
    ++pos;
    while (pos < in.size()) {
        if (in[pos] == ' ' || in[pos] == '\n') {
            ++pos;
            continue;
        }
        if (in[pos] == '"') {
            value->valueStr.emplace_back(ParseJsonString(pos, in));
        }
        if (in[pos] == '{') {
            value->valueObj.emplace_back(ParseJsonObject(pos, in));
        }
        if (in[pos] == ']') {
            return;
        }
        ++pos;
    }
}

OwnedPtr<JsonObject> ParseJsonObject(size_t& pos, const std::vector<uint8_t>& in)
{
    if (pos >= in.size() || in[pos] != '{') {
        return MakeOwned<JsonObject>();
    }
    ++pos;
    auto ret = MakeOwned<JsonObject>();
    auto mod = StringMod::KEY;
    while (pos < in.size()) {
        if (in[pos] == ' ' || in[pos] == '\n') {
            ++pos;
            continue;
        }
        if (in[pos] == '}') {
            return ret;
        }
        if (in[pos] == ':') {
            mod = StringMod::VALUE;
            if (ret->pairs.empty()) {
                auto newData = MakeOwned<JsonPair>();
                newData->key = "";
                ret->pairs.emplace_back(std::move(newData));
            }
        }
        if (in[pos] == ',') {
            mod = StringMod::KEY;
        }
        if (in[pos] == '"') {
            if (mod == StringMod::KEY) {
                auto newData = MakeOwned<JsonPair>();
                newData->key = ParseJsonString(pos, in);
                ret->pairs.emplace_back(std::move(newData));
            } else {
                ret->pairs.back()->valueStr.emplace_back(ParseJsonString(pos, in));
            }
        }
        if (in[pos] >= '0' && in[pos] <= '9' && mod == StringMod::VALUE) {
            CODEC_ASSERT(!ret->pairs.empty());
            ret->pairs.back()->valueNum.emplace_back(ParseJsonNumber(pos, in));
        }
        if (in[pos] == '{' && mod == StringMod::VALUE) {
            CODEC_ASSERT(!ret->pairs.empty());
            // The pos will be updated to the pos of matched '}'.
            ret->pairs.back()->valueObj.emplace_back(ParseJsonObject(pos, in));
        }
        if (in[pos] == '[' && mod == StringMod::VALUE) {
            CODEC_ASSERT(!ret->pairs.empty());
            // The pos will be updated to the pos of matched ']'.
            ParseJsonArray(pos, in, ret->pairs.back().get());
        }
        ++pos;
    }
    return ret;
}

std::vector<std::string> GetJsonString(Ptr<JsonObject> root, const std::string& key)
{
    if (root == nullptr) {
        return {};
    }
    for (auto& v : root->pairs) {
        if (v->key == key) {
            return v->valueStr;
        }
        for (auto& o : v->valueObj) {
            auto ret = GetJsonString(o.get(), key);
            if (!ret.empty()) {
                return ret;
            }
        }
    }
    return {};
}

Ptr<JsonObject> GetJsonObject(Ptr<JsonObject> root, const std::string& key, const size_t index)
{
    if (root == nullptr) {
        return nullptr;
    }
    for (auto& v : root->pairs) {
        if (v->key == key && v->valueObj.size() > index) {
            return v->valueObj[index].get();
        }
        for (auto& o : v->valueObj) {
            auto ret = GetJsonObject(o.get(), key, index);
            if (ret) {
                return ret;
            }
        }
    }
    return nullptr;
}
} // namespace PluginCheck
} // namespace Codira
