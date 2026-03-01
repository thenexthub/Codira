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

#ifndef ABC2PROGRAM_ABC_FIELD_PROCESSOR_H
#define ABC2PROGRAM_ABC_FIELD_PROCESSOR_H

#include "abc_file_entity_processor.h"
#include "field_data_accessor-inl.h"
#include "common/abc_type_converter.h"

namespace panda::abc2program {

class AbcFieldProcessor : public AbcFileEntityProcessor {
public:
    AbcFieldProcessor(panda_file::File::EntityId entity_id, Abc2ProgramEntityContainer &entity_container,
                      pandasm::Record &record);
    void FillProgramData() override;

private:
    void FillLiteralArrayMetaData();
    void FillStringMetaData();
    void FillU32MetaData();
    void FillFieldData();
    void FillFieldName();
    void FillFieldType();
    void FillFieldMetaData();
    void FillFieldAttributes();
    void FillFieldAnnotations();
    void FillMetaDataValue();
    pandasm::Record &record_;
    AbcTypeConverter type_converter_;
    std::unique_ptr<panda_file::FieldDataAccessor> field_data_accessor_;
    pandasm::Field field_;
};

}  // namespace panda::abc2program

#endif  // ABC2PROGRAM_ABC_FIELD_PROCESSOR_H
