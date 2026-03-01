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

#ifndef LIBPANDAFILE_CODE_DATA_ACCESSOR_INL_H
#define LIBPANDAFILE_CODE_DATA_ACCESSOR_INL_H

#include "code_data_accessor.h"

namespace panda::panda_file {

template <class Callback>
inline void CodeDataAccessor::TryBlock::EnumerateCatchBlocks(const Callback &cb)
{
    auto sp = catch_blocks_sp_;
    for (size_t i = 0; i < num_catches_; i++) {
        CatchBlock catch_block(sp);
        if (!cb(catch_block)) {
            return;
        }
        sp = sp.SubSpan(catch_block.GetSize());
    }
    size_ = sp.data() - data_.data();
}

inline void CodeDataAccessor::TryBlock::SkipCatchBlocks()
{
    EnumerateCatchBlocks([](const CatchBlock & /* unused */) { return true; });
}

template <class Callback>
inline void CodeDataAccessor::EnumerateTryBlocks(const Callback &cb)
{
    auto sp = try_blocks_sp_;
    for (size_t i = 0; i < tries_size_; i++) {
        TryBlock try_block(sp);
        if (!cb(try_block)) {
            return;
        }
        sp = sp.SubSpan(try_block.GetSize());
    }
    size_ = panda_file_.GetIdFromPointer(sp.data()).GetOffset() - code_id_.GetOffset();
}

inline void CodeDataAccessor::SkipTryBlocks()
{
    EnumerateTryBlocks([](const TryBlock & /* unused */) { return true; });
}

// static
inline uint32_t CodeDataAccessor::GetNumVregs(const File &pf, File::EntityId code_id)
{
    uint32_t num_vregs_;
    auto sp = pf.GetSpanFromId(code_id);
    num_vregs_ = helpers::ReadULeb128(&sp);
    return num_vregs_;
}

// static
inline const uint8_t *CodeDataAccessor::GetInstructions(const File &pf, File::EntityId code_id, uint32_t *vregs)
{
    auto sp = pf.GetSpanFromId(code_id);
    *vregs = helpers::ReadULeb128(&sp);
    helpers::SkipULeb128(&sp);  // num_args
    helpers::SkipULeb128(&sp);  // code_size
    helpers::SkipULeb128(&sp);  // tries_size
    return sp.data();
}

// static
inline const uint8_t *CodeDataAccessor::GetInstructions(const File &pf, File::EntityId code_id)
{
    auto sp = pf.GetSpanFromId(code_id);
    helpers::SkipULeb128(&sp);  // num_vregs
    helpers::SkipULeb128(&sp);  // num_args
    helpers::SkipULeb128(&sp);  // code_size
    helpers::SkipULeb128(&sp);  // tries_size
    return sp.data();
}

}  // namespace panda::panda_file

#endif  // LIBPANDAFILE_CODE_DATA_ACCESSOR_INL_H
