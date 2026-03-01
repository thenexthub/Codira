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

#include "extractor.h"

#include <regex>

namespace ark::extractor {
constexpr char EXT_NAME_ABC[] = ".abc";  // NOLINT(modernize-avoid-c-arrays)
constexpr const char *LOCAL_CODE_PATH = "/data/storage/el1/bundle";
constexpr std::array<const char *, 3> LOCAL_HSP_CODE_PATHS = {"/data/storage/el1/bundle", "/system/app/appServiceFwk",
                                                              "/system/app/shared_bundles"};
constexpr const char *FILE_SEPARATOR = "/";
constexpr const int MAX_INNER_HSP_SLASHES = 3;

static bool StringEndWith(const std::string &str, const char *endStr, size_t endStrLen)
{
    size_t len = str.length();
    return ((len >= endStrLen) && (str.compare(len - endStrLen, endStrLen, endStr) == 0));
}

static bool IsInnerHspPath(const std::string &path)
{
    return std::count(path.begin(), path.end(), '/') <= MAX_INNER_HSP_SLASHES;
}

static std::string GetRelativePath(const std::string &srcPath)
{
    if (srcPath.empty() || srcPath[0] != '/') {
        return srcPath;
    }
    std::regex srcPattern(LOCAL_CODE_PATH);
    std::string relativePath = std::regex_replace(srcPath, srcPattern, "");
    if (relativePath.find(FILE_SEPARATOR) == 0) {
        relativePath = relativePath.substr(1);
        relativePath = relativePath.substr(relativePath.find(std::string(FILE_SEPARATOR)) + 1);
    }
    return relativePath;
}

/**
 * @brief Converts a given absolute path to a relative path, specifically for HSP path handling.
 * This function processes to path based on its type (internal HSP path or external path) and returns a relative path
 * suitable for HSP modules.
 *
 * @param srcPath The input absolute path
 * @return std::string The processed relative path
 * @example
 * Input path: 1."/data/storage/el1/bundle/com.example.hsp/library/library/ets/module_static.abc"
 *             2."/system/app/appServiceFwk/com.example.hsp/library/library/ets/module_static.abc"
 *             3."/system/app/shared_bundles/com.example.hsp/library/library/ets/module_static.abc"
 * Output path: "ets/module_static.abc"
 */
static std::string GetRelativePathForHsp(const std::string &srcPath)
{
    if (srcPath.empty() || srcPath[0] != '/') {
        return srcPath;
    }
    std::string relativePath = srcPath;
    for (const auto &path : LOCAL_HSP_CODE_PATHS) {
        std::string tempRelativePath = std::regex_replace(srcPath, std::regex(path), "");
        if (tempRelativePath != srcPath) {
            relativePath = std::move(tempRelativePath);
            break;
        }
    }
    if (IsInnerHspPath(relativePath)) {
        if (relativePath.find(FILE_SEPARATOR) == 0) {
            relativePath = relativePath.substr(1);
            relativePath = relativePath.substr(relativePath.find(std::string(FILE_SEPARATOR)) + 1);
        }
    } else {
        if (relativePath.find(FILE_SEPARATOR) == 0) {
            std::string bundleName = relativePath.substr(1);
            std::string moduleName = bundleName;
            if (srcPath.find(LOCAL_CODE_PATH) == 0) {
                moduleName = bundleName.substr(bundleName.find(std::string(FILE_SEPARATOR)) + 1);
            }
            std::string hspName = moduleName.substr(moduleName.find(std::string(FILE_SEPARATOR)) + 1);
            relativePath = hspName.substr(hspName.find(std::string(FILE_SEPARATOR)) + 1);
        }
    }
    return relativePath;
}

Extractor::Extractor(const std::string &source) : zipFile_(source)
{
    filePath_ = source;
}

bool Extractor::Init()
{
    if (!zipFile_.Open()) {
        LOG(ERROR, ZIPARCHIVE) << "open zip file failed";
        return false;
    }
    return true;
}

std::shared_ptr<FileMapper> Extractor::GetSafeData(const std::string &fileName)
{
    std::string relativePath = GetRelativePath(fileName);
    if (!StringEndWith(relativePath, EXT_NAME_ABC, sizeof(EXT_NAME_ABC) - 1)) {
        return nullptr;
    }

    return zipFile_.CreateFileMapper(relativePath, FileMapperType::SAFE_ABC);
}

std::shared_ptr<FileMapper> Extractor::GetSafeDataForHsp(const std::string &fileName)
{
    std::string relativePath = GetRelativePathForHsp(fileName);
    if (!StringEndWith(relativePath, EXT_NAME_ABC, sizeof(EXT_NAME_ABC) - 1)) {
        return nullptr;
    }

    return zipFile_.CreateFileMapper(relativePath, FileMapperType::SAFE_ABC);
}
}  // namespace ark::extractor
