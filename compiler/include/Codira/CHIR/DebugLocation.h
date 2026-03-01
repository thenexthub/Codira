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
 * This file declares the DebugLocation in CHIR.
 */

#ifndef CODIRA_CHIR_DEBUGLOCATION_H
#define CODIRA_CHIR_DEBUGLOCATION_H

#include <string>
#include <vector>

namespace Codira::CHIR {
const std::string INVALID_NAME = ""; // Invalid file name

struct Position {
    unsigned line{0};
    unsigned column{0};

    bool IsLegal() const
    {
        return line != 0 && column != 0;
    }
    bool IsZero() const
    {
        return line == 0 && column == 0;
    }
};

/**
 * @brief A Debug location in source code.
 *
 */
class DebugLocation {
public:
    DebugLocation(const std::string& absPath, unsigned fileID,
        const Position& beginPos, const Position& endPos, const std::vector<int>& scopeInfo = {0})
        : absPath(&absPath), fileID(fileID), beginPos(beginPos), endPos(endPos), scopeInfo(scopeInfo)
    {
    }

    DebugLocation() : absPath(&INVALID_NAME), fileID(0), beginPos({0, 0}), endPos({0, 0})
    {
    }
    ~DebugLocation() = default;

    // ===--------------------------------------------------------------------===//
    // Position
    // ===--------------------------------------------------------------------===//
    Position GetBeginPos() const;
    void SetBeginPos(const Position& pos);

    Position GetEndPos() const;
    void SetEndPos(const Position& pos);

    bool IsInvalidPos() const;

    bool IsInvalidMacroPos() const;

    // ===--------------------------------------------------------------------===//
    // Scope Info
    // ===--------------------------------------------------------------------===//
    std::vector<int> GetScopeInfo() const;
    void SetScopeInfo(const std::vector<int>& scope);

    // ===--------------------------------------------------------------------===//
    // File Info
    // ===--------------------------------------------------------------------===//
    unsigned GetFileID() const;

    const std::string& GetAbsPath() const;

    std::string GetFileName() const;

    // ===--------------------------------------------------------------------===//
    // Others
    // ===--------------------------------------------------------------------===//
    bool operator==(const DebugLocation& other) const;
    std::string ToString() const;
    void Dump() const;

private:
    const std::string* absPath; /* the absolute path of file */
    unsigned fileID;            /* the file id */
    Position beginPos;          /* the begin position in file, start from 1, 1 */
    Position endPos;            /* the end position in file, start from 1, 1 */
    std::vector<int> scopeInfo; /* scope info, like 0-0-0 0-1 */
};

const DebugLocation INVALID_LOCATION = DebugLocation();
} // namespace Codira::CHIR
#endif // CODIRA_CHIR_DEBUGLOCATION_H
