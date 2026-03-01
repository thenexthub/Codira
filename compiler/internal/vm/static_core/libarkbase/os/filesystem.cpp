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

#include <string>
#include "libarkbase/os/filesystem.h"
#include "libarkbase/os/file.h"
#include "libarkbase/utils/logger.h"
#include <sys/types.h>
#include <sys/stat.h>
#if defined(PANDA_TARGET_WINDOWS)
#include <fileapi.h>
#endif

namespace ark::os {

void CreateDirectories(const std::string &folderName)
{
#ifdef PANDA_TARGET_MOBILE
    constexpr auto DIR_PERMISSIONS = 0775;
    mkdir(folderName.c_str(), DIR_PERMISSIONS);
#elif PANDA_TARGET_MACOS || PANDA_TARGET_OHOS
    std::filesystem::create_directories(std::filesystem::path(folderName));
#elif PANDA_TARGET_WINDOWS
    CreateDirectory(folderName.c_str(), NULL);
#else
    constexpr auto DIR_PERMISSIONS = 0775;
    auto status = mkdir(folderName.c_str(), DIR_PERMISSIONS);
    if (status != 0) {
        LOG(WARNING, COMMON) << "Failed to create directory \"" << folderName.c_str() << "\"\n";
        LOG(WARNING, COMMON) << "Return status :" << status << "\n";
    }
#endif
}

bool IsFileExists(const std::string &filepath)
{
    if (filepath.empty()) {
        return false;
    }
#if PANDA_TARGET_WINDOWS
    DWORD fileAttr = GetFileAttributesA(filepath.c_str());
    return (fileAttr != INVALID_FILE_ATTRIBUTES);
#else
    std::ifstream openedFile(filepath);
    return openedFile.good();
#endif
}

std::string GetParentDir(const std::string &filepath)
{
    size_t found = filepath.find_last_of("/\\");
    if (found == std::string::npos) {
        return std::string("");
    }
    return filepath.substr(0, found);
}

bool IsDirExists(const std::string &dirpath)
{
    if (dirpath == std::string("")) {
        return true;
    }
    struct stat info {};
    return (stat(dirpath.c_str(), &info) == 0) && ((info.st_mode & static_cast<unsigned int>(S_IFDIR)) != 0U);
}

std::string RemoveExtension(const std::string &filepath)
{
    auto pos = filepath.find_last_of('.');
    if (pos != std::string::npos) {
        return filepath.substr(0, pos);
    }
    return filepath;
}

std::string NormalizePath(const std::string &path)
{
    if (path.empty()) {
        return path;
    }

    auto delim = file::File::GetPathDelim();
    ASSERT(delim.length() == 1);
    auto delimChar = delim[0];

    std::vector<std::string_view> parts;
    size_t begin = 0;
    size_t length = 0;
    size_t i = 0;
    while (i < path.size()) {
        if (path[i++] != delimChar) {
            ++length;
            continue;
        }

        std::string_view sv(&path[begin], length);

        while (path[i] == delimChar) {
            ++i;
        }

        begin = i;
        length = 0;

        if (sv == ".") {
            continue;
        }

        if (sv == "..") {
            if (parts.empty()) {
                parts.emplace_back("");
                continue;
            }

            if (!parts.back().empty()) {
                parts.pop_back();
            }
            continue;
        }

        parts.push_back(sv);
    }

    std::string_view sv(&path[begin], length);
    parts.push_back(sv);

    std::stringstream ss;

    ASSERT(!parts.empty());
    i = 0;
    ss << parts[i++];

    while (i < parts.size()) {
        ss << delim;
        ss << parts[i++];
    }

    return ss.str();
}

std::string GetCurrentWorkingDirectory()
{
#ifdef PANDA_TARGET_UNIX
    std::array<char, PATH_MAX> cwd {};
    if (getcwd(cwd.data(), sizeof(char) * cwd.size()) == nullptr) {
        LOG(WARNING, COMMON) << "Failed to get current working directory";
        return {};
    }
    return std::string {cwd.data()};
#else
    UNREACHABLE();
#endif
}

void ChangeCurrentWorkingDirectory([[maybe_unused]] const std::string &path)
{
#ifdef PANDA_TARGET_UNIX
    auto status = chdir(path.c_str());
    if (status != 0) {
        LOG(WARNING, COMMON) << "Failed to change current working directory\n";
    }
#else
    UNREACHABLE();
#endif
}

}  // namespace ark::os
