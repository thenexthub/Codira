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

#include "code_info.h"
#include "libarkbase/utils/bit_memory_region-inl.h"

namespace ark::compiler {

void CodeInfo::Dump(std::ostream &stream) const
{
    stream << "CodeInfo: vregs_num=" << GetHeader().GetVRegsCount() << ", frame_size=" << GetHeader().GetFrameSize()
           << std::endl;
    EnumerateTables([this, &stream](size_t index, auto member) {
        if (HasTable(index)) {
            const auto &table = this->*member;
            table.Dump(stream);
        }
    });
}

void CodeInfo::Dump(std::ostream &stream, const StackMap &stackMap, Arch arch) const
{
    stream << "Stackmap #" << stackMap.GetRow() << ": npc=0x" << std::hex << stackMap.GetNativePcUnpacked(arch)
           << ", bpc=0x" << std::hex << stackMap.GetBytecodePc();
    if (stackMap.HasInlineInfoIndex()) {
        stream << ", inline_depth=" << (GetInlineDepth(stackMap) + 1);
    }
    if (stackMap.HasRootsRegMaskIndex() || stackMap.HasRootsStackMaskIndex()) {
        stream << ", roots=[";
        const char *sep = "";
        if (stackMap.HasRootsRegMaskIndex()) {
            stream << "r:0x" << std::hex << GetRootsRegMask(stackMap);
            sep = ",";
        }
        if (stackMap.HasRootsStackMaskIndex()) {
            auto region = GetRootsStackMask(stackMap);
            stream << sep << "s:" << region;
        }
        stream << "]";
    }
    if (stackMap.HasVRegMaskIndex()) {
        stream << ", vregs=" << GetVRegMask(stackMap);
    }
}

void CodeInfo::DumpInlineInfo(std::ostream &stream, const StackMap &stackMap, int depth) const
{
    auto ii = GetInlineInfo(stackMap, depth);
    stream << "InlineInfo #" << depth << ": bpc=0x" << std::hex << ii.GetBytecodePc() << std::dec
           << ", vregs_num: " << ii.GetVRegsCount();
}

}  // namespace ark::compiler
