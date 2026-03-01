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

#include "guard_name_cache.h"

#include <regex>

#include "utils/logger.h"
#include "utils/json_builder.h"

#include "util/assert_util.h"
#include "util/file_util.h"
#include "util/json_util.h"
#include "version.h"

namespace {
constexpr std::string_view TAG = "[Guard_NameCache]";
constexpr std::string_view IDENTIFIER_CACHE = "IdentifierCache";
constexpr std::string_view MEMBER_METHOD_CACHE = "MemberMethodCache";
constexpr std::string_view OBF_NAME = "obfName";
constexpr std::string_view ORI_SOURCE_FILE = "OriSourceFile";
constexpr std::string_view OBF_SOURCE_FILE = "ObfSourceFile";
constexpr std::string_view ENTRY_PACKAGE_INFO = "entryPackageInfo";
constexpr std::string_view COMPILE_SDK_VERSION = "compileSdkVersion";
constexpr std::string_view PROPERTY_CACHE = "PropertyCache";
constexpr std::string_view FILE_NAME_CACHE = "FileNameCache";

/**
 * Remove the scope identifier # and its line number information from the mapping key value for incremental nameCache
 * scenario
 * @param originMap The format of the map key value is a string with scope identifier # and line number information
 * @return The format of the map key value is a string without scope identifier # and line number information
 * e.g. DeleteScopeAndLineNum({"#func:1:10", "a"}) => {"func", "a"}
 */
std::map<std::string, std::string> DeleteScopeAndLineNum(const std::map<std::string, std::string> &originMap)
{
    std::map<std::string, std::string> res;
    for (const auto &item : originMap) {
        auto key = item.first;
        size_t pos = key.rfind('#');
        if (pos != std::string::npos) {
            key = key.substr(pos + 1);
        }
        pos = key.find(':');
        if (pos != std::string::npos) {
            key = key.substr(0, pos);
        }
        res.emplace(key, item.second);
    }
    return res;
}

bool HasLineNumberInfo(const std::map<std::string, std::string> &table, const std::string &str)
{
    std::string match = str + ":\\d+:\\d+";
    std::regex pattern(match);
    return std::any_of(table.begin(), table.end(),
                       [&](const auto &item) { return std::regex_search(item.first, pattern); });
}

std::function<void(panda::JsonObjectBuilder &)> MapToJson(const std::map<std::string, std::string> &mapInfo,
                                                          bool deduplicate = false)
{
    return [mapInfo, deduplicate](panda::JsonObjectBuilder &builder) {
        for (const auto &item : mapInfo) {
            if (deduplicate && item.first == item.second) {
                continue;
            }
            builder.AddProperty(item.first, item.second);
        }
    };
}

std::function<void(panda::JsonObjectBuilder &)> FileNameCacheToJson(const panda::guard::FileNameCacheInfo &cacheInfo)
{
    return [cacheInfo](panda::JsonObjectBuilder &builder) {
        builder.AddProperty(IDENTIFIER_CACHE, MapToJson(cacheInfo.identifierCacheMap));
        builder.AddProperty(MEMBER_METHOD_CACHE, MapToJson(cacheInfo.memberMethodCacheMap));
        builder.AddProperty(OBF_NAME, cacheInfo.obfName);
        if (!cacheInfo.oriSourceFile.empty()) {
            builder.AddProperty(ORI_SOURCE_FILE, cacheInfo.oriSourceFile);
            builder.AddProperty(OBF_SOURCE_FILE, cacheInfo.obfSourceFile);
        }
    };
}
}  // namespace

void panda::guard::NameCache::Load(const std::string &applyNameCachePath)
{
    if (applyNameCachePath.empty()) {
        return;
    }
    std::string content = FileUtil::GetFileContent(applyNameCachePath);
    if (content.empty()) {
        LOG(WARNING, PANDAGUARD) << TAG << "get apply name cache file content failed";
        return;
    }
    LOG(INFO, PANDAGUARD) << TAG << "load apply name cache:" << applyNameCachePath;
    LOG(INFO, PANDAGUARD) << TAG << "apply name cache content:" << content;
    ParseAppliedNameCache(content);
    ParseHistoryNameCacheToUsedNames();
}

const std::set<std::string> &panda::guard::NameCache::GetHistoryUsedNames() const
{
    return historyUsedNames_;
}

std::map<std::string, std::string> panda::guard::NameCache::GetHistoryFileNameMapping() const
{
    return historyNameCache_.fileNameCacheMap;
}

std::map<std::string, std::string> panda::guard::NameCache::GetHistoryNameMapping() const
{
    std::map<std::string, std::string> res = historyNameCache_.propertyCacheMap;
    for (const auto &item : historyNameCache_.fileCacheInfoMap) {
        res.insert(item.second.identifierCacheMap.begin(), item.second.identifierCacheMap.end());
        res.insert(item.second.memberMethodCacheMap.begin(), item.second.memberMethodCacheMap.end());
    }
    return res;
}

void panda::guard::NameCache::AddObfIdentifierName(const std::string &filePath, const std::string &origin,
                                                   const std::string &obfName)
{
    if (filePath.empty() || origin.empty() || obfName.empty()) {
        LOG(WARNING, PANDAGUARD) << TAG << "[" << __FUNCTION__ << "], filePath or origin or obfName is empty";
        LOG(INFO, PANDAGUARD) << TAG << "filePath:" << filePath;
        LOG(INFO, PANDAGUARD) << TAG << "origin:" << origin;
        LOG(INFO, PANDAGUARD) << TAG << "obfName:" << obfName;
        return;
    }
    auto fileItem = newNameCache_.fileCacheInfoMap.find(filePath);
    if (fileItem != newNameCache_.fileCacheInfoMap.end()) {
        if (!HasLineNumberInfo(fileItem->second.identifierCacheMap, origin)) {
            fileItem->second.identifierCacheMap.emplace(origin, obfName);
        }
        return;
    }
    FileNameCacheInfo cacheInfo;
    cacheInfo.identifierCacheMap.emplace(origin, obfName);
    newNameCache_.fileCacheInfoMap.emplace(filePath, cacheInfo);
}

void panda::guard::NameCache::AddObfMemberMethodName(const std::string &filePath, const std::string &origin,
                                                     const std::string &obfName)
{
    if (filePath.empty() || origin.empty() || obfName.empty()) {
        LOG(WARNING, PANDAGUARD) << TAG << "[" << __FUNCTION__ << "], filePath or origin or obfName is empty";
        LOG(INFO, PANDAGUARD) << TAG << "filePath:" << filePath;
        LOG(INFO, PANDAGUARD) << TAG << "origin:" << origin;
        LOG(INFO, PANDAGUARD) << TAG << "obfName:" << obfName;
        return;
    }
    auto fileItem = newNameCache_.fileCacheInfoMap.find(filePath);
    if (fileItem != newNameCache_.fileCacheInfoMap.end()) {
        fileItem->second.memberMethodCacheMap.emplace(origin, obfName);
        return;
    }
    FileNameCacheInfo cacheInfo;
    cacheInfo.memberMethodCacheMap.emplace(origin, obfName);
    newNameCache_.fileCacheInfoMap.emplace(filePath, cacheInfo);
}

void panda::guard::NameCache::AddObfName(const std::string &filePath, const std::string &obfName)
{
    if (filePath.empty() || obfName.empty()) {
        LOG(WARNING, PANDAGUARD) << TAG << "[" << __FUNCTION__ << "], filePath or obfName is empty";
        LOG(INFO, PANDAGUARD) << TAG << "filePath:" << filePath;
        LOG(INFO, PANDAGUARD) << TAG << "obfName:" << obfName;
        return;
    }
    auto fileItem = newNameCache_.fileCacheInfoMap.find(filePath);
    if (fileItem != newNameCache_.fileCacheInfoMap.end()) {
        fileItem->second.obfName = obfName;
        return;
    }
    FileNameCacheInfo cacheInfo;
    cacheInfo.obfName = obfName;
    newNameCache_.fileCacheInfoMap.emplace(filePath, cacheInfo);
}

void panda::guard::NameCache::AddSourceFile(const std::string &filePath, const std::string &oriSource,
                                            const std::string &obfSource)
{
    if (filePath.empty() || oriSource.empty() || obfSource.empty()) {
        LOG(WARNING, PANDAGUARD) << TAG << "[" << __FUNCTION__ << "], filePath or oriSource or obfSource is empty";
        LOG(INFO, PANDAGUARD) << TAG << "filePath:" << filePath;
        LOG(INFO, PANDAGUARD) << TAG << "oriSourceFile:" << oriSource;
        LOG(INFO, PANDAGUARD) << TAG << "obfSourceFile:" << obfSource;
        return;
    }
    auto fileItem = newNameCache_.fileCacheInfoMap.find(filePath);
    if (fileItem != newNameCache_.fileCacheInfoMap.end()) {
        fileItem->second.oriSourceFile = oriSource;
        fileItem->second.obfSourceFile = obfSource;
        return;
    }
    FileNameCacheInfo cacheInfo;
    cacheInfo.oriSourceFile = oriSource;
    cacheInfo.obfSourceFile = obfSource;
    newNameCache_.fileCacheInfoMap.emplace(filePath, cacheInfo);
}

void panda::guard::NameCache::AddNewNameCacheObfFileName(const std::string &origin, const std::string &obfName)
{
    if (origin.empty() || obfName.empty()) {
        LOG(WARNING, PANDAGUARD) << TAG << "[" << __FUNCTION__ << "], origin or obfName is empty";
        LOG(INFO, PANDAGUARD) << TAG << "origin:" << origin;
        LOG(INFO, PANDAGUARD) << TAG << "obfName:" << obfName;
        return;
    }
    newNameCache_.fileNameCacheMap.emplace(origin, obfName);
}

void panda::guard::NameCache::AddObfPropertyName(const std::string &origin, const std::string &obfName)
{
    if (origin.empty() || obfName.empty()) {
        LOG(WARNING, PANDAGUARD) << TAG << "[" << __FUNCTION__ << "], origin or obfName is empty";
        LOG(INFO, PANDAGUARD) << TAG << "origin:" << origin;
        LOG(INFO, PANDAGUARD) << TAG << "obfName:" << obfName;
        return;
    }
    newNameCache_.propertyCacheMap.emplace(origin, obfName);
}

void panda::guard::NameCache::MergeNameCache(ProjectNameCacheInfo &merge)
{
    merge.fileCacheInfoMap = newNameCache_.fileCacheInfoMap;
    merge.entryPackageInfo = options_->GetEntryPackageInfo();
    merge.compileSdkVersion = options_->GetCompileSdkVersion();
    merge.propertyCacheMap = historyNameCache_.propertyCacheMap;
    merge.propertyCacheMap.insert(newNameCache_.propertyCacheMap.begin(), newNameCache_.propertyCacheMap.end());
    merge.fileNameCacheMap = historyNameCache_.fileNameCacheMap;
    merge.fileNameCacheMap.insert(newNameCache_.fileNameCacheMap.begin(), newNameCache_.fileNameCacheMap.end());
}

void panda::guard::NameCache::WriteCache()
{
    std::string defaultPath = options_->GetDefaultNameCachePath();
    std::string printPath = options_->GetPrintNameCache();
    PANDA_GUARD_ASSERT_PRINT(defaultPath.empty() && printPath.empty(), TAG,
                             ErrorCode::NOT_CONFIGURED_DEFAULT_AND_PRINT_NAME_CACHE_PATH,
                             "the configuration file not configured field defaultNameCachePath and printNameCache");

    ProjectNameCacheInfo merge;
    MergeNameCache(merge);

    std::string jsonStr = BuildJson(merge);
    if (!defaultPath.empty()) {
        FileUtil::WriteFile(defaultPath, jsonStr);
    }

    if (!printPath.empty()) {
        FileUtil::WriteFile(printPath, jsonStr);
    }
}

void panda::guard::NameCache::ParseAppliedNameCache(const std::string &content)
{
    JsonObject nameCacheObj(content);
    PANDA_GUARD_ASSERT_PRINT(!nameCacheObj.IsValid(), TAG, ErrorCode::NAME_CACHE_FILE_FORMAT_ERROR,
                             "the name cache file is not a valid json");

    for (size_t idx = 0; idx < nameCacheObj.GetSize(); idx++) {
        auto key = nameCacheObj.GetKeyByIndex(idx);
        if (key == ENTRY_PACKAGE_INFO) {
            historyNameCache_.entryPackageInfo = JsonUtil::GetStringValue(&nameCacheObj, ENTRY_PACKAGE_INFO);
        } else if (key == COMPILE_SDK_VERSION) {
            historyNameCache_.compileSdkVersion = JsonUtil::GetStringValue(&nameCacheObj, COMPILE_SDK_VERSION);
        } else if (key == PROPERTY_CACHE) {
            historyNameCache_.propertyCacheMap = JsonUtil::GetMapStringValue(&nameCacheObj, PROPERTY_CACHE);
        } else if (key == FILE_NAME_CACHE) {
            historyNameCache_.fileNameCacheMap = JsonUtil::GetMapStringValue(&nameCacheObj, FILE_NAME_CACHE);
        } else {
            auto fileCacheInfoObj = JsonUtil::GetJsonObject(&nameCacheObj, key);
            if (!fileCacheInfoObj) {
                continue;
            }
            auto identifierCache = JsonUtil::GetMapStringValue(fileCacheInfoObj, IDENTIFIER_CACHE);
            auto memberMethodCache = JsonUtil::GetMapStringValue(fileCacheInfoObj, MEMBER_METHOD_CACHE);

            FileNameCacheInfo fileCacheInfo;
            fileCacheInfo.identifierCacheMap = DeleteScopeAndLineNum(identifierCache);
            fileCacheInfo.memberMethodCacheMap = DeleteScopeAndLineNum(memberMethodCache);
            fileCacheInfo.obfName = JsonUtil::GetStringValue(fileCacheInfoObj, OBF_NAME);

            historyNameCache_.fileCacheInfoMap.emplace(key, fileCacheInfo);
        }
    }
}

void panda::guard::NameCache::ParseHistoryNameCacheToUsedNames()
{
    AddHistoryUsedNames(historyNameCache_.entryPackageInfo);
    AddHistoryUsedNames(historyNameCache_.compileSdkVersion);
    AddHistoryUsedNames(historyNameCache_.propertyCacheMap);
    AddHistoryUsedNames(historyNameCache_.fileNameCacheMap);
    for (const auto &item : historyNameCache_.fileCacheInfoMap) {
        AddHistoryUsedNames(item.second.identifierCacheMap);
        AddHistoryUsedNames(item.second.memberMethodCacheMap);
        AddHistoryUsedNames(item.second.obfName);
    }
}

void panda::guard::NameCache::AddHistoryUsedNames(const std::string &value)
{
    historyUsedNames_.emplace(value);
}

void panda::guard::NameCache::AddHistoryUsedNames(const std::map<std::string, std::string> &values)
{
    for (const auto &item : values) {
        historyUsedNames_.emplace(item.second);
    }
}

std::string panda::guard::NameCache::BuildJson(const ProjectNameCacheInfo &nameCacheInfo)
{
    JsonObjectBuilder builder;
    for (const auto &item : nameCacheInfo.fileCacheInfoMap) {
        builder.AddProperty(item.first, FileNameCacheToJson(item.second));
    }
    builder.AddProperty(ENTRY_PACKAGE_INFO, nameCacheInfo.entryPackageInfo);
    builder.AddProperty(COMPILE_SDK_VERSION, nameCacheInfo.compileSdkVersion);
    builder.AddProperty(GUARD_DYNAMIC_VERSION, OBFUSCATION_TOOL_VERSION);

    if (options_->IsPropertyObfEnabled() || options_->IsExportObfEnabled()) {
        builder.AddProperty(PROPERTY_CACHE, MapToJson(nameCacheInfo.propertyCacheMap, true));
    }
    if (options_->IsFileNameObfEnabled()) {
        builder.AddProperty(FILE_NAME_CACHE, MapToJson(nameCacheInfo.fileNameCacheMap, true));
    }
    return std::move(builder).Build();
}
