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

#ifndef ABC2PROGRAM_ABC_CLASS_PROCESSOR_H
#define ABC2PROGRAM_ABC_CLASS_PROCESSOR_H

#include <map>
#include "abc_file_entity_processor.h"
#include "libarkfile/class_data_accessor-inl.h"

namespace ark::abc2program {

class AbcClassProcessor : public AbcFileEntityProcessor {
public:
    AbcClassProcessor(panda_file::File::EntityId entityId, Abc2ProgramKeyData &keyData);
    void FillProgramData();

private:
    void FillRecord();
    void FillRecordData(pandasm::Record &record);
    void FillRecordSourceFile(pandasm::Record &record);
    void FillRecordMetaData(pandasm::Record &record);
    void FillFields(pandasm::Record &record);
    void FillFunctions();
    void FillRecordAnnotations(pandasm::Record &record);
    std::unique_ptr<panda_file::ClassDataAccessor> classDataAccessor_;
};

}  // namespace ark::abc2program

#endif  // ABC2PROGRAM_ABC_CLASS_PROCESSOR_H
