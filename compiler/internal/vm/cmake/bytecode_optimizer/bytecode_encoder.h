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

#ifndef BYTECODE_OPTIMIZER_BYTECODE_ENCODER_H
#define BYTECODE_OPTIMIZER_BYTECODE_ENCODER_H

#include <stdint.h>

#include "compiler/optimizer/code_generator/encode.h"

namespace panda::bytecodeopt {
class BytecodeEncoder final : public compiler::Encoder {
public:
    explicit BytecodeEncoder(ArenaAllocator *allocator) : Encoder(allocator, Arch::NONE) {}

    void Finalize() override {};

    static bool CanEncodeImmHelper(int64_t imm, uint32_t size, int64_t min, int64_t max)
    {
        constexpr uint8_t HALF_SIZE = 32;
        if (size != HALF_SIZE) {
            return false;
        }

        return imm >= min && imm <= max;
    }

    bool CanEncodeImmAddSubCmp(int64_t imm, uint32_t size, [[maybe_unused]] bool signed_compare) override
    {
        return CanEncodeImmHelper(imm, size, INT8_MIN, INT8_MAX);
    }

    bool CanEncodeImmMulDivMod(uint64_t imm, uint32_t size) override
    {
        return CanEncodeImmAddSubCmp(imm, size, false);
    }

    bool CanEncodeImmLogical(uint64_t imm, uint32_t size) override
    {
        return CanEncodeImmHelper(imm, size, INT32_MIN, INT32_MAX);
    }

    bool CanEncodeShift(uint32_t size) override
    {
        constexpr uint32_t UNSUPPORTED_SHIFT_SIZE = 64;
        return size != UNSUPPORTED_SHIFT_SIZE;
    }
    size_t GetLabelAddress(compiler::LabelHolder::LabelId) override
    {
        return 0;
    }
    bool LabelHasLinks(compiler::LabelHolder::LabelId) override
    {
        return false;
    }
};  // BytecodeEncoder
}  // namespace panda::bytecodeopt

#endif  // BYTECODE_OPTIMIZER_BYTECODE_ENCODER_H
