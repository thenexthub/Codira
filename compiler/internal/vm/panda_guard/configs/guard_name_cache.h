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

#ifndef PANDA_GUARD_CONFIGS_GUARD_NAME_CACHE_H
#define PANDA_GUARD_CONFIGS_GUARD_NAME_CACHE_H

#include <map>
#include <memory>
#include <set>
#include <string>

#include "guard_options.h"

namespace panda::guard {

struct FileNameCacheInfo {
    std::map<std::string, std::string> identifierCacheMap;
    std::map<std::string, std::string> memberMethodCacheMap;
    std::string obfName;
    std::string oriSourceFile;
    std::string obfSourceFile;
};

struct ProjectNameCacheInfo {
    std::map<std::string, FileNameCacheInfo> fileCacheInfoMap;
    std::string entryPackageInfo;
    std::string compileSdkVersion;
    std::map<std::string, std::string> propertyCacheMap;
    std::map<std::string, std::string> fileNameCacheMap;
};

class NameCache {
public:
    explicit NameCache(std::shared_ptr<GuardOptions> options) : options_(std::move(options)) {}

    void Load(const std::string &applyNameCachePath);

    [[nodiscard]] const std::set<std::string> &GetHistoryUsedNames() const;

    [[nodiscard]] std::map<std::string, std::string> GetHistoryFileNameMapping() const;

    [[nodiscard]] std::map<std::string, std::string> GetHistoryNameMapping() const;

    void AddObfIdentifierName(const std::string &filePath, const std::string &origin, const std::string &obfName);

    void AddObfMemberMethodName(const std::string &filePath, const std::string &origin, const std::string &obfName);

    void AddObfName(const std::string &filePath, const std::string &obfName);

    void AddSourceFile(const std::string &filePath, const std::string &oriSource, const std::string &obfSource);

    void AddNewNameCacheObfFileName(const std::string &origin, const std::string &obfName);

    void AddObfPropertyName(const std::string &origin, const std::string &obfName);

    void WriteCache();

private:
    void ParseAppliedNameCache(const std::string &content);

    void ParseHistoryNameCacheToUsedNames();

    void AddHistoryUsedNames(const std::string &value);

    void AddHistoryUsedNames(const std::map<std::string, std::string> &values);

    std::string BuildJson(const ProjectNameCacheInfo &nameCacheInfo);

    void MergeNameCache(ProjectNameCacheInfo &merge);

    std::shared_ptr<GuardOptions> options_ = nullptr;

    std::set<std::string> historyUsedNames_ {};

    ProjectNameCacheInfo historyNameCache_ {};

    ProjectNameCacheInfo newNameCache_ {};
};

}  // namespace panda::guard

#endif  // PANDA_GUARD_CONFIGS_GUARD_NAME_CACHE_H
