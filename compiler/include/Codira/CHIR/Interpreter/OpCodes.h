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

/**
 * @file
 *
 * This file declares the bytecode operation codes for the BCHIR interpreter.
 */

#ifndef CODIRA_CHIR_INTERPRETER_OPCODES_H
#define CODIRA_CHIR_INTERPRETER_OPCODES_H

#include "Codira/Utils/Utils.h"

namespace Codira::CHIR::Interpreter {

enum class OpCode {
#define OPCODE(ID, VALUE, SIZE, HAS_EXC_HANDLER) ID,
#include "Codira/CHIR/Interpreter/OpCodes.inc"
#undef OPCODE
};

const std::string OpCodeLabel[static_cast<size_t>(OpCode::INVALID) + 1]{
#define OPCODE(ID, VALUE, SIZE, HAS_EXC_HANDLER) (VALUE),
#include "Codira/CHIR/Interpreter/OpCodes.inc"
#undef OPCODE
};

const uint32_t OpCodeArgSize[static_cast<size_t>(OpCode::INVALID) + 1]{
#define OPCODE(ID, VALUE, SIZE, HAS_EXC_HANDLER) (SIZE),
#include "Codira/CHIR/Interpreter/OpCodes.inc"
#undef OPCODE
};

constexpr bool OpHandlesException[static_cast<size_t>(OpCode::INVALID) + 1]{
#define OPCODE(ID, VALUE, SIZE, HAS_EXC_HANDLER) (HAS_EXC_HANDLER),
#include "Codira/CHIR/Interpreter/OpCodes.inc"
#undef OPCODE
};

uint32_t inline GetOpCodeArgSize(OpCode opCode)
{
    return OpCodeArgSize[static_cast<size_t>(opCode)];
}

std::string inline GetOpCodeLabel(OpCode opCode)
{
    return OpCodeLabel[static_cast<size_t>(opCode)];
}

constexpr bool inline OpHasExceptionHandler(OpCode opCode)
{
    return OpHandlesException[static_cast<size_t>(opCode)];
}

}; // namespace Codira::CHIR::Interpreter

#endif // CODIRA_CHIR_INTERPRETER_OPCODES_H
