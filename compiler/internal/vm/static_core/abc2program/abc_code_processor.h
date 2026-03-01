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

#ifndef ABC2PROGRAM_ABC_CODE_PROCESSOR_H
#define ABC2PROGRAM_ABC_CODE_PROCESSOR_H

#include "abc_file_entity_processor.h"
#include "assembly-function.h"
#include "common/abc_code_converter.h"
#include "libarkfile/code_data_accessor-inl.h"

namespace ark::abc2program {

using LabelTable = std::map<size_t, std::string>;
class AbcCodeProcessor : public AbcFileEntityProcessor {
public:
    AbcCodeProcessor(panda_file::File::EntityId entityId, Abc2ProgramKeyData &keyData,
                     panda_file::File::EntityId methodId);
    void FillProgramData();
    std::vector<pandasm::Ins> &&GetIns();
    std::vector<pandasm::Function::CatchBlock> &&GetCatchBlocks();
    uint32_t GetNumVregs() const;

private:
    LabelTable GetExceptions(panda_file::File::EntityId methodId, panda_file::File::EntityId codeId);
    void CollectExternalFields(const panda_file::File::EntityId &fieldId);
    panda_file::File::EntityId methodId_;
    std::unique_ptr<panda_file::CodeDataAccessor> codeDataAccessor_;
    std::unique_ptr<AbcCodeConverter> codeConverter_;
    std::vector<pandasm::Ins> ins_;
    std::vector<pandasm::Function::CatchBlock> catchBlocks_;
};  // AbcCodeProcessor

}  // namespace ark::abc2program

#endif  // ABC2PROGRAM_ABC_CODE_PROCESSOR_H
