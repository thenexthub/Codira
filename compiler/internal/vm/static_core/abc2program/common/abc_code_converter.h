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
#include "libarkfile/bytecode_instruction-inl.h"
#include "libarkfile/bytecode_instruction.h"
#include "abc_string_table.h"
#include "abc2program_key_data.h"

namespace ark::abc2program {

class AbcCodeConverter {
public:
    explicit AbcCodeConverter(Abc2ProgramKeyData &keyData);
    pandasm::Ins BytecodeInstructionToPandasmInstruction(BytecodeInstruction bcIns,
                                                         panda_file::File::EntityId methodId) const;
    pandasm::Opcode BytecodeOpcodeToPandasmOpcode(BytecodeInstruction::Opcode opcode) const;
    std::string IDToString(BytecodeInstruction bcIns, panda_file::File::EntityId methodId) const;

private:
    Abc2ProgramKeyData &keyData_;
    const panda_file::File *file_ = nullptr;
    AbcStringTable *stringTable_ = nullptr;
};  // class AbcCodeConverter

}  // namespace ark::abc2program

#endif  // ABC2PROGRAM_COMMON_ABC_CODE_CONVERTER_H