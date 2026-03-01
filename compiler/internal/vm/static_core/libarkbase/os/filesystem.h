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

#ifndef PANDA_FILESYSTEM_H
#define PANDA_FILESYSTEM_H

#include "libarkbase/macros.h"
#include <string>

#if defined(PANDA_TARGET_WINDOWS)
#ifndef NAME_MAX
constexpr size_t NAME_MAX = 255;
#endif  // NAME_MAX
#elif defined(USE_STD_FILESYSTEM)
#include <filesystem>
#else
#include <experimental/filesystem>
#endif

namespace ark::os {

PANDA_PUBLIC_API std::string GetAbsolutePath(std::string_view path);

PANDA_PUBLIC_API void CreateDirectories(const std::string &folderName);

PANDA_PUBLIC_API bool IsFileExists(const std::string &filepath);

PANDA_PUBLIC_API std::string GetParentDir(const std::string &filepath);

PANDA_PUBLIC_API bool IsDirExists(const std::string &dirpath);

PANDA_PUBLIC_API std::string RemoveExtension(const std::string &filepath);

PANDA_PUBLIC_API std::string NormalizePath(const std::string &filepath);

// Get the current working directory of the running process
PANDA_PUBLIC_API std::string GetCurrentWorkingDirectory();

// Change current working directory of the running process
PANDA_PUBLIC_API void ChangeCurrentWorkingDirectory(const std::string &path);

}  // namespace ark::os

#endif  // PANDA_FILESYSTEM_H
