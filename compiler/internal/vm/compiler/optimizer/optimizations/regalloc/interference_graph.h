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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_REGALLOC_INTERFERENCE_GRAPH_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_REGALLOC_INTERFERENCE_GRAPH_H

#include <algorithm>
#include <climits>
#include <iostream>
#include <unordered_set>
#include "compiler/optimizer/ir/constants.h"
#include "libpandabase/macros.h"
#include "libpandabase/utils/bit_utils.h"
#include "utils/arena_containers.h"
#include "utils/small_vector.h"
#include "compiler_logger.h"

namespace panda::compiler {
class LifeIntervals;

class ColorNode {
public:
    // Special color saying that color not in free registers but should be used on mapping
    static constexpr uint16_t NO_BIAS = 0xffff;

    template <typename T>
    ColorNode(unsigned number, T alloc) : cs_point_set_(alloc), number_(number), physical_(), fixed_()
    {
    }

    unsigned GetNumber() const noexcept
    {
        return number_;
    }

    void SetColor(Register color) noexcept
    {
        color_ = color;
    }

    Register GetColor() const noexcept
    {
        return color_;
    }

    void SetBias(uint16_t bias) noexcept
    {
        bias_ = bias;
    }

    bool HasBias() const noexcept
    {
        return bias_ != NO_BIAS;
    }

    uint16_t GetBias() const noexcept
    {
        return bias_;
    }

    void Assign(LifeIntervals *life_intervals) noexcept
    {
        life_intervals_ = life_intervals;
    }

    bool IsFixedColor() const noexcept
    {
        return fixed_;
    }

    bool IsPhysical() const noexcept
    {
        return physical_;
    }

    void SetFixedColor(Register color, bool is_physical) noexcept
    {
        fixed_ = true;
        physical_ = is_physical;
        SetColor(color);
    }

    LifeIntervals *GetLifeIntervals() const noexcept
    {
        return life_intervals_;
    }

    void AddCallsite(LifeNumber point) noexcept
    {
        cs_point_set_.insert(point);
    }

    unsigned GetCallsiteIntersectCount() const noexcept
    {
        return cs_point_set_.size();
    }

private:
    LifeIntervals *life_intervals_ = nullptr;
    ArenaUnorderedSet<LifeNumber> cs_point_set_;
    unsigned number_;
    uint16_t bias_ = NO_BIAS;  // Bias is a group of nodes for coalescing
    Register color_ = INVALID_REG;
    uint8_t physical_ : 1;
    uint8_t fixed_ : 1;
};

class GraphMatrix {
public:
    explicit GraphMatrix(ArenaAllocator *alloc) : matrix_(alloc->Adapter()) {}
    void SetCapacity(unsigned capacity)
    {
        capacity_ = RoundUp(capacity, sizeof(uintptr_t));
        matrix_.clear();
        matrix_.resize(capacity_ * capacity_);
    }

    bool AddEdge(unsigned a, unsigned b);

    bool HasEdge(unsigned a, unsigned b) const
    {
        return matrix_[FindEdge(a, b)];
    }

    bool AddAffinityEdge(unsigned a, unsigned b);

    bool HasAffinityEdge(unsigned a, unsigned b) const
    {
        return matrix_[FindAffinityEdge(a, b)];
    }

    unsigned GetCapacity() const noexcept
    {
        return capacity_;
    }

private:
    template <typename T>
    void OrderNodes(T &a, T &b) const noexcept
    {
        ASSERT(a < capacity_);
        ASSERT(b < capacity_);
        if (a < b) {
            std::swap(a, b);
        }
    }

    unsigned FindEdge(unsigned a, unsigned b) const
    {
        // Placement in lower triangulated adjacency matrix
        OrderNodes(a, b);
        return a * capacity_ + b;
    }

    unsigned FindAffinityEdge(unsigned a, unsigned b) const
    {
        // Placement in lower triangulated adjacency matrix
        OrderNodes(a, b);
        return b * capacity_ + a;
    }

    ArenaVector<bool> matrix_;
    unsigned capacity_ = 0;
};

using NodeVector = ArenaVector<ColorNode>;

// TODO (Evgeny.Erokhin): In Appel's book described usage of 2 structures in parallel to hold interference:
// one is for random checks (here is a matrix) and lists on adjacency for sequental access. It's worth to add!
class InterferenceGraph {
public:
    explicit InterferenceGraph(ArenaAllocator *alloc) : nodes_(alloc->Adapter()), matrix_(alloc) {}
    ColorNode *AllocNode();

    NodeVector &GetNodes() noexcept
    {
        return nodes_;
    }

    const NodeVector &GetNodes() const noexcept
    {
        return nodes_;
    }

    ColorNode &GetNode(unsigned num)
    {
        return nodes_[num];
    }

    const ColorNode &GetNode(unsigned num) const
    {
        return nodes_[num];
    }

    unsigned Size() const noexcept
    {
        return nodes_.size();
    }

    void Reserve(size_t count);

    struct Bias {
        unsigned callsites = 0;
        Register color = INVALID_REG;
    };

    Bias &AddBias() noexcept
    {
        return biases_.emplace_back();
    }

    void UpdateBiasData(Bias *bias, const ColorNode &node)
    {
        ASSERT(bias != nullptr);
        if (node.GetColor() != INVALID_REG) {
            bias->color = node.GetColor();
        }
        bias->callsites += node.GetCallsiteIntersectCount();
    }

    unsigned GetBiasCount() const noexcept
    {
        return biases_.size();
    }

    void AddEdge(unsigned a, unsigned b);
    bool HasEdge(unsigned a, unsigned b) const;
    void AddAffinityEdge(unsigned a, unsigned b);
    bool HasAffinityEdge(unsigned a, unsigned b) const;

    // Build LexBFS, so reverse order gives minimal coloring
    ArenaVector<unsigned> LexBFS() const;

    bool IsChordal() const;
    void Dump(const std::string &name = "IG", bool skip_physical = true, std::ostream &out = std::cout) const;

    template <unsigned MAX_COLORS>
    Register AssignColors(size_t colors, size_t callee_offset)
    {
        ASSERT(colors <= MAX_COLORS);
        std::bitset<MAX_COLORS> nbr_colors;
        std::bitset<MAX_COLORS> nbr_bias_colors;
        Register max_color = 0;

        for (unsigned id = 0; id < Size(); id++) {
            auto &node = GetNode(id);

            // Skip colored
            if (node.GetColor() != INVALID_REG) {
                continue;
            }

            // Make busy colors maps
            MakeBusyBitmap(id, &nbr_colors, &nbr_bias_colors);

            // Try bias color first if free
            size_t try_color;
            if (node.HasBias() && biases_[node.GetBias()].color != INVALID_REG &&
                !nbr_colors[biases_[node.GetBias()].color]) {
                try_color = biases_[node.GetBias()].color;
                COMPILER_LOG(DEBUG, REGALLOC) << "Bias color chosen " << try_color;
            } else {
                // The best case is to find color that is not in neighbour colors and not in biased
                nbr_bias_colors |= nbr_colors;  // Make compound busy bitmap

                // For nodes that take part in bias with callsites intersection, prefer callee saved registers.
                // In cases first interval of bias isn't intersected it will be placed in callersaved register,
                // that will affect entire coalesced bias group.
                size_t off = 0;
                if (node.HasBias() && biases_[node.GetBias()].callsites > 0) {
                    off = callee_offset;
                }

                // Find first not allocated disregard biasing
                if ((try_color = FirstFree(nbr_colors, nbr_bias_colors, colors, off)) == colors) {
                    return 0;  // No colors left
                }

                // Assign bias color if first observed in component
                if (node.HasBias() && biases_[node.GetBias()].color == INVALID_REG) {
                    biases_[node.GetBias()].color = try_color;
                    COMPILER_LOG(DEBUG, REGALLOC) << "Set bias color " << try_color;
                }
            }

            // Assign color
            node.SetColor(try_color);
            COMPILER_LOG(DEBUG, REGALLOC) << "Node " << node.GetNumber() << ": Set color: "
                                          << " " << unsigned(node.GetColor());
            max_color = std::max<Register>(try_color, max_color);
        }

        return max_color + 1;
    }

private:
    template <typename T>
    void MakeBusyBitmap(unsigned id, T *nbr_colors, T *nbr_bias_colors)
    {
        nbr_colors->reset();
        nbr_bias_colors->reset();
        // Collect neighbors colors
        for (unsigned nbr_id = 0; nbr_id < Size(); nbr_id++) {
            auto &nbr_node = GetNode(nbr_id);

            // Collect neighbour color
            if (nbr_node.GetColor() != INVALID_REG && HasEdge(id, nbr_id)) {
                ASSERT(nbr_node.GetColor() < nbr_colors->size());
                nbr_colors->set(nbr_node.GetColor());
            } else if (nbr_id != id && nbr_node.HasBias() && HasEdge(id, nbr_id)) {
                // Collect biased neighbour color
                ASSERT(nbr_node.GetBias() < GetBiasCount());
                if (biases_[nbr_node.GetBias()].color != INVALID_REG) {
                    nbr_bias_colors->set(biases_[nbr_node.GetBias()].color);
                }
            }
        }
    }

    template <typename T>
    static size_t FirstFree(const T &nbr_bs, const T &nbr_bs_bias, size_t colors, size_t off)
    {
        // Find first free
        size_t try_color;
        for (try_color = off; try_color < colors + off; try_color++) {
            if (!nbr_bs[try_color % colors]) {
                break;
            }
        }

        // Find free regarding biasing (higher part of bitmap)
        for (auto i = try_color; i < colors + off; i++) {
            if (!nbr_bs_bias[i % colors]) {
                try_color = i;
                break;
            }
        }

        return try_color == colors + off ? colors : try_color % colors;
    }

    NodeVector nodes_;
    GraphMatrix matrix_;
    static const size_t DEFAULT_BIAS_SIZE = 16;
    SmallVector<Bias, DEFAULT_BIAS_SIZE> biases_;
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_REGALLOC_INTERFERENCE_GRAPH_H
