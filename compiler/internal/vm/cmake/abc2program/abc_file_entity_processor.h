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

#ifndef ABC2PROGRAM_ABC_FILE_ENTITY_PROCESSOR_H
#define ABC2PROGRAM_ABC_FILE_ENTITY_PROCESSOR_H

#include "common/abc2program_entity_container.h"
#include "abc2program_log.h"
#include "common/abc_file_utils.h"

namespace panda::abc2program {

class AbcFileEntityProcessor {
public:
    AbcFileEntityProcessor(panda_file::File::EntityId entity_id, Abc2ProgramEntityContainer &entity_container);

    std::string GetStringById(const panda_file::File::EntityId &entity_id) const
    {
        return entity_container_.GetStringById(entity_id);
    }

protected:
    virtual void FillProgramData() = 0;
    panda_file::File::EntityId entity_id_;
    Abc2ProgramEntityContainer &entity_container_;
    const panda_file::File *file_ = nullptr;
    pandasm::Program *program_ = nullptr;
};  // class AbcFileEntityProcessor

}  // namespace panda::abc2program

#endif  // ABC2PROGRAM_ABC_FILE_ENTITY_PROCESSOR_H
