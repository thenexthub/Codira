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

#ifndef ABC2PROGRAM_COMMON_ABC_CODE_CONVERTER_H
#define ABC2PROGRAM_COMMON_ABC_CODE_CONVERTER_H

#include <assembly-ins.h>
#include "abc2program_entity_container.h"
#include "bytecode_instruction-inl.h"
#include "bytecode_instruction.h"

namespace panda::abc2program {

class AbcCodeConverter {
public:
    explicit AbcCodeConverter(Abc2ProgramEntityContainer &entity_container)
        : entity_container_(entity_container), file_(entity_container_.GetAbcFile()) {}
    pandasm::Ins *BytecodeInstructionToPandasmInstruction(BytecodeInstruction bc_ins,
                                                         panda_file::File::EntityId method_id) const;
    pandasm::Opcode BytecodeOpcodeToPandasmOpcode(BytecodeInstruction::Opcode opcode) const;
    std::string IDToString(BytecodeInstruction bc_ins, panda_file::File::EntityId method_id, size_t idx) const;
private:
    Abc2ProgramEntityContainer &entity_container_;
    const panda_file::File &file_;
};  // class AbcCodeConverter

}  // namespace panda::abc2program

#endif  // ABC2PROGRAM_COMMON_ABC_CODE_CONVERTER_H