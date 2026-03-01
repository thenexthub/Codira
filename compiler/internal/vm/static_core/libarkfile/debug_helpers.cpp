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

#include "libarkfile/debug_helpers.h"

namespace ark::panda_file::debug_helpers {

size_t GetLineNumber(ark::panda_file::MethodDataAccessor mda, uint32_t bcOffset,
                     const ark::panda_file::File *pandaDebugFile)
{
    auto debugInfoId = mda.GetDebugInfoId();
    if (!debugInfoId) {
        return -1;
    }

    ark::panda_file::DebugInfoDataAccessor dda(*pandaDebugFile, debugInfoId.value());
    const uint8_t *program = dda.GetLineNumberProgram();

    ark::panda_file::LineProgramState state(*pandaDebugFile, ark::panda_file::File::EntityId(0), dda.GetLineStart(),
                                            dda.GetConstantPool());

    BytecodeOffsetResolver resolver(&state, bcOffset);
    ark::panda_file::LineNumberProgramProcessor<BytecodeOffsetResolver> programProcessor(program, &resolver);
    programProcessor.Process();

    return resolver.GetLine();
}

const char *GetStringFromConstantPool(const File &pf, uint32_t offset)
{
    auto id = File::EntityId(offset);
    ASSERT(id.IsValid());
    return utf::Mutf8AsCString(pf.GetStringData(id).data);
}

}  // namespace ark::panda_file::debug_helpers
