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

#ifndef LIBPANDAFILE_BYTECODE_EMITTER_H
#define LIBPANDAFILE_BYTECODE_EMITTER_H

#include <bytecode_instruction.h>
#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <list>

namespace panda {

class BytecodeEmitter;

/**
 * Label represents a branch target.
 * User can associate a labe with the special place by calling
 * BytecodeEmitter.Bind(const Label& label) method.
 * It is not allowed to share labels between different instancies
 * of BytecodeEmitter.
 * Lifetime of a label must match lifetime of the emitter.
 **/
class Label {
private:
    explicit Label(std::list<uint32_t>::iterator pc) : pc_(pc) {}

    uint32_t GetPc() const
    {
        return *pc_;
    }

private:
    std::list<uint32_t>::iterator pc_;

    friend class BytecodeEmitter;
};

class BytecodeEmitter {
public:
    enum class ErrorCode {
        SUCCESS,
        /* Opcode is unsupported. It means there is no functionality yet or some bug. */
        INTERNAL_ERROR,
        /* There are branches to the labels for which Bind haven't been called. */
        UNBOUND_LABELS,
    };

    enum class BitImmSize {
        BITSIZE_4,
        BITSIZE_8,
        BITSIZE_16,
        BITSIZE_32,
    };

public:
    BytecodeEmitter() : pc_(0) {}

    ~BytecodeEmitter() = default;

    NO_COPY_SEMANTIC(BytecodeEmitter);
    NO_MOVE_SEMANTIC(BytecodeEmitter);

    Label CreateLabel()
    {
        pc_list_.push_front(0);
        return Label(pc_list_.begin());
    }

    /**
     * Bind the label with the current place in the final bytecode.
     */
    void Bind(const Label &label);

    /**
     * Generate mov <reg> <reg> instruction.
     * The method chooses appropriate instruction encoding.
     */
    ErrorCode Build(std::vector<uint8_t> *output);

#include <bytecode_emitter_def_gen.h>

private:
    ErrorCode ReserveSpaceForOffsets();
    ErrorCode DoReserveSpaceForOffset(const BytecodeInstruction &insn, uint32_t insn_pc, BitImmSize expected_imm_size,
                                      size_t *extra_bytes_ptr, uint32_t *target_ptr);
    ErrorCode UpdateBranches();
    void UpdateLabelTargets(uint32_t pc, size_t bias);
    int32_t EstimateMaxDistance(uint32_t insn_pc, uint32_t target_pc, uint32_t bias) const;
    ErrorCode CheckLabels();

    static size_t GetSizeByOpcode(BytecodeInstruction::Opcode opcode);
    static BytecodeInstruction::Opcode RevertConditionCode(BytecodeInstruction::Opcode opcode);
    static void UpdateBranchOffs(uint8_t *insn, int32_t offs);
    static BitImmSize GetBitImmSizeByOpcode(BytecodeInstruction::Opcode opcode);
    static BytecodeInstruction::Opcode GetLongestJump(BytecodeInstruction::Opcode opcode);
    BytecodeInstruction::Opcode GetSuitableJump(BytecodeInstruction::Opcode opcode, BytecodeEmitter::BitImmSize width);

private:
    struct LabelCmp {
        bool operator()(const Label &l1, const Label &l2) const
        {
            return *l1.pc_ < *l2.pc_;
        }
    };

private:
    uint32_t pc_ {0};
    std::map<uint32_t, Label> branches_;
    std::multiset<Label, LabelCmp> targets_;
    std::list<uint32_t> pc_list_;
    std::vector<uint8_t> bytecode_;
};
}  // namespace panda

#endif  // LIBPANDAFILE_BYTECODE_EMITTER_H
