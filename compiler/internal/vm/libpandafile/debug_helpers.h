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

#ifndef LIBPANDAFILE_DEBUG_HELPERS_H
#define LIBPANDAFILE_DEBUG_HELPERS_H

#include "debug_data_accessor-inl.h"
#include "file.h"
#include "file_items.h"
#include "method_data_accessor-inl.h"
#include "line_number_program.h"
#include "libpandabase/utils/span.h"

namespace panda::panda_file::debug_helpers {

class BytecodeOffsetResolver {
public:
    BytecodeOffsetResolver(panda_file::LineProgramState *state, uint32_t bc_offset)
        : state_(state), bc_offset_(bc_offset), prev_line_(state->GetLine()), line_(0)
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

    bool HandleAdvanceLine(int32_t line_diff) const
    {
        state_->AdvanceLine(line_diff);
        return true;
    }

    bool HandleAdvancePc(uint32_t pc_diff) const
    {
        state_->AdvancePc(pc_diff);
        return true;
    }

    bool HandleSetFile([[maybe_unused]] uint32_t source_file_id) const
    {
        return true;
    }

    bool HandleSetSourceCode([[maybe_unused]] uint32_t source_code_id) const
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

    bool HandleStartLocal([[maybe_unused]] int32_t reg_number, [[maybe_unused]] uint32_t name_id,
                          [[maybe_unused]] uint32_t type_id) const
    {
        return true;
    }

    bool HandleStartLocalExtended([[maybe_unused]] int32_t reg_number, [[maybe_unused]] uint32_t name_id,
                                  [[maybe_unused]] uint32_t type_id, [[maybe_unused]] uint32_t type_signature_id) const
    {
        return true;
    }

    bool HandleEndLocal([[maybe_unused]] int32_t reg_number) const
    {
        return true;
    }

    bool HandleSetColumn([[maybe_unused]] int32_t column_number) const
    {
        return true;
    }

    bool HandleSpecialOpcode(uint32_t pc_offset, int32_t line_offset)
    {
        state_->AdvancePc(pc_offset);
        state_->AdvanceLine(line_offset);

        if (state_->GetAddress() == bc_offset_) {
            line_ = state_->GetLine();
            return false;
        }

        if (state_->GetAddress() > bc_offset_) {
            line_ = prev_line_;
            return false;
        }

        prev_line_ = state_->GetLine();

        return true;
    }

private:
    panda_file::LineProgramState *state_;
    uint32_t bc_offset_;
    uint32_t prev_line_;
    uint32_t line_;
};

inline size_t GetLineNumber(panda::panda_file::MethodDataAccessor mda, uint32_t bc_offset,
                            const panda::panda_file::File *panda_debug_file)
{
    auto debug_info_id = mda.GetDebugInfoId();
    if (!debug_info_id) {
        return -1;
    }

    panda::panda_file::DebugInfoDataAccessor dda(*panda_debug_file, debug_info_id.value());
    const uint8_t *program = dda.GetLineNumberProgram();

    panda::panda_file::LineProgramState state(*panda_debug_file, panda::panda_file::File::EntityId(0),
                                              dda.GetLineStart(), dda.GetConstantPool());

    BytecodeOffsetResolver resolver(&state, bc_offset);
    panda::panda_file::LineNumberProgramProcessor<BytecodeOffsetResolver> program_processor(program, &resolver);
    program_processor.Process();

    return resolver.GetLine();
}

}  // namespace panda::panda_file::debug_helpers

#endif  // LIBPANDAFILE_DEBUG_HELPERS_H
