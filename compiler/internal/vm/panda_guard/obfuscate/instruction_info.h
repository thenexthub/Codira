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

#ifndef PANDA_GUARD_OBFUSCATE_INSTRUCTION_INFO_H
#define PANDA_GUARD_OBFUSCATE_INSTRUCTION_INFO_H

#include "assembly-program.h"

namespace panda::guard {

class Function;

/**
 * Instruction information
 * Due to STL's memory reallocation mechanism, internal pointers are only accessible during traversal
 */
class InstructionInfo {
public:
    explicit InstructionInfo() = default;

    InstructionInfo(Function *function, pandasm::Ins *ins, const size_t index)
        : function_(function), ins_(ins), index_(index)
    {
    }

    /**
     * Determine whether the instruction uses registers within the function
     */
    [[nodiscard]] bool IsInnerReg() const;

    /**
     * Determine whether it is valid instruction information
     */
    [[nodiscard]] bool IsValid() const;

    /**
     * update ins name
     * @param generateNewName true:generate new name false:use origin name
     */
    void UpdateInsName(bool generateNewName = true);

    /**
     * update dynamic import ins filename
     */
    void UpdateInsFileName();

    /**
     * write name cache
     */
    void WriteNameCache() const;

    /**
     * compare opcode is equal to input instruction
     * @param opcode input opcode
     * @return ins_.opcode == opcode
     */
    [[nodiscard]] bool equalToOpcode(pandasm::Opcode opcode) const;

    /**
     * compare opcode is not equal to input instruction
     * @param opcode input opcode
     * @return ins_.opcode != opcode
     */
    [[nodiscard]] bool notEqualToOpcode(pandasm::Opcode opcode) const;

    Function *function_ = nullptr;
    pandasm::Ins *ins_ = nullptr;
    size_t index_ = 0;

private:
    std::string origin_ {};
    std::string obfName_ {};
};
}  // namespace panda::guard

#endif  // PANDA_GUARD_OBFUSCATE_INSTRUCTION_INFO_H
