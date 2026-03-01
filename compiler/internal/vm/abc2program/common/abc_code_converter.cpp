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

#include "abc_code_converter.h"

namespace panda::abc2program {

std::string AbcCodeConverter::IDToString(BytecodeInstruction bc_ins, panda_file::File::EntityId method_id,
                                         size_t idx) const
{
    panda_file::File::EntityId entity_id = file_.ResolveOffsetByIndex(method_id, bc_ins.GetId(idx).AsIndex());
    if (bc_ins.IsIdMatchFlag(idx, BytecodeInstruction::Flags::METHOD_ID)) {
        std::string full_method_name = entity_container_.GetFullMethodNameById(entity_id);
        entity_container_.AddProgramString(full_method_name);
        return full_method_name;
    } else if (bc_ins.IsIdMatchFlag(idx, BytecodeInstruction::Flags::STRING_ID)) {
        std::string str_constant = entity_container_.GetStringById(entity_id);
        entity_container_.AddProgramString(str_constant);
        return str_constant;
    } else {
        ASSERT(bc_ins.IsIdMatchFlag(idx, BytecodeInstruction::Flags::LITERALARRAY_ID));
        entity_container_.AddUnnestedLiteralArrayId(entity_id.GetOffset());
        return entity_container_.GetLiteralArrayIdName(entity_id.GetOffset());
    }
}

}  // namespace panda::abc2program