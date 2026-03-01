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

#include "libarkfile/code_data_accessor.h"
#include <algorithm>

namespace ark::panda_file {

CodeDataAccessor::CatchBlock::CatchBlock(Span<const uint8_t> data)
{
    auto sp = data;
    typeIdx_ = helpers::ReadULeb128(&sp) - 1;
    handlerPc_ = helpers::ReadULeb128(&sp);
    codeSize_ = helpers::ReadULeb128(&sp);
    size_ = sp.data() - data.data();
}

CodeDataAccessor::TryBlock::TryBlock(Span<const uint8_t> data) : data_(data)
{
    startPc_ = helpers::ReadULeb128(&data);
    length_ = helpers::ReadULeb128(&data);
    numCatches_ = helpers::ReadULeb128(&data);
    catchBlocksSp_ = data;
}

CodeDataAccessor::CodeDataAccessor(const File &pandaFile, File::EntityId codeId)
    : pandaFile_(pandaFile), codeId_(codeId)
{
    auto sp = pandaFile_.GetSpanFromId(codeId_);

    numVregs_ = helpers::ReadULeb128(&sp);
    numArgs_ = helpers::ReadULeb128(&sp);
    codeSize_ = helpers::ReadULeb128(&sp);
    triesSize_ = helpers::ReadULeb128(&sp);
    instructionsPtr_ = sp.data();
    sp = sp.SubSpan(codeSize_);
    tryBlocksSp_ = sp;
}

// static
const uint8_t *CodeDataAccessor::GetInstructions(const File &pf, File::EntityId codeId)
{
    auto sp = pf.GetSpanFromId(codeId);
    uint32_t dataPrefix;
    // with reading *reinterpret_cast<const uint32_t *>(sp.Data()) unaligned read occurs
    // according to decompiler memcpy is optimized to a single load
    std::copy_n(sp.Data(), 4U, reinterpret_cast<uint8_t *>(&dataPrefix));
    if (UNLIKELY(dataPrefix & 0x80808080)) {
        helpers::SkipULeb128(&sp);  // num_vregs
        helpers::SkipULeb128(&sp);  // num_args
        helpers::SkipULeb128(&sp);  // code_size
        helpers::SkipULeb128(&sp);  // tries_size
        return sp.data();
    }

    return sp.SubSpan(4U).data();
}

}  // namespace ark::panda_file
