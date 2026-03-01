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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_SPILL_FILLS_RESOLVER_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_SPILL_FILLS_RESOLVER_H

#include "compiler/optimizer/ir/graph_visitor.h"
#include "optimizer/code_generator/registers_description.h"
#include "optimizer/ir/inst.h"
#include "libarkbase/utils/arena_containers.h"

namespace ark::compiler {
class RegAllocBase;

/*
 * There are 3 spill-fill location types:
 * - general register
 * - vector register
 * - stack slot
 *
 * Each spill-fill's source and destination is described by index inside these locations;
 *
 * Since  all destinations are unique, we can use them as keys for table, which collects information about
 * all moves:
 * - genenral register index maps to table key one by one
 * - vector register index offsets on number of genenral registers (MAX_NUM_REGS)
 * - stack slot index offsets on number of all registers (MAX_NUM_REGS + MAX_NUM_VREGS)
 *
 * Iterating over this table `SpillFillsResolver` builds non-conflict chains of spill-fills.
 * Building process is described before `SpillFillsResolver::Reorder()` method.
 */
class SpillFillsResolver : public GraphVisitor {
    static_assert(sizeof(Register) == sizeof(StackSlot), "Register and StackSlot should be the same size");

    using LocationIndex = uint16_t;
    static constexpr auto INVALID_LOCATION_INDEX = std::numeric_limits<LocationIndex>::max();

    struct LocationInfo {
        LocationType location;
        LocationIndex idx;
    };

    struct MoveInfo {
        LocationIndex src;
        DataType::Type regType;
    };

public:
    explicit SpillFillsResolver(Graph *graph);
    SpillFillsResolver(Graph *graph, Register resolver, size_t regsCount, size_t vregsCount = 0);
    NO_COPY_SEMANTIC(SpillFillsResolver);
    NO_MOVE_SEMANTIC(SpillFillsResolver);
    ~SpillFillsResolver() override = default;

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override;

    void Run();

    void Resolve(SpillFillInst *spillFillInst);

    void ResolveIfRequired(SpillFillInst *spillFillInst);

    Graph *GetGraph() const;

protected:
    static void VisitSpillFill(GraphVisitor *visitor, Inst *inst);

private:
    bool NeedToResolve(const ArenaVector<SpillFillData> &spillFills);
    void ResolveCallSpillFill(SpillFillInst *spillFillInst);
    void CollectSpillFillsData(SpillFillInst *spillFillInst);
    void Reorder(SpillFillInst *spillFillInst);
    LocationIndex CheckAndResolveCyclicDependency(LocationIndex dstFirst);
    template <bool CICLYC>
    void AddMovesChain(LocationIndex dst, ArenaVector<LocationIndex> *remap, SpillFillInst *spillFillInst);
    LocationIndex GetResolver(DataType::Type type);

    // Get table index by Location type
    LocationIndex Map(Location location)
    {
        if (location.IsRegister()) {
            return location.GetValue();
        }
        if (location.IsFpRegister()) {
            return location.GetValue() + vregsTableOffset_;
        }
        if (location.IsStack()) {
            return location.GetValue() + slotsTableOffset_;
        }
        ASSERT(location.IsStackParameter());
        return location.GetValue() + parameterSlotsOffset_;
    }

    // Fetch location type and element number inside this location from table index
    Location ToLocation(LocationIndex reg)
    {
        if (reg >= parameterSlotsOffset_) {
            return Location::MakeStackParameter(reg - parameterSlotsOffset_);
        }
        if (reg >= slotsTableOffset_) {
            return Location::MakeStackSlot(reg - slotsTableOffset_);
        }
        if (reg >= vregsTableOffset_) {
            return Location::MakeFpRegister(reg - vregsTableOffset_);
        }
        return Location::MakeRegister(reg);
    }

    static inline bool IsPairedReg(Arch arch, DataType::Type type)
    {
        return arch != Arch::NONE && !Is64BitsArch(arch) && Is64Bits(type, arch);
    }

private:
    Graph *graph_ {nullptr};
    ArenaVector<MoveInfo> movesTable_;
    ArenaVector<uint8_t> loadsCount_;
    // Group of moves which can be safely inserted before all others
    ArenaVector<SpillFillData> preMoves_;
    // Group of moves which can be safely inserted after all others
    ArenaVector<SpillFillData> postMoves_;
    Register resolver_;

    const size_t vregsTableOffset_;
    const size_t slotsTableOffset_;
    const size_t parameterSlotsOffset_;
    const size_t locationsCount_;
    ArenaVector<bool> regWrite_;
    ArenaVector<bool> stackWrite_;

#include "optimizer/ir/visitor.inc"
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_SPILL_FILLS_RESOLVER_H
