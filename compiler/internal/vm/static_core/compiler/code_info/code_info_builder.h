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

#ifndef PANDA_CODE_INFO_BUILDER_H
#define PANDA_CODE_INFO_BUILDER_H

#include "code_info.h"
#include "libarkbase/utils/arena_containers.h"
#include "libarkbase/utils/bit_vector.h"

namespace ark::compiler {

class SaveStateInst;

class CodeInfoBuilder {
public:
    CodeInfoBuilder(Arch arch, ArenaAllocator *allocator)
        : arch_(arch),
          stackMaps_(allocator),
          inlineInfos_(allocator),
          rootsRegMasks_(allocator),
          rootsStackMasks_(allocator),
          methodIds_(allocator),
          vregsCatalogue_(allocator),
          vregsMap_(allocator),
          vregMasks_(allocator),
          implicitNullchecks_(allocator),
          constantTable_(allocator),
          currentVregs_(allocator->Adapter()),
          lastVregs_(allocator->Adapter()),
          vregsLastChange_(allocator->Adapter()),
          inlineInfoStack_(allocator->Adapter()),
          vregsMapStorage_(allocator->Adapter()),
          vregsMaskStorage_(allocator)
    {
    }

    NO_COPY_SEMANTIC(CodeInfoBuilder);
    NO_MOVE_SEMANTIC(CodeInfoBuilder);
    ~CodeInfoBuilder() = default;

    void BeginMethod(uint32_t frameSize, uint32_t vregsCount);

    void EndMethod();

    void BeginStackMap(uint32_t bpc, uint32_t npc, SaveStateInst *ss, bool requireVregMap);

    void EndStackMap();

    void BeginInlineInfo(void *method, uint32_t methodId, uint32_t bpc, uint32_t vregsCount,
                         uint32_t pandaFileIndex = 0);

    void EndInlineInfo();

    void AddVReg(VRegInfo reg)
    {
        // Constant should be added via `AddConstant` method
        ASSERT(reg.GetLocation() != VRegInfo::Location::CONSTANT);
        currentVregs_.push_back(reg);
    }

    void AddConstant(uint64_t value, VRegInfo::Type type, VRegInfo::VRegType vregType);

    void SetFrameSize(uint32_t size)
    {
        header_.SetFrameSize(size);
    }

    void Encode(ArenaVector<uint8_t> *stream, size_t offset = 0);

    void SetSavedCalleeRegsMask(uint32_t mask, uint32_t vmask)
    {
        header_.SetCalleeRegMask(mask);
        header_.SetCalleeFpRegMask(vmask);
    }

    void AddImplicitNullCheck(uint32_t instructionNativePc, uint32_t offset)
    {
        implicitNullchecks_.Add({instructionNativePc, offset});
    }

    void SetHasFloatRegs(bool has)
    {
        header_.SetHasFloatRegs(has);
    }

    template <typename Func>
    constexpr void EnumerateTables(Func func)
    {
        size_t index = 0;
        func(index++, &stackMaps_);
        func(index++, &inlineInfos_);
        func(index++, &rootsRegMasks_);
        func(index++, &rootsStackMasks_);
        func(index++, &methodIds_);
        func(index++, &vregMasks_);
        func(index++, &vregsMap_);
        func(index++, &vregsCatalogue_);
        func(index++, &implicitNullchecks_);
        func(index++, &constantTable_);
        ASSERT(index == CodeInfo::TABLES_COUNT);
    }

    void DumpCurrentStackMap(std::ostream &stream) const;

private:
    void EmitVRegs();

private:
    Arch arch_;
    uint32_t vregsCount_ {0};
    uint32_t currentVregsCount_ {0};

    CodeInfoHeader header_ {};

    // Tables
    BitTableBuilder<StackMap> stackMaps_;
    BitTableBuilder<InlineInfo> inlineInfos_;
    BitTableBuilder<RegisterMask> rootsRegMasks_;
    BitmapTableBuilder rootsStackMasks_;
    BitTableBuilder<MethodId> methodIds_;
    BitTableBuilder<VRegisterInfo> vregsCatalogue_;
    BitTableBuilder<VRegisterCatalogueIndex> vregsMap_;
    BitmapTableBuilder vregMasks_;
    BitTableBuilder<ImplicitNullChecks> implicitNullchecks_;
    BitTableBuilder<ConstantTable> constantTable_;

    // Auxiliary containers
    BitTableBuilder<StackMap>::Entry currentStackMap_;
    ArenaVector<VRegInfo> currentVregs_;
    ArenaVector<VRegInfo> lastVregs_;
    ArenaVector<uint32_t> vregsLastChange_;
    ArenaVector<BitTableBuilder<InlineInfo>::Entry> inlineInfoStack_;
    ArenaVector<BitTableBuilder<VRegisterCatalogueIndex>::Entry> vregsMapStorage_;
    ArenaBitVector vregsMaskStorage_;

#ifndef NDEBUG
    bool wasMethodBegin_ {false};
    bool wasStackMapBegin_ {false};
    bool wasInlineInfoBegin_ {false};
#endif

    static constexpr size_t MAX_VREG_LIVE_DISTANCE = 32;
};

}  // namespace ark::compiler

#endif  // PANDA_CODE_INFO_BUILDER_H
