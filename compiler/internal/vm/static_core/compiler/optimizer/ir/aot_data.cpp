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

#include "graph.h"
#include "code_info/code_info.h"

namespace ark::compiler {
intptr_t AotData::GetEpTableOffset() const
{
    return -(static_cast<size_t>(RuntimeInterface::IntrinsicId::COUNT) * PointerSize(graph_->GetArch()));
}

intptr_t AotData::GetSharedSlowPathOffset(RuntimeInterface::EntrypointId id, uintptr_t pc) const
{
    auto offset = slowPathData_->GetSharedSlowPathOffset(id);
    if (offset == 0) {
        return 0;
    }
    return offset - (codeAddress_ + pc + CodeInfo::GetCodeOffset(graph_->GetArch()));
}

void AotData::SetSharedSlowPathOffset(RuntimeInterface::EntrypointId id, uintptr_t pc)
{
    slowPathData_->SetSharedSlowPathOffset(id, codeAddress_ + pc + CodeInfo::GetCodeOffset(graph_->GetArch()));
}

intptr_t AotData::GetEntrypointOffset(uint64_t pc, int32_t slotId) const
{
    // Initialize offset by offset to the origin of the entrypoint table
    intptr_t offset = GetEpTableOffset();
    // Increment/decrement offset to specified slot
    offset += slotId * static_cast<intptr_t>(PointerSize(graph_->GetArch()));
    // Decrement by sum of method code start address and current pc
    offset -= static_cast<intptr_t>(codeAddress_ + pc);
    // Decrement by header size that prepend method code
    offset -= static_cast<intptr_t>(CodeInfo::GetCodeOffset(graph_->GetArch()));
    return offset;
}

intptr_t AotData::GetPltSlotOffset(uint64_t pc, uint32_t methodId, uint32_t fileIndex)
{
    int32_t slotId = GetPltSlotId(methodId, fileIndex);
    return GetEntrypointOffset(pc, slotId);
}

int32_t AotData::GetPltSlotId(uint32_t methodId, uint32_t fileIndex)
{
    int32_t slotId = FindOrInsertSlotId(gotPlt_, methodId, fileIndex);
    return slotId;
}

intptr_t AotData::GetVirtIndexSlotOffset(uint64_t pc, uint32_t methodId, uint32_t fileIndex)
{
    int32_t slotId = FindOrInsertSlotId(gotVirtIndexes_, methodId, fileIndex);
    return GetEntrypointOffset(pc, slotId);
}

intptr_t AotData::GetClassSlotOffset(uint64_t pc, uint32_t klassId, bool init, uint32_t fileIndex)
{
    int32_t slotId = GetClassSlotId(klassId, fileIndex);
    return GetEntrypointOffset(pc, init ? slotId - 1 : slotId);
}

intptr_t AotData::GetStringSlotOffset(uint64_t pc, uint32_t stringId, uint32_t fileIndex)
{
    int32_t slotId = GetStringSlotId(stringId, fileIndex);
    return GetEntrypointOffset(pc, slotId);
}

int32_t AotData::GetClassSlotId(uint32_t klassId, uint32_t fileIndex)
{
    int32_t slotId = FindOrInsertSlotId(gotClass_, klassId, fileIndex);
    return slotId;
}

int32_t AotData::GetStringSlotId(uint32_t stringId, uint32_t fileIndex)
{
    int32_t slotId = FindOrInsertSlotId(gotString_, stringId, fileIndex);
    return slotId;
}

intptr_t AotData::GetCommonSlotOffset(uint64_t pc, uint32_t id)
{
    int32_t slotId;
    auto slot = gotCommon_->find({pfile_, id});
    if (slot != gotCommon_->end()) {
        slotId = slot->second;
    } else {
        slotId = GetSlotId();
        gotCommon_->insert({{pfile_, id}, slotId});
    }
    return GetEntrypointOffset(pc, slotId);
}

uint64_t AotData::GetInfInlineCacheSlotOffset(uint64_t pc, uint64_t cacheIdx)
{
    int32_t slotId = GetIntfInlineCacheSlotId(cacheIdx);
    return GetEntrypointOffset(pc, slotId);
}

int32_t AotData::GetIntfInlineCacheSlotId(uint64_t cacheIdx)
{
    int32_t slotId;
    auto slot = gotIntfInlineCache_->find({pfile_, cacheIdx});
    if (slot != gotIntfInlineCache_->end()) {
        slotId = slot->second;
    } else {
        slotId = GetSlotId();
        gotIntfInlineCache_->insert({{pfile_, cacheIdx}, slotId});
    }
    return slotId;
}

int32_t AotData::GetSlotId() const
{
    constexpr auto IMM_3 = 3;
    constexpr auto IMM_2 = 2;
    return -1 - IMM_3 * (gotPlt_->size() + gotClass_->size()) - IMM_2 * (gotVirtIndexes_->size() + gotString_->size()) -
           gotIntfInlineCache_->size() - gotCommon_->size();
}

std::pair<const panda_file::File *, bool> AotData::GetPandaFileByIndex(uint32_t fileIndex) const
{
    if (fileIndex == panda_file::File::INVALID_FILE_INDEX) {
        return {pfile_, false};
    }
    auto pfile =
        reinterpret_cast<const panda_file::File *>(graph_->GetRuntime()->GetAOTBinaryFileBySnapshotIndex(fileIndex));
    auto isExternal = pfile != pfile_;
    return {pfile, isExternal};
}

uint32_t AotData::FindOrInsertSlotId(
    std::map<std::pair<const panda_file::File *, uint32_t>, std::pair<int32_t, bool>> *gotTable, uint32_t id,
    uint32_t fileIndex)
{
    int32_t slotId;
    auto [pfile, isExternal] = GetPandaFileByIndex(fileIndex);
    auto slot = gotTable->find({pfile, id});
    if (slot != gotTable->end()) {
        slotId = slot->second.first;
        if (isExternal && !slot->second.second) {
            slot->second.second = true;
        }
    } else {
        slotId = GetSlotId();
        gotTable->insert({{pfile, id}, {slotId, isExternal}});
    }
    return slotId;
}

}  // namespace ark::compiler
