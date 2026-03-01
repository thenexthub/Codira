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
 * This file declares some utility functions.
 */

#ifndef CODIRA_DRIVER_UTILS_H
#define CODIRA_DRIVER_UTILS_H

#include <optional>
#include <string>
#include <vector>

namespace Codira {
/**
 * @brief Get the input string quoted with single quotes.
 * Note: Single quotes in the input are transform to '\'' instead of \'.
 *
 * @param str The input string.
 * @return std::string The single quoted string.
 */
std::string GetSingleQuoted(const std::string& str);

/**
 * @brief Get the input string quoted for passing as a command line argument.
 * - In the case of Linux, the argument is quoted with single quotes. Nested single quotes are
 *   transformed to '\''.
 * - In the case of Windows, the argument is quoted with double quotes. Nested double quotes and
 *   backslashes are escaped by \.
 *
 * @param arg The input string.
 * @return std::string The quoted argument.
 */
std::string GetCommandLineArgumentQuoted(const std::string& arg);

/**
 * @brief Prepend to paths.
 *
 * @param prefix The path prefix to be added.
 * @param paths The path vector.
 * @param quoted Determine whether the path string add single quotes.
 * @return std::vector<std::string> The vector of paths with a prefix added.
 */
std::vector<std::string> PrependToPaths(
    const std::string& prefix, const std::vector<std::string>& paths, bool quoted = false);

/**
 * @brief Get darwin SDK version.
 *
 * @param sdkPath The sdk path.
 * @return std::optional<std::string> The sdk version info.
 */
std::optional<std::string> GetDarwinSDKVersion(const std::string& sdkPath);
} // namespace Codira

#endif // CODIRA_DRIVER_UTILS_H
