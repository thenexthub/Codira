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
 * This file implements the DebugLocation in CHIR.
 */

#include "Codira/CHIR/DebugLocation.h"

#include <iostream>
#include <sstream>

using namespace Codira::CHIR;

Position DebugLocation::GetBeginPos() const
{
    return beginPos;
}

bool DebugLocation::operator==(const DebugLocation& other) const
{
    return beginPos.line == other.beginPos.line && beginPos.column == other.beginPos.column &&
        endPos.line == other.endPos.line && endPos.column == other.endPos.column &&
        fileID == other.fileID;
}

Position DebugLocation::GetEndPos() const
{
    return endPos;
}

void DebugLocation::SetBeginPos(const Position& pos)
{
    beginPos = pos;
}

void DebugLocation::SetEndPos(const Position& pos)
{
    endPos = pos;
}

void DebugLocation::SetScopeInfo(const std::vector<int>& scope)
{
    scopeInfo = scope;
}

/**
 * @brief get the ID of the file.
 */
unsigned DebugLocation::GetFileID() const
{
    return fileID;
}

const std::string& DebugLocation::GetAbsPath() const
{
    return *absPath;
}

std::vector<int> DebugLocation::GetScopeInfo() const
{
    return scopeInfo;
}

bool DebugLocation::IsInvalidPos() const
{
    return beginPos.line == 0 || beginPos.column == 0 || endPos.line == 0 || endPos.column == 0;
}

bool DebugLocation::IsInvalidMacroPos() const
{
    return beginPos.line == 0 || beginPos.column == 0;
}

std::string DebugLocation::GetFileName() const
{
#ifdef _WIN32
    const std::string dirSeparator = "\\/";
#else
    const std::string dirSeparator = "/";
#endif
    auto fileName = absPath->substr(absPath->find_last_of(dirSeparator) + 1);
    return fileName;
}

std::string DebugLocation::ToString() const
{
    if (*this == INVALID_LOCATION) {
        return "";
    }
#ifdef _WIN32
    const std::string dirSeparator = "\\/";
#else
    const std::string dirSeparator = "/";
#endif
    std::stringstream ss;
    std::string name = absPath->substr(absPath->find_last_of(dirSeparator) + 1);
    ss << "loc: \"" << name << "\"-" << beginPos.line << "-" << beginPos.column;
    if (!scopeInfo.empty()) {
        ss << ", scope: " << scopeInfo[0];
    }
    for (size_t t = 1; t < scopeInfo.size(); t++) {
        ss << "-" << scopeInfo[t];
    }
    return ss.str();
}

void DebugLocation::Dump() const
{
    std::cout << ToString() << std::endl;
}
