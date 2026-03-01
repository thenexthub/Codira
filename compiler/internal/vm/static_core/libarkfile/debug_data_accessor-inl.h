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

#ifndef LIBPANDAFILE_DEBUG_DATA_ACCESSOR_INL_H_
#define LIBPANDAFILE_DEBUG_DATA_ACCESSOR_INL_H_

#include "libarkfile/debug_data_accessor.h"
#include "libarkfile/helpers.h"

namespace ark::panda_file {

template <class Callback>
inline void DebugInfoDataAccessor::EnumerateParameters(const Callback &cb)
{
    auto sp = parametersSp_;

    for (size_t i = 0; i < numParams_; i++) {
        File::EntityId id(helpers::ReadULeb128(&sp));
        cb(id);
    }

    constantPoolSizeSp_ = sp;
}

inline void DebugInfoDataAccessor::SkipParameters()
{
    EnumerateParameters([](File::EntityId /* unused */) {});
}

inline Span<const uint8_t> DebugInfoDataAccessor::GetConstantPool()
{
    if (constantPoolSizeSp_.data() == nullptr) {
        SkipParameters();
    }

    auto sp = constantPoolSizeSp_;

    uint32_t size = helpers::ReadULeb128(&sp);
    lineNumProgramOffSp_ = sp.SubSpan(size);

    return sp.First(size);
}

inline void DebugInfoDataAccessor::SkipConstantPool()
{
    GetConstantPool();
}

inline const uint8_t *DebugInfoDataAccessor::GetLineNumberProgram()
{
    if (lineNumProgramOffSp_.data() == nullptr) {
        SkipConstantPool();
    }

    auto sp = lineNumProgramOffSp_;
    uint32_t index = helpers::ReadULeb128(&sp);
    auto lineNumProgramId = pandaFile_.ResolveLineNumberProgramIndex(index);

    size_ = pandaFile_.GetIdFromPointer(sp.data()).GetOffset() - debugInfoId_.GetOffset();

    return pandaFile_.GetSpanFromId(lineNumProgramId).data();
}

inline void DebugInfoDataAccessor::SkipLineNumberProgram()
{
    GetLineNumberProgram();
}

}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_DEBUG_DATA_ACCESSOR_INL_H_
