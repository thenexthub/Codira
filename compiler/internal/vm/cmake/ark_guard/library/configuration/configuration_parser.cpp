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

#include "configuration_parser.h"

#include <regex>
#include <algorithm>

#include "libarkbase/utils/json_parser.h"

#include "error.h"
#include "keep_option_parser.h"
#include "util/assert_util.h"
#include "util/file_util.h"
#include "util/json_util.h"
#include "util/string_util.h"

namespace {
constexpr std::string_view ABC_PATH = "abcPath";
constexpr std::string_view OBF_ABC_PATH = "obfAbcPath";
constexpr std::string_view DEFAULT_NAME_CACHE_PATH = "defaultNameCachePath";

constexpr std::string_view OBFUSCATION_RULES = "obfuscationRules";
constexpr std::string_view DISABLE_OBFUSCATION = "disableObfuscation";
constexpr std::string_view PRINT_NAME_CACHE = "printNameCache";
constexpr std::string_view APPLY_NAME_CACHE = "applyNameCache";
constexpr std::string_view REMOVE_LOG = "removeLog";

constexpr std::string_view PRINT_SEEDS_OPTION = "printSeedsOption";
constexpr std::string_view ENABLE = "enable";
constexpr std::string_view FILE_PATH = "filePath";

constexpr std::string_view FILE_NAME_OBFUSCATION = "fileNameObfuscation";
constexpr std::string_view RESERVED_FILE_NAMES = "reservedFileNames";
constexpr std::string_view UNIVERSAL_RESERVED_FILE_NAMES = "universalReservedFileNames";

constexpr std::string_view KEEP_OPTIONS = "keepOptions";
constexpr std::string_view KEEP_PATH = "keepPath";
constexpr std::string_view RESERVED_PATHS = "reservedPaths";
constexpr std::string_view UNIVERSAL_RESERVED_PATHS = "universalReservedPaths";

constexpr std::string_view KEEPS = "keeps";

constexpr std::string_view PATH_END = ".";
constexpr std::string_view UNIVERSAL_PATH_END = "\\.";
constexpr std::string_view UNIVERSAL_PATH_START = "^";

std::string NormalizeFilePath(const std::string &input)
{
    return ark::guard::StringUtil::EnsureEndWithSuffix(input, PATH_END.data());
}

std::string NormalizeUniversalPath(const std::string &input)
{
    return ark::guard::StringUtil::EnsureStartWithPrefix(
        ark::guard::StringUtil::EnsureEndWithSuffix(ark::guard::StringUtil::ConvertWildcardToRegex(input),
                                                    UNIVERSAL_PATH_END.data()),
        UNIVERSAL_PATH_START.data());
}
}  // namespace

void ark::guard::ConfigurationParser::Parse(Configuration &configuration)
{
    std::string fileContent = FileUtil::GetFileContent(configPath_);
    GUARD_ASSERT(fileContent.empty(), ErrorCode::CONFIGURATION_FILE_FORMAT_ERROR,
                     "Configuration parsing failed: configuration file is empty:" + configPath_);

    ParseConfigurationFile(fileContent, configuration);
    GUARD_ASSERT(!configuration.IsValid(), ErrorCode::CONFIGURATION_FILE_FORMAT_ERROR,
                     "Configuration parsing failed: configuration is invalid");
}

void ark::guard::ConfigurationParser::ParseConfigurationFile(const std::string &content, Configuration &configuration)
{
    JsonObject object(content);
    GUARD_ASSERT(!object.IsValid(), ErrorCode::CONFIGURATION_FILE_FORMAT_ERROR,
                     "Configuration parsing failed: the configuration file is not a valid json");

    configuration.abcPath = JsonUtil::GetStringValue(&object, ABC_PATH, false);
    configuration.obfAbcPath = JsonUtil::GetStringValue(&object, OBF_ABC_PATH, false);
    configuration.defaultNameCachePath = JsonUtil::GetStringValue(&object, DEFAULT_NAME_CACHE_PATH, false);

    auto obfRulesObject = JsonUtil::GetJsonObject(&object, OBFUSCATION_RULES);
    if (!obfRulesObject) {
        return;
    }
    configuration.obfuscationRules.disableObfuscation = JsonUtil::GetBoolValue(obfRulesObject, DISABLE_OBFUSCATION);
    configuration.obfuscationRules.printNameCache = JsonUtil::GetStringValue(obfRulesObject, PRINT_NAME_CACHE);
    configuration.obfuscationRules.applyNameCache = JsonUtil::GetStringValue(obfRulesObject, APPLY_NAME_CACHE);
    configuration.obfuscationRules.removeLog = JsonUtil::GetBoolValue(obfRulesObject, REMOVE_LOG);

    ParsePrintSeedsOption(obfRulesObject, configuration);
    ParseFileNameObfuscation(obfRulesObject, configuration);
    ParseKeepOption(obfRulesObject, configuration);
}

void ark::guard::ConfigurationParser::ParsePrintSeedsOption(const ark::JsonObject *object, Configuration &configuration)
{
    auto innerObject = JsonUtil::GetJsonObject(object, PRINT_SEEDS_OPTION);
    if (!innerObject) {
        return;
    }
    auto option = &configuration.obfuscationRules.printSeedsOption;
    option->enable = JsonUtil::GetBoolValue(innerObject, ENABLE);
    option->filePath = JsonUtil::GetStringValue(innerObject, FILE_PATH);
}

void ark::guard::ConfigurationParser::ParseFileNameObfuscation(const ark::JsonObject *object,
                                                               Configuration &configuration)
{
    auto innerObject = JsonUtil::GetJsonObject(object, FILE_NAME_OBFUSCATION);
    if (!innerObject) {
        return;
    }
    auto option = &configuration.obfuscationRules.fileNameOption;
    option->enable = JsonUtil::GetBoolValue(innerObject, ENABLE, true);
    option->reservedFileNames =
        StringUtil::ReplaceSlashWithDot(JsonUtil::GetArrayStringValue(innerObject, RESERVED_FILE_NAMES));
    option->universalReservedFileNames =
        StringUtil::ReplaceSlashWithDot(JsonUtil::GetArrayStringValue(innerObject, UNIVERSAL_RESERVED_FILE_NAMES));
    std::transform(option->universalReservedFileNames.begin(), option->universalReservedFileNames.end(),
                   option->universalReservedFileNames.begin(), StringUtil::ConvertWildcardToRegex);
}

void ark::guard::ConfigurationParser::ParseKeepOption(const ark::JsonObject *object, Configuration &configuration)
{
    auto innerObject = JsonUtil::GetJsonObject(object, KEEP_OPTIONS);
    if (!innerObject) {
        return;
    }
    auto keepPathObject = JsonUtil::GetJsonObject(innerObject, KEEP_PATH);
    if (keepPathObject) {
        auto pathOption = &configuration.obfuscationRules.keepOptions.pathOption;
        pathOption->reservedPaths =
            StringUtil::ReplaceSlashWithDot(JsonUtil::GetArrayStringValue(keepPathObject, RESERVED_PATHS));
        std::transform(pathOption->reservedPaths.begin(), pathOption->reservedPaths.end(),
                       pathOption->reservedPaths.begin(), NormalizeFilePath);

        pathOption->universalReservedPaths =
            StringUtil::ReplaceSlashWithDot(JsonUtil::GetArrayStringValue(keepPathObject, UNIVERSAL_RESERVED_PATHS));
        std::transform(pathOption->universalReservedPaths.begin(), pathOption->universalReservedPaths.end(),
                       pathOption->universalReservedPaths.begin(), NormalizeUniversalPath);
    }

    auto keeps = JsonUtil::GetArrayStringValue(innerObject, KEEPS);
    configuration.obfuscationRules.keepOptions.classSpecifications.reserve(keeps.size());
    for (const auto &keepOptionStr : keeps) {
        KeepOptionParser parser(keepOptionStr);
        auto result = parser.Parse();
        if (result.has_value()) {
            configuration.obfuscationRules.keepOptions.classSpecifications.emplace_back(result.value());
        }
    }
}