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

#ifndef CODEFMT_FORMATCODEPROCESSOR_H
#define CODEFMT_FORMATCODEPROCESSOR_H

#include "Format/ASTToFormatSource.h"
#include "Format/Doc.h"
#include "Codira/Basic/Print.h"
#include "Codira/Parse/Parser.h"
#include "Codira/Utils/FileUtil.h"

#include <dirent.h>
#include <fstream>
#include <istream>
#include <map>
#include <optional>
#include <string>
#include <sys/stat.h>
#include <vector>
namespace Codira::Format {
constexpr int OK = 0;
constexpr int ERR = 1;
const int DEPTH_OF_RECURSION = 0;
/* Sets the maximum recursion depth.The value -1 indicates that the recursion depth is not limited.It will be used as a
 * configuration option later. */
const int MAX_RECURSION_DEPTH = -1;

int FmtDir(const std::string& fmtDirPath, const std::string& dirOutputPath);
std::optional<std::string> FormatText(const std::string& rawCode, const std::string& filepath, Region regionToFormat);
bool FormatFile(std::string& rawCode, const std::string& filepath, std::string& sourceFormat, Region regionToFormat);
bool HasEnding(std::string const& fullString, std::string const& ending);
void TraveDepthLimitedDirs(
    const std::string& path, std::map<std::string, std::string>& fileMap, int depth, const int maxDepth);
std::string GetTargetName(
    const std::string& file, const std::string& fileName, const std::string& fmtDirPath, const std::string& outPath);
std::string PathJoin(const std::string& baseDir, const std::string& baseName);
} // namespace Codira::Format
#endif // CODEFMT_FORMATCODEPROCESSOR_H
