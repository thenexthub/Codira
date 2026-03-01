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

#include "name_cache_parser.h"

#include "logger.h"
#include "name_cache_constants.h"
#include "util/assert_util.h"
#include "util/file_util.h"
#include "util/json_util.h"

namespace {
constexpr char OPEN_PAREN = '(';

bool IsObfName(const std::string &str)
{
    return str == ark::guard::NameCacheConstants::OBF_NAME;
}

bool IsFunction(const std::string &str)
{
    return std::count(str.begin(), str.end(), OPEN_PAREN) == 1;
}
}  // namespace

void ark::guard::NameCacheParser::Parse()
{
    if (applyNameCache_.empty()) {
        return;
    }
    std::string content = FileUtil::GetFileContent(applyNameCache_);
    GUARD_ASSERT(content.empty(), ErrorCode::NAME_CACHE_FORMAT_ERROR,
                     "NameCache parsing failed, can not read apply name cache file:" + applyNameCache_);

    LOG_I << "Name Cache Content: " << content;
    ParseNameCache(content);
}

const ark::guard::ObjectNameCacheTable &ark::guard::NameCacheParser::GetNameCache()
{
    return this->moduleCaches_;
}

void ark::guard::NameCacheParser::ParseNameCache(const std::string &content)
{
    JsonObject object(content);
    GUARD_ASSERT(!object.IsValid(), ErrorCode::NAME_CACHE_FORMAT_ERROR,
                     "NameCache parsing failed: Invalid JSON format detected in name cache data");

    for (size_t i = 0; i < object.GetSize(); ++i) {
        const auto &key = object.GetKeyByIndex(i);
        if (key == ObfuscationToolConstants::NAME) {
            continue;
        }
        auto moduleObject = JsonUtil::GetJsonObject(&object, key);
        if (moduleObject && moduleObject->IsValid()) {
            ParseObjectNameCache(moduleObject, key, moduleCaches_);
        }
    }
}

/*
 * Parse object name cache. currently only support parsing string and object formats.
 * if format is string, the following values may be field, annotation, or function name.
 * string format:
 *  "field" : "a"
 *  "Anno1" : "b"
 *  "foo()void" : "c"
 * if format is object, the following values may be function, class, namespace, enum or annotation interface
 * object format:
 *  "ClassA" : { #obfName": "e", "method1()void":"a", "field1":"b"}
 *  "EnumColor" : { #obfName": "f", "RED xxx.xxx.EnumColor":"a"}
 */
void ark::guard::NameCacheParser::ParseObjectNameCache(const JsonObject *object, const std::string &objectName,
                                                       ObjectNameCacheTable &objectCaches)
{
    auto objectCache = std::make_shared<ObjectNameCache>();
    for (size_t i = 0; i < object->GetSize(); ++i) {
        auto key = object->GetKeyByIndex(i);

        auto value = JsonUtil::GetStringValue(object, key);
        if (!value.empty()) {
            if (IsObfName(key)) {
                objectCache->obfName = value;
            } else if (IsFunction(key)) {
                objectCache->methods.emplace(key, value);
            } else {
                objectCache->fields.emplace(key, value);
            }
            continue;
        }

        auto innerObject = JsonUtil::GetJsonObject(object, key);
        if (innerObject) {
            ParseObjectNameCache(innerObject, key, objectCache->objects);
            continue;
        }
        GUARD_ABORT(ErrorCode::NAME_CACHE_MODULE_FORMAT_ERROR,
                        "NameCache parsing failed: unknown name cache module format, key:" + key);
    }
    objectCaches.emplace(objectName, objectCache);
}

