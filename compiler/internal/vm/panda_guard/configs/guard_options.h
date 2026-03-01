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

#ifndef PANDA_GUARD_CONFIGS_GUARD_OPTIONS_H
#define PANDA_GUARD_CONFIGS_GUARD_OPTIONS_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace panda::guard {

struct ObfuscationOption {
    bool enable = false;
    std::vector<std::string> reservedList;
    std::vector<std::string> universalReservedList;
};

struct FileNameOption {
    bool enable = false;
    std::vector<std::string> reservedFileNames;
    std::vector<std::string> universalReservedFileNames;
    std::vector<std::string> reservedRemoteHarPkgNames;
};

struct KeepOption {
    bool enable = false;
    std::vector<std::string> keepPaths;
};

struct ObfuscationRules {
    bool disableObfuscation = false;
    bool enableExportObfuscation = false;
    bool enableRemoveLog = false;
    bool enableDecorator = false;
    bool enableCompact = false;
    std::string printNameCache;
    std::string applyNameCache;
    std::vector<std::string> reservedNames;
    std::vector<std::string> recordNameWhiteList;
    ObfuscationOption propertyOption;
    ObfuscationOption toplevelOption;
    FileNameOption fileNameOption;
    KeepOption keepOption;
};

struct ObfuscationConfig {
    std::string abcFilePath;
    std::string obfAbcFilePath;
    std::string obfPaFilePath;
    std::string compileSdkVersion;
    uint8_t targetApiVersion;
    std::string targetApiSubVersion;  // This field represents compatibleSdkVersions Stage
    std::string filesInfoPath;
    std::string sourceMapsPath;
    std::string entryPackageInfo;
    std::string defaultNameCachePath;
    std::vector<std::string> skippedRemoteHarList;
    bool useNormalizedOHMUrl;
    ObfuscationRules obfuscationRules;
};

class GuardOptions {
public:
    void Load(const std::string &configFilePath);

    [[nodiscard]] const std::string &GetAbcFilePath() const;

    [[nodiscard]] const std::string &GetObfAbcFilePath() const;

    [[nodiscard]] const std::string &GetObfPaFilePath() const;

    [[nodiscard]] const std::string &GetCompileSdkVersion() const;

    [[nodiscard]] uint8_t GetTargetApiVersion() const;

    [[nodiscard]] const std::string &GetTargetApiSubVersion() const;

    [[nodiscard]] const std::string &GetEntryPackageInfo() const;

    [[nodiscard]] const std::string &GetDefaultNameCachePath() const;

    [[nodiscard]] const std::string &GetPrintNameCache() const;

    [[nodiscard]] const std::string &GetApplyNameCache() const;

    [[nodiscard]] bool DisableObfuscation() const;

    [[nodiscard]] bool IsExportObfEnabled() const;

    [[nodiscard]] bool IsRemoveLogObfEnabled() const;

    [[nodiscard]] bool IsDecoratorObfEnabled() const;

    [[nodiscard]] bool IsCompactObfEnabled() const;

    [[nodiscard]] bool IsPropertyObfEnabled() const;

    [[nodiscard]] bool IsToplevelObfEnabled() const;

    [[nodiscard]] bool IsFileNameObfEnabled() const;

    [[nodiscard]] bool IsKeepPath(const std::string &path) const;

    [[nodiscard]] bool IsReservedNames(const std::string &name) const;

    [[nodiscard]] bool IsInRecordNameWhiteList(const std::string &name) const;

    [[nodiscard]] bool IsReservedProperties(const std::string &name) const;

    [[nodiscard]] bool IsReservedToplevelNames(const std::string &name) const;

    [[nodiscard]] bool IsReservedFileNames(const std::string &name) const;

    [[nodiscard]] const std::string &GetSourceName(const std::string &name) const;

    [[nodiscard]] bool IsSkippedRemoteHar(const std::string &pkgName) const;

    [[nodiscard]] bool IsUseNormalizedOhmUrl() const;

    [[nodiscard]] bool IsReservedRemoteHarPkgNames(const std::string &name) const;

private:
    ObfuscationConfig obfConfig_ {};

    std::unordered_map<std::string, std::string> sourceNameTable_ {};
};

}  // namespace panda::guard

#endif  // PANDA_GUARD_CONFIGS_GUARD_OPTIONS_H
