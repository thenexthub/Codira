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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_REGALLOC_SPILL_FILLS_RESOLVER_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_REGALLOC_SPILL_FILLS_RESOLVER_H

#include "compiler/optimizer/ir/graph_visitor.h"
#include "optimizer/ir/inst.h"
#include "utils/arena_containers.h"

namespace panda::compiler {
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
        DataType::Type reg_type;
    };

public:
    explicit SpillFillsResolver(Graph *graph);
    SpillFillsResolver(Graph *graph, Register resolver, size_t regs_count, size_t vregs_count = 0);
    NO_COPY_SEMANTIC(SpillFillsResolver);
    NO_MOVE_SEMANTIC(SpillFillsResolver);
    ~SpillFillsResolver() override = default;

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override;

    void Run();

    void Resolve(SpillFillInst *spill_fill_inst);

    void ResolveIfRequired(SpillFillInst *spill_fill_inst);

    Graph *GetGraph() const;

protected:
    static void VisitSpillFill(GraphVisitor *visitor, Inst *inst);

private:
    bool NeedToResolve(const ArenaVector<SpillFillData> &spill_fills);
    void ResolveCallSpillFill(SpillFillInst *spill_fill_inst);
    void CollectSpillFillsData(SpillFillInst *spill_fill_inst);
    void Reorder(SpillFillInst *spill_fill_inst);
    LocationIndex CheckAndResolveCyclicDependency(LocationIndex dst_first);
    template <bool ciclyc>
    void AddMovesChain(LocationIndex dst, ArenaVector<LocationIndex> *remap, SpillFillInst *spill_fill_inst);
    LocationIndex GetResolver(DataType::Type type);

    // Get table index by Location type
    LocationIndex Map(Location location)
    {
        if (location.IsRegister()) {
            return location.GetValue();
        }
        if (location.IsFpRegister()) {
            return location.GetValue() + VREGS_TABLE_OFFSET;
        }
        if (location.IsStack()) {
            return location.GetValue() + SLOTS_TABLE_OFFSET;
        }
        ASSERT(location.IsStackParameter());
        return location.GetValue() + PARAMETER_SLOTS_OFFSET;
    }

    // Fetch location type and element number inside this location from table index
    Location ToLocation(LocationIndex reg)
    {
        if (reg >= PARAMETER_SLOTS_OFFSET) {
            return Location::MakeStackParameter(reg - PARAMETER_SLOTS_OFFSET);
        }
        if (reg >= SLOTS_TABLE_OFFSET) {
            return Location::MakeStackSlot(reg - SLOTS_TABLE_OFFSET);
        }
        if (reg >= VREGS_TABLE_OFFSET) {
            return Location::MakeFpRegister(reg - VREGS_TABLE_OFFSET);
        }
        return Location::MakeRegister(reg);
    }

    static inline bool IsPairedReg(Arch arch, DataType::Type type)
    {
        return arch != Arch::NONE && !Is64BitsArch(arch) && Is64Bits(type, arch);
    }

private:
    Graph *graph_ {nullptr};
    ArenaVector<MoveInfo> moves_table_;
    ArenaVector<uint8_t> loads_count_;
    // Group of moves which can be safely inserted before all others
    ArenaVector<SpillFillData> pre_moves_;
    // Group of moves which can be safely inserted after all others
    ArenaVector<SpillFillData> post_moves_;
    Register resolver_;

    const size_t VREGS_TABLE_OFFSET;
    const size_t SLOTS_TABLE_OFFSET;
    const size_t PARAMETER_SLOTS_OFFSET;
    const size_t LOCATIONS_COUNT;
    ArenaVector<bool> reg_write_;
    ArenaVector<bool> stack_write_;

#include "optimizer/ir/visitor.inc"
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_REGALLOC_SPILL_FILLS_RESOLVER_H
