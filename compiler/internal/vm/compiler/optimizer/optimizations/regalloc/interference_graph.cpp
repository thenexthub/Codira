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
#include <numeric>
#include "optimizer/analysis/liveness_analyzer.h"

namespace panda::compiler {
bool GraphMatrix::AddEdge(unsigned a, unsigned b)
{
    auto it = matrix_.begin() + FindEdge(a, b);
    bool old_val = *it;
    *it = true;
    return old_val;
}

bool GraphMatrix::AddAffinityEdge(unsigned a, unsigned b)
{
    auto it = matrix_.begin() + FindAffinityEdge(a, b);
    bool old_val = *it;
    *it = true;
    return old_val;
}

// -------------------------------------------------------

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
    SmallVector<unsigned, DEFAULT_BOUNDARY_STACK> boundary_stack;
    boundary_stack.reserve(num);
    boundary_stack.push_back(num);  // Sentinel
    unsigned pos = 0;               // Initialy we have set S of all elements

    while (true) {
        ASSERT(pos < out.size());
        auto id = out[pos];
        pos++;

        // Check for boundaries colapse
        ASSERT(!boundary_stack.empty());
        auto prev_end = boundary_stack.back();
        ASSERT(pos <= prev_end);
        if (pos == prev_end) {
            if (pos == num) {
                break;
            }
            boundary_stack.resize(boundary_stack.size() - 1);
            ASSERT(!boundary_stack.empty());
            prev_end = boundary_stack.back();
        }

        // Partition on 2 groups: adjacent and not adjacent(last)
        ASSERT(pos <= prev_end);
        auto it = std::stable_partition(out.begin() + pos, out.begin() + prev_end,
                                        [id, &out, this](unsigned val) { return HasEdge(id, out[val]); });
        auto pivot = static_cast<unsigned>(std::distance(out.begin(), it));
        // Split group if needed
        if (pivot > pos && pivot != prev_end) {
            boundary_stack.push_back(pivot);
        }
    }

    return out;
}

namespace {
constexpr size_t DEFAULT_VECTOR_SIZE = 64;
}  // namespace

bool InterferenceGraph::IsChordal() const
{
    const auto &peo = LexBFS();
    SmallVector<Register, DEFAULT_VECTOR_SIZE> processed_nbr;

    for (size_t i = 0; i < peo.size(); i++) {
        processed_nbr.clear();

        // Collect processed neighbors
        for (size_t j = 0; j < i; j++) {
            if (HasEdge(peo[i], peo[j])) {
                processed_nbr.push_back(j);
            }
        }

        // Check that all processed neighbors in clique
        for (auto nbr1 : processed_nbr) {
            for (auto nbr2 : processed_nbr) {
                if (nbr1 != nbr2 && !HasEdge(peo[nbr1], peo[nbr2])) {
                    return false;
                }
            }
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
        for (unsigned j = 0; j < ig.Size(); j++) {
            if (i != j && ig.HasEdge(i, j) && ig.GetNode(j).IsPhysical()) {
                shape = "hexagon";
                break;
            }
        }
    }
    return shape;
}
}  // namespace

void InterferenceGraph::Dump(const std::string &name, bool skip_physical, std::ostream &out) const
{
    auto transformed_name = name;
    std::replace(transformed_name.begin(), transformed_name.end(), ':', '_');
    out << "Nodes: " << Size() << "\n\n"
        << "\ngraph " << transformed_name << " {\nnode [colorscheme=spectral9]\n";
    auto size = Size();
    if (size == 0) {
        out << "}\n";
        return;
    }

    // Map to colors
    std::array<Register, std::numeric_limits<Register>::max()> colors {};
    colors.fill(INVALID_REG);
    Register cur_color = 0;

    for (auto &node : GetNodes()) {
        if (!(skip_physical && node.IsPhysical()) && colors[node.GetColor()] == INVALID_REG) {
            colors[node.GetColor()] = cur_color;
            cur_color++;
        }
    }

    // Print header
    for (unsigned i = 0; i < size; i++) {
        if (skip_physical && GetNode(i).IsPhysical()) {
            continue;
        }
        auto color = GetNode(i).GetColor();
        out << i << " [color=" << unsigned(colors[color]) << ", xlabel=\"";
        out << unsigned(color) << "\", tooltip=\"" << GetNode(i).GetLifeIntervals()->ToString<true>();
        out << "\", shape=\"" << GetNodeShape(*this, i) << "\"]\n";
    }

    auto edge_printer = [this, &out, skip_physical](auto node_num) {
        for (unsigned j = 0; j < node_num; j++) {
            if (!(skip_physical && GetNode(j).IsPhysical()) && HasEdge(node_num, j)) {
                if (GetNode(node_num).GetColor() == GetNode(j).GetColor() &&
                    GetNode(node_num).GetColor() != INVALID_REG) {
                    out << "Error: Same color\n";
                }
                out << node_num << "--" << j << "\n";
            }
        }
    };

    // Print edges
    for (unsigned i = 1; i < size; i++) {
        if (skip_physical && GetNode(i).IsPhysical()) {
            continue;
        }
        edge_printer(i);
    }

    out << "}\n";
}
}  // namespace panda::compiler
