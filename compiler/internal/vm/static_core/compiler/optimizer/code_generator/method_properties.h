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

#ifndef COMPILER_OPTIMIZER_CODEGEN_METHOD_PROPERTIES_H
#define COMPILER_OPTIMIZER_CODEGEN_METHOD_PROPERTIES_H

#include <cstddef>
#include <iostream>
#include "libarkbase/mem/arena_allocator.h"
#include "libarkbase/utils/bit_field.h"

namespace ark::compiler {
class Graph;

class MethodProperties {
public:
    explicit MethodProperties(const Graph *graph);
    MethodProperties(const MethodProperties &) = default;
    MethodProperties &operator=(const MethodProperties &) = default;
    MethodProperties(MethodProperties &&) = default;
    MethodProperties &operator=(MethodProperties &&) = default;
    ~MethodProperties() = default;

    static MethodProperties *Create(ArenaAllocator *arenaAllocator, const Graph *graph)
    {
        return arenaAllocator->New<MethodProperties>(graph);
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define MPROP_GET_FIELD(name, type)                                        \
    type Get##name() const                                                 \
    {                                                                      \
        /* CC-OFFNXT(G.PRE.05, G.PRE.02) function gen, namespace member */ \
        return name::Get(fields_);                                         \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define MPROP_SET_FIELD(name, type)                \
    void Set##name(type val)                       \
    {                                              \
        /* CC-OFFNXT(G.PRE.02) namespace member */ \
        name::Set(val, &fields_);                  \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define MPROP_FIELD(name, type) \
    MPROP_GET_FIELD(name, type) \
    MPROP_SET_FIELD(name, type)

    using FieldsTy = uint32_t;

    MPROP_FIELD(CanThrow, bool);
    MPROP_FIELD(HasCalls, bool);
    MPROP_FIELD(HasLibCalls, bool);
    MPROP_FIELD(HasRuntimeCalls, bool);
    MPROP_FIELD(HasSafepoints, bool);
    MPROP_FIELD(HasRequireState, bool);
    MPROP_FIELD(HasDeopt, bool);
    MPROP_FIELD(HasParamsOnStack, bool);
    MPROP_FIELD(CompactPrologueAllowed, bool);
    MPROP_FIELD(RequireFrameSetup, bool);

    using CanThrow = BitField<bool, 0, 1>;
    using HasCalls = CanThrow::NextFlag;
    using HasLibCalls = HasCalls::NextFlag;
    using HasRuntimeCalls = HasLibCalls::NextFlag;
    using HasSafepoints = HasRuntimeCalls::NextFlag;
    using HasRequireState = HasSafepoints::NextFlag;
    using HasDeopt = HasRequireState::NextFlag;
    using HasParamsOnStack = HasDeopt::NextFlag;
    using CompactPrologueAllowed = HasParamsOnStack::NextFlag;
    using RequireFrameSetup = CompactPrologueAllowed::NextFlag;

    bool IsLeaf() const
    {
        return !GetHasCalls() && !GetHasRuntimeCalls() && !GetHasLibCalls() && !GetHasRequireState() && !GetCanThrow();
    }

    Inst *GetLastReturn() const
    {
        return lastReturn_;
    }

#undef MPROP_GET_FIELD
#undef MPROP_SET_FIELD
#undef MPROP_FIELD

private:
    void CheckAndSetStackParams(Inst *inst);

private:
    Inst *lastReturn_ {nullptr};
    FieldsTy fields_ {0};
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_CODEGEN_METHOD_PROPERTIES_H
