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
 * This file implements GCCPath and GCCPathScanner.
 */

#include "Codira/Driver/Toolchains/GCCPathScanner.h"

#include <limits>

#include "Codira/Utils/FileUtil.h"

using namespace Codira;

bool GCCPathScanner::IsCompatiableTripleName(const Triple::Info& info, const std::string& name)
{
    if (info.os == Triple::OSType::LINUX && info.env == Triple::Environment::GNU &&
        info.vendor == Triple::Vendor::UNKNOWN) {
        std::string archString = info.ArchToString();
        std::vector<std::string> compatibleNames = {
            info.ToFullTripleString(),
            archString + "-linux-gnu",
            archString + "-pc-linux-gnu",
            archString + "-suse-linux",
            archString + "-redhat-linux"
        };
        return std::find(compatibleNames.begin(), compatibleNames.end(), name) != compatibleNames.end();
    }
    std::string tripleName = info.ToFullTripleString();
    if (tripleName == "x86_64-unknown-windows-gnu") {
        return name == tripleName || name == "x86_64-w64-mingw32";
    }
    return false;
}

std::vector<FileUtil::Directory> GCCPathScanner::GetAllMatchingSubDirectories(
    const Triple::Info& target, const std::vector<FileUtil::Directory>& directories)
{
    std::vector<FileUtil::Directory> possibleDirectories;
    for (const FileUtil::Directory& directory : directories) {
        if (IsCompatiableTripleName(target, directory.name) ||
            Triple::IsPossibleMatchingTripleName(target, directory.name)) {
            auto subDirectories = FileUtil::GetDirectories(directory.path);
            possibleDirectories.insert(possibleDirectories.end(), subDirectories.begin(), subDirectories.end());
        }
    }
    return possibleDirectories;
}

std::optional<GCCPath> GCCPathScanner::Scan()
{
    if (searchPaths.empty()) {
        return std::nullopt;
    }
    std::string selectedGccPath;
    GCCVersion selectedGccVersion{0, 0, 0};
    for (std::string& dirPrefix : searchPaths) {
        auto gccDirName = FileUtil::JoinPath(dirPrefix, "gcc");
        auto directories = GetAllMatchingSubDirectories(tripleTarget, FileUtil::GetDirectories(gccDirName));
        for (const auto& dir : directories) {
            std::optional<GCCVersion> gccVersion = StrToGCCVersion(dir.name);
            if (gccVersion && selectedGccVersion < *gccVersion && AllForFilesExist(dir.path)) {
                selectedGccPath = dir.path;
                selectedGccVersion = *gccVersion;
            }
        }
    }
    if (selectedGccPath.empty()) {
        return std::nullopt;
    } else {
        return {GCCPath{selectedGccPath, selectedGccVersion}};
    }
}

std::optional<GCCVersion> GCCPathScanner::StrToGCCVersion(const std::string& versionStr)
{
    // For std::isdigit, non-zero value if the character is a numeric character, zero otherwise.
    auto hasInvalidDigits = [](unsigned char c) { return std::isdigit(c) == 0; };

    GCCVersion ver{0, 0, 0};
    auto majorEndIndex = versionStr.find_first_of('.');
    if (majorEndIndex == 0) {
        // e.g. ".whatever", ".y.z" or such things are not valid gcc version names
        return std::nullopt;
    }
    // If . is not found, it may have only major number, e.g. "7", "8" or such things are valid gcc version name
    if (majorEndIndex == std::string::npos) {
        majorEndIndex = versionStr.length();
    }
    // If it is not a number, it is not a valid gcc version name
    bool invalidMajorVer = std::any_of(
        versionStr.begin(), versionStr.begin() + static_cast<std::ptrdiff_t>(majorEndIndex), hasInvalidDigits);
    if (invalidMajorVer) {
        return std::nullopt;
    }
    int majorVer = std::stoi(versionStr.substr(0, majorEndIndex));
    CODEC_ASSERT(majorVer >= std::numeric_limits<uint8_t>::min() && majorVer <= std::numeric_limits<uint8_t>::max());
    ver.major = static_cast<uint8_t>(majorVer);

    if (majorEndIndex == versionStr.length()) {
        return {ver};
    }

    auto minorEndIndex = versionStr.find_first_of('.', majorEndIndex + 1);
    if (minorEndIndex == majorEndIndex + 1) {
        return std::nullopt;
    }

    if (minorEndIndex == std::string::npos) {
        minorEndIndex = versionStr.length();
    }

    bool invalidMinorVer = std::any_of(versionStr.begin() + static_cast<std::ptrdiff_t>(majorEndIndex + 1),
        versionStr.begin() + static_cast<std::ptrdiff_t>(minorEndIndex), hasInvalidDigits);
    if (invalidMinorVer) {
        return std::nullopt;
    }

    int minorVer = std::stoi(versionStr.substr(majorEndIndex + 1, (minorEndIndex - majorEndIndex) - 1));
    CODEC_ASSERT(minorVer >= std::numeric_limits<uint8_t>::min() && minorVer <= std::numeric_limits<uint8_t>::max());
    ver.minor = static_cast<uint8_t>(minorVer);

    if (minorEndIndex == versionStr.length()) {
        return {ver};
    }

    // Build is empty, it is not a valid gcc version name
    if (versionStr.length() == minorEndIndex + 1) {
        return std::nullopt;
    }

    bool invalidBuildVer = std::any_of(
        versionStr.begin() + static_cast<std::ptrdiff_t>(minorEndIndex + 1), versionStr.end(), hasInvalidDigits);
    if (invalidBuildVer) {
        return std::nullopt;
    }

    int buildVer = std::stoi(versionStr.substr(minorEndIndex + 1, (versionStr.length() - minorEndIndex) - 1));
    CODEC_ASSERT(buildVer >= std::numeric_limits<uint8_t>::min() && buildVer <= std::numeric_limits<uint8_t>::max());
    ver.build = static_cast<uint8_t>(buildVer);
    return {ver};
}

bool GCCPathScanner::AllForFilesExist(const std::string& path)
{
    return std::all_of(forFiles.cbegin(), forFiles.cend(),
        [&path](const std::string& filename) { return FileUtil::FileExist(FileUtil::JoinPath(path, filename)); });
}
