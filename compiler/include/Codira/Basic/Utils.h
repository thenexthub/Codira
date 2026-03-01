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
 * This file declares some common utility functions.
 */

#ifndef CODIRA_BASIC_UTILS_H
#define CODIRA_BASIC_UTILS_H

#include <string>
#include <vector>
#include <cstdint>

namespace Codira::Utils {

const uint8_t LINUX_LINE_TERMINATOR_LENGTH = 1;
const uint8_t WINDOWS_LINE_TERMINATOR_LENGTH = 2;

/// Get hash value by std::hash<std::string>.
uint64_t GetHash(const std::string& content);
/// Split a string by '\r\n' and '\n' to form a vector of strings.
std::vector<std::string> SplitLines(const std::string& str);
/// Split a string by customised \ref delimiter. Typically used in command line arguments parsing.
std::vector<std::string> SplitString(const std::string& str, const std::string& delimiter);
/// \param splitDc whether to split by '::' as well.
std::vector<std::string> SplitQualifiedName(const std::string& qualifiedName, bool splitDc = false);
/// Join strings \ref strs with \ref delimiter.
std::string JoinStrings(const std::vector<std::string>& strs, const std::string& delimiter);
/**
 * check whether character(s) start from pStr is `LineTerminator`
 * if it is a Windows LineTerminator '\r\n', return 2
 * if it is a Linux LineTerminator '\n', return 1
 * otherwise, 0 is returned.
 */
uint8_t GetLineTerminatorLength(const char* pStr, const char* pEnd);

#ifdef _WIN32
struct WindowsOsVersion {
    unsigned long dwMajorVersion;
    unsigned long dwMinorVersion;
    unsigned long dwBuildNumber;
    unsigned long dwPlatformId;
};

WindowsOsVersion GetOSVersion();
#endif

inline std::string GetLineTerminator()
{
#ifdef _WIN32
    return "\r\n";
#else
    return "\n";
#endif
}
} // namespace Codira::Utils

#endif
