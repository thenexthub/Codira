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

#ifndef LIBPANDAFILE_DEBUG_DATA_ACCESSOR_H_
#define LIBPANDAFILE_DEBUG_DATA_ACCESSOR_H_

#include "libarkfile/file.h"

#include "libarkbase/utils/span.h"

namespace ark::panda_file {

class DebugInfoDataAccessor {
public:
    DebugInfoDataAccessor(const File &pandaFile, File::EntityId debugInfoId);

    ~DebugInfoDataAccessor() = default;

    NO_COPY_SEMANTIC(DebugInfoDataAccessor);
    NO_MOVE_SEMANTIC(DebugInfoDataAccessor);

    uint32_t GetLineStart() const
    {
        return lineStart_;
    }

    uint32_t GetNumParams() const
    {
        return numParams_;
    }

    template <class Callback>
    void EnumerateParameters(const Callback &cb);

    Span<const uint8_t> GetConstantPool();

    const uint8_t *GetLineNumberProgram();

    size_t GetSize()
    {
        if (size_ == 0) {
            SkipLineNumberProgram();
        }

        return size_;
    }

    const File &GetPandaFile() const
    {
        return pandaFile_;
    }

    File::EntityId GetDebugInfoId() const
    {
        return debugInfoId_;
    }

private:
    void SkipParameters();

    void SkipConstantPool();

    void SkipLineNumberProgram();

    const File &pandaFile_;
    File::EntityId debugInfoId_;

    uint32_t lineStart_;
    uint32_t numParams_;
    Span<const uint8_t> parametersSp_ {nullptr, nullptr};
    Span<const uint8_t> constantPoolSizeSp_ {nullptr, nullptr};
    Span<const uint8_t> lineNumProgramOffSp_ {nullptr, nullptr};

    size_t size_ {0};
};

}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_DEBUG_DATA_ACCESSOR_H_
