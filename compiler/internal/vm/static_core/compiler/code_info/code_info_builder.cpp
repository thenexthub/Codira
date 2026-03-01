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

#include "code_info_builder.h"
#include "libarkbase/utils/bit_memory_region-inl.h"
#include "optimizer/ir/inst.h"

namespace ark::compiler {

void CodeInfoBuilder::BeginMethod(uint32_t frameSize, uint32_t vregsCount)
{
#ifndef NDEBUG
    ASSERT(!wasMethodBegin_);
    ASSERT(!wasStackMapBegin_);
    ASSERT(!wasInlineInfoBegin_);
    wasMethodBegin_ = true;
#endif

    SetFrameSize(frameSize);
    vregsCount_ = vregsCount;
    constantTable_.Add({0});
}

void CodeInfoBuilder::EndMethod()
{
#ifndef NDEBUG
    ASSERT(wasMethodBegin_);
    ASSERT(!wasStackMapBegin_);
    ASSERT(!wasInlineInfoBegin_);
    wasMethodBegin_ = false;
#endif
}

void CodeInfoBuilder::BeginStackMap(uint32_t bpc, uint32_t npc, SaveStateInst *ss, bool requireVregMap)
{
#ifndef NDEBUG
    ASSERT(wasMethodBegin_);
    ASSERT(!wasStackMapBegin_);
    ASSERT(!wasInlineInfoBegin_);
    wasStackMapBegin_ = true;
#endif
    ArenaBitVector *stackRoots = ss->GetRootsStackMask();
    uint32_t regsRoots = ss->GetRootsRegsMask().to_ulong();
    bool isOsr = ss->GetOpcode() == Opcode::SaveStateOsr;

    inlineInfoStack_.clear();
    currentVregs_.clear();

    ASSERT(stackMaps_.GetSize() == 0 || npc >= stackMaps_.GetLast()[StackMap::COLUMN_NATIVE_PC]);

    currentVregsCount_ = requireVregMap ? vregsCount_ : 0;

    currentStackMap_ = BitTableBuilder<StackMap>::Entry();
    currentStackMap_[StackMap::COLUMN_PROPERTIES] = StackMap::CreateProperties(isOsr, requireVregMap);
    currentStackMap_[StackMap::COLUMN_BYTECODE_PC] = bpc;
    currentStackMap_[StackMap::COLUMN_NATIVE_PC] = StackMap::PackAddress(npc, arch_);
    if (regsRoots != 0) {
        currentStackMap_[StackMap::COLUMN_ROOTS_REG_MASK_INDEX] = rootsRegMasks_.Add({regsRoots});
    }
    if (stackRoots != nullptr && !stackRoots->empty()) {
        currentStackMap_[StackMap::COLUMN_ROOTS_STACK_MASK_INDEX] = rootsStackMasks_.Add(stackRoots->GetFixed());
    }
    // Ensure that stackmaps are inserted in sorted order
    if (stackMaps_.GetRowsCount() != 0) {
        ASSERT(currentStackMap_[StackMap::COLUMN_NATIVE_PC] >= stackMaps_.GetLast()[StackMap::COLUMN_NATIVE_PC]);
    }
}

void CodeInfoBuilder::EndStackMap()
{
#ifndef NDEBUG
    ASSERT(wasMethodBegin_);
    ASSERT(wasStackMapBegin_);
    ASSERT(!wasInlineInfoBegin_);
    wasStackMapBegin_ = false;
#endif
    if (!inlineInfoStack_.empty()) {
        inlineInfoStack_.back()[InlineInfo::COLUMN_IS_LAST] = static_cast<uint32_t>(true);
        currentStackMap_[StackMap::COLUMN_INLINE_INFO_INDEX] = inlineInfos_.AddArray(Span(inlineInfoStack_));
    }

    EmitVRegs();

    stackMaps_.Add(currentStackMap_);
}

void CodeInfoBuilder::DumpCurrentStackMap(std::ostream &stream) const
{
    stream << "Stackmap #" << (stackMaps_.GetRowsCount() - 1) << ": npc=0x" << std::hex
           << StackMap::UnpackAddress(currentStackMap_[StackMap::COLUMN_NATIVE_PC], arch_) << ", bpc=0x" << std::hex
           << currentStackMap_[StackMap::COLUMN_BYTECODE_PC];
    if (currentStackMap_[StackMap::COLUMN_INLINE_INFO_INDEX] != StackMap::NO_VALUE) {
        stream << ", inline_depth=" << inlineInfoStack_.size();
    }
    if (currentStackMap_[StackMap::COLUMN_ROOTS_REG_MASK_INDEX] != StackMap::NO_VALUE ||
        currentStackMap_[StackMap::COLUMN_ROOTS_STACK_MASK_INDEX] != StackMap::NO_VALUE) {
        stream << ", roots=[";
        const char *sep = "";
        if (currentStackMap_[StackMap::COLUMN_ROOTS_REG_MASK_INDEX] != StackMap::NO_VALUE) {
            auto &entry = rootsRegMasks_.GetEntry(currentStackMap_[StackMap::COLUMN_ROOTS_REG_MASK_INDEX]);
            stream << "r:0x" << std::hex << entry[RegisterMask::COLUMN_MASK];
            sep = ",";
        }
        if (currentStackMap_[StackMap::COLUMN_ROOTS_STACK_MASK_INDEX] != StackMap::NO_VALUE) {
            auto region = rootsStackMasks_.GetEntry(currentStackMap_[StackMap::COLUMN_ROOTS_STACK_MASK_INDEX]);
            stream << sep << "s:" << region;
        }
        stream << "]";
    }
    if (currentStackMap_[StackMap::COLUMN_VREG_MASK_INDEX] != StackMap::NO_VALUE) {
        stream << ", vregs=" << vregMasks_.GetEntry(currentStackMap_[StackMap::COLUMN_VREG_MASK_INDEX]);
    }
}

void CodeInfoBuilder::BeginInlineInfo(void *method, uint32_t methodId, uint32_t bpc, uint32_t vregsCount,
                                      uint32_t pandaFileIndex)
{
#ifndef NDEBUG
    ASSERT(wasMethodBegin_);
    ASSERT(wasStackMapBegin_);
    wasInlineInfoBegin_ = true;
#endif
    BitTableBuilder<InlineInfo>::Entry inlineInfo;
    currentVregsCount_ += vregsCount;

    inlineInfo[InlineInfo::COLUMN_IS_LAST] = static_cast<uint32_t>(false);
    inlineInfo[InlineInfo::COLUMN_FILE_INDEX] = pandaFileIndex;
    inlineInfo[InlineInfo::COLUMN_BYTECODE_PC] = bpc;
    inlineInfo[InlineInfo::COLUMN_VREGS_COUNT] = currentVregsCount_;
    if (method != nullptr) {
        inlineInfo[InlineInfo::COLUMN_METHOD_HI] = High32Bits(method);
        inlineInfo[InlineInfo::COLUMN_METHOD_LOW] = Low32Bits(method);
    } else {
        ASSERT(methodId != 0);
        inlineInfo[InlineInfo::COLUMN_METHOD_ID_INDEX] = methodIds_.Add({methodId});
    }

    inlineInfoStack_.push_back(inlineInfo);
}

void CodeInfoBuilder::EndInlineInfo()
{
#ifndef NDEBUG
    ASSERT(wasMethodBegin_);
    ASSERT(wasStackMapBegin_);
    ASSERT(wasInlineInfoBegin_);
    wasInlineInfoBegin_ = false;
#endif
    ASSERT(currentVregs_.size() == currentVregsCount_);
}

void CodeInfoBuilder::AddConstant(uint64_t value, VRegInfo::Type type, VRegInfo::VRegType vregType)
{
    VRegInfo vreg(0, VRegInfo::Location::CONSTANT, type, vregType);
    uint32_t low = value & ((1LLU << BITS_PER_UINT32) - 1);
    uint32_t hi = (value >> BITS_PER_UINT32) & ((1LLU << BITS_PER_UINT32) - 1);
    vreg.SetConstantIndices(constantTable_.Add({low}), constantTable_.Add({hi}));
    currentVregs_.push_back(vreg);
}

void CodeInfoBuilder::EmitVRegs()
{
    ASSERT(currentVregs_.size() == currentVregsCount_);
    if (currentVregs_.empty()) {
        return;
    }

    if (currentVregs_.size() > lastVregs_.size()) {
        lastVregs_.resize(currentVregs_.size(), VRegInfo::Invalid());
        vregsLastChange_.resize(currentVregs_.size());
    }

    ArenaVector<BitTableBuilder<VRegisterCatalogueIndex>::Entry> &vregsMap = vregsMapStorage_;
    ArenaBitVector &vregsMask = vregsMaskStorage_;
    vregsMap.clear();
    vregsMask.clear();

    for (size_t i = 0; i < currentVregs_.size(); i++) {
        auto &vreg = currentVregs_[i];
        uint32_t distatnce = stackMaps_.GetRowsCount() - vregsLastChange_[i];
        if (lastVregs_[i] != vreg || distatnce > MAX_VREG_LIVE_DISTANCE) {
            BitTableBuilder<VRegisterInfo>::Entry vregEntry;
            vregEntry[VRegisterInfo::COLUMN_INFO] = vreg.GetInfo();
            vregEntry[VRegisterInfo::COLUMN_VALUE] = vreg.GetValue();
            uint32_t index = vreg.IsLive() ? vregsCatalogue_.Add(vregEntry) : decltype(vregsCatalogue_)::NO_VALUE;
            vregsMap.push_back({index});
            vregsMask.SetBit(i);
            lastVregs_[i] = vreg;
            vregsLastChange_[i] = stackMaps_.GetRowsCount();
        }
    }

    BitMemoryRegion rgn(vregsMask.data(), vregsMask.size());
    ASSERT(vregsMask.PopCount() == vregsMap.size());
    if (vregsMask.PopCount() != 0) {
        currentStackMap_[StackMap::COLUMN_VREG_MASK_INDEX] = vregMasks_.Add(vregsMask.GetFixed());
    }
    if (!currentVregs_.empty()) {
        currentStackMap_[StackMap::COLUMN_VREG_MAP_INDEX] = vregsMap_.AddArray(Span(vregsMap));
    }
}

void CodeInfoBuilder::Encode(ArenaVector<uint8_t> *stream, size_t offset)
{
    BitMemoryStreamOut out(stream, offset);

    uint32_t tablesMask = 0;
    EnumerateTables([&tablesMask](size_t index, const auto &table) {
        if (table->GetRowsCount() != 0) {
            tablesMask |= (1U << index);
        }
    });

    header_.SetTableMask(tablesMask);
    header_.SetVRegsCount(vregsCount_);
    header_.Encode(out);

    EnumerateTables([&out]([[maybe_unused]] size_t index, const auto &table) {
        if (table->GetRowsCount() != 0) {
            table->Encode(out);
        }
    });
    stream->resize(RoundUp(stream->size(), CodeInfo::SIZE_ALIGNMENT));
}

}  // namespace ark::compiler
