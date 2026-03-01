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

#ifndef ABC2PROGRAM_ABC_FILE_UTILS_H
#define ABC2PROGRAM_ABC_FILE_UTILS_H

#include <string>
#include "assembly-program.h"
#include "file.h"

namespace panda::abc2program {

// type name
constexpr std::string_view GLOBAL_TYPE_NAME = "_GLOBAL";
constexpr std::string_view ARRAY_TYPE_PREFIX = "[";
constexpr std::string_view ANY_TYPE_NAME = "any";
constexpr std::string_view ES_TYPE_ANNOTATION_NAME = "_ESTypeAnnotation";
constexpr std::string_view ES_MODULE_RECORD = "_ESModuleRecord";
constexpr std::string_view ES_SCOPE_NAMES_RECORD = "_ESScopeNamesRecord";
constexpr std::string_view JSON_FILE_CONTENT = "jsonFileContent";
constexpr std::string_view MODULE_RECORD_IDX = "moduleRecordIdx";
constexpr std::string_view SCOPE_NAMES = "scopeNames";
constexpr std::string_view MODULE_REQUEST_PAHSE_IDX = "moduleRequestPhaseIdx";
constexpr std::string_view CONCURRENT_MODULE_REQUEST_RECORD_NAME = "_ESConcurrentModuleRequestsAnnotation";
constexpr std::string_view SLOT_NUMBER_RECORD_NAME = "_ESSlotNumberAnnotation";
constexpr std::string_view EXPECTED_PROPERTY_COUNT_RECORD_NAME = "_ESExpectedPropertyCountAnnotation";
constexpr std::string_view DOT = ".";
constexpr std::string_view RECORD_ANNOTATION_SEPARATOR = ".";
constexpr char AT_SEPARATOR = '@';
constexpr char COLON_SEPARATOR = ':';
constexpr char NORMALIZED_OHMURL_SEPARATOR = '&';
constexpr char SLASH_TAG = '/';
constexpr uint8_t ORIGINAL_PKG_NAME_POS = 0;
constexpr uint8_t TARGET_PKG_NAME_POS = 1;
constexpr uint8_t NORMALIZED_IMPORT_POS = 1;
const std::string FIELD_NAME_PREFIX = "pkgName@";

// attribute constant
constexpr std::string_view ABC_ATTR_EXTERNAL = "external";
constexpr std::string_view ABC_ATTR_STATIC = "static";

// literal array name
constexpr std::string_view UNDERLINE = "_";

// width constant
constexpr uint32_t LINE_WIDTH = 40;
constexpr uint32_t COLUMN_WIDTH = 10;
constexpr uint32_t START_WIDTH = 5;
constexpr uint32_t END_WIDTH = 6;
constexpr uint32_t REG_WIDTH = 8;
constexpr uint32_t NAME_WIDTH = 13;

// ins constant
constexpr std::string_view LABEL_PREFIX = "label@";

// module literal
constexpr uint8_t LITERAL_NUM_OF_REGULAR_IMPORT = 3;
constexpr uint8_t LITERAL_NUM_OF_NAMESPACE_IMPORT = 2;
constexpr uint8_t LITERAL_NUM_OF_LOCAL_EXPORT = 2;
constexpr uint8_t LITERAL_NUM_OF_INDIRECT_EXPORT = 3;
constexpr uint8_t LITERAL_NUM_OF_STAR_EXPORT = 1;
constexpr uint8_t LITERAL_NUMS[] = {
    LITERAL_NUM_OF_REGULAR_IMPORT,
    LITERAL_NUM_OF_NAMESPACE_IMPORT,
    LITERAL_NUM_OF_LOCAL_EXPORT,
    LITERAL_NUM_OF_INDIRECT_EXPORT,
    LITERAL_NUM_OF_STAR_EXPORT
};

std::string GetPkgNameFromNormalizedImport(const std::string &normalizedImport);
std::string GetPkgNameFromRecordName(const std::string &recordName);
std::vector<std::string> Split(const std::string &str, const char delimiter);
class AbcFileUtils {
public:
    static bool IsGlobalTypeName(const std::string &type_name);
    static bool IsArrayTypeName(const std::string &type_name);
    static bool IsSystemTypeName(const std::string &type_name);
    static bool IsESTypeAnnotationName(const std::string &type_name);
    static std::string GetLabelNameByInstIdx(size_t inst_idx);
};  // class AbcFileUtils

}  // namespace panda::abc2program

#endif  // ABC2PROGRAM_ABC_FILE_UTILS_H
