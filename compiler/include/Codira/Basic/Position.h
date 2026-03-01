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
 * This file declares the Position, which represents the position in a source file.
 */

#ifndef CODIRA_BASIC_POSITION_H
#define CODIRA_BASIC_POSITION_H

#include <string>
#include <cstdint>

namespace Codira {
enum class PositionStatus {
    KEEP,   /**< Mark the position is valid and should be kept. */
    IGNORE, /**< Mark the position should be ignored when emitting debug info. */
};

/**
 * A position in a source file. Line and column start at 1 (byte count for column). */
struct Position {
    Position(unsigned int fileID, int line, int column) noexcept : fileID(fileID), line(line), column(column)
    {
    }
    Position(int line, int column) noexcept : line(line), column(column)
    {
    }
    Position() = default;
    
    Position(unsigned int fileID, int line, int column, bool curfile) noexcept
        : fileID(fileID), line(line), column(column), isCurFile(curfile)
    {
    }

    unsigned int fileID = 0;
    int line = 0;
    int column = 0;
    bool isCurFile{false};
    bool operator==(const Position& rhs) const;
    bool operator!=(const Position& rhs) const;
    bool operator<(const Position& rhs) const;
    bool operator<=(const Position& rhs) const;
    bool operator>(const Position& rhs) const;
    bool operator>=(const Position& rhs) const;
    Position operator+(const Position& rhs) const;
    Position& operator+=(const Position& rhs);
    Position operator-(const Position& rhs) const;
    Position& operator-=(const Position& rhs);
    Position operator+(const size_t w) const;
    Position operator-(const size_t w) const;
    std::string ToString() const;
    /**
     * Whether line and column are both zero.
     */
    bool IsZero() const;
    void Mark(PositionStatus newStatus);
    PositionStatus GetStatus() const;

    friend std::ostream& operator<<(std::ostream& out, const Position& pos)
    {
        out << pos.ToString();
        return out;
    }
    inline uint64_t Hash64() const
    {
        return (static_cast<uint64_t>(fileID) << 32u) ^ (static_cast<uint64_t>(line) << 16u) ^
            (static_cast<uint64_t>(column));
    }
    // Hash without fileID for Macro.
    inline uint32_t Hash32() const
    {
        return (static_cast<uint32_t>(line) << 16u) ^ (static_cast<uint32_t>(column));
    }
    // Get the pair<line, column> from the hash value that created by Position.Hash32().
    static std::pair<int, int> RestorePosFromHash(uint32_t hash)
    {
        return std::pair(static_cast<int>(hash >> 16u), static_cast<int>(hash & 0xFFFF));
    }

private:
    PositionStatus status{PositionStatus::KEEP};
};

const Position INVALID_POSITION = Position{0, 0, 0};
const Position BEGIN_POSITION = Position{0, 1, 1};
const Position DEFAULT_POSITION = Position{0, -1, -1};
} // namespace Codira

#endif // CODIRA_BASIC_POSITION_H
