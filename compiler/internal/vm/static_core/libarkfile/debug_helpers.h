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

#ifndef PANDA_FILE_DEBUG_HELPERS_
#define PANDA_FILE_DEBUG_HELPERS_

#include "libarkfile/debug_data_accessor-inl.h"
#include "libarkfile/file.h"
#include "libarkfile/file_items.h"
#include "libarkfile/method_data_accessor-inl.h"
#include "libarkfile/line_number_program.h"
#include "libarkbase/utils/span.h"

namespace ark::panda_file::debug_helpers {

class BytecodeOffsetResolver {
public:
    BytecodeOffsetResolver(panda_file::LineProgramState *state, uint32_t bcOffset)
        : state_(state), bcOffset_(bcOffset), prevLine_(state->GetLine())
    {
    }

    ~BytecodeOffsetResolver() = default;

    DEFAULT_MOVE_SEMANTIC(BytecodeOffsetResolver);
    DEFAULT_COPY_SEMANTIC(BytecodeOffsetResolver);

    panda_file::LineProgramState *GetState() const
    {
        return state_;
    }

    uint32_t GetLine() const
    {
        return line_;
    }

    void ProcessBegin() const {}

    void ProcessEnd()
    {
        if (line_ == 0) {
            line_ = state_->GetLine();
        }
    }

    bool HandleAdvanceLine(int32_t lineDiff) const
    {
        state_->AdvanceLine(lineDiff);
        return true;
    }

    bool HandleAdvancePc(uint32_t pcDiff) const
    {
        state_->AdvancePc(pcDiff);
        return true;
    }

    bool HandleSetFile([[maybe_unused]] uint32_t sourceFileId) const
    {
        return true;
    }

    bool HandleSetSourceCode([[maybe_unused]] uint32_t sourceCodeId) const
    {
        return true;
    }

    bool HandleSetPrologueEnd() const
    {
        return true;
    }

    bool HandleSetEpilogueBegin() const
    {
        return true;
    }

    bool HandleStartLocal([[maybe_unused]] int32_t regNumber, [[maybe_unused]] uint32_t nameId,
                          [[maybe_unused]] uint32_t typeId) const
    {
        return true;
    }

    bool HandleStartLocalExtended([[maybe_unused]] int32_t regNumber, [[maybe_unused]] uint32_t nameId,
                                  [[maybe_unused]] uint32_t typeId, [[maybe_unused]] uint32_t typeSignatureId) const
    {
        return true;
    }

    bool HandleEndLocal([[maybe_unused]] int32_t regNumber) const
    {
        return true;
    }

    bool HandleRestartLocal([[maybe_unused]] int32_t regNumber) const
    {
        return true;
    }

    bool HandleSetColumn([[maybe_unused]] int32_t columnNumber) const
    {
        return true;
    }

    bool HandleSpecialOpcode(uint32_t pcOffset, int32_t lineOffset)
    {
        state_->AdvancePc(pcOffset);
        state_->AdvanceLine(lineOffset);

        if (state_->GetAddress() == bcOffset_) {
            line_ = state_->GetLine();
            return false;
        }

        if (state_->GetAddress() > bcOffset_) {
            line_ = prevLine_;
            return false;
        }

        prevLine_ = state_->GetLine();

        return true;
    }

private:
    panda_file::LineProgramState *state_;
    uint32_t bcOffset_;
    uint32_t prevLine_;
    uint32_t line_ {0};
};

size_t GetLineNumber(ark::panda_file::MethodDataAccessor mda, uint32_t bcOffset,
                     const ark::panda_file::File *pandaDebugFile);

const char *GetStringFromConstantPool(const File &pf, uint32_t offset);

}  // namespace ark::panda_file::debug_helpers

#endif  // PANDA_FILE_DEBUG_HELPERS_
