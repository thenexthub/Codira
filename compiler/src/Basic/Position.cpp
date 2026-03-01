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
 * This file implements the Position.
 */

#include "Codira/Basic/Position.h"

#include <sstream>
#include <tuple>

using namespace Codira;
using Codira::Position;

bool Position::operator==(const Position& rhs) const
{
    return std::tie(line, column) == std::tie(rhs.line, rhs.column);
}

bool Position::operator!=(const Position& rhs) const
{
    return !(*this == rhs);
}

bool Position::operator<(const Position& rhs) const
{
    return std::tie(line, column) < std::tie(rhs.line, rhs.column);
}

bool Position::operator<=(const Position& rhs) const
{
    return std::tie(line, column) <= std::tie(rhs.line, rhs.column);
}

bool Position::operator>(const Position& rhs) const
{
    return !(*this <= rhs);
}

bool Position::operator>=(const Position& rhs) const
{
    return !(*this < rhs);
}

Position Position::operator+(const Position& rhs) const
{
    Position ret;
    ret.fileID = fileID + rhs.fileID;
    ret.line = line + rhs.line;
    ret.column = column + rhs.column;
    ret.isCurFile = isCurFile;
    return ret;
}

Position& Position::operator+=(const Position& rhs)
{
    fileID += rhs.fileID;
    line += rhs.line;
    column += rhs.column;
    return *this;
}

Position Position::operator-(const Position& rhs) const
{
    Position ret;
    ret.fileID = fileID - rhs.fileID;
    ret.line = line - rhs.line;
    ret.column = column - rhs.column;
    return ret;
}

Position& Position::operator-=(const Position& rhs)
{
    fileID -= rhs.fileID;
    line -= rhs.line;
    column -= rhs.column;
    return *this;
}

std::string Position::ToString() const
{
    std::stringstream ss;
    ss << "(" << fileID << ", " << line << ", " << column << ")";
    return ss.str();
}

Position Position::operator+(const size_t w) const
{
    auto ret = Position{fileID, line, column + static_cast<int>(w)};
    ret.isCurFile = isCurFile;
    return ret;
}

Position Position::operator-(const size_t w) const
{
    auto ret = Position{fileID, line, column - static_cast<int>(w)};
    ret.isCurFile = isCurFile;
    return ret;
}

bool Position::IsZero() const
{
    return line == 0 && column == 0;
}

void Position::Mark(PositionStatus newStatus)
{
    status = newStatus;
}

PositionStatus Position::GetStatus() const
{
    return status;
}
