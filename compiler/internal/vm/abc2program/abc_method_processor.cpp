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

#include "abc_method_processor.h"
#include "abc_code_processor.h"

namespace panda::abc2program {

AbcMethodProcessor::AbcMethodProcessor(panda_file::File::EntityId entity_id,
                                       Abc2ProgramEntityContainer &entity_container)
    : AbcFileEntityProcessor(entity_id, entity_container),
      type_converter_(entity_container),
      function_(pandasm::Function(entity_container_.GetFullMethodNameById(entity_id_), DEFUALT_SOURCE_LANG)),
      debug_info_extractor_(entity_container.GetDebugInfoExtractor())
{
    method_data_accessor_ = std::make_unique<panda_file::MethodDataAccessor>(*file_, entity_id_);
    function_.language = method_data_accessor_->GetSourceLang().value_or(DEFUALT_SOURCE_LANG);
}

void AbcMethodProcessor::FillProgramData()
{
    FillFunctionData();
    program_->function_table.emplace(function_.name, std::move(function_));
}

void AbcMethodProcessor::FillFunctionData()
{
    FillProto();
    FillFunctionKind();
    FillFunctionMetaData();
    FillCodeData();
    FillDebugInfo();
    FillFuncAnnotation();
    FillAnnotationData();
}

void AbcMethodProcessor::FillProto()
{
    static const uint32_t maxArgNum = 0xFFFF;
    uint32_t params_num = GetNumArgs();
    if (params_num > maxArgNum) {
        LOG(ERROR, ABC2PROGRAM) << "> error encountered at " << entity_id_.GetOffset() << " (0x" << std::hex
                                << entity_id_.GetOffset() << "). number of function's arguments (" << std::dec
                                << params_num << ") exceeds maxArgNum (" << maxArgNum << ") !";
        return;
    }

    pandasm::Type any_type = pandasm::Type(ANY_TYPE_NAME, 0);
    function_.return_type = any_type;
    for (uint32_t i = 0; i < params_num; i++) {
        function_.params.emplace_back(
            pandasm::Function::Parameter(any_type, function_.language)
        );
    }
}

uint32_t AbcMethodProcessor::GetNumArgs() const
{
    std::optional<panda_file::File::EntityId> code_id = method_data_accessor_->GetCodeId();
    if (!code_id.has_value()) {
        return 0;
    }
    panda_file::File::EntityId code_id_value = code_id.value();
    panda_file::CodeDataAccessor code_data_accessor(*file_, code_id_value);
    return code_data_accessor.GetNumArgs();
}

void AbcMethodProcessor::FillFunctionKind()
{
    uint32_t method_acc_flags = method_data_accessor_->GetAccessFlags();
    uint32_t func_kind_u32 = ((method_acc_flags & panda_file::FUNCTION_KIND_MASK) >> panda_file::FLAG_WIDTH);
    panda_file::FunctionKind func_kind = static_cast<panda_file::FunctionKind>(func_kind_u32);
    function_.SetFunctionKind(func_kind);
}

void AbcMethodProcessor::FillFunctionMetaData()
{
    FillFunctionAttributes();
    FillAccessFlags();
}

void AbcMethodProcessor::FillFunctionAttributes()
{
    if (method_data_accessor_->IsStatic()) {
        function_.metadata->SetAttribute(ABC_ATTR_STATIC);
    }
    if (file_->IsExternal(method_data_accessor_->GetMethodId())) {
        function_.metadata->SetAttribute(ABC_ATTR_EXTERNAL);
    }
}

void AbcMethodProcessor::FillAccessFlags()
{
    uint32_t access_flags = method_data_accessor_->GetAccessFlags();
    access_flags = (access_flags & panda_file::FLAG_MASK);
    function_.metadata->SetAccessFlags(access_flags);
}

void AbcMethodProcessor::FillCodeData()
{
    std::optional<panda_file::File::EntityId> code_id = method_data_accessor_->GetCodeId();
    if (code_id.has_value()) {
        AbcCodeProcessor code_processor(code_id.value(), entity_container_, entity_id_, function_);
        code_processor.FillProgramData();
    }
}

void AbcMethodProcessor::FillDebugInfo()
{
    FillSourceFile();
    FillSourceCode();
}

void AbcMethodProcessor::FillSourceFile()
{
    function_.source_file = debug_info_extractor_.GetSourceFile(entity_id_);
}

void AbcMethodProcessor::FillSourceCode()
{
    function_.source_code = debug_info_extractor_.GetSourceCode(entity_id_);
}

void AbcMethodProcessor::FillFuncAnnotation()
{
    method_data_accessor_->EnumerateAnnotations([&](panda_file::File::EntityId annotation_id) {
        AbcAnnotationProcessor annotation_processor(annotation_id, entity_container_, function_);
        annotation_processor.FillProgramData();
    });
}

void AbcMethodProcessor::FillAnnotationData()
{
    for (auto annotation : function_.metadata->GetAnnotations()) {
        const auto& name = annotation.GetName();
        const auto& elements = annotation.GetElements();

        if (name == SLOT_NUMBER_RECORD_NAME && !elements.empty()) {
            uint32_t slots_num = elements[0].GetValue()->GetAsScalar()->GetValue<uint32_t>();
            function_.SetSlotsNum(static_cast<size_t>(slots_num));
        } else if (name == CONCURRENT_MODULE_REQUEST_RECORD_NAME) {
            for (auto &elem : elements) {
                function_.concurrent_module_requests.emplace_back(
                    elem.GetValue()->GetAsScalar()->GetValue<uint32_t>());
            }
        } else if (name == EXPECTED_PROPERTY_COUNT_RECORD_NAME && !elements.empty()) {
            uint32_t expected_property = elements[0].GetValue()->GetAsScalar()->GetValue<uint32_t>();
            function_.SetExpectedPropertyCount(static_cast<size_t>(expected_property));
        }
    }
}
} // namespace panda::abc2program