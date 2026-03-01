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

#ifndef LIBPANDAFILE_CODE_DATA_ACCESSOR_H_
#define LIBPANDAFILE_CODE_DATA_ACCESSOR_H_

#include "libarkfile/file-inl.h"

namespace ark::panda_file {

class CodeDataAccessor {
public:
    class TryBlock {
    public:
        PANDA_PUBLIC_API explicit TryBlock(Span<const uint8_t> data);

        ~TryBlock() = default;

        NO_COPY_SEMANTIC(TryBlock);
        NO_MOVE_SEMANTIC(TryBlock);

        uint32_t GetStartPc() const
        {
            return startPc_;
        }

        uint32_t GetLength() const
        {
            return length_;
        }

        uint32_t GetNumCatches() const
        {
            return numCatches_;
        }

        template <class Callback>
        void EnumerateCatchBlocks(const Callback &cb);

        size_t GetSize()
        {
            if (size_ == 0) {
                SkipCatchBlocks();
            }

            return size_;
        }

    private:
        void SkipCatchBlocks();

        Span<const uint8_t> data_;

        uint32_t startPc_;
        uint32_t length_;
        uint32_t numCatches_;
        Span<const uint8_t> catchBlocksSp_ {nullptr, nullptr};

        size_t size_ {0};
    };

    class CatchBlock {
    public:
        PANDA_PUBLIC_API explicit CatchBlock(Span<const uint8_t> data);

        ~CatchBlock() = default;

        NO_COPY_SEMANTIC(CatchBlock);
        NO_MOVE_SEMANTIC(CatchBlock);

        uint32_t GetTypeIdx() const
        {
            return typeIdx_;
        }

        uint32_t GetHandlerPc() const
        {
            return handlerPc_;
        }

        uint32_t GetCodeSize() const
        {
            return codeSize_;
        }

        size_t GetSize() const
        {
            return size_;
        }

    private:
        uint32_t typeIdx_;
        uint32_t handlerPc_;
        uint32_t codeSize_;

        size_t size_;
    };

    PANDA_PUBLIC_API CodeDataAccessor(const File &pandaFile, File::EntityId codeId);

    ~CodeDataAccessor() = default;

    NO_COPY_SEMANTIC(CodeDataAccessor);
    NO_MOVE_SEMANTIC(CodeDataAccessor);

    static uint32_t GetNumVregs(const File &pf, File::EntityId codeId);

    static const uint8_t *GetInstructions(const File &pf, File::EntityId codeId, uint32_t *vregs);

    static const uint8_t *GetInstructions(const File &pf, File::EntityId codeId);

    uint32_t GetNumVregs() const
    {
        return numVregs_;
    }

    uint32_t GetNumArgs() const
    {
        return numArgs_;
    }

    uint32_t GetCodeSize() const
    {
        return codeSize_;
    }

    uint32_t GetTriesSize() const
    {
        return triesSize_;
    }

    const uint8_t *GetInstructions() const
    {
        return instructionsPtr_;
    }

    template <class Callback>
    void EnumerateTryBlocks(const Callback &cb);

    size_t GetSize()
    {
        if (size_ == 0) {
            SkipTryBlocks();
        }

        return size_;
    }

    const File &GetPandaFile() const
    {
        return pandaFile_;
    }

    File::EntityId GetCodeId()
    {
        return codeId_;
    }

private:
    void SkipTryBlocks();

    const File &pandaFile_;
    File::EntityId codeId_;

    uint32_t numVregs_;
    uint32_t numArgs_;
    uint32_t codeSize_;
    uint32_t triesSize_;
    const uint8_t *instructionsPtr_;
    Span<const uint8_t> tryBlocksSp_ {nullptr, nullptr};

    template <class Callback>
    void EnumerateTryBlocksImpl(const Callback &cb);

    size_t size_ {0};
};

}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_CODE_DATA_ACCESSOR_H_
