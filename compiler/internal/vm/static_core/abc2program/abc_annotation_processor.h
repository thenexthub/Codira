/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ABC2PROGRAM_ABC_ANNOTATION_PROCESSOR_H
#define ABC2PROGRAM_ABC_ANNOTATION_PROCESSOR_H

#include "abc_file_entity_processor.h"
#include "libarkfile/annotation_data_accessor.h"
#include "abc_file_utils.h"
#include <variant>

namespace ark::abc2program {

class AbcAnnotationProcessor : public AbcFileEntityProcessor {
public:
    AbcAnnotationProcessor(panda_file::File::EntityId entityId, Abc2ProgramKeyData &keyData, pandasm::Record &record);
    AbcAnnotationProcessor(panda_file::File::EntityId entityId, Abc2ProgramKeyData &keyData,
                           pandasm::Function &function);
    AbcAnnotationProcessor(panda_file::File::EntityId entityId, Abc2ProgramKeyData &keyData, pandasm::Field &field);
    void FillProgramData();

private:
    void InitAnnotationName();
    void FillAnnotation();
    void FillAnnotationElements(std::vector<pandasm::AnnotationElement> &elements);
    void FillLiteralArrayAnnotation(std::vector<pandasm::AnnotationElement> &elements,
                                    const std::string &annotationElemName, uint32_t value);
    void ProcessArrayAnnotationElement(std::vector<pandasm::AnnotationElement> &elements,
                                       const panda_file::AnnotationDataAccessor::Elem &elem, const std::string &name,
                                       pandasm::Value::Type arrayType);
    std::optional<pandasm::AnnotationElement> CreateAnnotationElement(
        const panda_file::AnnotationDataAccessor::Elem &annotationDataAccessorElem, std::string &annotationElemName,
        pandasm::Value::Type valueType);
    std::unique_ptr<panda_file::AnnotationDataAccessor> annotationDataAccessor_;
    std::variant<pandasm::Record *, pandasm::Function *, pandasm::Field *> target_;
    std::string annotationName_;
};  // class AbcAnnotationProcessor

}  // namespace ark::abc2program

#endif  // ABC2PROGRAM_ABC_ANNOTATION_PROCESSOR_H
