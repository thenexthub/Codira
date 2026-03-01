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

#include "abc_annotation_processor.h"
#include "abc_literal_array_processor.h"
#include "file-inl.h"

namespace panda::abc2program {

AbcAnnotationProcessor::AbcAnnotationProcessor(panda_file::File::EntityId entity_id,
                                               Abc2ProgramEntityContainer &entity_container,
                                               pandasm::Function &function)
    : AbcFileEntityProcessor(entity_id, entity_container), function_(function)
{
    annotation_data_accessor_ = std::make_unique<panda_file::AnnotationDataAccessor>(*file_, entity_id_);
    annotation_name_ = pandasm::Type::FromDescriptor(GetStringById(annotation_data_accessor_->GetClassId())).GetName();
}

void AbcAnnotationProcessor::FillProgramData()
{
    if (annotation_name_.empty()) {
        return;
    }
    entity_container_.ModifyRecordName(annotation_name_);
    FillAnnotation();
}

void AbcAnnotationProcessor::FillAnnotation()
{
    std::vector<pandasm::AnnotationElement> elements;
    FillAnnotationElements(elements);
    pandasm::AnnotationData annotation_data(annotation_name_, elements);
    std::vector<pandasm::AnnotationData> annotations;
    annotations.emplace_back(std::move(annotation_data));
    function_.metadata->AddAnnotations(annotations);
}

void AbcAnnotationProcessor::FillLiteralArrayAnnotation(std::vector<pandasm::AnnotationElement> &elements,
                                                        const std::string &annotation_elem_name, uint32_t value)
{
    panda_file::LiteralDataAccessor lit_data_accessor(*file_, file_->GetLiteralArraysId());
    AbcLiteralArrayProcessor lit_array_proc(panda_file::File::EntityId{value}, entity_container_,
                                            lit_data_accessor);
    lit_array_proc.FillProgramData();
    std::string name = entity_container_.GetLiteralArrayIdName(value);
    pandasm::AnnotationElement annotation_element(
        annotation_elem_name, std::make_unique<pandasm::ScalarValue>(
        pandasm::ScalarValue::Create<pandasm::Value::Type::LITERALARRAY>(name)));
    elements.emplace_back(annotation_element);
}

void AbcAnnotationProcessor::FillAnnotationElements(std::vector<pandasm::AnnotationElement> &elements)
{
    for (uint32_t i = 0; i < annotation_data_accessor_->GetCount(); ++i) {
        auto annotation_data_accessor_elem = annotation_data_accessor_->GetElement(i);
        auto annotation_elem_name = GetStringById(annotation_data_accessor_elem.GetNameId());
        auto value_type = pandasm::Value::GetCharAsType(annotation_data_accessor_->GetTag(i).GetItem());
        switch (value_type) {
            case pandasm::Value::Type::U1: {
                auto value = annotation_data_accessor_elem.GetScalarValue().Get<bool>();
                pandasm::AnnotationElement annotation_element(
                    annotation_elem_name, std::make_unique<pandasm::ScalarValue>(
                    pandasm::ScalarValue::Create<pandasm::Value::Type::U1>(value)));
                elements.emplace_back(annotation_element);
                break;
            }
            case pandasm::Value::Type::U32: {
                auto value = annotation_data_accessor_elem.GetScalarValue().Get<uint32_t>();
                pandasm::AnnotationElement annotation_element(
                    annotation_elem_name, std::make_unique<pandasm::ScalarValue>(
                    pandasm::ScalarValue::Create<pandasm::Value::Type::U32>(value)));
                elements.emplace_back(annotation_element);
                break;
            }
            case pandasm::Value::Type::F64: {
                auto value = annotation_data_accessor_elem.GetScalarValue().Get<double>();
                pandasm::AnnotationElement annotation_element(
                    annotation_elem_name, std::make_unique<pandasm::ScalarValue>(
                    pandasm::ScalarValue::Create<pandasm::Value::Type::F64>(value)));
                elements.emplace_back(annotation_element);
                break;
            }
            case pandasm::Value::Type::STRING: {
                auto value = annotation_data_accessor_elem.GetScalarValue().Get<uint32_t>();
                std::string_view string_value {
                    reinterpret_cast<const char *>(file_->GetStringData(panda_file::File::EntityId(value)).data)};
                pandasm::AnnotationElement annotation_element(
                    annotation_elem_name, std::make_unique<pandasm::ScalarValue>(
                    pandasm::ScalarValue::Create<pandasm::Value::Type::STRING>(string_value)));
                elements.emplace_back(annotation_element);
                break;
            }
            case pandasm::Value::Type::LITERALARRAY: {
                auto value = annotation_data_accessor_elem.GetScalarValue().Get<uint32_t>();
                FillLiteralArrayAnnotation(elements, annotation_elem_name, value);
                break;
            }
            default:
                UNREACHABLE();
        }
    }
}

}  // namespace panda::abc2program
