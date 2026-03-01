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

#ifndef COMPILER_OPTIMIZER_IR_MARKER_H
#define COMPILER_OPTIMIZER_IR_MARKER_H

#include <array>
#include <bitset>
#include <cstdint>
#include "libarkbase/macros.h"

namespace ark::compiler {
using Marker = uint32_t;
constexpr uint32_t MARKERS_SHIFT = 2;
constexpr uint32_t MARKERS_NUM = (1U << MARKERS_SHIFT);
constexpr uint32_t MARKERS_MASK = (MARKERS_NUM - 1U);
constexpr uint32_t MARKER_SIZE = 32;
constexpr uint32_t MAX_MARKER = ((1U << (MARKER_SIZE - MARKERS_NUM)) - 1U);
constexpr uint32_t UNDEF_MARKER = 0;

class MarkerMgr {
public:
    MarkerMgr() = default;

    NO_COPY_SEMANTIC(MarkerMgr);
    NO_MOVE_SEMANTIC(MarkerMgr);
    virtual ~MarkerMgr() = default;

    Marker NewMarker() const
    {
        ASSERT_PRINT(currentIndex_ < MAX_MARKER, "Markers overflow. Please check recursion");
        ++currentIndex_;
        for (uint32_t i = 0; i < MARKERS_NUM; i++) {
            if (!spaces_[i]) {
                Marker mrk = (currentIndex_ << MARKERS_SHIFT) | i;
                spaces_[i] = true;
                ASSERT(mrk != UNDEF_MARKER);
                return mrk;
            }
        }
        UNREACHABLE();
    }

    void EraseMarker(Marker mrk) const
    {
        spaces_[mrk & MARKERS_MASK] = false;
    }

    uint32_t GetCurrentMarkerIdx()
    {
        return currentIndex_;
    }
    void SetMaxMarkerIdx(uint32_t ixd)
    {
        currentIndex_ = std::max(currentIndex_, ixd);
    }

private:
    mutable uint32_t currentIndex_ {0};
    mutable std::bitset<MARKERS_NUM> spaces_ {};
};

class MarkerSet {
public:
    MarkerSet()
    {
        ClearMarkers();
    }

    NO_COPY_SEMANTIC(MarkerSet);
    NO_MOVE_SEMANTIC(MarkerSet);
    virtual ~MarkerSet() = default;

    // returns true if the marker was set before, otherwise set marker and return false
    bool SetMarker(Marker mrk)
    {
        uint32_t index = mrk & MARKERS_MASK;
        uint32_t value = mrk >> MARKERS_SHIFT;
        ASSERT(index < MARKERS_NUM);
        if (markers_[index] == value) {
            return true;
        }
        markers_[index] = value;
        return false;
    }

    // returns true if the marker was set before, otherwise false
    bool IsMarked(Marker mrk)
    {
        uint32_t index = mrk & MARKERS_MASK;
        uint32_t value = mrk >> MARKERS_SHIFT;
        ASSERT(index < MARKERS_NUM);
        return markers_[index] == value;
    }

    // unset marker and returns true if the marker was set before, otherwise false
    bool ResetMarker(Marker mrk)
    {
        uint32_t index = mrk & MARKERS_MASK;
        uint32_t value = mrk >> MARKERS_SHIFT;
        bool wasSet = (markers_[index] == value);
        ASSERT(index < MARKERS_NUM);
        markers_[index] = UNDEF_MARKER;
        return wasSet;
    }

    void ClearMarkers()
    {
        for (unsigned int &marker : markers_) {
            marker = UNDEF_MARKER;
        }
    }

private:
    friend class MarkerMgr;
    std::array<Marker, MARKERS_NUM> markers_ {};
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_IR_MARKER_H
