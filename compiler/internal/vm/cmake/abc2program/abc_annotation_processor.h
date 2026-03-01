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

#ifndef ABC2PROGRAM_ABC_ANNOTATION_PROCESSOR_H
#define ABC2PROGRAM_ABC_ANNOTATION_PROCESSOR_H

#include "abc_file_entity_processor.h"
#include "annotation_data_accessor.h"
#include "common/abc_file_utils.h"

namespace panda::abc2program {

class AbcAnnotationProcessor : public AbcFileEntityProcessor {
public:
    AbcAnnotationProcessor(panda_file::File::EntityId entity_id, Abc2ProgramEntityContainer &entity_container,
                           pandasm::Function &function);
    void FillProgramData() override;

private:
    void InitAnnotationName();
    void FillAnnotation();
    void FillAnnotationElements(std::vector<pandasm::AnnotationElement> &elements);
    void FillLiteralArrayAnnotation(std::vector<pandasm::AnnotationElement> &elements,
                                    const std::string &annotation_elem_name, uint32_t value);
    std::unique_ptr<panda_file::AnnotationDataAccessor> annotation_data_accessor_;
    pandasm::Function &function_;
    std::string annotation_name_;
};  // class AbcAnnotationProcessor

}  // namespace panda::abc2program

#endif  // ABC2PROGRAM_ABC_ANNOTATION_PROCESSOR_H
