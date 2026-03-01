/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_DEBUG_INFO_H
#define PANDA_DEBUG_INFO_H

#include <string>
#include <limits>
#include <cstdint>

namespace ark::compiler {

class InstDebugInfo {
public:
    static constexpr uint32_t INVALID_LINE_NUMBER = std::numeric_limits<uint32_t>::max();
    static constexpr uint32_t INVALID_FILE_INDEX = std::numeric_limits<uint32_t>::max();
    static constexpr uint32_t INVALID_DIR_INDEX = std::numeric_limits<uint32_t>::max();

    InstDebugInfo(uint32_t dirIndex, uint32_t fileIndex, uint32_t lineNumber)
        : dirIndex_(dirIndex), fileIndex_(fileIndex), lineNumber_(lineNumber)
    {
    }

    uint32_t GetLineNumber() const
    {
        return lineNumber_;
    }

    uint32_t GetFileIndex() const
    {
        return fileIndex_;
    }

    uint32_t GetDirIndex() const
    {
        return dirIndex_;
    }

    size_t GetOffset() const
    {
        return offset_;
    }

    void SetOffset(size_t offset)
    {
        offset_ = offset;
    }

private:
    uint32_t dirIndex_ {INVALID_DIR_INDEX};
    uint32_t fileIndex_ {INVALID_FILE_INDEX};
    uint32_t lineNumber_ {INVALID_LINE_NUMBER};
    size_t offset_ {0};
};

}  // namespace ark::compiler

#endif  // PANDA_DEBUG_INFO_H
