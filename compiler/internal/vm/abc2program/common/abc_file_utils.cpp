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

#include "abc_file_utils.h"

namespace panda::abc2program {

std::string GetPkgNameFromNormalizedImport(const std::string &normalizedImport)
{
    std::string pkgName {};
    size_t pos = normalizedImport.find(SLASH_TAG);
    if (pos != std::string::npos) {
        pkgName = normalizedImport.substr(0, pos);
    }
    if (normalizedImport[0] == AT_SEPARATOR) {
        pos = normalizedImport.find(SLASH_TAG, pos + 1);
        if (pos != std::string::npos) {
            pkgName = normalizedImport.substr(0, pos);
        }
    }
    return pkgName;
}

std::string GetPkgNameFromRecordName(const std::string &recordName)
{
    std::string normalizedImport {};
    std::string pkgName {};
    auto items = Split(recordName, NORMALIZED_OHMURL_SEPARATOR);
    if (items.size() <= NORMALIZED_IMPORT_POS) {
        return pkgName;
    }
    normalizedImport = items[NORMALIZED_IMPORT_POS];
    return GetPkgNameFromNormalizedImport(normalizedImport);
}

std::vector<std::string> Split(const std::string &str, const char delimiter)
{
    std::vector<std::string> items;
    size_t start = 0;
    size_t pos = str.find(delimiter);
    while (pos != std::string::npos) {
        std::string item = str.substr(start, pos - start);
        items.emplace_back(item);
        start = pos + 1;
        pos = str.find(delimiter, start);
    }
    std::string tail = str.substr(start);
    items.emplace_back(tail);
    return items;
}

bool AbcFileUtils::IsGlobalTypeName(const std::string &type_name)
{
    return (type_name == GLOBAL_TYPE_NAME);
}

bool AbcFileUtils::IsArrayTypeName(const std::string &type_name)
{
    return (type_name.find(ARRAY_TYPE_PREFIX) != std::string::npos);
}

bool AbcFileUtils::IsESTypeAnnotationName(const std::string &type_name)
{
    return (type_name == ES_TYPE_ANNOTATION_NAME);
}

bool AbcFileUtils::IsSystemTypeName(const std::string &type_name)
{
    return (IsGlobalTypeName(type_name) || IsArrayTypeName(type_name));
}

std::string AbcFileUtils::GetLabelNameByInstIdx(size_t inst_idx)
{
    std::stringstream name;
    name << LABEL_PREFIX << inst_idx;
    return name.str();
}

}  // namespace panda::abc2program
