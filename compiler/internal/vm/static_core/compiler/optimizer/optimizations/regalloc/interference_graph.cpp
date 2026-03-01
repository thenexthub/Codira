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

#include "interference_graph.h"
#include <array>
#include <iterator>
#include <numeric>
#include "optimizer/analysis/liveness_analyzer.h"
#include "libarkbase/utils/small_vector.h"

namespace ark::compiler {

ColorNode *InterferenceGraph::AllocNode()
{
    unsigned cur = nodes_.size();
    nodes_.emplace_back(cur, nodes_.get_allocator());

    // Check matrix capacity
    ASSERT(nodes_.size() <= matrix_.GetCapacity());

    return &nodes_.back();
}

void InterferenceGraph::Reserve(size_t count)
{
    nodes_.clear();
    nodes_.reserve(count);
    matrix_.SetCapacity(count);
    biases_.clear();
}

void InterferenceGraph::AddEdge(unsigned a, unsigned b)
{
    matrix_.AddEdge(a, b);
}

bool InterferenceGraph::HasEdge(unsigned a, unsigned b) const
{
    return matrix_.HasEdge(a, b);
}

void InterferenceGraph::AddAffinityEdge(unsigned a, unsigned b)
{
    matrix_.AddAffinityEdge(a, b);
}

bool InterferenceGraph::HasAffinityEdge(unsigned a, unsigned b) const
{
    return matrix_.HasAffinityEdge(a, b);
}

namespace {
constexpr size_t MIN_SIMPLITIAL_NODES = 3;
constexpr size_t DEFAULT_BOUNDARY_STACK = 16;
}  // namespace

ArenaVector<unsigned> InterferenceGraph::LexBFS() const
{
    // Initialize out to sequentaly from 0
    unsigned num = nodes_.size();
    ArenaVector<unsigned> out(num, nodes_.get_allocator());
    std::iota(out.begin(), out.end(), 0);

    // Less then 3 are all simplicial
    if (out.size() < MIN_SIMPLITIAL_NODES) {
        return out;
    }

    // Control sub-sequences boundaries in stack maner
    SmallVector<unsigned, DEFAULT_BOUNDARY_STACK> boundaryStack;
    boundaryStack.reserve(num);
    boundaryStack.push_back(num);  // Sentinel
    unsigned pos = 0;              // Initialy we have set S of all elements

    while (true) {
        ASSERT(pos < out.size());
        auto id = out[pos];
        pos++;

        // Check for boundaries colapse
        ASSERT(!boundaryStack.empty());
        auto prevEnd = boundaryStack.back();
        ASSERT(pos <= prevEnd);
        if (pos == prevEnd) {
            if (pos == num) {
                break;
            }
            boundaryStack.resize(boundaryStack.size() - 1);
            ASSERT(!boundaryStack.empty());
            prevEnd = boundaryStack.back();
        }

        // Partition on 2 groups: adjacent and not adjacent(last)
        ASSERT(pos <= prevEnd);
        auto it = std::stable_partition(out.begin() + pos, out.begin() + prevEnd,
                                        [id, &out, this](unsigned val) { return HasEdge(id, out[val]); });
        auto pivot = static_cast<unsigned>(std::distance(out.begin(), it));
        // Split group if needed
        if (pivot > pos && pivot != prevEnd) {
            boundaryStack.push_back(pivot);
        }
    }

    return out;
}

ArenaVector<unsigned> InterferenceGraph::GetOrderedNodesIds() const
{
    // Initialize out to sequentaly from 0
    unsigned num = nodes_.size();
    ArenaVector<unsigned> out(num, nodes_.get_allocator());
    std::iota(out.begin(), out.end(), 0);
    // If spill weights were counted, prefer nodes with higher weight, to increase their chances to be colored
    if (IsUsedSpillWeight()) {
        std::sort(out.begin(), out.end(), [&](unsigned lhsId, unsigned rhsId) {
            return GetNode(lhsId).GetSpillWeight() > GetNode(rhsId).GetSpillWeight();
        });
    }
    return out;
}

namespace {
constexpr size_t DEFAULT_VECTOR_SIZE = 64;
}  // namespace

template <>
bool InterferenceGraph::CheckNeighborsInClique(const ArenaVector<unsigned> &peo,
                                               SmallVector<Register, DEFAULT_VECTOR_SIZE> &processedNbr) const
{
    for (auto nbr1 : processedNbr) {
        for (auto nbr2 : processedNbr) {
            if (nbr1 != nbr2 && !HasEdge(peo[nbr1], peo[nbr2])) {
                return false;
            }
        }
    }
    return true;
}

bool InterferenceGraph::IsChordal() const
{
    const auto &peo = LexBFS();
    SmallVector<Register, DEFAULT_VECTOR_SIZE> processedNbr;

    for (size_t i = 0; i < peo.size(); i++) {
        processedNbr.clear();

        // Collect processed neighbors
        for (size_t j = 0; j < i; j++) {
            if (HasEdge(peo[i], peo[j])) {
                processedNbr.push_back(j);
            }
        }

        // Check that all processed neighbors in clique
        if (!CheckNeighborsInClique(peo, processedNbr)) {
            return false;
        }
    }

    return true;
}

namespace {
const char *GetNodeShape(const InterferenceGraph &ig, unsigned i)
{
    const char *shape = "ellipse";
    if (ig.GetNode(i).IsPhysical()) {
        shape = "box";
    } else {
        auto size = ig.Size();
        for (unsigned j = 0; j < size; j++) {
            if (i != j && ig.HasEdge(i, j) && ig.GetNode(j).IsPhysical()) {
                shape = "hexagon";
                break;
            }
        }
    }
    return shape;
}
}  // namespace

void InterferenceGraph::Dump(const std::string &name, bool skipPhysical, std::ostream &out) const
{
    auto transformedName = name;
    std::replace(transformedName.begin(), transformedName.end(), ':', '_');
    out << "Nodes: " << Size() << "\n\n"
        << "\ngraph " << transformedName << " {\nnode [colorscheme=spectral9]\n";
    auto size = Size();
    if (size == 0) {
        out << "}\n";
        return;
    }

    // Map to colors
    std::array<Register, std::numeric_limits<Register>::max()> colors {};
    colors.fill(GetInvalidReg());
    Register curColor = 0;

    for (auto &node : GetNodes()) {
        if (!(skipPhysical && node.IsPhysical()) && colors[node.GetColor()] == GetInvalidReg()) {
            colors[node.GetColor()] = curColor;
            curColor++;
        }
    }

    // Print header
    for (unsigned i = 0; i < size; i++) {
        if (skipPhysical && GetNode(i).IsPhysical()) {
            continue;
        }
        auto color = GetNode(i).GetColor();
        out << i << " [color=" << unsigned(colors[color]) << ", xlabel=\"";
        out << unsigned(color) << "\", tooltip=\"" << GetNode(i).GetLifeIntervals()->ToString<true>();
        out << "\", shape=\"" << GetNodeShape(*this, i) << "\"]\n";
    }

    auto edgePrinter = [this, &out, skipPhysical](auto nodeNum) {
        for (unsigned j = 0; j < nodeNum; j++) {
            bool check = !(skipPhysical && GetNode(j).IsPhysical()) && HasEdge(nodeNum, j);
            if (!check) {
                continue;
            }
            if (GetNode(nodeNum).GetColor() == GetNode(j).GetColor() &&
                GetNode(nodeNum).GetColor() != GetInvalidReg()) {
                out << "Error: Same color\n";
            }
            out << nodeNum << "--" << j << "\n";
        }
    };

    // Print edges
    for (unsigned i = 1; i < size; i++) {
        if (skipPhysical && GetNode(i).IsPhysical()) {
            continue;
        }
        edgePrinter(i);
    }

    out << "}\n";
}
}  // namespace ark::compiler
