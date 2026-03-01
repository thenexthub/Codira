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

#ifndef LIBABCKIT_SRC_CODEGEN_IC_SLOT_ALLOCATOR_H
#define LIBABCKIT_SRC_CODEGEN_IC_SLOT_ALLOCATOR_H

#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/ir/graph.h"
#include "compiler/optimizer/ir/graph_visitor.h"
#include "compiler/optimizer/ir/constants.h"
#include "compiler/optimizer/ir/inst.h"
#include "compiler/optimizer/pass.h"

namespace libabckit {

using ark::compiler::BasicBlock;
using ark::compiler::Inst;
using ark::compiler::Opcode;

class ICSlotAllocator final : public ark::compiler::Optimization {
public:
    explicit ICSlotAllocator(ark::compiler::Graph *graph, uint16_t *icSlotNumber)
        : ark::compiler::Optimization(graph), icSlotNumber_(icSlotNumber)
    {
    }

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "ICSlotAllocator";
    }

private:
    template <typename Callback>
    void VisitInstructions(const Callback &callback);

    bool Allocate8BitIcSlots();
    bool Allocate8Or16BitIcSlots();
    bool Allocate16BitIcSlots();

    uint8_t nextFree8BitSlot_ = 0x0;
    uint16_t nextFree16BitSlot_ = INIT_16BIT_SLOT_VALE;
    uint16_t *icSlotNumber_ = nullptr;

    static constexpr uint16_t INIT_16BIT_SLOT_VALE = 0x100;
    static constexpr uint8_t INVALID_8_BIT_SLOT = 0xFF;
    static constexpr uint16_t INVALID_16_BIT_SLOT = 0xFFFF;
};

}  // namespace libabckit

#endif  // LIBABCKIT_SRC_CODEGEN_IC_SLOT_ALLOCATOR_H
