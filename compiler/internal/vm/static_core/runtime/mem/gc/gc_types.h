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
#ifndef RUNTIME_MEM_GC_GC_TYPES_H
#define RUNTIME_MEM_GC_GC_TYPES_H

#include <array>
#include <string_view>

namespace ark::mem {

enum class GCExecutionMode {
    GC_STW_NO_MT,  // Stop-the-world, single thread
    GC_EXECUTION_MODE_LAST = GC_STW_NO_MT
};

constexpr GCExecutionMode GC_EXECUTION_MODE = GCExecutionMode::GC_STW_NO_MT;

enum class GCType {
    INVALID_GC = 0,
    EPSILON_GC,
    EPSILON_G1_GC,
    STW_GC,
    G1_GC,
    CMC_GC,
    GCTYPE_LAST = CMC_GC,
};

constexpr bool IsGenerationalGCType(const GCType gcType)
{
    bool ret = false;
    ASSERT(gcType != GCType::INVALID_GC);
    switch (gcType) {
        case GCType::G1_GC:
        case GCType::EPSILON_G1_GC:
            ret = true;
            break;
        case GCType::INVALID_GC:
        case GCType::EPSILON_GC:
        case GCType::STW_GC:
        case GCType::CMC_GC:
            break;
        default:
            UNREACHABLE();
            break;
    }
    return ret;
}

constexpr size_t ToIndex(GCType type)
{
    return static_cast<size_t>(type);
}

constexpr size_t GC_TYPE_SIZE = ToIndex(GCType::GCTYPE_LAST) + 1;
constexpr std::array<char const *, GC_TYPE_SIZE> GC_NAMES = {"Invalid GC",        "Epsilon GC", "Epsilon-G1 GC",
                                                             "Stop-The-World GC", "G1 GC",      "CMC GC"};

constexpr bool StringsEqual(char const *a, char const *b)
{
    return std::string_view(a) == b;
}

static_assert(StringsEqual(GC_NAMES[ToIndex(GCType::INVALID_GC)], "Invalid GC"));
static_assert(StringsEqual(GC_NAMES[ToIndex(GCType::EPSILON_GC)], "Epsilon GC"));
static_assert(StringsEqual(GC_NAMES[ToIndex(GCType::EPSILON_G1_GC)], "Epsilon-G1 GC"));
static_assert(StringsEqual(GC_NAMES[ToIndex(GCType::STW_GC)], "Stop-The-World GC"));
static_assert(StringsEqual(GC_NAMES[ToIndex(GCType::G1_GC)], "G1 GC"));
static_assert(StringsEqual(GC_NAMES[ToIndex(GCType::CMC_GC)], "CMC GC"));

constexpr GCType GCTypeFromString(std::string_view gcTypeStr)
{
    if (gcTypeStr == "epsilon") {
        return GCType::EPSILON_GC;
    }
    if (gcTypeStr == "epsilon-g1") {
        return GCType::EPSILON_G1_GC;
    }
    if (gcTypeStr == "stw") {
        return GCType::STW_GC;
    }
    if (gcTypeStr == "g1-gc") {
        return GCType::G1_GC;
    }
    if (gcTypeStr == "cmc-gc") {
        return GCType::CMC_GC;
    }
    return GCType::INVALID_GC;
}

constexpr std::string_view GCStringFromType(GCType gcType)
{
    if (gcType == GCType::EPSILON_GC) {
        return "epsilon";
    }
    if (gcType == GCType::EPSILON_G1_GC) {
        return "epsilon-g1";
    }
    if (gcType == GCType::STW_GC) {
        return "stw";
    }
    if (gcType == GCType::G1_GC) {
        return "g1-gc";
    }
    if (gcType == GCType::CMC_GC) {
        return "cmc-gc";
    }
    return "invalid-gc";
}

enum GCCollectMode : uint8_t {
    GC_NONE = 0,          // Non collected objects
    GC_MINOR = 1U,        // Objects collected at the minor GC
    GC_MAJOR = 1U << 1U,  // Objects collected at the major GC (MAJOR usually includes MINOR)
    GC_FULL = 1U << 2U,   // Can collect objects from some spaces which very rare contains GC candidates
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    GC_ALL = GC_MINOR | GC_MAJOR | GC_FULL,  // Can collect objects at any phase
};

}  // namespace ark::mem

#endif  // RUNTIME_MEM_GC_GC_TYPES_H
