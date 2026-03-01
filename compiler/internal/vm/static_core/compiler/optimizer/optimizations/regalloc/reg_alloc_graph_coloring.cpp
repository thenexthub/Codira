/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "reg_alloc_graph_coloring.h"
#include <cmath>
#include "compiler_logger.h"
#include "interference_graph.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/code_generator/callconv.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/datatype.h"
#include "optimizer/ir/graph.h"
#include "reg_type.h"

namespace ark::compiler {
RegAllocGraphColoring::RegAllocGraphColoring(Graph *graph) : RegAllocBase(graph) {}
RegAllocGraphColoring::RegAllocGraphColoring(Graph *graph, size_t regsCount) : RegAllocBase(graph, regsCount) {}
RegAllocGraphColoring::RegAllocGraphColoring(Graph *graph, LocationMask mask) : RegAllocBase(graph, std::move(mask))
{
    ASSERT(!IsFrameSizeLarge() || graph->IsAbcKit());
}

void RegAllocGraphColoring::FillPhysicalNodes(InterferenceGraph *ig, WorkingRanges *ranges,
                                              ArenaVector<ColorNode *> &physicalNodes)
{
    for (auto physicalInterval : ranges->physical) {
        ColorNode *node = ig->AllocNode();
        node->Assign(physicalInterval);
        node->SetPhysical();
        physicalNodes.push_back(node);
    }
}

void RegAllocGraphColoring::BuildIG(InterferenceGraph *ig, WorkingRanges *ranges, bool rematConstants)
{
    ig->Reserve(ranges->regular.size() + ranges->physical.size());
    ArenaDeque<ColorNode *> activeNodes(GetGraph()->GetLocalAllocator()->Adapter());
    ArenaVector<ColorNode *> physicalNodes(GetGraph()->GetLocalAllocator()->Adapter());
    const auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();

    FillPhysicalNodes(ig, ranges, physicalNodes);

    for (auto currentInterval : ranges->regular) {
        auto rangeStart = currentInterval->GetBegin();

        if (rematConstants && TryToSpillConstant(currentInterval, GetGraph())) {
            continue;
        }

        // Expire active_ranges
        while (!activeNodes.empty() && activeNodes.front()->GetLifeIntervals()->GetEnd() <= rangeStart) {
            activeNodes.pop_front();
        }

        ColorNode *node = ig->AllocNode();
        node->Assign(currentInterval);

        if (ig->IsUsedSpillWeight()) {
            node->SetSpillWeight(CalcSpillWeight(la, currentInterval));
        }

        // Interfer node
        for (auto activeNode : activeNodes) {
            auto activeInterval = activeNode->GetLifeIntervals();
            if (currentInterval->IntersectsWith(activeInterval)) {
                ig->AddEdge(node->GetNumber(), activeNode->GetNumber());
            }
        }

        for (auto physicalNode : physicalNodes) {
            auto physicalInterval = physicalNode->GetLifeIntervals();
            if (currentInterval->IntersectsWith<true>(physicalInterval)) {
                ig->AddEdge(node->GetNumber(), physicalNode->GetNumber());
                node->AddCallsite(rangeStart);
            }
        }
        auto callback = [ig, node](Location location) {
            ASSERT(location.IsFixedRegister());
            auto physicalNode = ig->FindPhysicalNode(location);
            if (physicalNode == nullptr) {
                return;
            }
            ig->AddEdge(node->GetNumber(), physicalNode->GetNumber());
        };
        if (!currentInterval->HasInst()) {
            // current_interval - is additional life interval for an instruction required temp, add edges to the fixed
            // inputs' nodes of that instruction
            la.EnumerateFixedLocationsOverlappingTemp(currentInterval, callback);
        }

        // Add node to active_nodes sorted by End time
        auto rangesIter =
            std::upper_bound(activeNodes.begin(), activeNodes.end(), node, [](const auto &lhs, const auto &rhs) {
                return lhs->GetLifeIntervals()->GetEnd() <= rhs->GetLifeIntervals()->GetEnd();
            });
        activeNodes.insert(rangesIter, node);
    }
}

RegAllocGraphColoring::IndexVector RegAllocGraphColoring::PrecolorIG(InterferenceGraph *ig)
{
    // Walk nodes and propagate properties
    IndexVector affinityNodes;
    for (const auto &node : ig->GetNodes()) {
        AddAffinityEdgesToSiblings(ig, node, &affinityNodes);
    }
    return affinityNodes;
}

// Find precolorings and set registers to intervals in advance
RegAllocGraphColoring::IndexVector RegAllocGraphColoring::PrecolorIG(InterferenceGraph *ig, const RegisterMap &map)
{
    // Walk nodes and propagate properties
    IndexVector affinityNodes;
    for (auto &node : ig->GetNodes()) {
        const auto *interv = node.GetLifeIntervals();
        // Take in account preassigned registers in intervals
        if (interv->IsPhysical() || interv->IsPreassigned()) {
            // Translate preassigned register from interval to color graph
            auto color = map.CodegenToRegallocReg(interv->GetReg());
            node.SetFixedColor(color);
        }

        if (!interv->HasInst() || interv->IsSplitSibling()) {
            continue;
        }

        AddAffinityEdgesToSiblings(ig, node, &affinityNodes);

        const auto *inst = interv->GetInst();
        ASSERT(inst != nullptr);
        if (inst->IsPhi()) {
            AddAffinityEdgesToPhi(ig, node, &affinityNodes);
        }
    }
    AddAffinityEdgesToPhysicalNodes(ig, &affinityNodes);
    return affinityNodes;
}

void RegAllocGraphColoring::BuildBias(InterferenceGraph *ig, const IndexVector &affinityNodes)
{
    auto &nodes = ig->GetNodes();

    // Build affinity connected-components UCC(Unilaterally Connected Components) for coalescing (assign bias number to
    // nodes of same component)
    SmallVector<unsigned, DEFAULT_VECTOR_SIZE> walked;
    SmallVector<unsigned, DEFAULT_VECTOR_SIZE> biased;
    for (auto index : affinityNodes) {
        auto &node = nodes[index];

        // Skip already biased
        if (node.HasBias()) {
            continue;
        }

        // Find connected component of graph UCC by (DFS), and collect Call-sites intersections
        walked.clear();
        walked.push_back(node.GetNumber());
        biased.clear();
        biased.push_back(node.GetNumber());
        WalkNodes(std::make_pair(walked, biased), nodes, node, ig, affinityNodes);
    }
}

void RegAllocGraphColoring::WalkNodes(IndexVectorPair &&vectors, NodeVector &nodes, ColorNode node,
                                      InterferenceGraph *ig, const IndexVector &affinityNodes)
{
    auto &[walked, biased] = vectors;
    unsigned biasNum = ig->GetBiasCount();
    node.SetBias(biasNum);
    auto &bias = ig->AddBias();
    ig->UpdateBiasData(&bias, node);
    do {
        // Pop back
        unsigned curIndex = walked.back();
        walked.resize(walked.size() - 1);

        // Walk N affine nodes
        for (auto tryIndex : affinityNodes) {
            auto &tryNode = nodes[tryIndex];
            if (tryNode.HasBias() || !ig->HasAffinityEdge(curIndex, tryIndex)) {
                continue;
            }
            // Check if the `try_node` intersects one of the already biased
            auto it = std::find_if(biased.cbegin(), biased.cend(),
                                   [ig, tryIndex](auto id) { return ig->HasEdge(id, tryIndex); });
            if (it != biased.cend()) {
                continue;
            }

            tryNode.SetBias(biasNum);
            ig->UpdateBiasData(&bias, tryNode);
            walked.push_back(tryIndex);
            biased.push_back(tryIndex);
        }
    } while (!walked.empty());
}

void RegAllocGraphColoring::AddAffinityEdgesToPhi(InterferenceGraph *ig, const ColorNode &node,
                                                  IndexVector *affinityNodes)
{
    // Duplicates are possible but we tolerate it
    affinityNodes->push_back(node.GetNumber());

    auto phi = node.GetLifeIntervals()->GetInst();
    ASSERT(phi->IsPhi());
    auto la = &GetGraph()->GetAnalysis<LivenessAnalyzer>();
    // Iterate over Phi inputs
    for (size_t i = 0; i < phi->GetInputsCount(); i++) {
        // Add affinity edge
        auto inputLi = la->GetInstLifeIntervals(phi->GetDataFlowInput(i));
        AddAffinityEdge(ig, affinityNodes, node, inputLi);
    }
}

void RegAllocGraphColoring::AddAffinityEdgesToSiblings(InterferenceGraph *ig, const ColorNode &node,
                                                       IndexVector *affinityNodes)
{
    auto sibling = node.GetLifeIntervals()->GetSibling();
    if (sibling == nullptr) {
        return;
    }
    affinityNodes->push_back(node.GetNumber());
    while (sibling != nullptr) {
        AddAffinityEdge(ig, affinityNodes, node, sibling);
        sibling = sibling->GetSibling();
    }
}

void RegAllocGraphColoring::AddAffinityEdgesToPhysicalNodes(InterferenceGraph *ig, IndexVector *affinityNodes)
{
    auto la = &GetGraph()->GetAnalysis<LivenessAnalyzer>();
    for (auto *interval : la->GetLifeIntervals()) {
        if (!interval->HasInst()) {
            continue;
        }
        const auto *inst = interval->GetInst();
        ASSERT(inst != nullptr);
        // Add affinity edges to fixed locations
        for (auto i = 0U; i < inst->GetInputsCount(); i++) {
            auto location = inst->GetLocation(i);
            if (!location.IsFixedRegister()) {
                continue;
            }
            auto fixedNode = ig->FindPhysicalNode(location);
            // Possible when general intervals are processing, while input is fp-interval or vice versa
            if (fixedNode == nullptr) {
                continue;
            }
            affinityNodes->push_back(fixedNode->GetNumber());

            auto inputLi = la->GetInstLifeIntervals(inst->GetDataFlowInput(i));
            auto sibling = inputLi->FindSiblingAt(interval->GetBegin());
            ASSERT(sibling != nullptr);
            AddAffinityEdge(ig, affinityNodes, *fixedNode, sibling);
        }
    }
}

/**
 * Try to find node for the `li` interval in the IG;
 * If node exists, create affinity edge between it and the `node`
 */
void RegAllocGraphColoring::AddAffinityEdge(InterferenceGraph *ig, IndexVector *affinityNodes, const ColorNode &node,
                                            LifeIntervals *li)
{
    if (auto afNode = ig->FindNode(li)) {
        COMPILER_LOG(DEBUG, REGALLOC) << "AfEdge: " << node.GetNumber() << " " << afNode->GetNumber();
        ig->AddAffinityEdge(node.GetNumber(), afNode->GetNumber());
        affinityNodes->push_back(afNode->GetNumber());
    }
}

bool RegAllocGraphColoring::AllocateRegisters(InterferenceGraph *ig, WorkingRanges *ranges, WorkingRanges *stackRanges,
                                              const RegisterMap &map)
{
    if (GetGraph()->IsBytecodeOptimizer()) {
        BuildIG(ig, ranges);
        BuildBias(ig, PrecolorIG(ig, map));
        if (IsFrameSizeLarge()) {
            return ig->AssignColors<VIRTUAL_FRAME_SIZE_LARGE>(map.GetAvailableRegsCount(), map.GetBorder());
        }
        return ig->AssignColors<VIRTUAL_FRAME_SIZE>(map.GetAvailableRegsCount(), map.GetBorder());
    }

    Presplit(ranges);
    ig->SetUseSpillWeight(true);
    auto rounds = 0;
    static constexpr size_t ROUNDS_LIMIT = 30;
    while (true) {
        if (++rounds == ROUNDS_LIMIT) {
            return false;
        }
        BuildIG(ig, ranges);
        BuildBias(ig, PrecolorIG(ig, map));
        if (ig->AssignColors<MAX_NUM_REGS>(map.GetAvailableRegsCount(), map.GetBorder())) {
            break;
        }
        SparseIG(ig, map.GetAvailableRegsCount(), ranges, stackRanges);
    }
    return true;
}

bool RegAllocGraphColoring::AllocateSlots(InterferenceGraph *ig, WorkingRanges *stackRanges)
{
    ig->SetUseSpillWeight(false);
    BuildIG(ig, stackRanges, true);
    BuildBias(ig, PrecolorIG(ig));
    if (IsFrameSizeLarge()) {
        return ig->AssignColors<MAX_NUM_STACK_SLOTS_LARGE>(GetMaxNumStackSlots(), 0);
    }
    return ig->AssignColors<MAX_NUM_STACK_SLOTS>(GetMaxNumStackSlots(), 0);
}

/*
 * Coloring was unsuccessful, hence uncolored nodes should be split to sparse the interference graph
 */
void RegAllocGraphColoring::SparseIG(InterferenceGraph *ig, unsigned regsCount, WorkingRanges *ranges,
                                     WorkingRanges *stackRanges)
{
    for (const auto &node : ig->GetNodes()) {
        if (node.GetColor() != regsCount) {
            continue;
        }
        auto interval = node.GetLifeIntervals();
        if (interval->GetUsePositions().empty()) {
            SpillInterval(interval, ranges, stackRanges);
            continue;
        }

        interval->SplitAroundUses(GetGraph()->GetAllocator());
        bool isConst = interval->GetInst()->IsConst();
        if (isConst && interval->GetUsePositions().empty()) {
            SpillInterval(interval, ranges, stackRanges);
        }
        for (auto sibling = interval->GetSibling(); sibling != nullptr; sibling = sibling->GetSibling()) {
            if (isConst && sibling->GetUsePositions().empty()) {
                AddRange(sibling, &stackRanges->regular);
            } else {
                AddRange(sibling, &ranges->regular);
            }
        }
    }
}

void RegAllocGraphColoring::SpillInterval(LifeIntervals *interval, WorkingRanges *ranges, WorkingRanges *stackRanges)
{
    ASSERT(interval->GetUsePositions().empty());
    ranges->regular.erase(std::remove(ranges->regular.begin(), ranges->regular.end(), interval), ranges->regular.end());
    AddRange(interval, &stackRanges->regular);
}

void RegAllocGraphColoring::Remap(const InterferenceGraph &ig, const RegisterMap &map)
{
    // Map allocated colors to registers
    for (const auto &node : ig.GetNodes()) {
        auto *interval = node.GetLifeIntervals();
        if (!node.IsFixedColor()) {
            // Make interval's register
            auto color = node.GetColor();
            ASSERT(color != GetInvalidReg());
            auto reg = map.RegallocToCodegenReg(color);
            interval->SetReg(reg);
        }
    }
}

bool RegAllocGraphColoring::MapSlots(const InterferenceGraph &ig)
{
    // Map allocated colors to stack slots
    for (const auto &node : ig.GetNodes()) {
        auto *interval = node.GetLifeIntervals();
        // Constant definition on the stack slot is not supported now
        if (!interval->IsSplitSibling() && interval->GetInst()->IsConst()) {
            COMPILER_LOG(DEBUG, REGALLOC) << "Stack slots RA failed";
            return false;
        }
        auto slot = node.GetColor();
        interval->SetLocation(Location::MakeStackSlot(slot));
        GetStackMask().Set(slot);
    }
    return true;
}

bool RegAllocGraphColoring::Allocate()
{
    auto *gr = GetGraph();

    ReserveTempRegisters();
    // Create intervals sequences
    WorkingRanges generalRanges(gr->GetLocalAllocator());
    WorkingRanges fpRanges(gr->GetLocalAllocator());
    WorkingRanges stackRanges(gr->GetLocalAllocator());
    InitWorkingRanges(&generalRanges, &fpRanges);
    COMPILER_LOG(INFO, REGALLOC) << "Ranges reg " << generalRanges.regular.size() << " fp " << fpRanges.regular.size();

    // Register allocation
    InterferenceGraph ig(gr->GetLocalAllocator());
    RegisterMap map(gr->GetLocalAllocator());

    if (!generalRanges.regular.empty()) {
        InitMap(&map, false);
        if (!AllocateRegisters(&ig, &generalRanges, &stackRanges, map)) {
            COMPILER_LOG(DEBUG, REGALLOC) << "Integer RA failed";
            return false;
        }
        Remap(ig, map);
    }

    if (!fpRanges.regular.empty()) {
        GetGraph()->SetHasFloatRegs();
        InitMap(&map, true);
        if (!AllocateRegisters(&ig, &fpRanges, &stackRanges, map)) {
            COMPILER_LOG(DEBUG, REGALLOC) << "Vector RA failed";
            return false;
        }
        Remap(ig, map);
    }

    if (!stackRanges.regular.empty()) {
        if (AllocateSlots(&ig, &stackRanges) && MapSlots(ig)) {
            return true;
        }
        COMPILER_LOG(DEBUG, REGALLOC) << "Stack slots RA failed";
        return false;
    }
    return true;
}

void RegAllocGraphColoring::InitWorkingRanges(WorkingRanges *generalRanges, WorkingRanges *fpRanges)
{
    for (auto *interval : GetGraph()->GetAnalysis<LivenessAnalyzer>().GetLifeIntervals()) {
        if (interval->GetReg() == GetAccReg()) {
            continue;
        }

        if (interval->IsPreassigned() && interval->GetReg() == GetGraph()->GetZeroReg()) {
            ASSERT(interval->GetReg() != GetInvalidReg());
            continue;
        }

        // Skip instructions without destination register
        if (interval->HasInst() && interval->NoDest()) {
            ASSERT(interval->GetLocation().IsInvalid());
            continue;
        }

        bool isFp = DataType::IsFloatType(interval->GetType());
        auto *ranges = isFp ? fpRanges : generalRanges;
        if (interval->IsPhysical()) {
            auto mask = isFp ? GetVRegMask() : GetRegMask();
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

void RegAllocGraphColoring::InitMap(RegisterMap *map, bool isVector)
{
    auto arch = GetGraph()->GetArch();
    if (arch == Arch::NONE) {
        ASSERT(GetGraph()->IsBytecodeOptimizer());
        ASSERT(!isVector);
        map->SetMask(GetRegMask(), 0);
    } else {
        size_t firstCallee = GetFirstCalleeReg(arch, isVector);
        size_t lastCallee = GetLastCalleeReg(arch, isVector);
        map->SetCallerFirstMask(isVector ? GetVRegMask() : GetRegMask(), firstCallee, lastCallee);
    }
}

void RegAllocGraphColoring::Presplit(WorkingRanges *ranges)
{
    ArenaVector<LifeIntervals *> toSplit(GetGraph()->GetLocalAllocator()->Adapter());

    for (auto interval : ranges->regular) {
        if (!interval->GetLocation().IsFixedRegister()) {
            continue;
        }
        for (auto next : ranges->regular) {
            if (next->GetBegin() <= interval->GetBegin()) {
                continue;
            }
            if (interval->GetLocation() == next->GetLocation() && interval->IntersectsWith(next)) {
                toSplit.push_back(interval);
                break;
            }
        }

        if (!toSplit.empty() && toSplit.back() == interval) {
            // Already added to split
            continue;
        }
        for (auto physical : ranges->physical) {
            if (interval->GetLocation() == physical->GetLocation() && interval->IntersectsWith<true>(physical)) {
                toSplit.push_back(interval);
                break;
            }
        }
    }

    for (auto interval : toSplit) {
        COMPILER_LOG(DEBUG, REGALLOC) << "Split at the beginning: " << interval->ToString();
        auto split = interval->SplitAt(interval->GetBegin() + 1, GetGraph()->GetAllocator());
        AddRange(split, &ranges->regular);
    }
}
}  // namespace ark::compiler
