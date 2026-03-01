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

#include "guard_options.h"

#include <regex>
#include "utils/logger.h"

#include "util/assert_util.h"
#include "util/file_util.h"
#include "util/json_util.h"
#include "util/string_util.h"

namespace {
constexpr std::string_view TAG = "[Guard_Options]";

constexpr std::string_view ABC_FILE_PATH = "abcFilePath";
constexpr std::string_view OBF_ABC_FILE_PATH = "obfAbcFilePath";
constexpr std::string_view OBF_PA_FILE_PATH = "obfPaFilePath";
constexpr std::string_view COMPILE_SDK_VERSION = "compileSdkVersion";
constexpr std::string_view TARGET_API_VERSION = "targetApiVersion";
constexpr std::string_view TARGET_API_SUB_VERSION = "targetApiSubVersion";
constexpr std::string_view FILES_INFO_PATH = "filesInfoPath";
constexpr std::string_view SOURCE_MAPS_PATH = "sourceMapsPath";
constexpr std::string_view ENTRY_PACKAGE_INFO = "entryPackageInfo";
constexpr std::string_view DEFAULT_NAME_CACHE_PATH = "defaultNameCachePath";
constexpr std::string_view SKIPPED_REMOTE_HAR_LIST = "skippedRemoteHarList";
constexpr std::string_view USE_NORMALIZED_OHM_URL = "useNormalizedOHMUrl";
constexpr std::string_view OBFUSCATION_RULES = "obfuscationRules";
constexpr std::string_view DISABLE_OBFUSCATION = "disableObfuscation";
constexpr std::string_view ENABLE_EXPORT_OBFUSCATION = "enableExportObfuscation";
constexpr std::string_view ENABLE_REMOVE_LOG = "enableRemoveLog";
constexpr std::string_view ENABLE_DECORATOR_OBFUSCATION = "enableDecoratorObfuscation";
constexpr std::string_view COMPACT = "compact";
constexpr std::string_view PRINT_NAME_CACHE = "printNameCache";
constexpr std::string_view APPLY_NAME_CACHE = "applyNameCache";
constexpr std::string_view RESERVED_NAMES = "reservedNames";
constexpr std::string_view RECORD_NAME_WHITE_LIST = "recordNameWhiteList";
constexpr std::string_view ENABLE = "enable";
constexpr std::string_view PROPERTY_OBFUSCATION = "propertyObfuscation";
constexpr std::string_view RESERVED_PROPERTIES = "reservedProperties";
constexpr std::string_view UNIVERSAL_RESERVED_PROPERTIES = "universalReservedProperties";
constexpr std::string_view TOPLEVEL_OBFUSCATION = "toplevelObfuscation";
constexpr std::string_view RESERVED_TOPLEVEL_NAMES = "reservedToplevelNames";
constexpr std::string_view UNIVERSAL_RESERVED_TOPLEVEL_NAMES = "universalReservedToplevelNames";
constexpr std::string_view FILE_NAME_OBFUSCATION = "fileNameObfuscation";
constexpr std::string_view RESERVED_FILE_NAMES = "reservedFileNames";
constexpr std::string_view UNIVERSAL_RESERVED_FILE_NAMES = "universalReservedFileNames";
constexpr std::string_view RESERVED_REMOTE_HAR_PKG_NAMES = "reservedRemoteHarPkgNames";
constexpr std::string_view KEEP_OPTIONS = "keepOptions";
constexpr std::string_view KEEP_PATHS = "keepPaths";

constexpr std::string_view FILES_INFO_DELIMITER = ";";
constexpr size_t FILES_INFO_INDEX_1_RECORD_NAME = 1;
constexpr size_t FILES_INFO_INDEX_3_SOURCE_KEY = 3;
const std::string_view SOURCE_MAPS_SOURCES = "sources";

void ParseObfuscationOption(const panda::JsonObject *object, const std::string_view &objKey,
                            const std::string_view &reservedKey, const std::string_view &universalKey,
                            panda::guard::ObfuscationOption &option)
{
    auto innerObj = panda::guard::JsonUtil::GetJsonObject(object, objKey);
    if (!innerObj) {
        LOG(INFO, PANDAGUARD) << TAG << "fail to obtain object field :" << objKey << " from json object";
        return;
    }
    option.enable = panda::guard::JsonUtil::GetBoolValue(innerObj, ENABLE);
    option.reservedList = panda::guard::JsonUtil::GetArrayStringValue(innerObj, reservedKey);
    option.universalReservedList = panda::guard::JsonUtil::GetArrayStringValue(innerObj, universalKey);
    for (auto &str : option.universalReservedList) {
        panda::guard::StringUtil::RemoveSlashFromBothEnds(str);
    }
}

void ParsePropertyOption(const panda::JsonObject *obj, panda::guard::ObfuscationOption &option)
{
    ParseObfuscationOption(obj, PROPERTY_OBFUSCATION, RESERVED_PROPERTIES, UNIVERSAL_RESERVED_PROPERTIES, option);
}

void ParseToplevelOption(const panda::JsonObject *obj, panda::guard::ObfuscationOption &option)
{
    ParseObfuscationOption(obj, TOPLEVEL_OBFUSCATION, RESERVED_TOPLEVEL_NAMES, UNIVERSAL_RESERVED_TOPLEVEL_NAMES,
                           option);
}

void ParseFileNameOption(const panda::JsonObject *obj, panda::guard::FileNameOption &option)
{
    auto innerObj = panda::guard::JsonUtil::GetJsonObject(obj, FILE_NAME_OBFUSCATION);
    if (!innerObj) {
        LOG(INFO, PANDAGUARD) << TAG << "fail to obtain object field :" << FILE_NAME_OBFUSCATION << " from json object";
        return;
    }
    option.enable = panda::guard::JsonUtil::GetBoolValue(innerObj, ENABLE);
    option.reservedFileNames = panda::guard::JsonUtil::GetArrayStringValue(innerObj, RESERVED_FILE_NAMES);
    option.universalReservedFileNames =
        panda::guard::JsonUtil::GetArrayStringValue(innerObj, UNIVERSAL_RESERVED_FILE_NAMES);
    for (auto &str : option.universalReservedFileNames) {
        panda::guard::StringUtil::RemoveSlashFromBothEnds(str);
    }
    option.reservedRemoteHarPkgNames =
        panda::guard::JsonUtil::GetArrayStringValue(innerObj, RESERVED_REMOTE_HAR_PKG_NAMES);
}

void ParseKeepOption(const panda::JsonObject *obj, panda::guard::KeepOption &option)
{
    auto innerObj = panda::guard::JsonUtil::GetJsonObject(obj, KEEP_OPTIONS);
    if (!innerObj) {
        LOG(INFO, PANDAGUARD) << TAG << "fail to obtain object field :" << KEEP_OPTIONS << " from json object";
        return;
    }
    option.enable = panda::guard::JsonUtil::GetBoolValue(innerObj, ENABLE);
    option.keepPaths = panda::guard::JsonUtil::GetArrayStringValue(innerObj, KEEP_PATHS);
}

void ParseObfuscationConfigFile(const std::string &content, panda::guard::ObfuscationConfig &obfConfig)
{
    panda::JsonObject configObj(content);
    PANDA_GUARD_ASSERT_PRINT(!configObj.IsValid(), TAG, panda::guard::ErrorCode::CONFIG_FILE_FORMAT_ERROR,
                             "the config file is not a valid json");

    obfConfig.abcFilePath = panda::guard::JsonUtil::GetStringValue(&configObj, ABC_FILE_PATH, false);
    obfConfig.obfAbcFilePath = panda::guard::JsonUtil::GetStringValue(&configObj, OBF_ABC_FILE_PATH, false);
    obfConfig.obfPaFilePath = panda::guard::JsonUtil::GetStringValue(&configObj, OBF_PA_FILE_PATH);
    obfConfig.compileSdkVersion = panda::guard::JsonUtil::GetStringValue(&configObj, COMPILE_SDK_VERSION, false);
    obfConfig.targetApiVersion = (uint8_t)panda::guard::JsonUtil::GetDoubleValue(&configObj, TARGET_API_VERSION, false);
    obfConfig.targetApiSubVersion = panda::guard::JsonUtil::GetStringValue(&configObj, TARGET_API_SUB_VERSION);
    obfConfig.filesInfoPath = panda::guard::JsonUtil::GetStringValue(&configObj, FILES_INFO_PATH);
    obfConfig.sourceMapsPath = panda::guard::JsonUtil::GetStringValue(&configObj, SOURCE_MAPS_PATH);
    obfConfig.entryPackageInfo = panda::guard::JsonUtil::GetStringValue(&configObj, ENTRY_PACKAGE_INFO, false);
    obfConfig.defaultNameCachePath = panda::guard::JsonUtil::GetStringValue(&configObj, DEFAULT_NAME_CACHE_PATH, false);
    obfConfig.skippedRemoteHarList = panda::guard::JsonUtil::GetArrayStringValue(&configObj, SKIPPED_REMOTE_HAR_LIST);
    obfConfig.useNormalizedOHMUrl = panda::guard::JsonUtil::GetBoolValue(&configObj, USE_NORMALIZED_OHM_URL);

    auto rulesObj = panda::guard::JsonUtil::GetJsonObject(&configObj, OBFUSCATION_RULES);
    PANDA_GUARD_ASSERT_PRINT(!rulesObj, TAG, panda::guard::ErrorCode::NOT_CONFIGURED_OBFUSCATION_RULES,
                             "there is no confusion rule configured in the config file");

    auto obfRule = &obfConfig.obfuscationRules;
    obfRule->disableObfuscation = panda::guard::JsonUtil::GetBoolValue(rulesObj, DISABLE_OBFUSCATION);
    obfRule->enableExportObfuscation = panda::guard::JsonUtil::GetBoolValue(rulesObj, ENABLE_EXPORT_OBFUSCATION);
    obfRule->enableRemoveLog = panda::guard::JsonUtil::GetBoolValue(rulesObj, ENABLE_REMOVE_LOG);
    obfRule->enableDecorator = panda::guard::JsonUtil::GetBoolValue(rulesObj, ENABLE_DECORATOR_OBFUSCATION);
    obfRule->enableCompact = panda::guard::JsonUtil::GetBoolValue(rulesObj, COMPACT);
    obfRule->printNameCache = panda::guard::JsonUtil::GetStringValue(rulesObj, PRINT_NAME_CACHE);
    obfRule->applyNameCache = panda::guard::JsonUtil::GetStringValue(rulesObj, APPLY_NAME_CACHE);
    obfRule->reservedNames = panda::guard::JsonUtil::GetArrayStringValue(rulesObj, RESERVED_NAMES);
    obfRule->recordNameWhiteList = panda::guard::JsonUtil::GetArrayStringValue(rulesObj, RECORD_NAME_WHITE_LIST);
    ParsePropertyOption(rulesObj, obfRule->propertyOption);
    ParseToplevelOption(rulesObj, obfRule->toplevelOption);
    ParseFileNameOption(rulesObj, obfRule->fileNameOption);
    ParseKeepOption(rulesObj, obfRule->keepOption);
}

bool NeedToBeReserved(const std::vector<std::string> &reservedNames,
                      const std::vector<std::string> &universalReservedNames, const std::string &name)
{
    if (std::any_of(reservedNames.begin(), reservedNames.end(), [&](const auto &field) { return field == name; })) {
        return true;
    }

    return std::any_of(universalReservedNames.begin(), universalReservedNames.end(), [&](const auto &field) {
        std::regex pattern(field);
        return std::regex_search(name, pattern);
    });
}

void ParseFilesInfo(const std::string &filesInfoPath, std::unordered_map<std::string, std::string> &filesInfoTable)
{
    if (filesInfoPath.empty()) {
        LOG(INFO, PANDAGUARD) << TAG << "filesInfoPath is empty";
        return;
    }
    auto lineDataList = panda::guard::FileUtil::GetLineDataFromFile(filesInfoPath);
    if (lineDataList.empty()) {
        LOG(WARNING, PANDAGUARD) << TAG << "fail to get line data from filesInfoPath";
        return;
    }
    for (const auto &line : lineDataList) {
        auto infoList = panda::guard::StringUtil::StrictSplit(line, FILES_INFO_DELIMITER.data());
        if (infoList.size() < (FILES_INFO_INDEX_3_SOURCE_KEY + 1)) {
            LOG(WARNING, PANDAGUARD) << TAG << "line info is not of normal size : " << line;
            continue;
        }
        filesInfoTable.emplace(infoList[FILES_INFO_INDEX_1_RECORD_NAME], infoList[FILES_INFO_INDEX_3_SOURCE_KEY]);
    }
}

void ParseSourceMaps(const std::string &sourceMapsPath, std::unordered_map<std::string, std::string> &sourceMapsTable)
{
    if (sourceMapsPath.empty()) {
        LOG(INFO, PANDAGUARD) << TAG << "sourceMapsPath is empty";
        return;
    }
    std::string content = panda::guard::FileUtil::GetFileContent(sourceMapsPath);
    if (content.empty()) {
        LOG(WARNING, PANDAGUARD) << TAG << "get sourceMaps file content failed";
        return;
    }
    panda::JsonObject sourceMapsObj(content);
    if (!sourceMapsObj.IsValid()) {
        LOG(WARNING, PANDAGUARD) << TAG << "the sourceMaps file is not a valid json";
        return;
    }
    for (size_t idx = 0; idx < sourceMapsObj.GetSize(); idx++) {
        auto key = sourceMapsObj.GetKeyByIndex(idx);
        auto sourceObj = panda::guard::JsonUtil::GetJsonObject(&sourceMapsObj, key);
        if (!sourceObj) {
            LOG(WARNING, PANDAGUARD) << TAG << "failed to obtain object field :" << key << " from sourceMaps file";
            continue;
        }
        auto sources = panda::guard::JsonUtil::GetArrayStringValue(sourceObj, SOURCE_MAPS_SOURCES);
        if (sources.empty()) {
            LOG(WARNING, PANDAGUARD) << TAG << "the array field :" << key << " in the sourceMaps file is empty";
            continue;
        }
        sourceMapsTable.emplace(key, sources[0]);
    }
}

void ParseFilesInfoAndSourceMaps(const std::string &filesInfoPath, const std::string &sourceMapsPath,
                                 std::unordered_map<std::string, std::string> &sourceNameTable)
{
    std::unordered_map<std::string, std::string> filesInfoTable;
    ParseFilesInfo(filesInfoPath, filesInfoTable);
    if (filesInfoTable.empty()) {
        return;
    }

    std::unordered_map<std::string, std::string> sourceMapsTable;
    ParseSourceMaps(sourceMapsPath, sourceMapsTable);
    if (sourceMapsTable.empty()) {
        return;
    }

    for (const auto &[recordName, sourceMapsKey] : filesInfoTable) {
        auto item = sourceMapsTable.find(sourceMapsKey);
        if (item == sourceMapsTable.end()) {
            LOG(WARNING, PANDAGUARD) << TAG << sourceMapsKey << " in filesInfo, but not in sourceMaps";
            continue;
        }
        sourceNameTable.emplace(recordName, item->second);
    }
}
}  // namespace

void panda::guard::GuardOptions::Load(const std::string &configFilePath)
{
    std::string fileContent = FileUtil::GetFileContent(configFilePath);
    PANDA_GUARD_ASSERT_PRINT(fileContent.empty(), TAG, ErrorCode::CONFIG_FILE_CONTENT_EMPTY, "config file is empty");

    ParseObfuscationConfigFile(fileContent, this->obfConfig_);
    PANDA_GUARD_ASSERT_PRINT(obfConfig_.abcFilePath.empty(), TAG, ErrorCode::NOT_CONFIGURED_ABC_FILE_PATH,
                             "the value of field abcFilePath in the configuration file is invalid");
    PANDA_GUARD_ASSERT_PRINT(obfConfig_.obfAbcFilePath.empty(), TAG, ErrorCode::NOT_CONFIGURED_OBF_ABC_FILE_PATH,
                             "the value of field obfAbcFilePath in the configuration file is invalid");
    PANDA_GUARD_ASSERT_PRINT((obfConfig_.targetApiVersion == 0), TAG, ErrorCode::NOT_CONFIGURED_TARGET_API_VERSION,
                             "the value of field targetApiVersion in the configuration file is invalid");

    LOG(INFO, PANDAGUARD) << TAG << "disableObfuscation_:" << obfConfig_.obfuscationRules.disableObfuscation;
    LOG(INFO, PANDAGUARD) << TAG << "export obfuscation:" << obfConfig_.obfuscationRules.enableExportObfuscation;
    LOG(INFO, PANDAGUARD) << TAG << "removeLog obfuscation:" << obfConfig_.obfuscationRules.enableRemoveLog;
    LOG(INFO, PANDAGUARD) << TAG << "property obfuscation:" << obfConfig_.obfuscationRules.propertyOption.enable;
    LOG(INFO, PANDAGUARD) << TAG << "topLevel obfuscation:" << obfConfig_.obfuscationRules.toplevelOption.enable;
    LOG(INFO, PANDAGUARD) << TAG << "fileName obfuscation:" << obfConfig_.obfuscationRules.fileNameOption.enable;

    ParseFilesInfoAndSourceMaps(obfConfig_.filesInfoPath, obfConfig_.sourceMapsPath, this->sourceNameTable_);
}

const std::string &panda::guard::GuardOptions::GetAbcFilePath() const
{
    return obfConfig_.abcFilePath;
}

const std::string &panda::guard::GuardOptions::GetObfAbcFilePath() const
{
    return obfConfig_.obfAbcFilePath;
}

const std::string &panda::guard::GuardOptions::GetObfPaFilePath() const
{
    return obfConfig_.obfPaFilePath;
}

const std::string &panda::guard::GuardOptions::GetCompileSdkVersion() const
{
    return obfConfig_.compileSdkVersion;
}

uint8_t panda::guard::GuardOptions::GetTargetApiVersion() const
{
    return obfConfig_.targetApiVersion;
}

const std::string &panda::guard::GuardOptions::GetTargetApiSubVersion() const
{
    return obfConfig_.targetApiSubVersion;
}

const std::string &panda::guard::GuardOptions::GetEntryPackageInfo() const
{
    return obfConfig_.entryPackageInfo;
}

const std::string &panda::guard::GuardOptions::GetDefaultNameCachePath() const
{
    return obfConfig_.defaultNameCachePath;
}

bool panda::guard::GuardOptions::DisableObfuscation() const
{
    return obfConfig_.obfuscationRules.disableObfuscation;
}

bool panda::guard::GuardOptions::IsExportObfEnabled() const
{
    return obfConfig_.obfuscationRules.enableExportObfuscation;
}

bool panda::guard::GuardOptions::IsRemoveLogObfEnabled() const
{
    return obfConfig_.obfuscationRules.enableRemoveLog;
}

bool panda::guard::GuardOptions::IsDecoratorObfEnabled() const
{
    return obfConfig_.obfuscationRules.enableDecorator;
}

bool panda::guard::GuardOptions::IsCompactObfEnabled() const
{
    return obfConfig_.obfuscationRules.enableCompact;
}

const std::string &panda::guard::GuardOptions::GetPrintNameCache() const
{
    return obfConfig_.obfuscationRules.printNameCache;
}

const std::string &panda::guard::GuardOptions::GetApplyNameCache() const
{
    return obfConfig_.obfuscationRules.applyNameCache;
}

bool panda::guard::GuardOptions::IsPropertyObfEnabled() const
{
    return obfConfig_.obfuscationRules.propertyOption.enable;
}

bool panda::guard::GuardOptions::IsToplevelObfEnabled() const
{
    return obfConfig_.obfuscationRules.toplevelOption.enable;
}

bool panda::guard::GuardOptions::IsFileNameObfEnabled() const
{
    return obfConfig_.obfuscationRules.fileNameOption.enable;
}

bool panda::guard::GuardOptions::IsKeepPath(const std::string &path) const
{
    const auto keepOption = &obfConfig_.obfuscationRules.keepOption;
    if (!keepOption->enable || path.empty()) {
        return false;
    }

    std::vector<std::string> universalKeepPaths;  // keep paths not have universal
    return NeedToBeReserved(keepOption->keepPaths, universalKeepPaths, path);
}

bool panda::guard::GuardOptions::IsReservedNames(const std::string &name) const
{
    std::vector<std::string> universalReservedNames;  // names not have universal
    return NeedToBeReserved(obfConfig_.obfuscationRules.reservedNames, universalReservedNames, name);
}

bool panda::guard::GuardOptions::IsInRecordNameWhiteList(const std::string &name) const
{
    std::vector<std::string> universalWhiteList;  // not have universal white list
    return NeedToBeReserved(obfConfig_.obfuscationRules.recordNameWhiteList, universalWhiteList, name);
}

bool panda::guard::GuardOptions::IsReservedProperties(const std::string &name) const
{
    return NeedToBeReserved(obfConfig_.obfuscationRules.propertyOption.reservedList,
                            obfConfig_.obfuscationRules.propertyOption.universalReservedList, name);
}

bool panda::guard::GuardOptions::IsReservedToplevelNames(const std::string &name) const
{
    return NeedToBeReserved(obfConfig_.obfuscationRules.toplevelOption.reservedList,
                            obfConfig_.obfuscationRules.toplevelOption.universalReservedList, name);
}

bool panda::guard::GuardOptions::IsReservedFileNames(const std::string &name) const
{
    return NeedToBeReserved(obfConfig_.obfuscationRules.fileNameOption.reservedFileNames,
                            obfConfig_.obfuscationRules.fileNameOption.universalReservedFileNames, name);
}

const std::string &panda::guard::GuardOptions::GetSourceName(const std::string &name) const
{
    auto item = this->sourceNameTable_.find(name);
    if (item == this->sourceNameTable_.end()) {
        return name;
    }
    return item->second;
}

bool panda::guard::GuardOptions::IsSkippedRemoteHar(const std::string &pkgName) const
{
    return std::any_of(
        obfConfig_.skippedRemoteHarList.begin(), obfConfig_.skippedRemoteHarList.end(),
        [pkgName](const std::string &remoteHar) { return StringUtil::IsSuffixMatched(remoteHar, pkgName); });
}

bool panda::guard::GuardOptions::IsUseNormalizedOhmUrl() const
{
    return obfConfig_.useNormalizedOHMUrl;
}

bool panda::guard::GuardOptions::IsReservedRemoteHarPkgNames(const std::string &name) const
{
    return std::any_of(obfConfig_.obfuscationRules.fileNameOption.reservedRemoteHarPkgNames.begin(),
                       obfConfig_.obfuscationRules.fileNameOption.reservedRemoteHarPkgNames.end(),
                       [name](const std::string &remoteHar) { return remoteHar == name; });
}
