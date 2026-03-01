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

#include "reg_alloc_graph_coloring.h"

namespace panda::compiler {
RegAllocGraphColoring::RegAllocGraphColoring(Graph *graph) : RegAllocBase(graph) {}
RegAllocGraphColoring::RegAllocGraphColoring(Graph *graph, size_t regs_count) : RegAllocBase(graph, regs_count) {}

void RegAllocGraphColoring::BuildIG(InterferenceGraph *ig, WorkingRanges *ranges)
{
    ig->Reserve(ranges->regular.size() + ranges->physical.size());
    ArenaDeque<ColorNode *> active_nodes(GetGraph()->GetLocalAllocator()->Adapter());
    ArenaVector<ColorNode *> physical_nodes(GetGraph()->GetLocalAllocator()->Adapter());

    for (auto physical_interval : ranges->physical) {
        ColorNode *node = ig->AllocNode();
        node->Assign(physical_interval);
        physical_nodes.push_back(node);
    }

    for (auto current_interval : ranges->regular) {
        auto range_start = current_interval->GetBegin();

        // Expire active_ranges
        while (!active_nodes.empty() && active_nodes.front()->GetLifeIntervals()->GetEnd() <= range_start) {
            active_nodes.pop_front();
        }

        ColorNode *node = ig->AllocNode();
        node->Assign(current_interval);

        // Interfer node
        for (auto active_node : active_nodes) {
            auto active_interval = active_node->GetLifeIntervals();
            if (current_interval->IntersectsWith(active_interval)) {
                ig->AddEdge(node->GetNumber(), active_node->GetNumber());
            }
        }

        for (auto physical_node : physical_nodes) {
            auto physical_interval = physical_node->GetLifeIntervals();
            auto intersection = current_interval->GetFirstIntersectionWith(physical_interval);
            // Current interval can intersect the physical one at the beginning of its live range
            // only if it's a call and physical interval's range was created for it.
            // Try to find first intersection excluding the range blocking registers during a call.
            if (intersection == current_interval->GetBegin()) {
                intersection = current_interval->GetFirstIntersectionWith(physical_interval, intersection + 1U);
            }

            if (intersection != INVALID_LIFE_NUMBER) {
                ig->AddEdge(node->GetNumber(), physical_node->GetNumber());
                node->AddCallsite(range_start);
            }
        }

        // Add node to active_nodes sorted by End time
        auto ranges_iter =
            std::upper_bound(active_nodes.begin(), active_nodes.end(), node, [](const auto &lhs, const auto &rhs) {
                return lhs->GetLifeIntervals()->GetEnd() <= rhs->GetLifeIntervals()->GetEnd();
            });
        active_nodes.insert(ranges_iter, node);
    }
}

namespace {
const ColorNode *FindFixedNode(const NodeVector &nodes, Location location)
{
    // Find node of input
    auto it = std::find_if(nodes.begin(), nodes.end(), [location](const auto &el) {
        auto liveness = el.GetLifeIntervals();
        return liveness->IsPhysical() && liveness->GetLocation() == location;
    });

    return it != nodes.end() ? &*it : nullptr;
}

const ColorNode *FindNode(const NodeVector &nodes, const Inst *inst)
{
    // Find node of input
    auto it = std::find_if(nodes.begin(), nodes.end(), [inst](const auto &el) {
        auto liveness = el.GetLifeIntervals();
        return !liveness->IsPhysical() && liveness->GetInst() == inst && liveness->GetSibling() == nullptr;
    });

    return it != nodes.end() ? &*it : nullptr;
}
}  // namespace

// Find precolorings and set registers to intervals in advance
RegAllocGraphColoring::IndexVector RegAllocGraphColoring::PrecolorIG(InterferenceGraph *ig, const RegisterMap &map)
{
    auto &nodes = ig->GetNodes();

    // Walk nodes and propagate properties
    IndexVector affinity_nodes;
    for (auto &node : nodes) {
        const auto *interv = node.GetLifeIntervals();

        // Take in account preassigned registers in intervals
        if (interv->IsPhysical() || interv->IsPreassigned()) {
            ASSERT(node.GetCallsiteIntersectCount() == 0);
            // Translate preassigned register from interval to color graph
            auto color = map.CodegenToRegallocReg(interv->GetReg());
            node.SetFixedColor(color, interv->IsPhysical());
            AddAffinityEdgeToSibling(ig, &node, &affinity_nodes);
            continue;
        }

        const auto *inst = interv->GetInst();
        ASSERT(inst != nullptr);
        if (inst->IsPhi()) {
            // TODO (Evgeny.Erokhin): Add precoloring for the node which is input for return if callsites == 0
            // Add inputs to affinity subgraph
            AddAffinityEdges(ig, &node, &affinity_nodes);
            continue;
        }

        // Add affinity edges to fixed locations
        for (auto i = 0U; i < inst->GetInputsCount(); i++) {
            auto location = inst->GetLocation(i);
            if (location.IsFixedRegister()) {
                auto input_node = FindNode(nodes, inst->GetDataFlowInput(i));
                auto fixed_node = FindFixedNode(nodes, location);
                // Possible when general intervals are processing, while input is fp-interval or vice versa
                if (input_node == nullptr || fixed_node == nullptr) {
                    continue;
                }
                affinity_nodes.push_back(input_node->GetNumber());
                affinity_nodes.push_back(fixed_node->GetNumber());
                ig->AddAffinityEdge(input_node->GetNumber(), fixed_node->GetNumber());
            }
        }
    }

    return affinity_nodes;
}

void RegAllocGraphColoring::BuildBias(InterferenceGraph *ig, const IndexVector &affinity_nodes)
{
    auto &nodes = ig->GetNodes();

    // Build affinity connected-components UCC(Unilaterally Connected Components) for coalescing (assign bias number to
    // nodes of same component)
    SmallVector<unsigned, DEFAULT_VECTOR_SIZE> walked;
    for (auto index : affinity_nodes) {
        auto &node = nodes[index];

        // Skip already biased
        if (node.HasBias()) {
            continue;
        }

        // Find connected component of graph UCC by (DFS), and collect Call-sites intersections
        walked.clear();
        walked.push_back(node.GetNumber());
        unsigned bias_num = ig->GetBiasCount();
        node.SetBias(bias_num);
        auto &bias = ig->AddBias();
        ig->UpdateBiasData(&bias, node);
        do {
            // Pop back
            unsigned cur_index = walked.back();
            walked.resize(walked.size() - 1);

            // Walk N affine nodes
            for (auto try_index : affinity_nodes) {
                auto &try_node = nodes[try_index];
                if (try_node.HasBias() || !ig->HasAffinityEdge(cur_index, try_index)) {
                    continue;
                }
                try_node.SetBias(bias_num);
                ig->UpdateBiasData(&bias, try_node);
                walked.push_back(try_index);
            }
        } while (!walked.empty());
    }

    // TODO (Evgeny.Erokhin): Add precoloring for nodes that have Use in call argument but but callsites == 0
}

void RegAllocGraphColoring::AddAffinityEdges(InterferenceGraph *ig, ColorNode *node, IndexVector *affinity_nodes)
{
    auto &nodes = ig->GetNodes();

    // Duplicates are possible but we tolerate it
    affinity_nodes->push_back(node->GetNumber());

    // Iterate over Phi inputs
    SmallVector<unsigned, DEFAULT_VECTOR_SIZE> affine;
    for (const auto &input : node->GetLifeIntervals()->GetInst()->GetInputs()) {
        // Add affinity edge
        if (const auto *anbr = FindNode(nodes, input.GetInst())) {
            COMPILER_LOG(DEBUG, REGALLOC) << "AfEdge: " << node->GetNumber() << " " << anbr->GetNumber() << " "
                                          << *anbr->GetLifeIntervals()->GetInst();
            ig->AddAffinityEdge(node->GetNumber(), anbr->GetNumber());
            affinity_nodes->push_back(anbr->GetNumber());
        }
    }
}

void RegAllocGraphColoring::AddAffinityEdgeToSibling(InterferenceGraph *ig, ColorNode *node,
                                                     IndexVector *affinity_nodes)
{
    const auto *interv = node->GetLifeIntervals();
    if (interv->IsPhysical() || interv->GetSibling() == nullptr) {
        return;
    }
    auto node_split = FindNode(ig->GetNodes(), interv->GetInst());
    ASSERT(node_split != nullptr);
    COMPILER_LOG(DEBUG, REGALLOC) << "AfEdge: " << node->GetNumber() << " " << node_split->GetNumber() << " "
                                  << *node_split->GetLifeIntervals()->GetInst();
    ig->AddAffinityEdge(node->GetNumber(), node_split->GetNumber());
    affinity_nodes->push_back(node->GetNumber());
    affinity_nodes->push_back(node_split->GetNumber());
}

Register RegAllocGraphColoring::AllocateRegisters(InterferenceGraph *ig, WorkingRanges *ranges, const RegisterMap &map)
{
    Presplit(ranges);
    // Build and precolor IG
    BuildIG(ig, ranges);
    BuildBias(ig, PrecolorIG(ig, map));

#ifdef NDEBUG
    if (!ig->IsChordal()) {
        COMPILER_LOG(WARNING, REGALLOC) << "Nonchordal graph: nonoptimal coloring possible";
    }
#endif  // NDEBUG

    // Color IG
    unsigned colors = 0;
    if (GetGraph()->IsBytecodeOptimizer()) {
        colors = ig->AssignColors<VIRTUAL_FRAME_SIZE>(map.GetAvailableRegsCount(), map.GetBorder());
    } else {
        colors = ig->AssignColors<MAX_NUM_REGS>(map.GetAvailableRegsCount(), map.GetBorder());
    }

    if (colors == 0) {
        COMPILER_LOG(DEBUG, REGALLOC) << "SPILL REQUIRED";
        return 0;
    }
    COMPILER_LOG(INFO, REGALLOC) << "IG nodes " << ig->GetNodes().size() << " colored with " << unsigned(colors);

    return colors;
}

void RegAllocGraphColoring::Remap(const InterferenceGraph &ig, const RegisterMap &map)
{
    // Map allocated colors to registers
    for (const auto &node : ig.GetNodes()) {
        auto *interval = node.GetLifeIntervals();
        if (!node.IsFixedColor()) {
            // Make interval's register
            auto color = node.GetColor();
            ASSERT(color != INVALID_REG);
            auto reg = map.RegallocToCodegenReg(color);
            interval->SetReg(reg);
        }
    }
}

bool RegAllocGraphColoring::Allocate()
{
    auto *gr = GetGraph();

    ReserveTempRegisters();
    // Create intervals sequences
    WorkingRanges general_ranges(gr->GetLocalAllocator());
    WorkingRanges fp_ranges(gr->GetLocalAllocator());
    InitWorkingRanges(&general_ranges, &fp_ranges);
    COMPILER_LOG(INFO, REGALLOC) << "Ranges reg " << general_ranges.regular.size() << " fp "
                                 << fp_ranges.regular.size();

    // Register allocation
    InterferenceGraph ig(gr->GetLocalAllocator());
    RegisterMap map(gr->GetLocalAllocator());

    unsigned int_colors = 0;
    if (!general_ranges.regular.empty()) {
        InitMap(&map, false);
        int_colors = AllocateRegisters(&ig, &general_ranges, map);
        if (int_colors == 0) {
            gr->GetAnalysis<LivenessAnalyzer>().Cleanup();
            COMPILER_LOG(DEBUG, REGALLOC) << "Integer RA failed";
            return false;
        }
        Remap(ig, map);
    }

    unsigned vec_colors = 0;
    if (!fp_ranges.regular.empty()) {
        GetGraph()->SetHasFloatRegs();

        InitMap(&map, true);
        vec_colors = AllocateRegisters(&ig, &fp_ranges, map);
        if (vec_colors == 0) {
            gr->GetAnalysis<LivenessAnalyzer>().Cleanup();
            COMPILER_LOG(DEBUG, REGALLOC) << "Vector RA failed";
            return false;
        }
        Remap(ig, map);
    }

    COMPILER_LOG(DEBUG, REGALLOC) << "GC RA passed " << gr->GetRuntime()->GetMethodName(gr->GetMethod()) << " int "
                                  << int_colors << " vec " << vec_colors;

    return true;
}

namespace {
/*
 * Add range in sorted order
 */
void AddRange(LifeIntervals *interval, InstructionsRanges *dest)
{
    auto iter = std::upper_bound(dest->begin(), dest->end(), interval,
                                 [](const auto &lhs, const auto &rhs) { return lhs->GetBegin() < rhs->GetBegin(); });
    dest->insert(iter, interval);
}
}  // namespace

void RegAllocGraphColoring::InitWorkingRanges(WorkingRanges *general_ranges, WorkingRanges *fp_ranges)
{
    for (auto *interval : GetGraph()->GetAnalysis<LivenessAnalyzer>().GetLifeIntervals()) {
        if (interval->GetReg() == ACC_REG_ID) {
            continue;
        }

        if (interval->IsPreassigned() && interval->GetReg() == INVALID_REG) {
            ASSERT(interval->GetReg() != INVALID_REG);
            continue;
        }

        // Skip instructions without destination register
        if (!interval->IsPhysical() && interval->NoDest()) {
            ASSERT(interval->GetLocation().IsInvalid());
            continue;
        }

        bool is_fp = DataType::IsFloatType(interval->GetType());
        auto *ranges = is_fp ? fp_ranges : general_ranges;
        if (interval->IsPhysical()) {
            auto mask = is_fp ? GetVRegMask() : GetRegMask();
            if (mask.IsSet(interval->GetReg())) {
                // skip physical intervals for unavailable registers, they do not affect allocation
                continue;
            }
            AddRange(interval, &ranges->physical);
        } else {
            AddRange(interval, &ranges->regular);
        }
    }
}

void RegAllocGraphColoring::InitMap(RegisterMap *map, bool is_vector)
{
    auto arch = GetGraph()->GetArch();
    if (arch == Arch::NONE) {
        ASSERT(GetGraph()->IsBytecodeOptimizer());
        ASSERT(!is_vector);
        map->SetMask(GetRegMask(), 0);
    } else {
        size_t first_callee = GetFirstCalleeReg(arch, is_vector);
        size_t last_callee = GetLastCalleeReg(arch, is_vector);
        map->SetCallerFirstMask(is_vector ? GetVRegMask() : GetRegMask(), first_callee, last_callee);
    }
}

void RegAllocGraphColoring::Presplit(WorkingRanges *ranges)
{
    ArenaVector<LifeIntervals *> to_split(GetGraph()->GetLocalAllocator()->Adapter());

    for (auto interval : ranges->regular) {
        if (!interval->GetLocation().IsFixedRegister()) {
            continue;
        }
        for (auto next : ranges->regular) {
            if (next->GetBegin() <= interval->GetBegin()) {
                continue;
            }
            if (interval->GetLocation() == next->GetLocation() && interval->IntersectsWith(next)) {
                to_split.push_back(interval);
                break;
            }
        }

        if (!to_split.empty() && to_split.back() == interval) {
            // Already added to split
            continue;
        }
        for (auto physical : ranges->physical) {
            if (interval->GetLocation() == physical->GetLocation() && interval->IntersectsWith(physical)) {
                to_split.push_back(interval);
                break;
            }
        }
    }

    for (auto interval : to_split) {
        COMPILER_LOG(DEBUG, REGALLOC) << "Split at the beginning: " << interval->ToString();
        auto split = interval->SplitAt(interval->GetBegin() + 1, GetGraph()->GetAllocator());
        AddRange(split, &ranges->regular);
    }
}
}  // namespace panda::compiler
